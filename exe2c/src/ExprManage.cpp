// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

////#include "stdafx.h"
#include <cstdio>
#include	"CISC.h"


ExprManage::ExprManage()
{
    vList = new MLIST;
}

ExprManage::~ExprManage()
{
    delete vList;
}


void ExprManage::DeleteUnuse_VarList(MLIST* vlist)
{
    MLIST::iterator pos = vlist->begin();
    while (pos!=vlist->end())
    {
        if ((*pos)->tem_useno != 0)
        {
            ++pos;
            continue;
        }
        pos=vlist->erase(pos);
    }
}


uint32_t	regindex_2_regoff(uint32_t regindex)
{
    //	把reg也批定一个offset，用struct的概念来处理al,ax的问题
    // Set the reg also approved an offset, the concept of using struct to deal with al, ax the question
    switch (regindex)
    {
    case _EAX_:	return enum_EAX;
    case _ECX_:	return enum_ECX;
    case _EDX_:	return enum_EDX;
    case _EBX_:	return enum_EBX;
    case _ESP_:	return enum_ESP;
    case _EBP_:	return enum_EBP;
    case _ESI_:	return enum_ESI;
    case _EDI_:	return enum_EDI;
    }

    switch (regindex)
    {
    case _AX_:	return enum_AX;
    case _CX_:	return enum_CX;
    case _DX_:	return enum_DX;
    case _BX_:	return enum_BX;
    case _SP_:	return enum_SP;
    case _BP_:	return enum_BP;
    case _SI_:	return enum_SI;
    case _DI_:	return enum_DI;
    }

    switch (regindex)
    {
    case _AH_:	return enum_AH;
    case _CH_:	return enum_CH;
    case _DH_:	return enum_DH;
    case _BH_:	return enum_BH;

    case _AL_:	return enum_AL;
    case _CL_:	return enum_CL;
    case _DL_:	return enum_DL;
    case _BL_:	return enum_BL;
    }

    return regindex;
}

const char * RegName(uint32_t off, BYTE opsize)
{
    static char s[20];
    switch (opsize)
    {
    case 4:
        switch (off)
        {
        case enum_EAX:	return("EAX");
        case enum_ECX:	return("ECX");
        case enum_EBX:	return("EBX");
        case enum_EDX:	return("EDX");
        case enum_EBP:	return("EBP");
        case enum_ESP:	return("ESP");
        case enum_ESI:	return("ESI");
        case enum_EDI:	return("EDI");
        }
        break;
    case 2:
        switch (off)
        {
        case enum_AX:	return("AX");
        case enum_CX:	return("CX");
        case enum_BX:	return("BX");
        case enum_DX:	return("DX");
        case enum_BP:	return("BP");
        case enum_SP:	return("SP");
        case enum_SI:	return("SI");
        case enum_DI:	return("DI");
        }
        break;
    case 1:
        switch (off)
        {
        case enum_AH:	return("AH");
        case enum_CH:	return("CH");
        case enum_BH:	return("BH");
        case enum_DH:	return("DH");

        case enum_AL:	return("AL");
        case enum_CL:	return("CL");
        case enum_BL:	return("BL");
        case enum_DL:	return("DL");
        }
        break;
    }
    sprintf(s,"r_%x_%x", opsize, off);
    return s;
}
bool M_t::AnyOverlay(M_t* p)
{
    uint32_t d = p->s_off;
    {
        if (d >= this->s_off && d < this->end_off())
            return true;
    }
    d = p->end_off();
    {
        if (d > this->s_off && d < this->end_off())
            return true;
    }
    return false;
}
void M_t::Expand(M_t* p)
{
    if (this->s_off > p->s_off)
    {
        this->size += this->s_off - p->s_off;
        this->s_off = p->s_off;
    }
    if (this->end_off() < p->end_off())
    {
        this->size += p->end_off() - this->end_off();
    }
    this->m_DataTypeID = g_VarTypeManage->NewSimpleVarType(this->size);
}


