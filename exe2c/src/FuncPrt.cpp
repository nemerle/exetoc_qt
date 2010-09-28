// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include "CISC.h"
#include "exe2c.h"

#define m_instr_list Q->m_instr_list
#define m_exprs Q->m_exprs
#define m_head_off Q->m_head_off
#define m_funcname Q->m_funcname
#define VarName m_exprs->VarName
#define ll Q->ll
#define m_nStep Q->m_nStep
#define m_varll Q->m_varll

CPrtOut g_PrtOut;

const char * CFunc_Prt::BareVarName(const VAR* v)
{
        return this->m_exprs->BareVarName(v);
}

void	CFunc_Prt::prt_case(const INSTR * phead, const INSTR * plabel, XmlOutPro* out)
{
    CFunc_InstrList instrl(m_instr_list);

    INSTR * p;
    p = phead->begin.m_end;
    p = instrl.instr_prev_in_func(p);
    INSTR * pbreak = p;	//	For the time being that the last one is the break
    assert(pbreak->type == i_Label);

    if (plabel == pbreak)
    {
        out->ident_add1();
        out->prtl_ident("break;");
        out->ident_sub1();
        return;
    }
    if (p->type == i_Label)
        p = instrl.instr_next_in_func(plabel);
    if (p->type != i_Begin)
        p = instrl.instr_prev_in_func(p);

    if(p->type != i_Begin)
    {
        //alert_prtf("type in case is %s",hlcode_name(p->type));
        out->ident_add1();
        out->prtl_ident("error: not parsed");
        out->ident_sub1();
        return;
    }
    assert(p->type == i_Begin);

    out->ident_add1();
    prt_one_statement_mainbody(p, out);
    {// No need to add a break at the end
        // Since the last case followed by break

        INSTR * pend = p->begin.m_end;
        INSTR * plast = instrl.instr_prev_in_func(pend);
        if (plast->type != i_Jump)
        {
            INSTR * pnext = instrl.instr_next_in_func(pend);
            if (pnext == pbreak)
            {
                out->prtl_ident("break;");
            }
        }

    }
    out->ident_sub1();
}


bool IfTemVar(const VAR* var)
{
    if (var->type == v_Tem)
        return true;
    if (var->thevar == NULL)
        return false;
    if (var->thevar->type == MTT_tem)
        return true;
    if (var->thevar->bTem == true)
        return true;
    return false;
}
bool IfSameTemVar(const VAR* v1, const VAR* v2)
{
    if (!IfTemVar(v1))
        return false;
    if (!IfTemVar(v2))
        return false;
    if (v1->type == v_Tem)
    {
        return v1->temno == v2->temno;
    }
    else
    {
        return v1->thevar == v2->thevar;
    }
}

void	CFunc_Prt::prt_add(const INSTR *p, const char * s, XmlOutPro* out)
{
    if (strcmp(s,"*") == 0)
    {
        strcmp(s,"*");//FIXME out->prtt("*"); ?
    }
    if (IfTemVar(&p->var_w))
    {
        out->prtt("(");
        this->prt_va(p->va_r1, out);
        out->prtspace();
        out->prtt(s);
        out->prtspace();
        this->prt_va(p->va_r2, out);
        out->prtt(")");
        return;
    }
    this->prt_var(&p->var_w, out);

    if (p->var_w.thevar == p->var_r1.thevar)
    {
        out->prtspace();
        out->prtt(s);
        out->prtt("= ");  //  a += b
        this->prt_va(p->va_r2, out);
    }
    else if (p->var_w.thevar == p->var_r2.thevar)
    {
        out->prtspace();
        out->prtt(s);
        out->prtt("= ");  //  a += b
        this->prt_va(p->va_r1, out);
    }
    else
    {
        out->prtt(" = ");  //  a = b + c
        this->prt_va(p->va_r1, out);
        out->prtspace();
        out->prtt(s);
        out->prtspace();
        this->prt_va(p->va_r2, out);
    }
}

