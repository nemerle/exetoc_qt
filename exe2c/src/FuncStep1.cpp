// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//#include "stdafx.h"
#include	"CISC.h"
#include "exe2c.h"
#include "FuncStep1.h"

static void	Add_in_order(EALIST *list, ea_t i)
{
    if (list->size()==0)
    {
        list->push_back(i);
        return;
    }
    for(EALIST::iterator pos = list->begin();pos!=list->end();++pos)
    {
        ea_t ea = *pos;
        if (ea == i)
            return;		//	已经有了，就算了
        if (ea > i)
        {
            list->insert(pos,i);
            return;
        }
    }
    list->push_back(i);
}

void    CFuncStep1::check_if_switch_case(ea_t cur_off, CaseList* pcaselist,EALIST* pjxxlist,
                                         XCPUCODE* pxcpu)
{//static function
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
                return;		//	确认这个table有效

        //alert("switch case find 2");

        ea_t d = Peek_D(ptbl);
        if (! IfInWorkSpace(d))
                return;		//	确认第一条有效

        //alert("switch case find 3");

        ea_t break_off = 0;	//	下面，要确定break_off的值 //Next, to determine the value of break_off
        EALIST::iterator pos = pjxxlist->begin();
        while (pos!=pjxxlist->end())
        {
            ea_t ea = *pos;
            ++pos;
            if (ea > cur_off)
            {
                break_off = ea;
                break;
            }
        }
        if (break_off == 0)
                return;		//	没找到条件跳?不对劲

        //alert("switch case find 4");


        //if (pjxx->jmp.jmp_type != JMP_ja)
        //	return;		//	不是ja,也不对
        if (d < cur_off || d > break_off)
                return;		//	第一个就没对准，不行

        //alert("switch case find 5");

        //alert("rreally switch case");
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

static bool	any_free_ea(EALIST *jxxlist, EALIST *usedlist, ea_t* pea)
{
    EALIST::iterator pos = jxxlist->begin();
    while (pos!=jxxlist->end())
    {
        ea_t ea = *pos;//jxxlist->;
        ++pos;
        if (std::find(usedlist->begin(),usedlist->end(),ea)==usedlist->end())
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
    while (pos1!=caselist.end())
    {
        CASE_t *p1 = *pos1;//caselist.;
        ++pos1;
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
                this->m_asmlist->push_back(pnew);
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
            g_Cexe2c->func_new(pxcpu->op[0].nearptr.offset);
        }
    }
}
bool	CFuncStep1::Step_1(ea_t head_off)
// Find the function end address, generate asm opcode list
{
        //assert(m_nStep == 0);

        CaseList caselist;
        EALIST jxxlist;
        EALIST usedlist;


        ea_t ea = head_off;
        assert(ea < 0x10000000);
        assert(ea >= 0x400000);

        jxxlist.push_front(ea);

        while (any_free_ea(&jxxlist,&usedlist,&ea))
        {	//	Travell(/ed?) all the jxx
            for (;;)
            {
                Add_in_order(&usedlist,ea);

                CDisasm the;
                the.Disasm_OneCode(ea); //ea will be incremented

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
                if (std::find(usedlist.begin(),usedlist.end(),ea)!=usedlist.end())
                    break;
            }
        }

        EALIST::iterator pos = usedlist.begin();
        while (pos!=usedlist.end())
        {	//	生成asm opcode list
            if (this->m_asmlist == NULL)
            {
                this->m_asmlist = new AsmCodeList;
            }

            ea_t ea = *pos;//usedlist.;
            ++pos;
            AsmCode *p = AsmCode::new_AsmCode();
            p->linear = ea;

            CDisasm the;
            p->opsize = the.Disasm_OneCode(ea);
            p->xcpu = *the.get_xcpu();

            this->m_asmlist->push_back(p);

                //	查找是不是刚过一个swith case
        this->CheckIfJustSwitchCase(caselist, ea);
        }

        AsmCode *pasm = *this->m_asmlist->rbegin();
        m_end_off = pasm->linear + pasm->opsize;
        return true;
}

//==========================================

static bool	isLeave(AsmCode* p)
{	//	如果是leave或mov esp,??之类的指令，则当前stack状态对后面没有影响
    //If leave or mov esp,?? or similar instructions, then the current stack state has no effect on the back
        if (p->xcpu.opcode == C_LEAVE)
                return true;
        if (p->xcpu.opcode == C_MOV
                && (p->xcpu.op[0].mode == OP_Register)
                && (p->xcpu.op[0].reg.reg_index == _ESP_)	//mov esp,ebp
                )
                return true;
        return false;
}

AsmCode* CFuncLL::ea2pasm(ea_t ea)
{
    AsmCodeList::iterator pos = m_asmlist->begin();
    while (pos!=m_asmlist->end())
    {
        AsmCode* p = *pos;//m_asmlist->;
        ++pos;
        if (p->linear == ea)
            return p;
    }
    assert(!"whye here");
    return NULL;
}
bool	CFuncLL::stack_stack(AsmCode* p0, AsmCode* p1)
{    // Returns true if there were any changes made
    signed int & esp0 = p0->esp_level;
    signed int & esp0_next = p0->esp_level_next;
    signed int & esp1 = p1->esp_level;

    //	首先，是p0自身的建设-------
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
        if (esp0 != ESP_UNKNOWN
                && esp0_next != ESP_UNKNOWN
                && p0->xcpu.opcode == C_CALL
                && p0->xcpu.op[0].mode == OP_Address
                && p0->xcpu.op[0].addr.base_reg_index == _NOREG_
                && p0->xcpu.op[0].addr.off_reg_index == _NOREG_
                )
        {
            ea_t address = p0->xcpu.op[0].addr.off_value;
            Api *papi = g_ApiManage->get_api(address);	//find it
            if (papi)
            {
                if (esp0_next != esp0 + papi->m_stack_purge)
                    return false;	//find error
            }
            else
            {	//not find, insert it
                alert_prtf("error not find api %x", address);
                g_ApiManage->new_api(address,esp0_next - esp0);
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
    if (esp0_next != ESP_UNKNOWN
            && esp0_next != ESP_IGNORE
            && esp1 == ESP_UNKNOWN)
    {
        esp1 = esp0_next;
        return true;
    }
    if (esp0_next != ESP_UNKNOWN
            && esp0_next != ESP_IGNORE
            && esp1 != ESP_UNKNOWN)
    {	//do some check
        if (esp0_next != esp1)
            return false;
    }
    return false;
}
ea_t FindApiAddress_Reg(uint32_t regindex, XCPUCODE* pxcpu1, AsmCodeList* asmlist)
{
    //意思是，我要正确处理call eax
    //到前面去找一个mov eax, [405070]
    ea_t retn = 7;

    AsmCodeList::iterator pos = asmlist->begin();
        while (pos!=asmlist->end())
        {
                AsmCode* p = *pos;//asmlist->;
                ++pos;
        XCPUCODE* pxcpu = &p->xcpu;
                if (pxcpu == pxcpu1)
                        return retn;
        if (pxcpu->opcode == C_MOV
            && pxcpu->op[0].mode == OP_Register
            && pxcpu->op[0].reg.reg_index == regindex)
        {
            if (pxcpu->op[1].mode == OP_Address
                && pxcpu->op[1].addr.base_reg_index == _NOREG_
                && pxcpu->op[1].addr.off_reg_index == _NOREG_
                )   //call [405070]
            {
                ea_t address = pxcpu->op[1].addr.off_value;
                retn = address;
            }
        }
        }
    return retn;
}
bool	CFuncLL::Asm_Code_Change_ESP(int &esp, XCPUCODE* pxcpu)
{//算出来esp,就return true
        //	让我们假定一个func中只出现一次mov ebp,esp
        if (pxcpu->opcode == C_MOV
        && (pxcpu->op[0].mode == OP_Register)
        && (pxcpu->op[0].reg.reg_index == _ESP_)
        && (pxcpu->op[1].mode == OP_Register)
        && (pxcpu->op[1].reg.reg_index == _EBP_))
    {
        if (this->m_EBP_base != Not_EBP_based
            && esp == ESP_UNKNOWN)
        {
            esp = this->m_EBP_base;	//mov esp,ebp
            return true;
        }
    }
        if (pxcpu->opcode == C_MOV
        && (pxcpu->op[0].mode == OP_Register)
        && (pxcpu->op[0].reg.reg_index == _EBP_)
        && (pxcpu->op[1].mode == OP_Register)
        && (pxcpu->op[1].reg.reg_index == _ESP_))
    {
        if (esp != ESP_UNKNOWN
            && this->m_EBP_base == Not_EBP_based)
        {
            this->m_EBP_base = esp;		//mov ebp,esp
            return true;
        }
    }

        if (pxcpu->opcode == C_LEAVE)
        {
                esp = m_EBP_base;	//	mov	esp,ebp
                esp += 4;		//	pop	ebp
        }
        if (pxcpu->opcode == C_PUSH)
                esp -= 4;
        if (pxcpu->opcode == C_POP)
                esp += 4;
        if (pxcpu->opcode == C_SUB)
                if (pxcpu->op[0].mode == OP_Register)
                        if (pxcpu->op[0].reg.reg_index == _ESP_)
                                if (pxcpu->op[1].mode == OP_Immed)
                                {
                                        esp -= pxcpu->op[1].immed.immed_value;
                                }
        if (pxcpu->opcode == C_ADD)
                if (pxcpu->op[0].mode == OP_Register)
                        if (pxcpu->op[0].reg.reg_index == _ESP_)
                                if (pxcpu->op[1].mode == OP_Immed)
                                {
                                        esp += pxcpu->op[1].immed.immed_value;
                                }

        if (pxcpu->opcode == C_CALL)
        {
            if (pxcpu->op[0].mode == OP_Near)
            {
                Func* pfunc = g_Cexe2c->GetFunc(pxcpu->op[0].nearptr.offset);
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
        if (pxcpu->opcode == C_CALL &&
            pxcpu->op[0].mode == OP_Address &&
            pxcpu->op[0].addr.base_reg_index == _NOREG_ &&
            pxcpu->op[0].addr.off_reg_index == _NOREG_
            )   //call [405070]
        {
            ea_t address = pxcpu->op[0].addr.off_value;
            Api* papi = g_ApiManage->get_api(address);//,stacksub))	//find it
            if (papi)
            {
                esp += papi->m_stack_purge;
                return true;
            }
        }
        if (pxcpu->opcode == C_CALL &&
            pxcpu->op[0].mode == OP_Register
            )   //call eax
        {
            ea_t address = FindApiAddress_Reg(pxcpu->op[0].reg.reg_index, pxcpu, this->m_asmlist);
            Api* papi = g_ApiManage->get_api(address);//,stacksub))	//find it
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
        //	影响stack的有这几种情况：
        //	enter leave
        //	push ebp,mov ebp,esp,.....,mov esp,ebp,pop ebp
        //	push,pop
        //	call near
        //	ret时检查是否已平衡
        //	对call	[immed]等，暂不考虑s
}



void CFuncLL::AddRemoveSomeInstr()
{
    AsmCodeList::iterator pos = m_asmlist->begin();
    while (pos!=m_asmlist->end())
    {
        AsmCode* pasm = *pos;//m_asmlist->;
        ++pos;
        if (pasm->iAddRemove != 0)
            continue;

        XCPUCODE* pxcpu = &pasm->xcpu;
        if (pxcpu->opcode == C_CALL && pxcpu->op[0].mode == OP_Near)
        {
            Func* pfunc = g_Cexe2c->GetFunc(pxcpu->op[0].nearptr.offset);
            if (pfunc == NULL)
                continue;
            if (pfunc->m_IfLibFunc && pfunc->m_functype != NULL)
            {
                FuncType* fctype = pfunc->m_functype;
                if (!fctype->m_internal_name.compare("_EH_prolog"))
                {
                    // ??strcmp(fctype->m_internal_name, "_EH_prolog");
                    pasm->iAddRemove = 1;

                    AsmCode* pnew;
                    //push ebp
                    pnew = AsmCode::new_AsmCode();
                    pnew->iAddRemove = 2;
                    pnew->xcpu.opcode = C_PUSH;
                    pnew->xcpu.op[0].mode = OP_Register;
                    pnew->xcpu.op[0].opersize = 4;
                    pnew->xcpu.op[0].reg.reg_index = _EBP_;
                    m_asmlist->insert(pos, pnew);

                    //mov ebp, esp
                    pnew = AsmCode::new_AsmCode();
                    pnew->iAddRemove = 2;
                    pnew->xcpu.opcode = C_MOV;
                    pnew->xcpu.op[0].mode = OP_Register;
                    pnew->xcpu.op[0].opersize = 4;
                    pnew->xcpu.op[0].reg.reg_index = _EBP_;
                    pnew->xcpu.op[1].mode = OP_Register;
                    pnew->xcpu.op[1].opersize = 4;
                    pnew->xcpu.op[1].reg.reg_index = _ESP_;
                    m_asmlist->insert(pos, pnew);

                }
            }
        }
    }
}
void CFuncLL::Prepare_CallFunc()
{//把本函数中call到的函数都prepare一遍
    //Put this function call to the function of all prepare again
    AsmCodeList::iterator pos = m_asmlist->begin();
    while (pos!=m_asmlist->end())
    {
        AsmCode* pasm = *pos;//m_asmlist->;
        ++pos;
        XCPUCODE* pxcpu = &pasm->xcpu;
        if (pxcpu->opcode == C_CALL && pxcpu->op[0].mode == OP_Near)
        {
            Func* pfunc = g_Cexe2c->GetFunc(pxcpu->op[0].nearptr.offset);
            if (pfunc == NULL)
                continue;
            pfunc->PrepareFunc();
        }
    }
}
bool	CFuncLL::Fill_Stack_1()
{//Why is it always true ?
    AsmCodeList::iterator pos = m_asmlist->begin();
        AsmCode* pasm = 0;
        if (pos!=m_asmlist->end())
        {
            pasm = *pos;//m_asmlist->;
            ++pos;
        }
        while (pos!=m_asmlist->end())
        {
                AsmCode* p0 = pasm;
                pasm = *pos;//m_asmlist->;
                ++pos;
                if (stack_stack(p0,pasm))
                        return true;
        }
        return false;
}

bool	CFuncLL::Check_Stack()
//	检查该函数的stack是否平衡
{
    AsmCodeList::iterator pos = this->m_asmlist->begin();
    signed int lastesp = 0;
    while (pos!=this->m_asmlist->end())
    {
        AsmCode* p = *pos;
        ++pos;
        signed int esp = p->esp_level;
        if (esp == ESP_UNKNOWN)
            return false;
        if (lastesp != ESP_UNKNOWN && lastesp != ESP_IGNORE && esp != lastesp)
            return false;

        if (p->xcpu.IsJxx() || p->xcpu.IsJmpNear())
        {
            ea_t jmpto = p->xcpu.op[0].nearptr.offset;
            AsmCode* p = ea2pasm(jmpto);
            if (esp != p->esp_level)
                return false;
        }
        if (p->xcpu.opcode == C_RET)
        {
            if (esp != 0)
                return false;
        }
        lastesp = p->esp_level_next;
    }
    return true;
}

bool	CFuncLL::Fill_Stack_Info()
{
    AsmCode* pasm = *this->m_asmlist->begin();
    if (pasm->linear == 0x401150)
    {
        pasm->linear = 0x401150;
    }
    pasm->esp_level = 0;	// I know the first one

    for (int i=0;;i++)
    { //这里会死循环
        if ((i % 1000) == 0)
            nop();
        if ( Fill_Stack_1() )
            continue;
        break;
    }

    if (! Check_Stack())
        return false;

    return true;
}

int CFuncLL::Get_Ret_Purge()
//return -1 for fail
{
        int retn = -1;
        AsmCodeList::iterator pos = this->m_asmlist->begin();
        while (pos!=this->m_asmlist->end())
        {
                AsmCode* pasm = *pos;
                ++pos;
                XCPUCODE* pxcpu = &pasm->xcpu;
                if (pxcpu->opcode == C_RET) //找到了ret语句
                {
                        int r = 0;
                        if (pxcpu->op[0].mode == OP_Immed)	// means RET n
                        {
                                r = pxcpu->op[0].immed.immed_value;
                        }

                        if (retn == -1)
                                retn = r;
                        else if (retn != r) //两个Ret不一致？
                                return -1;
                }
        }
        if (retn == -1)
        {
                //alert_prtf("why not find RET ? func = %x", this->m_head_off);
                //myexit(2);
                //return false;
        }
    return retn;
}

std::string CFuncLL::GetLabelName(ea_t ea)
{
    std::string retn;

    Func* pfunc = g_Cexe2c->GetFunc(ea);
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
void	CFuncLL::prtout_asm(Func* pfunc, VarLL* pvarll, XmlOutPro* out)
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
void	CFuncLL::prtout_asm_1(VarLL* pvarll, XmlOutPro* out)
{   //	按运行代码以ASM显示func，对其中的GAP能指出来

        ea_t last = 0xffffffff;
        AsmCodeList::iterator pos = this->m_asmlist->begin();
        while (pos!=this->m_asmlist->end())
        {
            AsmCode* pasm = *pos;
            ++pos;
            if (pasm->iAddRemove == 2)
                continue;
            ea_t ea = pasm->linear;

            char	buf[280];
            uint32_t n;
            if (pasm->xcpu.opcode == C_JCASE)
            {
                n = 0;
                sprintf(buf,"case jmp to %lx",pasm->xcpu.op[0].nearptr.offset);
            }
            else
            {
                st_IDA_OUT idaout;
            CDisasm the;
            //n = the.Disassembler(buf, ea2ptr(ea), ea);
            n = the.Disassembler_X(ea2ptr(ea), ea, &idaout);
            XCPUCODE* pxcpu = the.get_xcpu();
            if (pxcpu->op[0].mode == OP_Near)
            {
                ea_t linear = pxcpu->op[0].nearptr.offset;
                std::string labelname = this->GetLabelName(linear);
                if (labelname.size()!=0)
                {
                    idaout.Par1Str = labelname;
                }
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
                        idaout.Par1Str += '.';
                        idaout.Par1Str += p->Name;
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
                        idaout.Par2Str += '.';
                        idaout.Par2Str += p->Name;
                    }
                }
            }
            idaout.output(buf);
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
            out->prtt("    ");  //四个空格的位置要留
        }
        else
            out->prtf("%4x", -pasm->esp_level);
        out->prtspace();
        out->XMLbegin(XT_AsmOffset, (void*)ea);
        out->prtf("%x", ea);
        out->XMLend(XT_AsmOffset);
        out->prtt(buf);
        out->endline();

                last = ea+n;
        }
}


void CFuncLL::GetVarRange(signed int& VarRange_L, signed int& VarRange_H)
{
    /*
    如果一个函数开头是：
   0 401010 SUB    ESP,00000190
 190 401016 LEA    ECX,[ESP+00]
 则 VarRange_L = -190h
 则 VarRange_H = 0
 写作
    0 401010 SUB    ESP,00000190
  190 401016 LEA    ECX,[ESP+v_00]
    */
    signed int L = 0;
    signed int H = 0;

    AsmCodeList::iterator pos = this->m_asmlist->begin();
        while (pos!=this->m_asmlist->end())
        {
                AsmCode* pasm = *pos;
                ++pos;
        signed int last = pasm->esp_level;
        signed int here = pasm->esp_level_next;
        if (pasm->xcpu.opcode == C_SUB || pasm->xcpu.opcode == C_ADD)
        {
            if (last - here > H - L)
            {
                H = last;
                L = here;
            }
        }
    }
    if (H - L > 0)
    {
        VarRange_H = H;
        VarRange_L = L;
    }
}


void VarLL::prtout(XmlOutPro* out)
{
    int curlevel = 0;
    int maxlevel = this->m_VarRange_H - this->m_VarRange_L;

    VarLL_LIST::iterator pos = this->m_varll_list.begin();
    while (pos!=this->m_varll_list.end())
    {
        st_VarLL* p = *pos;
        ++pos;
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
                 (p->size == 4) ? "uint32_t ptr" : "",
                 p->off); */

        out->prtspace(4);
        out->XMLbegin(XT_Symbol, p);
        out->prtt(p->Name);
        out->XMLend(XT_Symbol);
        out->prtt("equ");
        out->prtspace();
        out->prtt((p->size == 1) ? "BYTE ptr" :
                          (p->size == 2) ? "WORD ptr" :
                          (p->size == 4) ? "uint32_t ptr" : "");
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
    VarLL_LIST::iterator pos = this->m_varll_list.begin();
    while (pos!=this->m_varll_list.end())
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
    if (level < this->m_VarRange_H && level >= this->m_VarRange_L)
    {
    }
    else
        return;
    int off = level - this->m_VarRange_L; //这个是>=0的
    st_VarLL* pnew = this->LookUp_VarLL(off);
    if (pnew != NULL)
        return;
    pnew = new st_VarLL;
    pnew->off = off;
    pnew->size = opersize;
    sprintf(pnew->Name, "v_%x", off);

    if (this->m_varll_list.empty())
    {
        this->m_varll_list.push_back(pnew);
    }
    else
    {//有序列表
        VarLL_LIST::iterator pos = this->m_varll_list.begin();
        while (pos!=this->m_varll_list.end())
        {
            st_VarLL* p = *pos;
            if (p->off > off)
            {
                this->m_varll_list.insert(pos, pnew);
                return;
            }
            ++pos;
        }
        this->m_varll_list.push_back(pnew);
    }

}
void CFuncLL::VarLL_Analysis_1(VarLL* pvarll, OPERITEM* op, AsmCode* pasm)
{
    if (op->mode != OP_Address)
        return;
        if (op->addr.base_reg_index == _ESP_
        || (op->addr.base_reg_index == _NOREG_
                        && op->addr.off_reg_index == _ESP_
                        && op->addr.off_reg_scale == 1))
    {
        signed int level = pasm->esp_level + op->addr.off_value;
        pvarll->AddRef(level, op->opersize);
    }
        if (op->addr.base_reg_index == _EBP_)
    {
        //怎么写？
    }
}
void CFuncLL::VarLL_Analysis(VarLL* pvarll)
{
    AsmCodeList::iterator pos = this->m_asmlist->begin();
    while (pos!=this->m_asmlist->end())
    {
        AsmCode* pasm = *pos;
        ++pos;
        if (pasm->xcpu.op[0].mode == OP_Address)
            this->VarLL_Analysis_1(pvarll, &pasm->xcpu.op[0], pasm);
        if (pasm->xcpu.op[1].mode == OP_Address)
            this->VarLL_Analysis_1(pvarll, &pasm->xcpu.op[1], pasm);
    }
}

AsmCode* ea2pasm(ea_t ea, AsmCodeList* m_asmlist)
{
    AsmCodeList::iterator pos = m_asmlist->begin();
    while (pos!=m_asmlist->end())
    {
        AsmCode* p = *pos;
        ++pos;
        if (p->linear == ea)
            return p;
    }
    assert(!"whye here");
    return NULL;
}

void CJxxLabel::Label_Analysis()
{
    AsmCodeList::iterator pos = this->m_asmlist->begin();
    while (pos!=this->m_asmlist->end())
    {
        AsmCode* p = *pos;
        ++pos;
        assert(p->h.type == i_Nothing);  //以前没对h分析过//Not previously analyzed for h

        if (p->xcpu.IsJxx() || p->xcpu.IsJmpNear())
        {
            ea_t jmpto = p->xcpu.op[0].nearptr.offset;
            AsmCode* plabel = ea2pasm(jmpto, m_asmlist);

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
}
