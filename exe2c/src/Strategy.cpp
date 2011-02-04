// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include <algorithm>
#include "Strategy.h"

Strategy g_CStrategy;

bool Strategy::DoIt_Addon(INSTR_LIST& list, ExprManage* expr)
{
    assert(m_es == ES_Instr_can_Elim_21E);
//    M_t* v = m_pvar;
//    Instruction * p1 = this->m_can_elim.p1;
//    Instruction * p2 = this->m_can_elim.p2;

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
void Strategy::DoIt(INSTR_LIST& list, ExprManage* expr)
{
    switch (m_es)
    {
    case ES_Instr_can_Delete:
        {
            INSTR_LIST::iterator remove_val= std::find(list.begin(), list.end(), can_delete.pinstr);
            assert(remove_val!=list.end());
            list.erase(remove_val);
        }
        break;
    case ES_Instr_can_Elim_21E:
        {
            if (this->DoIt_Addon(list,expr))
                break;

            Instruction * p1 = m_can_elim.p1;
            Instruction * p2 = m_can_elim.p2;

            M_t* v = m_pvar;
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
            Instruction * p1 = m_can_elim.p1;
            Instruction * p2 = m_can_elim.p2;
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
            Instruction * p1 = this->m_can_elim.p1;
            Instruction * p2 = this->m_can_elim.p2;
            p1->var_w = p2->var_w;
            INSTR_LIST::iterator rem_iter=std::find(list.begin(),list.end(),p2);
            list.erase(rem_iter);
        }
        break;
    case ES_Instr_can_Elim_63:
        {
            Instruction * p1 = this->m_can_elim.p1;
            Instruction * p2 = this->m_can_elim.p2;

            if (p2->var_r1.thevar == this->m_pvar)
            {
                p2->var_r1 = p1->var_r1;
                AddOn_serial(p2->va_r1.pao, p1->va_r1.pao);
            }
            if (p2->var_r2.thevar == this->m_pvar)
            {
                p2->var_r2 = p1->var_r1;
                AddOn_serial(p2->va_r2.pao, p1->va_r1.pao);
            }
            INSTR_LIST::iterator rem_iter=std::find(list.begin(),list.end(),p1);
            list.erase(rem_iter);
        }
        break;
    case ES_Error:
        assert(false);
        break;
    }

    this->m_es = ES_Error;
}

void Strategy::PrintIt(const INSTR_LIST& list, Func* pFunc)
{
    log_prtl("---");
    switch (m_es)
    {
    case ES_Instr_can_Delete:
    {
        log_prtl("%s: %s", m_reason.c_str(), this->m_pvar->GetName().c_str());
        std::string s = this->PrintOne(list, this->can_delete.pinstr, pFunc);
        s += "  <----";
    }
        break;
    case ES_Instr_can_Elim_63:
    case ES_Instr_can_Elim_31E:
    case ES_Instr_can_Elim_25E:
    case ES_Instr_can_Elim_21E:
    {
        log_prtl("%s: %s", m_reason.c_str(), m_pvar->GetName().c_str());
        std::string s1 = this->PrintOne(list, this->m_can_elim.p1, pFunc);
        std::string s2 = this->PrintOne(list, this->m_can_elim.p2, pFunc);
        log_prtl(s1.c_str());
        log_prtl(s2.c_str());
    }
        break;
    case ES_Error:
        assert(false);
        break;
    }
    log_prtl("---");
}

std::string Strategy::PrintOne(const INSTR_LIST& list, const Instruction * pinstr, Func* pFunc) const
{
    INSTR_LIST::const_iterator pos = list.begin();
    while (pos!=list.end())
    {
        const Instruction *p = *pos;//list.;
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

void Strategy::AddOne_CanDelete(M_t* pvar, Instruction * pinstr, const std::string &reason)
{
    this->m_es = ES_Instr_can_Delete;
    m_pvar = pvar;
    this->can_delete.pinstr = pinstr;
    m_reason = reason;
}
void Strategy::AddOne_CanEliminate_25E(M_t* pvar, Instruction * p1, Instruction * p2, const std::string & reason)
{
    //can be eliminated
    this->m_es = ES_Instr_can_Elim_25E;
    m_pvar = pvar;
    this->m_can_elim.p1 = p1;
    this->m_can_elim.p2 = p2;
    m_reason = reason;
}
void Strategy::AddOne_CanEliminate_31E(M_t* pvar, Instruction * p1, Instruction * p2, const std::string & reason)
{
    //can be eliminated
    this->m_es = ES_Instr_can_Elim_31E;
    m_pvar = pvar;
    this->m_can_elim.p1 = p1;
    this->m_can_elim.p2 = p2;
    m_reason = reason;
}
void Strategy::AddOne_CanEliminate_21E(M_t* pvar, Instruction * p1, Instruction * p2, const std::string & reason)
//can be eliminated
{
        this->m_es = ES_Instr_can_Elim_21E;
        m_pvar = pvar;
        this->m_can_elim.p1 = p1;
        this->m_can_elim.p2 = p2;
        m_reason = reason;
}
void Strategy::AddOne_CanEliminate_63(M_t* pvar, Instruction * p1, Instruction * p2, const std::string & reason)
//can be eliminated
{
        this->m_es = ES_Instr_can_Elim_63;
        m_pvar = pvar;
        this->m_can_elim.p1 = p1;
        this->m_can_elim.p2 = p2;
        m_reason = reason;
}
