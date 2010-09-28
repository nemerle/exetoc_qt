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


class Strategy
{
    ENUM_STRATEGY m_es;
    union
    {
        struct
        {
            M_t* pvar;  //which variables
            INSTR * pinstr;  //Used when we need to delete a statement
            char reason[80];
        }can_delete;    //for m_es == ES_Instr_can_Delete
        struct
        {
            M_t* pvar;  //which variable
            INSTR * p1;  //previous to my assignment
            INSTR * p2;  //1 after my use
            char reason[80];
        }m_can_elim;    //for m_es == ES_Instr_can_Elim
    };
    char m_buf[256];

    std::string PrintOne(const INSTR_LIST & list, const INSTR *p, Func* pFunc);

public:
    Strategy()
    {
        m_es = ES_Error;
    }
    void AddOne_CanDelete(M_t* pvar, INSTR * pinstr, const char * reason);
    void AddOne_CanEliminate_25E(M_t* pvar, INSTR * p1, INSTR * p2, const char * reason);
    void AddOne_CanEliminate_31E(M_t* pvar, INSTR * p1, INSTR * p2, const char * reason);
    void AddOne_CanEliminate_21E(M_t* pvar, INSTR * p1, INSTR * p2, const char * reason);
    void AddOne_CanEliminate_63(M_t* pvar, INSTR * p1, INSTR * p2, const char * reason);
    void PrintIt(const INSTR_LIST& list, Func* pFunc);
    void DoIt(INSTR_LIST& list, ExprManage* expr);
    bool DoIt_Addon(INSTR_LIST& list, ExprManage* expr);
    bool IfAny()
    {
        return this->m_es != ES_Error;
    }
};

extern Strategy g_CStrategy;

#endif // CStrategy__H