void    CFunc_Prt::out_PointTo(st_InstrAddOn* pa,const VAR* pv, XmlOutPro* out)
{
    if (pv->thevar != NULL)
    {
        Class_st* p3 = g_VarTypeManage->is_classpoint(pv->thevar->m_DataTypeID);
        if (p3 != NULL)
        {
            if (pa == NULL)
            {//Access structure, the first element
                prt_var(pv, out);
                out->prtt("->");
                out->prtt(p3->getclassitemname(0));
                return;
            }
            if (pa->type == IA_AddImmed)
            {
                prt_var(pv, out);
                out->prtt("->");
                out->prtt(p3->getclassitemname(pa->addimmed.iAddon));
                return;
            }
        }
    }
    out->prtt("*");
    prt_va_1(pa, pv, out);
}
void 	CFunc_Prt::prt_va_1(const st_InstrAddOn* pa, const VAR* pv, XmlOutPro* out)
{
    if (pa != NULL && pa->type == IA_ReadPointTo
            && pv->thevar != NULL
            && g_VarTypeManage->GetPointTo(pv->thevar->m_DataTypeID) != 0)
    {
        out_PointTo(pa->pChild, pv, out);
        return;
    }

    if (pa == NULL)
    {
        prt_var(pv, out);
        return;
    }
    switch (pa->type)
    {
    case IA_Nothing:
        prt_var(pv, out);
        break;
    case IA_ReadPointTo:
        {
            out->prtt("*");
            prt_va_1(pa->pChild,pv, out);
            break;
        }
    case IA_AddImmed:
        out->prtt("(");
        prt_va_1(pa->pChild,pv, out);
        out->prtf(" + %d)", pa->addimmed.iAddon);
        break;
    case IA_MulImmed:
        prt_va_1(pa->pChild,pv, out);
        out->prtf(" * %d", pa->addimmed.iAddon);
        break;
    case IA_GetAddress:
        out->prtt("&");
        prt_va_1(pa->pChild,pv, out);
        break;
    default:
        out->prtt("Error_PrtAddon_before");
        break;
    }
}
void 	CFunc_Prt::prt_va(const VAR_ADDON& va, XmlOutPro* out)
{
        st_InstrAddOn* pa = va.pao;
    VAR* pv = va.pv;
    prt_va_1(pa,pv,out);
    return;
}
void 	CFunc_Prt::prt_var(const VAR* var, XmlOutPro* out)
{
    if (!IfTemVar(var))
    {
        //out->XMLbegin(XT_Symbol, var->thevar);
        m_exprs->prt_var(var, out);
        //out->prtt(VarName(var));
        //out->XMLend(XT_Symbol);
        return;
    }

    //现在，确认这是一个临时变量
    //this->Get_TemVar_Name(v->temno);
    INSTR * lastcall = NULL;
    POSITION lastcallposition;

    POSITION pos = m_instr_list.begin();
    for (;pos!=m_instr_list.end();++pos)
    {
        INSTR * p = *pos;
        if (p->type == i_Call || p->type == i_CallApi)
        {
            lastcall = p;
            lastcallposition = pos;
        }

        if (IfSameTemVar(var, &p->var_w))
        {
            if (p->type == i_CallRet)
            {
                this->prt_instr_call(lastcall,out);
                break;
            }
            this->prt_the_instr_1(p, out);
            break;
        }
    }
}
void	CFunc_Prt::prt_jxx_compare_false(INSTR * &pjxx, XmlOutPro* out)
{	//	after this, pjxx really point to the JXX
    CFunc_InstrList instrl(m_instr_list);
        INSTR * p1 = pjxx;
        if (p1->type == i_Begin)
        {	// 这里可能会有一小段，比如if ((x=getx()) != 0)
                prt_compare(p1, out);
                p1 = p1->begin.m_end;
                pjxx = instrl.instr_next_in_func(p1);
        }

        if (pjxx->type != i_Jump)
        {
                alert_prtf("pjxx->type = %x",pjxx->type);
        }
        assert(pjxx->type == i_Jump);

        if (pjxx->var_r1.type)
        {
        this->prt_va(pjxx->va_r1, out);

                //cpp_prtf(" >< ");
                const char * str = " >< ";
                switch (pjxx->jmp.jmp_type)
                {
                case JMP_jnz:	str = " == ";	break;
                case JMP_jz:	str = " != ";	break;

                case JMP_ja:	str = " <= ";	break;	//unsigned
                case JMP_jb:	str = " >= ";	break;	//unsigned
                case JMP_jna:	str = " > ";	break;	//unsigned
                case JMP_jnb:	str = " < ";	break;	//unsigned

                case JMP_jg:	str = " <= ";	break;	//signed
                case JMP_jl:	str = " >= ";	break;	//signed
                case JMP_jng:	str = " > ";	break;	//signed
                case JMP_jnl:	str = " < ";	break;	//signed
                }
                out->prtt(str);
        this->prt_va(pjxx->va_r2, out);
        }
        else
                out->prtt(" ?? >< ?? ");
}

void	CFunc_Prt::prt_one_statement(const INSTR *phead, XmlOutPro* out)
{
    if (phead == NULL)
        return;
    POSITION pos = std::find(m_instr_list.begin(),m_instr_list.end(),phead);

    INSTR * begin = *pos;
    if (begin->type != i_Begin)
    {
        alert_prtf("func %x, type = %s != i_Begin",m_head_off,hlcode_name(begin->type));
        out->prtl("error statement");
        return;
    }
    assert(begin->type == i_Begin);

    POSITION endpos = std::find(m_instr_list.begin(),m_instr_list.end(),begin->begin.m_end);
    ++endpos;	//	要包括end

    while(pos != endpos)
    {
        assert(pos != m_instr_list.end());
        INSTR * p = *pos;
        ++pos;
        prt_instr(p, pos, out);
        if (p == phead && this->m_flag_prt_var_delare)
        {	//	打印过左括号后，要打印变量声明
            this->m_flag_prt_var_delare = false;
            prt_var_declares(out);
            out->endline();
        }
    }
}

