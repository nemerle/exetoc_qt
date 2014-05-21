// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include <cstdio>
#include <QString>
#include "CISC.h"
#include "exe2c.h"
#include "FuncStep1.h"

//Fill in some of the other information CFunc
void	fill_func_info( ea_t pos,Func* pfnc)
{
    pfnc->m_funcname=QString("sub_%1").arg(pos,0,16).toStdString();
}

Func::Func(ea_t start)
{
    m_IfLibFunc = false;
    m_VarRange_L = 0;   //Instance=-40h
    m_VarRange_H = 0;   //Example= 0
    m_head_off = start;

    m_nStep = STEP_Init;
	m_args = 0;

    m_EBP_base = Not_EBP_based;   //invalid
    m_stack_purge = 0;

    this->ll.m_asmlist = new AsmCodeList;   //new_AsmCodeList
    this->m_exprs = new ExprManage;    //new_CExprManage

    m_functype = NULL;
}
Func::~Func()
{
    m_instr_list.clear();
    delete m_exprs;
    delete ll.m_asmlist;
}


void	Code_GetArgs(VAR* v,uint32_t &maxesp)
{
    if (v->type != v_Par)
        return;
    if (v->par_off > maxesp)
        maxesp = v->par_off;
}
UINT Func::GetVaryParaSize(POSITION pos)
{
    for (;pos!=m_instr_list.end();++pos)
    {
        Instruction * pinstr = *pos;
        if (pinstr->type == i_EspReport)
        {
            return pinstr->espreport.howlen;
        }
    }
    return 0;
}
bool	Func::Func_FillCallParas()
{
    POSITION pos = m_instr_list.begin();
    while (pos!=m_instr_list.end())
    {
        Instruction * pinstr = *pos;
        ++pos;
        if (pinstr->type == i_Call)
        {
            FuncType* pfctype = pinstr->call.call_func->m_functype;
            if (pfctype != NULL && pfctype->m_class != NULL)
            {//这是一个ecx->func
                Instruction * p = new Instruction(i_CallThis);   //new_INSTR
                p->var_r1.type = v_Reg;
                p->var_r1.opsize = BIT32_is_4;
                p->var_r1.reg = enum_ECX;
                p->call_addon.p_thecall = pinstr;
                m_instr_list.insert(pos, p); // Appending i_CallPara at i_Call
            }
            if (pfctype != NULL && pfctype->m_args != 0)
            {
                Instruction * p = new Instruction(i_CallPara);   //new_INSTR
                p->call_addon.p_thecall = pinstr;
                pinstr->call.p_callpara = p;
                p->var_r1.type = v_Var;
                UINT parasize = pfctype->para_total_size();
                if (pfctype->m_varpar)
                {
                    parasize = GetVaryParaSize(pos);
                }
                p->var_r1.opsize = parasize;
                p->var_r1.var_off = stack2varoff(pinstr->call.esp_level);

                m_instr_list.insert(pos, p); //Appending at i_Call i_CallPara
            }
			else if (pfctype == NULL && pinstr->call.call_func->m_args != 0)
            {
                Instruction * p = new Instruction(i_CallPara);   //new_INSTR
                p->call_addon.p_thecall = pinstr;
                pinstr->call.p_callpara = p;
                p->var_r1.type = v_Var;
                p->var_r1.opsize = pinstr->call.call_func->m_args*4;
                p->var_r1.var_off = stack2varoff(pinstr->call.esp_level);

                m_instr_list.insert(pos, p); //Appending at i_Call i_CallPara
            }
            //How to say? The return value of call eax
            {
                //In every call followed by i_CallRet is no problem. If this function does not return to value-added,
                //This will certainly be optimized away i_CallRet
                Instruction * p = new Instruction(i_CallRet);   //new_INSTR

                p->call_addon.p_thecall = pinstr;
                pinstr->call.p_callret = p;
                p->var_w.type = v_Reg;
                p->var_w.opsize = BIT32_is_4;
                p->var_w.reg = enum_EAX;

                m_instr_list.insert(pos, p); //i_Call followed by the i_CallRet
            }
        }
        if (pinstr->type != i_CallApi)
            continue;
        Api* papi = pinstr->call.papi;
        if (papi != NULL)
        {
            if (papi->m_functype->para_total_size() != 0)
            {
                Instruction * p = new Instruction(i_CallPara);   //new_INSTR

                p->call_addon.p_thecall = pinstr;
                pinstr->call.p_callpara = p;
                p->var_r1.type = v_Var;
                p->var_r1.opsize = papi->m_functype->para_total_size();
                p->var_r1.var_off = stack2varoff(pinstr->call.esp_level);

                m_instr_list.insert(pos, p); //i_Call followed by the multiple i_CallPara
            }
        }
        //关于返回值
        if (papi != NULL)
        {
            int n = GG_VarType_ID2Size(papi->m_functype->m_retdatatype_id);
            if (n == 4 || n == 2 || n == 1)
            {
                Instruction * p = new Instruction(i_CallRet);   //new_INSTR
                p->call_addon.p_thecall = pinstr;
                pinstr->call.p_callret = p;
                p->var_w.type = v_Reg;
                p->var_w.opsize = n;
                p->var_w.reg = enum_EAX;

                m_instr_list.insert(pos, p); //在i_Call后面加i_CallRet
            }
        }
    }

    return true;
}
bool	Func::Step5_GetArgs()
{
    if (this->m_stack_purge != 0)
    {
        this->m_args = m_stack_purge / 4;
        return true;
    }
    uint32_t maxesp = 0;
    for(POSITION pos = m_instr_list.begin(); pos!=m_instr_list.end(); ++pos)
    {
        Instruction * p = *pos;
        Code_GetArgs(&p->var_w,maxesp);		//change maxesp if need
        Code_GetArgs(&p->var_r1,maxesp);
        Code_GetArgs(&p->var_r2,maxesp);
    }
    this->m_args = maxesp / 4;
    return true;
}
bool	Func::Step2_GetRetPurge()
{
    if (this->m_IfLibFunc)
        return false;

    FuncLL the(this->ll.m_asmlist);
    int retn = the.Get_Ret_Purge();
    if (retn == -1)
    {
        return false;
    }

    m_stack_purge = retn;
    return true;
}


