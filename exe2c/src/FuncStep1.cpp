// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
#include <QDebug>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
using namespace boost::lambda;
#include <set>
#include <algorithm>

#include	"CISC.h"
#include "exe2c.h"
#include "FuncStep1.h"
static void	Add_in_order(EALIST *list, ea_t i)
{
    if (list->empty())
    {
        list->push_front(i);
        return;
    }
    for(EALIST::iterator pos = list->begin();pos!=list->end();++pos)
    {
        ea_t ea = *pos;
        if (ea == i)
            return;		//	Already have it, even if the
        if (ea > i)
        {
            list->insert(pos,i);
            return;
        }
    }
    list->push_back(i);
}
//static function
void    CFuncStep1::check_if_switch_case(ea_t cur_off, CaseList* pcaselist,EALIST* pjxxlist, XCPUCODE* pxcpu)
{
        if (pxcpu->opcode != C_JMP)
                return;
        if (pxcpu->op[0].mode != OP_Address)
                return;
        if (pxcpu->op[0].addr.base_reg_index != _NOREG_)
                return;
        if (pxcpu->op[0].addr.off_reg_index == _NOREG_)
                return;
        if (pxcpu->op[0].addr.off_reg_scale != 4)
                return;
        if (pxcpu->op[0].addr.off_value <= 0x401000)
                return;
        //alert("switch case find 1");
        //Code_SwitchCase1();

        ea_t ptbl = pxcpu->op[0].addr.off_value;
        //alert_prtf("table is %x",ptbl);
        if (! IfInWorkSpace(ptbl))
                return;		//	Confirm this table is valid

        //alert("switch case find 2");

        ea_t d = Peek_D(ptbl);
        if (! IfInWorkSpace(d))
                return;		//	Confirm the first is valid

        //alert("switch case find 3");

        ea_t break_off = 0;	//Next, to determine the value of break_off
        EALIST::iterator pos = pjxxlist->begin();

        for (;pos!=pjxxlist->end(); ++pos)
        {
            ea_t ea = *pos;
            if (ea > cur_off)
            {
                break_off = ea;
                break;
            }
        }
        if (break_off == 0)
                return;		//	Jump condition not found? something wrong

        //alert("switch case find 4");


        //if (pjxx->jmp.jmp_type != JMP_ja)
        //	return;		//	Is not ja, not right
        if (d < cur_off || d > break_off)
                return;		//	Alignment of the first one, not in range (conditon .. break target) -> will not work

        //alert("switch case find 5");

        //alert("really switch case");
        CASE_t *pnew = new CASE_t;
        pnew->jxx_opcode = cur_off;
        pnew->caselist = new EALIST;
        pcaselist->push_front(pnew);

        for (int i=0;;i++)
        {
                d = Peek_D(ptbl+i*4);
//		if (! IfInWorkSpace(d))
                if (d < cur_off || d > break_off)
                        break;
                Add_in_order(pjxxlist,d);
                pnew->caselist->push_back(d);
        }
}

static bool	any_free_ea(EALIST *jxxlist, std::set<ea_t> &visited_set, ea_t* pea)
{
    EALIST::iterator pos = jxxlist->begin();
    // find jxxlist entry that is not in usedlist already
    for (;pos!=jxxlist->end();++pos)
    {
        ea_t ea = *pos;//jxxlist->;

        if (visited_set.find(ea)==visited_set.end())
        {
            *pea = ea;
            return true;
        }
    }
    return false;
}
void CFuncStep1::CheckIfJustSwitchCase(CaseList& caselist, ea_t ea)
{
    CaseList::iterator pos1 = caselist.begin();
    CASE_t *p1;
    while (pos1!=caselist.end())
    {
        p1 = *(pos1++);//caselist.;
        if (p1->jxx_opcode == ea)	//	really
        {	//	now, add some jcase instruction
            EALIST::iterator pos2 = p1->caselist->begin();
            while (pos2!=p1->caselist->end())
            {
                ea_t case_ea = *pos2;
                ++pos2;
                assert(case_ea);

                AsmCode *pnew = AsmCode::new_AsmCode();
                pnew->linear = ea;
                pnew->xcpu.opcode = C_JCASE;
                pnew->xcpu.op[0].mode = OP_Near;
                pnew->xcpu.op[0].nearptr.offset = case_ea;
                m_asmlist->push_back(pnew);
                //alert("insert 1 C_JCAES");
            }
            break;	//	only one can be true
        }
    }
}