void	CFunc_Prt::prt_switch_case(CasePrt_List* list, const INSTR* phead, XmlOutPro* out)
{
        out->prtl_ident("// list count = %d",list->size());
        CasePrt_List::iterator pos = list->begin();
        while (pos!=list->end())
        {
                OneCase* p = *pos;
                ++pos;
                if (pos == list->end())
                {	//	this is default
                        assert(p->case_n == 0xffffffff);
                        out->prtl_ident("default:");
                        prt_case(phead, p->thelabel, out);
                        return;
                }
                out->prtl_ident("case %d:", p->case_n);

                CasePrt_List::iterator savpos = pos;
                OneCase* pnext = *savpos;
                if (p->thelabel->label.label_off == pnext->thelabel->label.label_off)
                {	//	相同就不用打印了
                        continue;
                }

                prt_case(phead, p->thelabel, out);

        }
}

void	CFunc_Prt::prt_var_declares(XmlOutPro* out)
{	//	prt phead所指的begin_end区间中用到的变量。外层空间已经定义过的就不用了
    this->m_exprs->prt_var_declares(out);
}

void	CFunc_Prt::prt_statement_in_1_line(INSTR * &phead, XmlOutPro* out1)
{	//	',' between each state ment
    CFunc_InstrList instrl(m_instr_list);
    if (phead->type != i_Begin)
    {	//	immit
        return;
    }
    XmlPrt theprt;
    XmlOutPro out(&theprt);

    out.SetOneLine(true);

    //prt_one_statement(p);
    POSITION pos = std::find(m_instr_list.begin(),m_instr_list.end(),phead);
    ++pos;		//	skip first i_Begin
    POSITION endpos = std::find(m_instr_list.begin(),m_instr_list.end(),phead->begin.m_end);

    while ( pos != endpos)
    {
        assert(pos != m_instr_list.end());
        INSTR * p = *pos;
        ++pos;
        prt_instr(p,pos, &out);
    }

    phead = phead->begin.m_end;
    phead = instrl.instr_next_in_func(phead);

    theprt.CommaLast();
    theprt.prtprtout(out1);
}

void	CFunc_Prt::prt_jxx_compare_true(INSTR * &pjxx, XmlOutPro* out)
{	//	after this, pjxx really point to the JXX
    CFunc_InstrList instrl(m_instr_list);
    INSTR * p1 = pjxx;
    if (p1->type == i_Begin)
    {	// 这里可能会有一小段，比如if ((x=getx()) != 0)
        prt_compare(p1, out);
        p1 = p1->begin.m_end;
        pjxx = instrl.instr_next_in_func(p1);
    }

    if (pjxx->type != i_Jump)
    {
        alert_prtf("pjxx->type = %x",pjxx->type);
    }
    assert(pjxx->type == i_Jump);

    if (pjxx->var_r1.type)
    {
        this->prt_va(pjxx->va_r1, out);
        //cpp_prtf(" >< ");
        const char * str = " >< ";
        switch (pjxx->jmp.jmp_type)
        {
        case JMP_jz:	str = " == ";	break;
        case JMP_jnz:	str = " != ";	break;

        case JMP_jna:	str = " <= ";	break;	//unsigned
        case JMP_jnb:	str = " >= ";	break;	//unsigned
        case JMP_ja:	str = " > ";	break;	//unsigned
        case JMP_jb:	str = " < ";	break;	//unsigned

        case JMP_jng:	str = " <= ";	break;	//signed
        case JMP_jnl:	str = " >= ";	break;	//signed
        case JMP_jg:	str = " > ";	break;	//signed
        case JMP_jl:	str = " < ";	break;	//signed
        }
        out->prtt(str);
        this->prt_va(pjxx->va_r2, out);
    }
    else
        out->prtt(" ?? >< ?? ");
}

void	CFunc_Prt::prt_compare(const INSTR * phead, XmlOutPro* out)
// 打印while()括号中的东西。
//	特点：不换行，最后不分号
{
        // 暂时先这样
        prt_one_statement(phead, out);
}


void	CFunc_Prt::prt_one_statement_mainbody(const INSTR * phead, XmlOutPro* out)
{	//	不包括{和},用于case
    if (phead == NULL)
        return;
    POSITION pos = std::find(m_instr_list.begin(),m_instr_list.end(),phead);

    INSTR * begin = *pos;
    ++pos;
    if (begin->type != i_Begin)
    {
        alert_prtf("func %x, type = %s != i_Begin",this->m_head_off,hlcode_name(begin->type));
        out->prtl("error statement");
        return;
    }
    assert(begin->type == i_Begin);

    POSITION endpos = std::find(m_instr_list.begin(),m_instr_list.end(),begin->begin.m_end);

    while ( pos != endpos)
    {
        assert(pos != m_instr_list.end());
        INSTR * p = *pos;
        ++pos;
        prt_instr(p,pos,out);
    }
}