const char * ExprManage::BareVarName(const VAR* v)
{
    if (v->type == v_Tem)
    {//That is, do not replace
        const char * retn = v->thevar->GetName();
        //IsBadReadPtr(retn,1);
        return retn;
    }
    const char * retn = VarName(v);
    //IsBadReadPtr(retn,1);
    return retn;
}
M_t* ExprManage::GetVarByName_1(MLIST* list, const char * varname)
//SuperC_func: Use only in <CExprManage::GetVarByName>
{
    MLIST::iterator pos = list->begin();
    for (;pos!=list->end(); ++pos)
    {
        M_t* p = *pos;
        if (strcmp(varname, p->namestr) == 0)
            return p;
    }
    return NULL;
}
//当前，仅支持par,var,reg三种类型
//Currently, only support par, var, reg three types of
M_t* ExprManage::GetVarByName(const char * varname)
{
    return this->GetVarByName_1(this->vList, varname);
}
const char * GetBeautyImmedValue(uint32_t n)
{
    static char s[80];
    if (n < 16)
        sprintf(s,"%d",n);
    else if (n < 100 && (n % 10) == 0)
        sprintf(s,"%d", n);
    else if (n >= 100 && (n % 100) == 0)
        sprintf(s,"%d", n);
    else
        sprintf(s,"0x%x", n);
    return s;
}


void Cstr_fmt(char * dst, const char * src)
{
    int n = 0;
    dst[n++] = '\"';

    char c;
    while ((c = *src++) != '\0')
    {
        switch (c)
        {
        case '\n':
            dst[n++] = '\\';
            dst[n++] = 'n';
            break;
        case '\t':
            dst[n++] = '\\';
            dst[n++] = 't';
            break;
        default:
            dst[n++] = c;
            break;
        }
        if (n > 240)
        {
            dst[n++] = '.';
            dst[n++] = '.';
            dst[n++] = '.';
            break;
        }
    }
    dst[n++] = '\"';
    dst[n] = '\0';
}

void 	ExprManage::prt_var_Immed(const VAR* v, XmlOutPro* out)
{
    if (v->thevar == NULL || g_VarTypeManage->is_simple(v->thevar->m_DataTypeID))
    {
        out->XMLbegin(XT_Number, NULL);
        out->prtt(GetBeautyImmedValue(v->d));
        out->XMLend(XT_Number);
        return;
    }
    VarTypeID id1 = v->thevar->m_DataTypeID;
    assert(id1 != 0);
    static char buf[180];

    VarTypeID id_p = g_VarTypeManage->GetPointTo(id1);
    if ( id_p != 0 && g_VarTypeManage->If_Based_on_idid(id_p,id_char))
    {
        static char strbuf[180];
        if ( ! if_valid_ea((ea_t)v->d))	//	奇怪
        {
            //sprintf(strbuf,"(WHY_PSTR)0x%x",v->d);
            //return strbuf;
            out->prtt("(");
            out->XMLbegin(XT_DataType, (void *)id1);
            out->prtt(GG_VarType_ID2Name(id1));
            out->XMLend(XT_DataType);
            out->nospace();
            out->prtt(")");
            out->XMLbegin(XT_Number, NULL);
            out->prtf("0x%x", v->d);
            out->XMLend(XT_Number);
            return;
        }
        const char * pstr = (const char *)ea2ptr((ea_t)v->d);
        Cstr_fmt(strbuf,pstr);
        //return strbuf;
        out->XMLbegin(XT_Number, NULL);
        out->prtt(strbuf);
        out->XMLend(XT_Number);
        return;
    }

    out->prtt("(");
    out->XMLbegin(XT_DataType, (void *)id1);
    out->prtt(GG_VarType_ID2Name(id1));
    out->XMLend(XT_DataType);
    out->nospace();
    out->prtt(")");

    out->XMLbegin(XT_Number, NULL);
    //sprintf(buf,"(%s)", GG_VarType_ID2Name(v->thevar->m_DataTypeID));
    if (v->thevar->immed.d == 0 && g_VarTypeManage->GetPointTo(v->thevar->m_DataTypeID) != 0)
    {
        out->prtt("NULL");
    }
    else
        out->prtt(GetBeautyImmedValue(v->d));

    out->XMLend(XT_Number);
}
const char * ExprManage::VarName_Immed(const VAR* v)
{
    if (v->thevar == NULL || g_VarTypeManage->is_simple(v->thevar->m_DataTypeID))
        return GetBeautyImmedValue(v->d);
    VarTypeID id1 = v->thevar->m_DataTypeID;
    assert(id1 != 0);
    static char buf[180];

    VarTypeID id_p = g_VarTypeManage->GetPointTo(id1);
    if ( id_p != 0 && g_VarTypeManage->If_Based_on_idid(id_p,id_char))
    {
        static char strbuf[180];
        if ( ! if_valid_ea((ea_t)v->d))	//	奇怪
        {
            sprintf(strbuf,"(WHY_PSTR)0x%x",v->d);
            return strbuf;
        }
        const char * pstr = (const char *)ea2ptr((ea_t)v->d);
        Cstr_fmt(strbuf,pstr);
        return strbuf;
    }

    sprintf(buf,"(%s)", GG_VarType_ID2Name(v->thevar->m_DataTypeID).c_str());
    if (v->thevar->immed.d == 0 && g_VarTypeManage->GetPointTo(v->thevar->m_DataTypeID) != 0)
    {
        strcat(buf, "NULL");
    }
    else
        strcat(buf, GetBeautyImmedValue(v->d));
    return buf;
}
const char * ExprManage::VarName(const VAR* v)
{
    assert(v->type != v_Tem);

    static char s[20];
    M_t* p;
    switch (v->type)
    {
    case v_Global:
    case v_Reg:
    case v_Var:
    case v_Par:
        //p = this->LookUpVar(v);
        p = v->thevar;
        if (p == NULL)
            return "WHAT";
        assert(p);
        if (p->size == v->opsize)
        {
            if (p->m_DataTypeID != 0)
            {
                UINT typesize = ::GG_VarType_ID2Size(p->m_DataTypeID);
                if (typesize == p->size)
                    nop();
            }
            return p->namestr;
        }
        if (v->part_flag != 0 && g_VarTypeManage->is_class(p->m_DataTypeID) != NULL)
        {
            static char buf[180];
            sprintf(buf, "%s.%s", p->namestr,
                    g_VarTypeManage->is_class(p->m_DataTypeID)->getclassitemname(v->part_flag - 1));
            return buf;
        }
        else
        {
            static char buf[180];
            sprintf(buf, "%s.part", p->namestr);
            return buf;
        }
    case v_Immed:
        return VarName_Immed(v);
    case v_Volatile:
        return "FS:[00]";
    default:
        sprintf(s,"?%d?",v->type);
        return s;
    }
}