void CFuncStep1::CreateNewFunc_if_CallNear()
{
    AsmCodeList::iterator pos1 = m_asmlist->begin();
    while (pos1!=m_asmlist->end())
    {
        AsmCode* pasm = *pos1;//m_asmlist->;
        ++pos1;
        XCPUCODE* pxcpu = &pasm->xcpu;
        if ( pxcpu->opcode == C_CALL && pxcpu->op[0].mode == OP_Near)
        {
            //log_prtl("find Call %x",xcpu.op[0].nearptr.offset);
            Exe2c::get()->func_new(pxcpu->op[0].nearptr.offset);
        }
    }
}
bool	CFuncStep1::Step_1(ea_t head_off)
// Find the function end address, generate asm opcode list
{
        //assert(m_nStep == 0);

        CaseList caselist;
        EALIST jxxlist;
        std::set<ea_t> visited_set;

        ea_t ea = head_off;
        assert(ea < 0x10000000);
        assert(ea >= 0x400000);

        jxxlist.push_front(ea);

        while (any_free_ea(&jxxlist,visited_set,&ea))
        {	//	Travell(/ed?) all the jxx
            for (;;)
            {
                visited_set.insert(ea);

                Disasm the;
                uint8_t opcode_len = the.Disasm_OneCode(ea);
				assert(opcode_len!=0);
                ea+=opcode_len;
				if (the.get_xcpu()->IsJxx() || the.get_xcpu()->IsJmpNear())
                {
					Add_in_order(&jxxlist, the.get_xcpu()->op[0].nearptr.offset);
				}
				else
				{
					check_if_switch_case(ea,&caselist,&jxxlist, the.get_xcpu());
				}

                if (the.get_xcpu()->opcode == C_RET || the.get_xcpu()->opcode == C_JMP )
                    break;
                if(visited_set.find(ea)!=visited_set.end())
                    break; // we've visited this ea already, next please
            }
        }

        std::set<ea_t>::iterator pos = visited_set.begin();

        if(!visited_set.empty())
            if (m_asmlist == NULL)
                m_asmlist = new AsmCodeList; //	Create asm opcode list

        while (pos!=visited_set.end())
        {
            ea_t ea = *(pos++);
            AsmCode *p = AsmCode::new_AsmCode();
            p->linear = ea;

            Disasm the;
            p->opsize = the.Disasm_OneCode(ea);
            p->xcpu = *the.get_xcpu();

            m_asmlist->push_back(p);
            //	Lookup a swith case are not just after
            this->CheckIfJustSwitchCase(caselist, ea);
        }

        AsmCode *pasm = *m_asmlist->rbegin();
        m_end_off = pasm->linear + pasm->opsize;
        return true;
}

//==========================================

static bool	isLeave(AsmCode* p)
{
    //If (leave) or (mov esp,??) or similar instructions, then the current stack state has no effect afterwards
        if (p->xcpu.opcode == C_LEAVE)
                return true;
        if (p->xcpu.opcode == C_MOV && p->xcpu.op[0].isRegOp(_ESP_))	//mov esp,ebp //Nem, actually mov esp,??
                return true;
        return false;
}

