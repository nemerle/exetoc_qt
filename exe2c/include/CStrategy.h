// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//#include "CStrategy.h"
#ifndef CStrategy__H
#define CStrategy__H
#include	"CISC.h"

//log_prtl("asign it, and end: %s", this->m_exprs->VarName(pvar));
enum ENUM_STRATEGY
{
    ES_Error = 0,
    ES_Instr_can_Delete,
    ES_Instr_can_Elim_63,
    ES_Instr_can_Elim_25E,
    ES_Instr_can_Elim_31E,
    ES_Instr_can_Elim_21E,
};


class CStrategy
{
    ENUM_STRATEGY m_es;
    union
    {
        struct
        {
            M_t* pvar;  //which variables
            PINSTR pinstr;  //Used when we need to delete a statement
            char reason[80];
        }can_delete;    //for m_es == ES_Instr_can_Delete
        struct
        {
            M_t* pvar;  //which variable
            PINSTR p1;  //previous to my assignment
            PINSTR p2;  //1 after my use
            char reason[80];
        }m_can_elim;    //for m_es == ES_Instr_can_Elim
    };
    char m_buf[256];

    std::string PrintOne(const INSTR_LIST & list, const INSTR *p, CFunc* pFunc);

public:
    CStrategy()
    {
        m_es = ES_Error;
    }
    void AddOne_CanDelete(M_t* pvar, PINSTR pinstr, const char * reason);
    void AddOne_CanEliminate_25E(M_t* pvar, PINSTR p1, PINSTR p2, const char * reason);
    void AddOne_CanEliminate_31E(M_t* pvar, PINSTR p1, PINSTR p2, const char * reason);
    void AddOne_CanEliminate_21E(M_t* pvar, PINSTR p1, PINSTR p2, const char * reason);
    void AddOne_CanEliminate_63(M_t* pvar, PINSTR p1, PINSTR p2, const char * reason);
    void PrintIt(const INSTR_LIST& list, CFunc* pFunc);
    void DoIt(INSTR_LIST& list, CExprManage* expr);
    bool DoIt_Addon(INSTR_LIST& list, CExprManage* expr);
    bool IfAny()
    {
        return this->m_es != ES_Error;
    }
};

extern CStrategy g_CStrategy;

#endif // CStrategy__H