bool    Func::AddRemoveSomeInstr()
{
    FuncLL the(this->ll.m_asmlist);
    the.AddRemoveSomeInstr();
    return true;
}
bool	Func::Step3_FillStackInfo()
{ //Fill in the stack information, check balance. If yes, then nStep = 3
    //If CFunc is ebp based, general ebp not change once established, it can save CFunc volume within the ebp_based
    m_EBP_base = Not_EBP_based;   //invalid

    FuncLL the(this->ll.m_asmlist);
    if (this->m_prepareTrue_analysisFalse == false)
    {
        the.Prepare_CallFunc();
    }
    if (!the.Fill_Stack_Info())
    {
        return false;
    }

    m_EBP_base = the.Get_EBP_base();
    the.GetVarRange(this->m_VarRange_L, this->m_VarRange_H);

    if (this->m_VarRange_H - this->m_VarRange_L > 0)
    {
        this->m_varll.Init(this->m_VarRange_L, this->m_VarRange_H);
        the.VarLL_Analysis(&this->m_varll);
    }

    this->m_exprs->m_VarRange_H = this->m_VarRange_H;
    this->m_exprs->m_VarRange_L = this->m_VarRange_L;

    return true;
}

bool	IfValideFuncName(const char * pname)
{
    if (pname == NULL)
        return false;
    if (*pname == 0 )
        return false;

    char c = *pname;
    if(isalnum(c))
        return true;
    if ( c=='_' || c=='~')
        return true;
    return false;
}