AsmCode* FuncLL::ea2pasm(ea_t ea)
{
    //todo: provide a map<ea,AsmCode *> to speed this up
    AsmCodeList::iterator biter = std::find_if(m_asmlist->begin(), m_asmlist->end(),
                                               boost::lambda::bind<ea_t>(&AsmCode::linear,_1)==ea);
    if(biter!=m_asmlist->end())
        return *biter;
    assert(!"address not found in instruction list");
    return NULL;
}
bool	FuncLL::stack_stack(AsmCode* p0, AsmCode* p1)
{    // Returns true if there were any changes made
    signed int & esp0 = p0->esp_level;
    signed int & esp0_next = p0->esp_level_next;
    signed int & esp1 = p1->esp_level;

    // First of all check for p0 construct
    if (isLeave(p0))
    {
        //p0 own esp and esp_next unrelated to the situation
        if (esp0_next == ESP_UNKNOWN)
        {
            signed int esp = ESP_UNKNOWN;
            if (Asm_Code_Change_ESP(esp, &p0->xcpu))
            {
                esp0_next = esp;
                return true;
            }
        }
    }
    else
    {
        if (esp0_next == ESP_UNKNOWN)
        {
            if (p0->xcpu.opcode == C_JMP || p0->xcpu.opcode == C_RET)
            {
                esp0_next = ESP_IGNORE;
                return true;
            }
            if (esp0 != ESP_UNKNOWN)
            {
                signed int esp = esp0;
                if ( Asm_Code_Change_ESP(esp, &p0->xcpu) )
                {
                    esp0_next = esp;
                    return true;
                }
            }
        }
        if (ESP_UNKNOWN != esp0_next && (ESP_IGNORE != esp0_next) && (ESP_UNKNOWN == esp0) )	// Anti-check
        {
            signed int esp = 0;
            if ( Asm_Code_Change_ESP(esp, &p0->xcpu) )
            {
                //esp0_next = esp0 + esp;
                esp0 = esp0_next - esp;
                return true;
            }
        }
        if (esp0 != ESP_UNKNOWN && esp0_next != ESP_UNKNOWN && p0->xcpu.opcode == C_CALL && p0->xcpu.op[0].isStaticOffset())
        {
            ea_t address = p0->xcpu.op[0].addr.off_value;
            Api *papi = ApiManage::get()->get_api(address);	//find it
            if (papi)
            {
                if (esp0_next != esp0 + papi->m_stack_purge)
                    return false;	//find error
            }
            else
            {	//not find, insert it
                alert_prtf("error not find api %x", address);
                ApiManage::get()->new_api(address,esp0_next - esp0);
                return true;
            }
        }
    }

    // Jmp -------------------------
    if (p1->xcpu.IsJxx() || p1->xcpu.IsJmpNear())
    {
        ea_t jmpto = p1->xcpu.op[0].nearptr.offset;
        AsmCode* p = ea2pasm(jmpto);
        if (p->esp_level == ESP_UNKNOWN && esp1 != ESP_UNKNOWN)
        {
            p->esp_level = esp1;
            return true;
        }
        if (p->esp_level != ESP_UNKNOWN && esp1 == ESP_UNKNOWN)
        {
            esp1 = p->esp_level;
            return true;
        }
        if (p->esp_level != ESP_UNKNOWN && esp1 != ESP_UNKNOWN)
        {
            if (p->esp_level != esp1)
                return false;
        }
    }
    // -----------------------------------
    if (p1->xcpu.opcode == C_RET)
    {
        if (esp1 == ESP_UNKNOWN)
        {
            esp1 = 0;
            return true;
        }
        if (esp1 != 0)
            return false;
    }
    // -----------------------------------
    if (esp0_next == ESP_UNKNOWN && esp1 != ESP_UNKNOWN)
    {
        esp0_next = esp1;
        return true;
    }
    if (esp0_next != ESP_UNKNOWN && esp0_next != ESP_IGNORE && esp1 == ESP_UNKNOWN)
    {
        esp1 = esp0_next;
        return true;
    }
    if (esp0_next != ESP_UNKNOWN && esp0_next != ESP_IGNORE && esp1 != ESP_UNKNOWN)
    {	//do some check
        if (esp0_next != esp1)
            return false;
    }
    return false;
}
ea_t FindApiAddress_Reg(uint32_t called_reg_index, XCPUCODE* pxcpu1, AsmCodeList* asmlist)
{
    // If I want to correctly handle (call eax)
    // I must look for the preceding (mov eax, [405070])

    //Nem
    //TODO: this function might be buggy, since it only scans lineary through instructions, and considers the closest
    //mov reg,val the authoritative information about the call's target
    ea_t retn = 7;

    //Nem
    //TODO: more efficient approach would be to scan backwards from the call insn through the list

    AsmCodeList::iterator pos = asmlist->begin();
    while (pos!=asmlist->end())
    {
        AsmCode* p = *pos;//asmlist->;
        ++pos;
        XCPUCODE* pxcpu = &p->xcpu;
        if (pxcpu == pxcpu1)
            return retn;
        if (pxcpu->opcode == C_MOV && pxcpu->op[0].isRegOp(called_reg_index) )
        {
            // MOV %REG,
            //call [405070]
            if (pxcpu->op[1].isStaticOffset())
            {
                // MOV %REG, [offset]
                retn = pxcpu->op[1].addr.off_value;
            }
        }
    }
    return retn;
}
bool	FuncLL::Asm_Code_Change_ESP(int &esp, XCPUCODE* pxcpu)
{
    //Calculated esp, to return true
    //This assumes that stack setup [mov ebp, esp] only occurs once a func
    switch(pxcpu->opcode)
    {
    case C_MOV:
        if(pxcpu->op[0].isRegOp(_ESP_) && pxcpu->op[1].isRegOp(_EBP_))
        {
            if (m_EBP_base != Not_EBP_based && esp == ESP_UNKNOWN)
            {
                esp = m_EBP_base;	//mov esp,ebp
                return true;
            }
        }
        if(pxcpu->op[0].isRegOp(_EBP_) && pxcpu->op[1].isRegOp(_ESP_))
        {
            if (esp != ESP_UNKNOWN && m_EBP_base == Not_EBP_based)
            {
                m_EBP_base = esp;		//mov ebp,esp
                return true;
            }
        }
        break;
    case C_LEAVE:
        esp = m_EBP_base;	//	mov	esp,ebp
        esp += 4;		//	pop	ebp
        break;
    //TODO: fix for PUSH ax etc
    case C_PUSH:
        esp -= 4;
        break;
    case C_POP:
        esp += 4;
        break;
    case C_SUB:
        if(pxcpu->op[0].isRegOp(_ESP_) && (pxcpu->op[1].mode == OP_Immed))
            esp -= pxcpu->op[1].immed.immed_value;
        break;
    case C_ADD:
        if(pxcpu->op[0].isRegOp(_ESP_) && (pxcpu->op[1].mode == OP_Immed))
            esp += pxcpu->op[1].immed.immed_value;
        break;
    case C_CALL:
        if (pxcpu->op[0].mode == OP_Near)
        {
            Func* pfunc = Exe2c::get()->GetFunc(pxcpu->op[0].nearptr.offset);
            if (pfunc == NULL)
                return false;
            if (pfunc->m_IfLibFunc)
            {
                //esp += pfunc->m_stack_purge;
                assert(pfunc->m_functype);
                esp += pfunc->m_functype->get_stack_purge();
                return true;
            }
            if (pfunc->m_nStep < STEP_IDA_1)
                return false;
            esp += pfunc->m_stack_purge;
            return true;
        }
        if (pxcpu->op[0].isStaticOffset()) //call [405070]
        {
            ea_t address = pxcpu->op[0].addr.off_value;
            Api* papi = ApiManage::get()->get_api(address);//,stacksub))	//find it
            if (papi)
            {
                esp += papi->m_stack_purge;
                return true;
            }
        }
        if (pxcpu->op[0].mode == OP_Register )   //call %reg
        {
            ea_t address = FindApiAddress_Reg(pxcpu->op[0].reg.reg_index, pxcpu, m_asmlist);
            Api* papi = ApiManage::get()->get_api(address);//,stacksub))	//find it
            if (papi)
            {
                esp += papi->m_stack_purge;
                return true;
            }
        }
        return false;
    }
    if (esp == ESP_UNKNOWN)
        return false;
    return true;
    //	stack has these types of situations:
    //	enter leave
    //	push ebp,mov ebp,esp,.....,mov esp,ebp,pop ebp
    //	push,pop
    //	call near
    //	ret is checked whether it has balanced
    //	for call [immed], etc., will not consider s
}