void 	ExprManage::prt_var(const VAR* v, XmlOutPro* out)
{
    if (v->type == v_Immed)
    {
        prt_var_Immed(v, out);
        return;
    }
    out->XMLbegin(XT_Symbol, v->thevar);
    out->prtt(VarName(v));
    out->XMLend(XT_Symbol);
}
void ExprManage::prt_var_declares(XmlOutPro* out)
{
    MLIST* list = this->vList;
    MLIST::iterator pos = list->begin();
    for (;pos!=list->end();++pos)
    {
        M_t* p = *pos;
        if (p->type != MTT_reg)
            continue;

        out->prtspace(4);

        out->XMLbegin(XT_DataType, p);
        out->prtt(GG_VarType_ID2Name(p->m_DataTypeID));
        out->XMLend(XT_DataType);

        out->XMLbegin(XT_Symbol, p);
        out->prtt(p->namestr);
        out->XMLend(XT_Symbol);

        if (GG_id2_VarType(p->m_DataTypeID)->type == vtt_array)
        {
            out->prtt("[");
            out->prtf("%d", GG_id2_VarType(p->m_DataTypeID)->m_array.arraynum);
            out->prtt("]");
        }
        out->prtt(";");
        out->endline();
    }

    pos = list->begin();
    for (;pos!=list->end();++pos)
    {
        M_t* p = *pos;
        if (p->type != MTT_var)
            continue;
        if (p->bTem)
            continue;

        out->prtspace(4);

        out->XMLbegin(XT_DataType, p);
        out->prtt(GG_VarType_ID2Name(p->m_DataTypeID));
        out->XMLend(XT_DataType);

        out->prtspace();
        out->XMLbegin(XT_Symbol, p);
        out->prtt(p->namestr);
        out->XMLend(XT_Symbol);
        if (GG_id2_VarType(p->m_DataTypeID)->type == vtt_array)
        {
            out->prtt("[");
            out->prtf("%d", GG_id2_VarType(p->m_DataTypeID)->m_array.arraynum);
            out->prtt("]");
        }
        out->prtt(";");
        out->endline();

    }
}
void ExprManage::prt_parameters(XmlOutPro* out)
{
    bool first = true;

    assert(this);
    MLIST* list = this->vList;
    assert(list);
    MLIST::iterator pos = list->begin();
    for (;pos!=list->end();++pos)
    {
        M_t* p = *pos;
        if (p->type != MTT_par)
            continue;

        if (first)
            first = false;
        else
        {
            out->nospace();
            out->prtt(", ");
        }

        out->XMLbegin(XT_DataType, (void *)p->m_DataTypeID);
        out->prtt(GG_VarType_ID2Name(p->m_DataTypeID));
        out->XMLend(XT_DataType);

        out->XMLbegin(XT_Symbol, p);
        out->prtt(p->namestr);
        out->XMLend(XT_Symbol);
    }
}