bool VAR::IsSame(const VAR* v1,const VAR* v2)
{
    return (VarCompare(v1,v2) == 1);
}
int	VAR::VarCompare(const VAR* v1,const VAR* v2)
{
    //	0:	no relationship
    //	1:	same
    //	2:	v1 include v2
    //	3:	v2 include v1

    if (v1->type != v2->type)
        return 0;
    //if (v1->opsize != v2->opsize)
    //	return false;
    uint32_t off1,off2;
    BYTE siz1 = v1->opsize;
    BYTE siz2 = v2->opsize;

    switch (v1->type)
    {
    case v_Volatile:
        return 1;   //same
    case v_Reg:
        off1 = v1->reg;
        off2 = v2->reg;
        break;
    case v_Immed:
        off1 = v1->d;
        off2 = v2->d;
        break;
    case v_Global:
        off1 = v1->off;
        off2 = v2->off;
        break;
    case v_Par:
        off1 = v1->par_off;
        off2 = v2->par_off;
        break;
    case v_Var:
        off1 = v1->var_off;
        off2 = v2->var_off;
        break;
    case v_Tem:
        if (v1->temno != v2->temno)
            return false;
        if (siz1 != siz2)
            return false;
        return true;
    default:
        return false;
    }

    if (off1 > off2)
    {
        if (off2 + siz2 > off1)
        {
            return 3;	//	3:	v2 include v1
        }
    }
    else if (off1 < off2)
    {
        if (off1 + siz1 > off2)
        {
            return 2;	//	2:	v1 include v2
        }
    }
    else
    {	//	off1 == off2
        if (siz1 == siz2)
            return 1;	//	same
        else if (siz1 > siz2)
            return 2;	//	2:	v1 include v2
        else
            return 3;	//	3:	v2 include v1
    }
    return 0;
}


//	在phead所指的complex中，找到第no个statement
Instruction *	Func::Get_no_Statement(Instruction * phead,int no)
{
    Instruction * p = phead;
    while (p)
    {
        p = instr_next(this->m_instr_list,p);
        if (p == NULL || p == phead->begin.m_end)
            return NULL;
        assert(p->type != i_CplxBegin);	//	不该在这里遇到这个
        if (p->type == i_Begin)
        {
            if ( no == 0 )
                return p;
            p = p->begin.m_end;
            no--;			//	计数，算一个
        }

    }
    return NULL;
}

void Func::MakeDownInstr(void* hline)
{
    Instruction * p0 = (Instruction *)hline;

    POSITION pos = m_instr_list.begin();
    while (pos!=m_instr_list.end())
    {
        POSITION savpos = pos;
        Instruction * pinstr = *pos;
        ++pos;
        if (pinstr == p0 && pos != m_instr_list.end())
        {
            //list->RemoveAt(savpos);
            //list->InsertAfter(pos, pinstr);
            m_instr_list.erase(savpos);
            m_instr_list.insert(++pos, pinstr);
            return;
        }
    }
}
//void	prt_partern(INSTR_LIST* list,Instruction * phead,char * partern_buf);




const char *	hlcode_name(HLType t)
{
    switch (t)
    {
    case i_Jump:		return("i_Jump        ");
    case i_Label:		return("i_Label       ");
    case i_Begin:       return("i_Begin       ");
    case i_End:         return("i_End         ");
    case i_Assign:      return("i_Assign      ");
    case i_Var:         return("i_Var         ");
    case i_Unknown:     return("i_Unknown     ");
    case i_RetPar:      return("i_RetPar      ");
    case i_Return:      return("i_Return      ");
    case i_Add:         return("i_Add         ");
    case i_Sub:         return("i_Sub         ");
    case i_Xor:         return("i_Xor         ");
    case i_Sar:         return("i_Sar         ");
    case i_And:         return("i_And         ");
    case i_Imul:        return("i_Imul        ");
    case i_Readpointto: return("i_Readpointto ");
    case i_Writepointto:return("i_Writepointto");
    case i_Cmp:         return("i_Cmp         ");
    case i_Test:        return("i_Test        ");
    case i_Lea:         return("i_Lea         ");
    case i_Address:     return("i_Address     ");
    case i_Call:        return("i_Call        ");
    case i_CallApi:     return("i_CallApi     ");
    case i_CallPara:    return("i_CallPara    ");
    case i_CallThis:    return("i_CallThis    ");
    case i_CallRet:     return("i_CallRet     ");
    case i_CplxBegin:   return("i_CplxBegin   ");
    case i_CplxEnd:     return("i_CplxEnd     ");
    case i_Nop:         return("i_Nop         ");
    case i_JmpAddr:		return("i_JmpAddr     ");
    case i_SignExpand:	return("i_SignExpand  ");
    case i_NosignExpand:return("i_NosignExpand");
    case i_GetAddr:     return("i_GetAddr     ");
    case i_EspReport:   return("i_EspReport   ");
    default:
    {
        static char buf[80];
        sprintf(buf, "unknown %x", t);
        return(buf);
    }
    }
    //return NULL;	//never here
}