void FuncLL::AddRemoveSomeInstr()
{
    Func* pfunc;
    XCPUCODE* pxcpu;
    AsmCode* pasm;
    AsmCode* pnew;
    AsmCodeList::iterator pos = m_asmlist->begin();
    //Nem
    //This inserts BP_based function prologue into all fully-specified library functions
    // Why is this needed ?
    while (pos!=m_asmlist->end())
    {
        pasm = *(pos++);
        if (pasm->iAddRemove != 0)
            continue;

        pxcpu = &pasm->xcpu;
        if (!pxcpu->IsCallNear())
            continue;
        pfunc = Exe2c::get()->GetFunc(pxcpu->op[0].nearptr.offset);
        if (pfunc == NULL) // couldn't get that function
            continue;
        if ( !pfunc->m_IfLibFunc || pfunc->m_functype == NULL) // fully specified lib function
            continue;
        FuncType* fctype = pfunc->m_functype;
        if (fctype->m_internal_name.compare("_EH_prolog"))
            continue;
        pasm->iAddRemove = 1;

        //push ebp
        pnew = AsmCode::new_AsmCode();
        pnew->iAddRemove = 2;
        pnew->xcpu.opcode = C_PUSH;
        pnew->xcpu.op[0] = OPERITEM::createReg(_EBP_,4);
        m_asmlist->insert(pos, pnew);

        //mov ebp, esp
        pnew = AsmCode::new_AsmCode();
        pnew->iAddRemove = 2;
        pnew->xcpu.opcode = C_MOV;
        pnew->xcpu.op[0] = OPERITEM::createReg(_EBP_,4);
        pnew->xcpu.op[1] = OPERITEM::createReg(_ESP_,4);
        m_asmlist->insert(pos, pnew);
    }
}
//call this function once again to prepare functions
void FuncLL::Prepare_CallFunc()
{
    Func* pfunc;
    AsmCode* pasm;
    XCPUCODE* pxcpu;
    AsmCodeList::iterator pos = m_asmlist->begin();
    while (pos!=m_asmlist->end())
    {
        pasm = *(pos++);
        pxcpu = &pasm->xcpu;
        if ( !pxcpu->IsCallNear() )
            continue;

        pfunc = Exe2c::get()->GetFunc(pxcpu->op[0].nearptr.offset);
        if (pfunc)
            pfunc->PrepareFunc();
    }
}
bool	FuncLL::Fill_Stack_1()
{
    //Why is it always true ?
    AsmCodeList::iterator pos = m_asmlist->begin();
    AsmCode* pasm = 0;
    AsmCode* p0 = 0;
    if (pos!=m_asmlist->end())
    {
        p0 = *pos;//m_asmlist->;
        ++pos;
    }
    while (pos!=m_asmlist->end())
    {
        pasm = *(pos++);
        if (stack_stack(p0,pasm))
            return true;
        p0 = pasm;
    }
    return false;
}