std::string	CFunc_Prt::prt_the_instr(const INSTR *p)
{
    XmlPrt theprt;

    XmlOutPro out(&theprt);
    out.m_f_prt_in_1line = true;
    out.m_f_prt_in_comma = true;

    prt_the_instr_1(p, &out);

    std::string retn = theprt.GetString();

    return retn;
}
void	CFunc_Prt::prt_the_instr_1(const INSTR *p, XmlOutPro* out)
{
    switch (p->type)
    {
        case i_Cmp:
                out->prtt("cmp ");
        this->prt_va(p->va_r1, out);
                out->prtt(" , ");
        this->prt_va(p->va_r2, out);
                break;

        case i_Unknown:
        out->prtt("unknown");
                break;
        case i_Return:
        out->XMLbegin(XT_Keyword, NULL);
        out->prtt("return");
        out->XMLend(XT_Keyword);
                break;
    case i_Imul:	prt_add(p, "*" , out); return;
    case i_Add:	prt_add(p, "+" , out); return;
        case i_Sub:	prt_sub(p, "-" , out); return;
        case i_And:	prt_add(p, "&" , out); return;
        case i_Sar:	prt_sub(p, ">>", out); return;
        case i_Shl:	prt_sub(p, "<<", out); return;
    case i_Shr:	prt_sub(p, ">>", out); return;
        case i_Xor:
                if (VAR::IsSame(&p->var_r1, &p->var_r2))
                {
            this->prt_var(&p->var_w, out);
            out->prtt(" = 0");	// xor eax,eax means eax = 0
            return;
                }
                prt_add(p,"^",out);
        return;
        case i_SignExpand:
        case i_NosignExpand:
    case i_Assign:
        {
            if (!IfTemVar(&p->var_w))
            {
                out->XMLbegin(XT_Symbol, p->var_w.thevar);
                out->prtt(BareVarName(&p->var_w));
                out->XMLend(XT_Symbol);
                out->prtt("= ");
            }
            this->prt_va(p->va_r1, out);

            return;
        }
    case i_Readpointto:
        {
            if (!IfTemVar(&p->var_w))
            {
                out->XMLbegin(XT_Symbol, p->var_w.thevar);
                out->prtt(BareVarName(&p->var_w));
                out->XMLend(XT_Symbol);
                out->prtt("= ");
            }
            out->prtt("*");
            this->prt_va(p->va_r1, out);

            return;
        }
    case i_Address:
        {
            if (!IfTemVar(&p->var_w))
            {
                this->prt_var(&p->var_w, out);
                out->prtt(" = ");
                this->prt_iAddress_out(p, out);
            }
            else
            {
                out->prtt("(");
                this->prt_iAddress_out(p, out);
                out->prtt(")");
            }

            return;
        }
        case i_GetAddr:
        {
            if (!IfTemVar(&p->var_w))
            {
                out->XMLbegin(XT_Symbol, p->var_w.thevar);
                out->prtt(BareVarName(&p->var_w));
                out->XMLend(XT_Symbol);
                out->prtt("= ");
            }
            out->prtt("&");
            this->prt_va(p->va_r1, out);
            return;
        }
    case i_Writepointto:
        this->out_PointTo(p->va_r1.pao, &p->var_r1, out);
                //out->prtt("*");
        //this->prt_va(p->va_r1);
                out->prtt(" = ");
        this->prt_va(p->va_r2, out);
                break;
    case i_EspReport:   //nothing
        break;
    default:
        {
            out->prtf("--XX-- %x", p->type);
            return;
        }
    }
}
void	CFunc_Prt::prt_sub(const INSTR *p, const char * s, XmlOutPro* out)
{
    if (IfTemVar(&p->var_w))
    {
        out->prtt("(");
        this->prt_va(p->va_r1, out);
        out->prtspace();
        out->prtt(s);
        out->prtspace();
        this->prt_va(p->va_r2, out);
        out->prtt(")");
        return;
    }
    this->prt_var(&p->var_w, out);

        if (VAR::IsSame(&p->var_w, &p->var_r1))
        {
        out->prtspace();
        out->prtt(s);
        out->prtt("= ");
        this->prt_va(p->va_r2, out);
        }
        else
        {
        out->prtt(" = ");
        this->prt_va(p->va_r1, out);
        out->prtt(s);
        this->prt_va(p->va_r2, out);
        }
}
const char * CallConvToName(enum_CallC ec);
void	CFunc_Prt::prt_func_head(XmlOutPro* out)
{
    FuncType* pfctype = this->Q->m_functype;
    if (pfctype != NULL)
    {
        if (pfctype->m_class != NULL && pfctype->m_class->is_GouZX(pfctype))
        {
            //Structure and the destructor is not necessary to write the return value
        }
        else
        {
            VarTypeID id = pfctype->m_retdatatype_id;
            out->XMLbegin(XT_DataType, (void *)id);
            out->prtt(GG_VarType_ID2Name(id));
            out->XMLend(XT_DataType);

            enum_CallC em = pfctype->m_callc;
            out->XMLbegin(XT_Keyword, NULL);
            out->prtt(CallConvToName(em));
            out->XMLend(XT_Keyword);
        }
    }
    else
    {
        out->XMLbegin(XT_DataType, NULL);
        out->prtt("uint32_t");
        out->XMLend(XT_DataType);
    }

    if (pfctype != NULL && pfctype->m_class != NULL)
    {
        out->XMLbegin(XT_DataType, pfctype->m_class);
        out->prtt(pfctype->m_class->getname());
        out->XMLend(XT_DataType);
        out->prtt("::");
    }
    out->XMLbegin(XT_FuncName, this->Q);
    out->prtt(this->m_funcname);
    out->XMLend(XT_FuncName);

    out->prtt("(");
    this->m_exprs->prt_parameters(out);
    out->prtl(")");
}