M_t* ExprManage::AddRef_immed(uint32_t d, uint32_t size)
{
    M_t* pnew = new M_t;    //new_M_t
    pnew->type = MTT_immed;
    pnew->immed.d = d;
    pnew->size = size;
    pnew->m_DataTypeID = g_VarTypeManage->NewSimpleVarType(pnew->size);

    vList->push_back(pnew);

    return pnew;
}
M_t* ExprManage::AddRef_tem(uint32_t temno, uint32_t size)
{
    MLIST::iterator pos = vList->begin();
    for (;pos!=vList->end();++pos)
    {
        M_t* p = *pos;
        if (p->type == MTT_tem && p->tem.temno == temno)
        {
            return p;
        }
    }
    //没找到，就new一个
    M_t* pnew = new M_t;    //new_M_t
    pnew->type = MTT_tem;
    pnew->tem.temno = temno;
    sprintf(pnew->namestr, "t_%x", temno);
    pnew->size = size;
    pnew->m_DataTypeID = g_VarTypeManage->NewSimpleVarType(pnew->size);

    vList->push_back(pnew);

    return pnew;
}
void Replace_Var(INSTR_LIST* instr_list, M_t* pvar, M_t* thevar);
//SuperC_func: Used only in ＜CExprManage::AddRef>
M_t* ExprManage::AddRef_with_name(en_MTTYPE type, uint32_t off, uint32_t size, const char * tj_name)
{
    assert(type != MTT_tem);

    // First look for is not already have
    {//先找一找是不是已经有了
        MLIST::iterator pos = vList->begin();
        for (;pos!=vList->end();++pos)
        {
            M_t* p = *pos;
            if (p->type == type && p->s_off == off && p->size == size)
            {
                if (!p->iThrowned)
                    return p;
            }
        }
    }

    M_t* pnew = new M_t;    //new_M_t
    pnew->type = type;
    pnew->s_off = off;
    pnew->size = size;
    pnew->m_DataTypeID = g_VarTypeManage->NewSimpleVarType(pnew->size);

    vList->push_back(pnew);

    this->Enlarge_Var(pnew, ::g_Cur_Func->m_instr_list);

    strcpy(pnew->namestr, tj_name);

    return pnew;
}

signed int varoff2stack(UINT off);
void ExprManage::EspReport(signed int esplevel)
{
    static int static_iThrown = 1;
    static_iThrown++;

    MLIST::iterator pos = vList->begin();
    for (;pos!=vList->end();++pos)
    {
        M_t* p = *pos;
        if (p->type != MTT_var)
            continue;

        if (esplevel > varoff2stack(p->s_off))
        {
            p->iThrowned = static_iThrown;    //扔掉
        }
    }
}


M_t* ExprManage::SearchMT(en_MTTYPE type, uint32_t off)
{
    MLIST::iterator pos = vList->begin();
    for (;pos!=vList->end();++pos)
    {
        M_t* p = *pos;
        if (p->type == type && p->s_off == off)
            return p;
    }
    return NULL;
}

ExprManage g_GlobalExpr;