//	Checks if the function stack is balanced
bool	FuncLL::Check_Stack()
{
	signed int esp;
	ea_t jmpto;
	AsmCode* p;
	AsmCodeList::iterator pos = m_asmlist->begin();
	signed int lastesp = 0;
	while (pos!=m_asmlist->end())
	{
		p = *(pos++);
		esp = p->esp_level;
		if (esp == ESP_UNKNOWN)
			return false;
		if (lastesp != ESP_UNKNOWN && lastesp != ESP_IGNORE && esp != lastesp)
			return false;

        if (p->xcpu.IsJxx() || p->xcpu.IsJmpNear())
        {
            jmpto = p->xcpu.op[0].nearptr.offset;
            if (esp != ea2pasm(jmpto)->esp_level)
                return false;
        }
        if ((p->xcpu.opcode == C_RET)&&(esp != 0))
            return false; // unbalanced stack at return.
        lastesp = p->esp_level_next;
    }
    return true;
}

bool	FuncLL::Fill_Stack_Info()
{
    AsmCode* pasm = *m_asmlist->begin();
    pasm->esp_level = 0;	// I know the first one

    while(Fill_Stack_1())
        ;
    if (! Check_Stack())
        return false;

    return true;
}

//return -1 for fail
int FuncLL::Get_Ret_Purge()
{
    AsmCode* pasm;
    XCPUCODE* pxcpu;
    int retn = -1;
    AsmCodeList::iterator pos = m_asmlist->begin();
    while (pos!=m_asmlist->end())
    {
        pasm = *(pos++);
        pxcpu = &pasm->xcpu;
        if (pxcpu->opcode != C_RET)
            continue;

        //Found the ret statement
        int r = 0;
        if (pxcpu->op[0].mode == OP_Immed)	// means RET n
            r = pxcpu->op[0].immed.immed_value;
        if (retn == -1)
            retn = r;
        else if (retn != r) //inconsistent RETs ?
            return -1;
    }
    if (retn == -1)
    {
        //alert_prtf("why not find RET ? func = %x", this->m_head_off);
        //myexit(2);
        //return false;
    }
    return retn;
}