void Func::report_info()
{
    log_prtl("func name = %s", this->m_funcname.c_str());
    log_prtl("func linear address = %x", this->m_head_off);
    if (m_VarRange_L >= 0)
        log_prtl("func m_VarRange_L = %x", this->m_VarRange_L);
    else
        log_prtl("func m_VarRange_L = -%x", -this->m_VarRange_L);
    if (m_VarRange_H >= 0)
        log_prtl("func m_VarRange_H = %x", this->m_VarRange_H);
    else
        log_prtl("func m_VarRange_H = -%x", -this->m_VarRange_H);
}
//SuperC_func: 只在＜CFunc::prtout_internal＞中使用
QString PrtAddOn_internal(const char * varname, Pst_InstrAddOn pAddOn)
{
    if (pAddOn == NULL)
        return varname;
    switch (pAddOn->type)
    {
    case IA_Nothing:
        return varname;
    case IA_ReadPointTo:
        return QString("*%1").arg(PrtAddOn_internal(varname,pAddOn->pChild));
    case IA_AddImmed:
        return QString("(%1+%2)").arg(PrtAddOn_internal(varname,pAddOn->pChild)).arg(pAddOn->addimmed.iAddon);
    case IA_MulImmed:
        return QString("%1 * %2").arg(PrtAddOn_internal(varname,pAddOn->pChild)).arg(pAddOn->addimmed.iAddon);
    case IA_GetAddress:
        return QString("&%1").arg(PrtAddOn_internal(varname,pAddOn->pChild));
    }
    return "Error_PrtAddOn";
}
void	Func::prtout_internal(XmlOutPro* out)
{

    for(POSITION pos=m_instr_list.begin(); pos != m_instr_list.end(); ++pos)
    {
        Instruction * p = *pos;

        if (p->type == i_End || p->type == i_CplxEnd)
            out->ident_sub1();

        out->ident();

        out->prtt(hlcode_name(p->type));
        if (p->var_w.type != 0)
        {
            out->prtt(" w=");
            out->XMLbegin(XT_Symbol, p->var_w.thevar);
            out->prtt(this->m_exprs->BareVarName(&p->var_w));
            out->XMLend(XT_Symbol);
        }
        if (p->var_r1.type != 0)
        {
            out->prtt(" r1=");
            out->XMLbegin(XT_Symbol, p->var_r1.thevar);
            out->prtt(PrtAddOn_internal(this->m_exprs->BareVarName(&p->var_r1), p->va_r1.pao));
            out->XMLend(XT_Symbol);
        }
        if (p->var_r2.type != 0)
        {
            out->prtt(" r2=");
            out->XMLbegin(XT_Symbol, p->var_r2.thevar);
            out->prtt(PrtAddOn_internal(this->m_exprs->BareVarName(&p->var_r2), p->va_r2.pao));
            out->XMLend(XT_Symbol);
        }
        if (p->type == i_Address)
        {
            out->prtf(" i1=%d", p->i1);
            out->prtf(" i2=0x%x", p->i2);
        }
        if (p->type == i_CallApi)
        {
            out->prtt(p->call.papi->name);
        }

        out->EOL();
        if (p->type == i_Begin || p->type == i_CplxBegin)
            out->ident_add1();
    }
}


void Func::DeleteUnusedVar()
{
    this->m_exprs->ClearUse();
    for(POSITION pos=m_instr_list.begin(); pos != m_instr_list.end(); ++pos)
    {
        Instruction * p = *pos;
        M_t* pt;
        pt = p->var_r1.thevar; if (pt != NULL) pt->tem_useno++;
        pt = p->var_r2.thevar; if (pt != NULL) pt->tem_useno++;
        pt = p->var_w .thevar; if (pt != NULL) pt->tem_useno++;
    }
    this->m_exprs->DeleteUnusedVars();
    //m_exprs->DeleteUnuse_VarList(this->m_exprs->vList);
}


VarTypeID GetMemDataType(VAR* pvar)
{
    /*
    assert(pvar);
    assert(pvar->thevar);
    assert(pvar->thevar->m_DataType);
    assert(pvar->thevar->m_DataType->m_type == vtt_class);
    */
    Class_st* pstruc = VarTypeMng::get()->is_class(pvar->thevar->m_DataTypeID);
    assert(pstruc);

    if (pvar->part_flag == 0)
    {//There is only one possibility -> the structure is only one large
        assert(pstruc->m_DataItems.size() == 1);
        return pstruc->GetClassItem(0)->m_vartypeid;
    }
    else
    {
        return pstruc->GetClassItem(pvar->part_flag-1)->m_vartypeid;
    }
}

