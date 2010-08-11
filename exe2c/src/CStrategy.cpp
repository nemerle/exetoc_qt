// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//#include "stdafx.h"
#include "CStrategy.h"


CStrategy g_CStrategy;


bool CStrategy::DoIt_Addon(INSTR_LIST& list, CExprManage* expr)
{
    assert(m_es == ES_Instr_can_Elim_21E);
    M_t* v = this->m_can_elim.pvar;
    PINSTR p1 = this->m_can_elim.p1;
    PINSTR p2 = this->m_can_elim.p2;

    return false;
}
void AddOn_serial(Pst_InstrAddOn &p1, st_InstrAddOn* p2)
{
    Pst_InstrAddOn* pp = &p1;
    while (*pp != NULL)
    {
        pp = &p1->pChild;
    }
    *pp = p2;
}
void CStrategy::DoIt(INSTR_LIST& list, CExprManage* expr)
{
    switch (m_es)
    {
    case ES_Instr_can_Delete:
        {
            INSTR_LIST::iterator remove_val=
                    std::find(list.begin(),
                              list.end(),
                              can_delete.pinstr);
            assert(remove_val!=list.end());
            list.erase(remove_val);
        }
        break;
    case ES_Instr_can_Elim_21E:
        {
            if (this->DoIt_Addon(list,expr))
                break;

            PINSTR p1 = m_can_elim.p1;
            PINSTR p2 = m_can_elim.p2;

            M_t* v = m_can_elim.pvar;
            M_t* varnew = expr->CreateNewTemVar(v->size);

            p1->var_w.thevar = varnew;
            if (p2->var_r1.thevar == v)
                p2->var_r1.thevar = varnew;
            if (p2->var_r2.thevar == v)
                p2->var_r2.thevar = varnew;
        }
        break;
    case ES_Instr_can_Elim_31E:
        {
            PINSTR p1 = m_can_elim.p1;
            PINSTR p2 = m_can_elim.p2;
            assert(p2->var_r2.type == v_Immed);
            signed int d = p1->var_r2.d;
            if (p1->type == i_Add)
                d = -d;
            p2->var_r2.d += d;
            INSTR_LIST::iterator rem_iter=std::find(list.begin(),list.end(),p1);
            list.erase(rem_iter);
        }
        break;
    case ES_Instr_can_Elim_25E:
        {
            PINSTR p1 = this->m_can_elim.p1;
            PINSTR p2 = this->m_can_elim.p2;
            p1->var_w = p2->var_w;
            /*
            if (IsSame(&p2->var_r1, this->can_elim.pvar))
            p2->var_r1 = p1->var_r1;
            if (IsSame(&p2->var_r2, this->can_elim.pvar))
            p2->var_r2 = p1->var_r1;
            */
            INSTR_LIST::iterator rem_iter=std::find(list.begin(),list.end(),p2);
            list.erase(rem_iter);
        }
        break;
    case ES_Instr_can_Elim_63:
        {
            PINSTR p1 = this->m_can_elim.p1;
            PINSTR p2 = this->m_can_elim.p2;

            if (p2->var_r1.thevar == this->m_can_elim.pvar)
            {
                p2->var_r1 = p1->var_r1;
                AddOn_serial(p2->va_r1.pao, p1->va_r1.pao);
            }
            if (p2->var_r2.thevar == this->m_can_elim.pvar)
            {
                p2->var_r2 = p1->var_r1;
                AddOn_serial(p2->va_r2.pao, p1->va_r1.pao);
            }
            INSTR_LIST::iterator rem_iter=std::find(list.begin(),list.end(),p1);
            list.erase(rem_iter);
        }
        break;
    }

    this->m_es = ES_Error;
}

void CStrategy::PrintIt(const INSTR_LIST& list, CFunc* pFunc)
{
    log_prtl("---");
    switch (m_es)
    {
    case ES_Instr_can_Delete:
    {
        log_prtl("%s: %s", this->can_delete.reason, this->can_delete.pvar->GetName());
        std::string s = this->PrintOne(list, this->can_delete.pinstr, pFunc);
        s += "  <----";
    }
        break;
    case ES_Instr_can_Elim_63:
    case ES_Instr_can_Elim_31E:
    case ES_Instr_can_Elim_25E:
    case ES_Instr_can_Elim_21E:
    {
        log_prtl("%s: %s", this->m_can_elim.reason, this->m_can_elim.pvar->GetName());
        std::string s1 = this->PrintOne(list, this->m_can_elim.p1, pFunc);
        std::string s2 = this->PrintOne(list, this->m_can_elim.p2, pFunc);
        log_prtl(s1.c_str());
        log_prtl(s2.c_str());
    }
        break;
    }
    log_prtl("---");
}

std::string CStrategy::PrintOne(const INSTR_LIST& list, const INSTR * pinstr, CFunc* pFunc)
{
    INSTR_LIST::const_iterator pos = list.begin();
    while (pos!=list.end())
    {
        const INSTR *p = *pos;//list.;
        ++pos;
        if (p == pinstr)
        {
            CFunc_Prt the(pFunc);
            //TODO: unused pos1 ??
            //POSITION pos1 = list.Find(pinstr);
            std::string s = the.prt_the_instr(p);
            return s;
        }
    }
    return "";
}

void CStrategy::AddOne_CanDelete(M_t* pvar, PINSTR pinstr, const char * reason)
{
    this->m_es = ES_Instr_can_Delete;
    this->can_delete.pvar = pvar;
    this->can_delete.pinstr = pinstr;
    strcpy(this->can_delete.reason, reason);
}
void CStrategy::AddOne_CanEliminate_25E(M_t* pvar, PINSTR p1, PINSTR p2, const char * reason)
{
    //can be eliminated
    this->m_es = ES_Instr_can_Elim_25E;
    this->m_can_elim.pvar = pvar;
    this->m_can_elim.p1 = p1;
    this->m_can_elim.p2 = p2;
    strcpy(this->m_can_elim.reason, reason);
}
void CStrategy::AddOne_CanEliminate_31E(M_t* pvar, PINSTR p1, PINSTR p2, const char * reason)
{
    //can be eliminated
    this->m_es = ES_Instr_can_Elim_31E;
    this->m_can_elim.pvar = pvar;
    this->m_can_elim.p1 = p1;
    this->m_can_elim.p2 = p2;
    strcpy(this->m_can_elim.reason, reason);
}
void CStrategy::AddOne_CanEliminate_21E(M_t* pvar, PINSTR p1, PINSTR p2, const char * reason)
//can be eliminated
{
        this->m_es = ES_Instr_can_Elim_21E;
        this->m_can_elim.pvar = pvar;
        this->m_can_elim.p1 = p1;
        this->m_can_elim.p2 = p2;
        strcpy(this->m_can_elim.reason, reason);
}
void CStrategy::AddOne_CanEliminate_63(M_t* pvar, PINSTR p1, PINSTR p2, const char * reason)
//can be eliminated
{
        this->m_es = ES_Instr_can_Elim_63;
        this->m_can_elim.pvar = pvar;
        this->m_can_elim.p1 = p1;
        this->m_can_elim.p2 = p2;
        strcpy(this->m_can_elim.reason, reason);
}