std::string FuncLL::GetLabelName(ea_t ea)
{
    std::string retn;

    Func* pfunc = Exe2c::get()->GetFunc(ea);
    if (pfunc != NULL)
    {
        retn = pfunc->m_funcname;
        return retn;
    }
    AsmCode* pasm = this->ea2pasm(ea);
    if (pasm != NULL && (pasm->h.label.ref_j != NULL))
    {
        retn = pasm->h.label.label_name;
        return retn;
    }
    return retn;
}
void	FuncLL::prtout_asm(Func* pfunc, VarLL* pvarll, XmlOutPro* out)
{
    out->XMLbegin(XT_Function, pfunc);

    out->XMLbegin(XT_FuncName, pfunc);
    out->prtt(pfunc->m_funcname);
    out->XMLend(XT_FuncName);
    out->prtspace();
    out->prtt("proc");
    out->endline();

    pvarll->prtout(out);

    out->endline();

    this->prtout_asm_1(pvarll, out);

    out->XMLbegin(XT_FuncName, pfunc);
    out->prtt(pfunc->m_funcname);
    out->XMLend(XT_FuncName);
    out->prtspace();
    out->prtt("endp");
    out->endline();
    out->XMLend(XT_Function);
}
void	FuncLL::prtout_asm_1(VarLL* pvarll, XmlOutPro* out)
{
    //Display by running the code to ASM func, GAP of them can point to
    ea_t last = 0xffffffff;
    AsmCodeList::iterator pos = m_asmlist->begin();
    while (pos!=m_asmlist->end())
    {
        AsmCode* pasm = *pos;
        ++pos;
        if (pasm->iAddRemove == 2)
            continue;
        ea_t ea = pasm->linear;
        QString out_buf;
        uint32_t n;
        if (pasm->xcpu.opcode == C_JCASE)
        {
            n = 0;
            out_buf += QString("case jmp to %1").arg(pasm->xcpu.op[0].nearptr.offset,0,16);
        }
        else
        {
            st_IDA_OUT idaout;
            Disasm the;
            //n = the.Disassembler(buf, ea2ptr(ea), ea);
            n = the.Disassembler_X(ea2ptr(ea), ea, &idaout);
            XCPUCODE* pxcpu = the.get_xcpu();
            if (pxcpu->op[0].mode == OP_Near)
            {
                ea_t linear = pxcpu->op[0].nearptr.offset;
                std::string labelname = this->GetLabelName(linear);
                if (labelname.size()!=0)
                    idaout.Par1Str = labelname;
            }
            else if (pxcpu->op[0].mode == OP_Address)
            {
                OPERITEM* op = &pxcpu->op[0];
                if (op->addr.base_reg_index == _ESP_
                        || (op->addr.base_reg_index == _NOREG_
                            && op->addr.off_reg_index == _ESP_
                            && op->addr.off_reg_scale == 1))
                {
                    signed int level = pasm->esp_level + op->addr.off_value;
                    st_VarLL* p = pvarll->LookUp_VarLL(level- pvarll->m_VarRange_L);
                    if (p != NULL)
                    {
                        idaout.Par1Str += (QString(".%1").arg(p->Name)).toStdString();
                    }
                }
            }
            else if (pxcpu->op[1].mode == OP_Address)
            {
                OPERITEM* op = &pxcpu->op[1];
                if (op->addr.base_reg_index == _ESP_
                        || (op->addr.base_reg_index == _NOREG_
                            && op->addr.off_reg_index == _ESP_
                            && op->addr.off_reg_scale == 1))
                {
                    signed int level = pasm->esp_level + op->addr.off_value;
                    st_VarLL* p = pvarll->LookUp_VarLL(level- pvarll->m_VarRange_L);
                    if (p != NULL)
                    {
                        idaout.Par2Str += (QString(".%1").arg(p->Name)).toStdString();
                    }
                }
            }
            idaout.output(out_buf);
        }
        if (last != 0xffffffff && ea != last)
            out->prtl("//      gap here");

        if (pasm->h.label.ref_j != NULL)
        {
            //asm_prtl("%s:", pasm->h.label.label_name);
            out->prtf("%s:", pasm->h.label.label_name);
            out->endline();
        }

        //asm_prtl("%4x %x %s",-pasm->esp_level, ea, buf);
        if (pasm->esp_level == ESP_UNKNOWN)
        {
            out->prtt("    ");  //Location of the four spaces to stay
        }
        else
            out->prtf("%4x", -pasm->esp_level);
        out->prtspace();
        out->XMLbegin(XT_AsmOffset, (void*)ea);
        out->prtf("%x", ea);
        out->XMLend(XT_AsmOffset);
        out->prtt(out_buf);
        out->endline();

        last = ea+n;
    }
}