std::string Func::Instr_prt_simple(Instruction * p)
{
    std::string s = hlcode_name(p->type);
    if (p->var_w.type != 0)
    {
        s += " w=";
        s += this->m_exprs->BareVarName(&p->var_w);
    }
    if (p->var_r1.type != 0)
    {
        s += " r1=";
        s += PrtAddOn_internal(this->m_exprs->BareVarName(&p->var_r1), p->va_r1.pao).toStdString();
    }
    if (p->var_r2.type != 0)
    {
        s += " r2=";
        s += PrtAddOn_internal(this->m_exprs->BareVarName(&p->var_r2), p->va_r2.pao).toStdString();
    }
    return s;
}

bool Func::Var_analysis()
{
    //This function can only be called once
    POSITION pos = m_instr_list.begin();
    while (pos!=m_instr_list.end())
    {
        Instruction * p = *pos;
        ++pos;

        if (p->type == i_EspReport)
        {
            this->m_exprs->EspReport(p->espreport.esp_level);

            //m_instr_list->RemoveAt(savpos); //This statement is again useless, forget deleted
            continue;
        }

        this->m_exprs->AddRef(&p->var_r1);
        this->m_exprs->AddRef(&p->var_r2);
        this->m_exprs->AddRef(&p->var_w);
    }

    return true;
}


Instruction *CFunc_InstrList::instr_next_in_func(const Instruction * p)
{
    return instr_next(m_instr_list,p);
}
Instruction *CFunc_InstrList::instr_prev_in_func(const Instruction * p)
{
    return instr_prev(m_instr_list, p);
}
Instruction *CFunc_InstrList::skip_compl(const Instruction * p)
{
    //p是一个begin，返回是end后一条
    assert(p->type == i_Begin);
    return instr_next_in_func(p->begin.m_end);
}
void Func::ReType(M_t* p, const char * newtype)
{
    VarTypeID vid = get_DataType(newtype);
    if (vid == 0)
        return;

    if (GG_VarType_ID2Size(vid) <= p->size)
    {//大变小，一般不会发生
        p->size = GG_VarType_ID2Size(vid);
        p->m_DataTypeID = vid;
    }
    else
    {//小变大，这可复杂了
        p->size = GG_VarType_ID2Size(vid);
        p->m_DataTypeID = vid;

        this->m_exprs->Enlarge_Var(p, this->m_instr_list);
    }
}

std::string GetToken(const char * &p)
{
    std::string s;
    while (*p != 0 && *p != ' ')
    {
        s += *p++;
    }
    while (*p == ' ')
        p++;
    return s;
}



void Func::Restart()
{//要再分析，保留 this->m_funcdefine
    this->m_nStep = STEP_IDA_4;
    //this->ll.m_asmlist = new AsmCodeList;   //new_AsmCodeList
    this->m_exprs = new ExprManage;    //new_CExprManage

    this->m_exprs->m_VarRange_H = this->m_VarRange_H;
    this->m_exprs->m_VarRange_L = this->m_VarRange_L;

    this->m_instr_list.clear();
}

bool Func::Step_Label_Analysis()
{
    CJxxLabel the(this->ll.m_asmlist);
    the.Label_Analysis();

    return true;
}


bool    Func::Step_1()
{
    bool bCreateNewFunc = true;
    if (this->m_IfLibFunc)
        return false;

    if (this->m_nStep != STEP_Init)
        return false;

    CFuncStep1 the(this->ll.m_asmlist);
    if (the.Step_1(this->m_head_off))
    {
        this->m_end_off = the.Get_end_off();

        if (bCreateNewFunc)
            the.CreateNewFunc_if_CallNear();

        return true;
    }
    return false;
}
void Func::Fill_this_ECX(VarTypeID id)
{
    //	This means that this is subroutine of a class ECX changes its name into 'this'
    M_t* p = this->m_exprs->SearchMT(MTT_reg, enum_ECX);
    if (p == NULL || p->size != BIT32_is_4)
        return;		//	why ?
    p->namestr = "this";
    p->m_DataTypeID = id;
}