void	CFunc_Prt::prtout_cpp(XmlOutPro* out)
{
    if (m_instr_list.size()==0)
    {//Not yet prepared for the E2C
        CFuncLL the(this->ll.m_asmlist);
        the.prtout_asm(this->Q, &this->m_varll, out);

        return;
    }

    out->XMLbegin(XT_Function, this->Q);

    this->prt_func_head(out);

    INSTR * phead = *m_instr_list.begin();

    this->m_flag_prt_var_delare = true;
    prt_one_statement(phead, out);
    out->XMLend(XT_Function);
}


void	CFunc_Prt::prt_iAddress_out(const INSTR * p, XmlOutPro* out)
{
    int n=0;
    if (p->var_r1.type)
        {
        this->prt_va(p->va_r1, out);
        n++;
        }
        if (p->var_r2.type)
        {
                if (n)
                        out->prtt(" + ");
        this->prt_va(p->va_r2, out);
                if (p->i1 != 1)
                        out->prtf(" * %d", p->i1);
        }
        if (p->i2)
        {
                if (n)
                        out->prtspace();
        int addoff = (int)p->i2;
        if (addoff < 0)
        {
                        out->prtt("-");
            out->prtt(prt_DWORD(-addoff));
        }
        else
        {
                        out->prtt("+");
            out->prtt(prt_DWORD(p->i2));
        }
        }
}
const char * CFunc_Prt::prt_iAddress(const INSTR * p)
{
        static char s[80];
        int n=0;
        s[0] = '\0';
        if (p->var_r1.type)
        {
                n += sprintf(s,"%s",VarName(&p->var_r1));
        }
        if (p->var_r2.type)
        {
                if (n)
                        n += sprintf(s+n," + ");
                n += sprintf(s+n,"%s",VarName(&p->var_r2));
                if (p->i1 != 1)
                        n += sprintf(s+n, " * %ld", p->i1);
        }
        if (p->i2)
        {
                if (n)
                        n += sprintf(s+n," + ");
                n += sprintf(s+n, "%s", prt_DWORD(p->i2));
                n;
        }
        return s;
}