void FuncLL::GetVarRange(signed int& VarRange_L, signed int& VarRange_H)
{
    /*
    If a function starts with:
   0 401010 SUB    ESP,00000190
 190 401016 LEA    ECX,[ESP+00]
 Then VarRange_L = -190h
 Then VarRange_H = 0
 Write:
    0 401010 SUB    ESP,00000190
  190 401016 LEA    ECX,[ESP+v_00]
    */
    signed int Low = 0;
    signed int High = 0;
    signed int last;
    signed int here;
    AsmCode* pasm;
    AsmCodeList::iterator iter = m_asmlist->begin();
    for( ; iter!=m_asmlist->end(); ++iter)
    {
        pasm = *iter;
        last = pasm->esp_level;
        here = pasm->esp_level_next;
        if (pasm->xcpu.opcode == C_SUB || pasm->xcpu.opcode == C_ADD)
        {
            if (last - here > High - Low)
            {
                High = last;
                Low = here;
            }
        }
    }
    if (High > Low)
    {
        VarRange_H = High;
        VarRange_L = Low;
    }
}

std::string VarLL::size_to_ptr_name(int size)
{
    switch(size)
    {
    case 1:
        return "BYTE ptr" ;
    case 2:
        return "WORD ptr";
    case 4:
        return "DWORD ptr";
    }
    return "UNKOWN ptr";
}