void ExprManage::AddRef(VAR* pvar)
{   // Tell soon as CExprManage, this var (belongs to?) someone
    // Ensure that all var will have a thevar
    switch (pvar->type)
    {
    case v_Invalid: return;
    case v_Immed:
        if (pvar->d == 4)
        {
            pvar->d = 4;
        }
        pvar->thevar = this->AddRef_immed(pvar->d, pvar->opsize);
        return;
    case v_Global:
    {
        char name[20];
        sprintf(name,"g_%x",pvar->off);
        pvar->thevar = g_GlobalExpr.AddRef_with_name(MTT_global, pvar->off, pvar->opsize, name);
        return;
    }
    case v_Volatile:
    case v_Tem:
    {
        pvar->thevar = this->AddRef_tem(pvar->temno, pvar->opsize);
        pvar->thevar->bTem = true;
        return;
    }
    case v_Var:
    {
        this->m_VarRange_H;
        this->m_VarRange_L;
        char name[20];
        if (0x7ffff - (signed int)pvar->var_off + 1 > -this->m_VarRange_L)
        {
            sprintf(name,"tem_%x",0x7ffff - pvar->var_off + 1);	//0x7ffff - p->s_off + 1
            pvar->thevar = this->AddRef_with_name(MTT_var, pvar->var_off, pvar->opsize, name);
            //pvar->thevar = this->CreateNewTemVar(pvar->opsize);
            pvar->thevar->bTem = true;
        }
        else
        {
            sprintf(name,"v_%x",0x7ffff - pvar->var_off + 1);	//0x7ffff - p->s_off + 1
            pvar->thevar = this->AddRef_with_name(MTT_var, pvar->var_off, pvar->opsize, name);
        }
    }
        break;
    case v_Par:
    {
        char name[20];
        sprintf(name,"a_%x",pvar->par_off);
        pvar->thevar = this->AddRef_with_name(MTT_par, pvar->par_off, pvar->opsize, name);
    }
        break;
    case v_Reg:
    {
        const char * pname = RegName(pvar->reg, pvar->opsize);
        pvar->thevar = this->AddRef_with_name(MTT_reg, pvar->reg, pvar->opsize, pname);
    }
        break;
    default:
        assert(0);
    }
}
uint32_t	stack2varoff(int32_t stackoff)
{
    assert(stackoff < 0);
    uint32_t off = (uint32_t)stackoff;		//	BYTE 转为 unsiuint32_tBYTE to unsiuint32_t
    //TODO: Why does this have influence over recognizing printf's arguments in petest ???
    off &= 0x7ffff; // Enough, right?
    return off;
}
signed int varoff2stack(UINT off)
{
    return -(0x7ffff - (signed int)off + 1);
}


int g_newtemno = 737;
//每次使用都加2，这样就把偶数的值留出来了
//Add 2 each time you use are so put out even the value of the left
M_t* ExprManage::CreateNewTemVar(UINT size)
{
    M_t* pnew = new M_t;
    pnew->type = MTT_tem;
    pnew->size = size;

    pnew->tem.temno = g_newtemno;
    g_newtemno += 2;
    sprintf(pnew->namestr, "tem_%x", pnew->tem.temno);
    pnew->m_DataTypeID = g_VarTypeManage->NewSimpleVarType(pnew->size);

    vList->push_back(pnew);
    return pnew;
}

void Replace_Var_1(VAR* var, M_t* pvar, M_t* thevar)
{
    if (var->thevar != pvar)
        return;

    var->thevar = thevar;
    if (var->opsize != thevar->size)
    {
        switch (var->type)
        {
        case v_Var:
            var->part_flag = 1+var->var_off-thevar->s_off;
            break;
        default:
            var->part_flag = 1;
            break;
        }
    }
}
void Replace_Var(INSTR_LIST& instr_list, M_t* pvar, M_t* thevar)
{
    INSTR_LIST::iterator pos = instr_list.begin();
    for (;pos!=instr_list.end();++pos)
    {
        PINSTR p = *pos;
        Replace_Var_1(&p->var_w,pvar,thevar);
        Replace_Var_1(&p->var_r1,pvar,thevar);
        Replace_Var_1(&p->var_r2,pvar,thevar);
    }
}
void ExprManage::Enlarge_Var(M_t* thevar, INSTR_LIST& instr_list)
{
    //下面，要删被我占了的var
    //The following should be deleted by my account of the var

    MLIST*	list = vList;
    MLIST::iterator pos = list->begin();
    while (pos!=list->end())
    {
        MLIST::iterator savpos = pos;
        M_t* p = *pos;
        ++pos;
        if (p == thevar)
        {
            Replace_Var(instr_list, p, thevar); //replace myself
            continue;
        }
        if (p->type != thevar->type)
            continue;
        if (p->iThrowned != 0 && p->iThrowned != thevar->iThrowned)
            continue;

        if (thevar->IfInclude(p->s_off) && thevar->IfInclude(p->s_off + p->size))
        {
            list->erase(savpos);
            // Just delete does not work, but also update instrlist
            Replace_Var(instr_list, p, thevar);
        }
    }
}

void ExprManage::ClearUse()
{
    MLIST::iterator pos = vList->begin();
    while (pos!=vList->end())
    {
        MLIST::iterator savpos = pos;
        M_t* p = *pos;
        ++pos;
        p->tem_useno = 0;
    }
}