void CFunc_Prt::prt_instr_callthis(POSITION nextpos, XmlOutPro* out)
{
    POSITION pos = nextpos;
    for (;;)
    {
        INSTR * p1 = *pos;
        ++pos;
        if (p1->type == i_CallPara)
            continue;
        if (p1->type == i_CallRet)
            continue;
        if (p1->type == i_CallThis)
        {
            if (p1->va_r1.pao == 0)
            {
                this->prt_var(&p1->var_r1, out);
                out->prtt("->");
            }
            else if (p1->va_r1.pao->type == IA_GetAddress)
            {
                this->prt_var(&p1->var_r1, out);
                out->prtt(".");
            }
            else
            {
                assert(0);
            }
        }
        break;
    }
}
bool CFunc_Prt::prt_instr_callret(POSITION nextpos, XmlOutPro* out)
{
    POSITION pos = nextpos;
    for (;;)
    {
        INSTR * p1 = *pos;
        ++pos;
        if (p1->type == i_CallThis)
            continue;
        if (p1->type == i_CallPara)
            continue;
        if (p1->type == i_CallRet)
        {
            if (p1->var_w.thevar != NULL && p1->var_w.thevar->type == MTT_tem)
                return true;

            out->ident();
            this->prt_var(&p1->var_w, out);

            out->prtt(" = ");
            return false;
        }
        break;
    }
    out->ident();
    return false;
}
void CFunc_Prt::prt_para_1(M_t* thevar, XmlOutPro* out)
{
    //this is for __stdcall and __cdecl
    int n = 0;
    INSTR_LIST::reverse_iterator pos=m_instr_list.rbegin();
    while (pos!=m_instr_list.rend())
    {
        INSTR * p = *pos;
        ++pos;
        if (p->var_w.thevar == thevar)
        {
            if (n != 0)
            {
                out->nospace();
                out->prtt(",");
            }

            if (p->type == i_GetAddr)
                out->prtt("&");
            this->prt_va(p->va_r1, out);
            n++;
        }
    }
    //This function is somehow wrong there is something wrong with processing order
}
void CFunc_Prt::prt_parameter(const INSTR * ppara, XmlOutPro* out)
{
    if (ppara->va_r1.pao != NULL || ppara->var_r1.thevar == NULL)
    {
        this->prt_va(ppara->va_r1, out);
        return;
    }
    if (ppara->var_r1.thevar->m_DataTypeID != 0
            && GG_VarType_ID2Size(ppara->var_r1.thevar->m_DataTypeID) <= 4)
    {
        this->prt_va(ppara->va_r1, out);
        return;
    }
    this->prt_para_1(ppara->var_r1.thevar, out);
}
#undef m_funcname
void CFunc_Prt::prt_instr_call(const INSTR * p, XmlOutPro* out)
{
    if (p->type == i_CallApi)
    {
        out->XMLbegin(XT_FuncName, p->call.papi);
        out->prtt(p->call.papi->name);
    }
    else
    {
        out->XMLbegin(XT_FuncName, p->call.call_func);
        out->prtt(p->call.call_func->m_funcname);
    }
    out->XMLend(XT_FuncName);
    out->prtt("(");

        if (p->call.p_callpara)
    {
                this->prt_parameter(p->call.p_callpara, out);
    }
    out->prtt(")");
}
void	CFunc_Prt::prt_instr(const INSTR * p, POSITION &nextpos, XmlOutPro* out)
{
    CFunc_InstrList instrl(m_instr_list);

    g_PrtOut.SetHline(p);
    if (p->var_w.type == v_Tem)
        nop();
    switch (p->type)
    {
    case i_CplxBegin:
        {
            //char buf[140];
            //prt_partern(this->m_instr_list,p,buf);
            //prtl_ident("// complex partern = %s",buf);
            switch (p->begin.type)
            {
            case COMP_if:
                {
                    out->ident();
                    out->XMLbegin(XT_Keyword, NULL);
                    out->prtt("if");
                    out->XMLend(XT_Keyword);
                    out->prtt("(");
                    INSTR * p1 = instrl.instr_next_in_func(p);
                    prt_jxx_compare_false(p1, out);
                    out->prtl(")");
                    p1 = instrl.instr_next_in_func(p1);
                    prt_one_statement(p1, out);
                }
                break;
            case COMP_long_if:
                {
                    out->ident();
                    out->XMLbegin(XT_Keyword, NULL);
                    out->prtt("if");
                    out->XMLend(XT_Keyword);
                    out->prtt("(");
                    INSTR * p1 = instrl.instr_next_in_func(p);
                    prt_jxx_compare_true(p1, out);
                    out->prtl(")");
                    p1 = instrl.instr_next_in_func(p1);	//	skip the jxx
                    p1 = instrl.instr_next_in_func(p1);	//	skip the jmp
                    p1 = instrl.instr_next_in_func(p1);	//	skip the label
                    prt_one_statement(p1, out);
                }
                break;
            case COMP_if_else:
                {	// = "0_jxx1_0_jmp2_from1_0_from2_";
                    out->ident();
                    out->XMLbegin(XT_Keyword, NULL);
                    out->prtt("if");
                    out->XMLend(XT_Keyword);
                    out->prtt("(");
                    INSTR * p1 = instrl.instr_next_in_func(p);
                    prt_jxx_compare_false(p1, out);
                    out->prtl(")");

                    p1 = instrl.instr_next_in_func(p1);
                    prt_one_statement(p1, out);

                    out->ident();
                    out->XMLbegin(XT_Keyword, NULL);
                    out->prtt("else");
                    out->XMLend(XT_Keyword);
                    out->endline();

                    p1 = instrl.instr_next_in_func(p1->begin.m_end);	//jmp2
                    p1 = instrl.instr_next_in_func(p1);				//from1
                    p1 = instrl.instr_next_in_func(p1);				//the statement

                    prt_one_statement(p1, out);
                }
                break;
            case COMP_while:
                {
                    out->ident();
                    out->XMLbegin(XT_Keyword, NULL);
                    out->prtt("while");
                    out->XMLend(XT_Keyword);
                    out->prtt("(");

                    INSTR * p1 = instrl.instr_next_in_func(p);
                    p1 = instrl.instr_next_in_func(p1);	//	skip the first, its label
                    prt_jxx_compare_false(p1, out);
                    out->prtl(")");
                    //prtl_ident("//while begin");
                    p1 = instrl.instr_next_in_func(p1);
                    prt_one_statement(p1, out);
                    //prtl_ident("//while end");
                }
                break;
            case COMP_do_while:
                {
                    out->ident();
                    out->XMLbegin(XT_Keyword, NULL);
                    out->prtt("do");
                    out->XMLend(XT_Keyword);
                    out->endline();

                    INSTR * p1 = instrl.instr_next_in_func(p);
                    p1 = instrl.instr_next_in_func(p1);	//	skip the first, its label
                    prt_one_statement(p1, out);

                    out->ident();
                    out->XMLbegin(XT_Keyword, NULL);
                    out->prtt("while");
                    out->XMLend(XT_Keyword);
                    out->prtt("(");

                    p1 = instrl.instr_next_in_func(p1->begin.m_end);
                    prt_jxx_compare_true(p1, out);
                    out->prtt(")");
                    out->EOL();
                }
                break;
            case COMP_for1:
                {
                    out->ident();
                    out->XMLbegin(XT_Keyword, NULL);
                    out->prtt("for");
                    out->XMLend(XT_Keyword);
                    out->prtt("(");

                    INSTR * p1 = instrl.instr_next_in_func(p);

                    prt_statement_in_1_line(p1, out);
                    assert(p1->type == i_Label);
                    INSTR * p2 = instrl.instr_next_in_func(p1);
                    p1 = p1->label.ref_instr;
                    prt_jxx_compare_true(p1, out);
                    out->prtt("; ");
                    {
                        INSTR * p3 = instrl.skip_compl(p2);
                        prt_statement_in_1_line(p3, out);
                        out->prtl(")");
                    }
                    prt_one_statement(p2, out);			//	the main body
                    break;
                }
            case COMP_for:
                //cpp_prtl("//for find");
                //break;
                {	//	finger_for[] = "0_jmp1_from2_0_from1_0_jxx3_0_jmp2_from3_";
                    out->ident();
                    out->XMLbegin(XT_Keyword, NULL);
                    out->prtt("for");
                    out->XMLend(XT_Keyword);
                    out->prtt("(");

                    INSTR * p1 = instrl.instr_next_in_func(p);

                    prt_statement_in_1_line(p1, out);
                    if (p1->type != i_Jump)
                    {
                        alert_prtf("type is %s",hlcode_name(p1->type));
                    }
                    assert(p1->type == i_Jump);
                    out->prtt("; ");

                    INSTR * p2 = instrl.instr_next_in_func(p1);
                    p2 = instrl.instr_next_in_func(p2);	//	skip the label

                    assert(p1->type == i_Jump);
                    p1 = p1->jmp.the_label;
                    p1 = instrl.instr_next_in_func(p1);	//	skip the label
                    prt_jxx_compare_false(p1, out);
                    out->prtt("; ");

                    prt_statement_in_1_line(p2, out);
                    out->prtl(")");


                    assert(p1->type == i_Jump);
                    p1 = instrl.instr_next_in_func(p1);	//	skip this jxx
                    prt_one_statement(p1, out);			//	the main body
                }
                break;
            case COMP_switch_case:
                {	//	小心了，swith_case的显示很困难的
                    out->prtf_ident("switch (");
                    INSTR * p1 = instrl.instr_next_in_func(p);
                    assert(p1->type == i_Jump);		//	开头肯定是个条件跳
                    //while (p1->type != i_JmpAddr)
                    //{
                    //	p1 = instrl.instr_next_in_func(p1);
                    //}
                    this->prt_va(p1->va_r1, out);
                    out->prtl(")");
                    out->prtl_ident("{");
                    CasePrt_List lstt;
                    while (p1->type != i_Jump || p1->jmp.jmp_type != JMP_case)
                    {
                        p1 = instrl.instr_next_in_func(p1);
                    }
                    int n = 0;
                    while (p1->type == i_Jump && p1->jmp.jmp_type == JMP_case)
                    {
                        //prtl_ident("case %d:",n++);
                        //prt_case(p,p1->jmp.the_label);
                        Add_case_entry(&lstt, n++, p1->jmp.the_label);
                        p1 = instrl.instr_next_in_func(p1);
                    }
                    p1 = instrl.instr_next_in_func(p);
                    //prtl_ident("default:");
                    //prt_case(p,p1->jmp.the_label);
                    add_default_entry(&lstt, p1->jmp.the_label);
                    prt_switch_case(&lstt,p,out);
                    out->prtl_ident("}");
                }
                break;
            case COMP_switch_case_multcomp:
                {	//	小心了，swith_case的显示很困难的
                    out->prtf_ident("switch (");
                    INSTR * p1 = instrl.instr_next_in_func(p);
                    assert(p1->type == i_Jump);		//	开头肯定是个条件跳
                    //while (p1->type != i_JmpAddr)
                    //{
                    //	p1 = instrl.instr_next_in_func(p1);
                    //}
                    this->prt_va(p1->va_r1, out);
                    out->prtl(")");
                    out->prtl_ident("{");
                    //	这里还有一点小麻烦，不能按case的次序打印，而要按实际处理地址的次序打印
                    //	才能解决case延续和default问题
                    CasePrt_List lstt;
                    while (p1->type == i_Jump && p1->jmp.jmp_type == JMP_jz)
                    {
                        assert(p1->var_r2.type == v_Immed);
                        Add_case_entry(&lstt, p1->var_r2.d, p1->jmp.the_label);
                        //prtl_ident("case %d:", p1->var_r2.d);
                        //prt_case(p,p1->jmp.the_label);
                        p1 = instrl.instr_next_in_func(p1);
                    }
                    if (p1->type == i_Jump && p1->jmp.jmp_type == JMP_jmp)
                    {
                        //prtl_ident("default:");
                        //prt_case(p,p1->jmp.the_label);
                        add_default_entry(&lstt, p1->jmp.the_label);
                    }
                    else
                    {
                        add_default_entry(&lstt, p1);
                        //error("default error");
                    }
                    prt_switch_case(&lstt,p,out);
                    out->prtl_ident("}");
                }
                break;
            default:
                out->prtl("//unknow cmplx statement");
                break;
            }
            //if (p->begin.type != COMP_unknown && p->begin.type != COMP_for)
            //if (p->begin.type != COMP_unknown && p->begin.type != COMP_switch_case)
            if (p->begin.type != COMP_unknown)
            {
                nextpos = std::find(m_instr_list.begin(),m_instr_list.end(),p->begin.m_end);
                ++nextpos;
            }
            else
            {
                out->prtl_ident("{");
                out->ident_add1();
            }
        }
        break;

    case i_Begin:
        out->prtl_ident("{");
        out->ident_add1();
        break;
    case i_CplxEnd:
    case i_End:
        out->ident_sub1();
        out->prtl_ident("}");
        break;
    case i_Label:
        out->XMLbegin(XT_AsmLabel, (void*)p->label.label_off);
        out->prtf("L_%08x", p->label.label_off);
        out->XMLend(XT_AsmLabel);
        out->prtl(":");
        break;
    case i_Jump:
        if (p->jmp.jmp_type == JMP_jmp)
        {
            const INSTR *p1 = p;
            while (p1->type != i_Begin && p1->type != i_CplxBegin)
            {
                p1 = instrl.instr_prev_in_func(p1);
            }
            if (p->jmp.the_label == p1->begin.m_break)
            {
                out->prtf_ident("break");
                out->EOL();
            }
            else if (p->jmp.the_label == p1->begin.m_conti
                     && p != p1->begin.m_not_conti)
            {
                out->prtf_ident("continue");
                out->EOL();
            }
            else
            {
                out->prtf_ident("JMP ");
                out->XMLbegin(XT_AsmLabel, (void*)p->jmp.jmpto_off);
                out->prtf("L_%08x", p->jmp.jmpto_off);
                out->XMLend(XT_AsmLabel);
                out->EOL();
            }
        }
        else
        {
            out->prtf_ident("Jxx ");
            out->XMLbegin(XT_AsmLabel, (void*)p->jmp.jmpto_off);
            out->prtf("L_%08x", p->jmp.jmpto_off);
            out->XMLend(XT_AsmLabel);
            if (p->var_r1.thevar != NULL)
            {
                out->prtt("(");
                this->prt_va(p->va_r1, out);
                out->prtt(" ");
                this->prt_va(p->va_r2, out);
                out->prtt(")");
            }
            out->EOL();
        }
        break;

#undef m_funcname
    case i_CallApi:
    case i_Call:
        if (prt_instr_callret(nextpos, out))
            break;  //这是因为i_CallRet是一个tem
        prt_instr_callthis(nextpos, out);
        prt_instr_call(p, out);
        out->EOL();

                break;
    case i_CallPara:
    case i_CallThis:
    case i_CallRet:
                break;
        case i_RetPar:
                {
                    out->ident();
                    out->XMLbegin(XT_Keyword, NULL);
                    out->prtt("return");
                    out->XMLend(XT_Keyword);
                    this->prt_va(p->va_r1, out);
                    POSITION pos = nextpos;
                    bool ffirst = true;
                    for (;;)
                    {
                        INSTR * p1 = *pos;
                        ++pos;
                        if (p1->type == i_RetPar)
                        {
                            //alert("prt here");
                            nextpos = pos;
                            if (! ffirst)
                            {
                                out->nospace();
                                out->prtt(",");
                            }
                            this->prt_va(p1->va_r1, out);
                            ffirst = false;
                        }
                        else if (p1->type == i_Return)
                        {
                            nextpos = pos;
                            break;
                        }
                        else
                            break;
                    }
                    out->EOL();
                }
                break;
    case i_EspReport:   //nothing
        break;
    default:

        if (!IfTemVar(&p->var_w))
        {
            out->ident(); prt_the_instr_1(p, out); out->EOL();
        }
        break;
        }
}
//---------------------

CPrtOut::CPrtOut()
{
    hline = NULL;
    b_Indent = true;
    b_Endl = true;
    b_OneLine = false;
    strcpy(m_buf.strbuf, "");
    m_buf.linesyntax[0].pos = -1;
}