void VarLL::prtout(XmlOutPro* out)
{
    int curlevel = 0;
    int maxlevel = m_VarRange_H - m_VarRange_L;

    VarLL_LIST::iterator iter = m_varll_list.begin();
    VarLL_LIST::iterator iter_end = m_varll_list.end();
    for(;iter!=iter_end; ++iter)
    {
        st_VarLL* p = *iter;
        if (curlevel > p->off)
        {
            out->prtl("error, var collapse!!!");
            curlevel = p->off;
        }
        else if (curlevel < p->off)
        {
            out->prtl("gap len = %x", p->off - curlevel);
            curlevel = p->off;
        }
        /*asm_prtl("%s equ %s %x", p->Name,
                 (p->size == 1) ? "BYTE ptr" :
                 (p->size == 2) ? "WORD ptr" :
                 (p->size == 4) ? "DWORD ptr" : "",
                 p->off); */

        out->prtspace(4);
        out->XMLbegin(XT_Symbol, p);
        out->prtt(p->Name);
        out->XMLend(XT_Symbol);
        out->prtt("equ");
        out->prtspace();
        out->prtt(size_to_ptr_name(p->size));
        out->prtspace();
        if (p->array != 1)
        {
            out->XMLbegin(XT_Number, NULL);
            out->prtf("%xh", p->array);
            out->XMLend(XT_Number);
            out->prtt("dup");
            out->prtspace();
        }
        out->XMLbegin(XT_Number, NULL);
        out->prtf("%xh", p->off);
        out->XMLend(XT_Number);
        out->endline();

        curlevel += p->size * p->array;
    }

    if (curlevel < maxlevel)
    {
        out->prtl("    gap len = %xh", maxlevel - curlevel);
    }
}
st_VarLL* VarLL::LookUp_VarLL(int off)
{
    VarLL_LIST::iterator pos = m_varll_list.begin();
    while (pos!=m_varll_list.end())
    {
        st_VarLL* p = *pos;
        ++pos;
        if (p->off == off)
            return p;
    }
    return NULL;
}
void VarLL::AddRef(signed int level, int opersize)
{
	if (level < m_VarRange_L || level >= m_VarRange_H ) // not in var range
		return;

    int off = level - m_VarRange_L; //this is >=0 since we know that level > m_VarRange_L
    st_VarLL* pnew = this->LookUp_VarLL(off);
    if (pnew != NULL)
        return;
    pnew = new st_VarLL;
    pnew->off = off;
    pnew->size = opersize;
    sprintf(pnew->Name, "v_%x", off);

    if (m_varll_list.empty())
    {
        m_varll_list.push_back(pnew);
    }
    else
    {
        //An ordered list
        VarLL_LIST::iterator pos = m_varll_list.begin();
        while (pos!=m_varll_list.end())
        {
            st_VarLL* p = *pos;
            if (p->off > off)
            {
                m_varll_list.insert(pos, pnew);
                return;
            }
            ++pos;
        }
        m_varll_list.push_back(pnew);
    }

}
//Nem
//This function most likely has to find/mark stack variable references in OPERITEM
// [0x0+ESP] [0x0+0+1*ESP] and such
void FuncLL::VarLL_Analysis_1(VarLL* pvarll, OPERITEM* op, AsmCode* pasm) const
{
    if (op->mode != OP_Address)
        return;
    if (op->addr.base_reg_index == _ESP_ || (op->addr.base_reg_index == _NOREG_ && op->addr.off_reg_index == _ESP_ && op->addr.off_reg_scale == 1))
    {
        signed int level = pasm->esp_level + op->addr.off_value;
        pvarll->AddRef(level, op->opersize);
    }
    if (op->addr.base_reg_index == _EBP_)
    {
        qDebug()<<"Unhandled _EBP_[] ref";
        //How to write ?
    }
}
void FuncLL::VarLL_Analysis(VarLL* pvarll)
{
    AsmCode* pasm;
    AsmCodeList::iterator pos = m_asmlist->begin();
    while (pos!=m_asmlist->end())
    {
        pasm = *(pos++);
        if (pasm->xcpu.op[0].mode == OP_Address)
            this->VarLL_Analysis_1(pvarll, &pasm->xcpu.op[0], pasm);
        if (pasm->xcpu.op[1].mode == OP_Address)
            this->VarLL_Analysis_1(pvarll, &pasm->xcpu.op[1], pasm);
    }
}

AsmCode* ea2pasm(ea_t ea, AsmCodeList* m_asmlist)
{
    AsmCode* p;
    AsmCodeList::iterator pos = m_asmlist->begin();
    while (pos!=m_asmlist->end())
    {
        p = *(pos++);
        if (p->linear == ea)
            return p;
    }
    assert(!"why here");
    return NULL;
}

void CJxxLabel::Label_Analysis()
{
    ea_t jmpto;
    AsmCode* plabel;
    AsmCode* p;
    AsmCodeList::iterator pos = m_asmlist->begin();
    while (pos!=m_asmlist->end())
    {
        p = *(pos++);
        assert(p->h.type == i_Nothing);  // Not previously analyzed for h

        if(! (p->xcpu.IsJxx() || p->xcpu.IsJmpNear()))
            continue;
        jmpto   = p->xcpu.op[0].nearptr.offset;
        plabel  = ea2pasm(jmpto, m_asmlist);

        p->h.type = i_Jump;
        if (p->xcpu.IsJmpNear())
            p->h.jmp.jmp_type = JMP_jmp;
        p->h.jmp.the_label = plabel;

        if (plabel->h.label.ref_j == NULL)
        {
            plabel->h.label.ref_j = p;
            sprintf(plabel->h.label.label_name, "loc_%lx", plabel->linear);
        }
        else
        {
            p->h.jmp.next_ref_of_this_label = plabel->h.label.ref_j;
            plabel->h.label.ref_j = p;
        }
    }
}
