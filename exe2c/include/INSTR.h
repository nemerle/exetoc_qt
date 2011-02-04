#ifndef INSTR__H
#define INSTR__H
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include <stdint.h>
#include <list>
#include <QString>
#include "enum.h"
#include "EXPR.h"
class	Func;

class Instruction;
enum em_InstrAddOn
{
    IA_Nothing = 0,
    IA_AddImmed,    //simply add the number immediately
    IA_MulImmed,    //simply multiply the number immediately
    IA_ReadPointTo, //*
    IA_GetAddress   //&
};
struct st_InstrAddOn
{
    em_InstrAddOn type;
    union
    {
        struct
        {
            signed int iAddon;
        } addimmed; //for IA_AddImmed
    };

    st_InstrAddOn* pChild;

    st_InstrAddOn() : type(IA_Nothing),pChild(0)
    {
        addimmed.iAddon=0;
    }
    ~st_InstrAddOn()
    {
        if (pChild)
            delete pChild;
    }
    static bool IsSame(st_InstrAddOn* p1, st_InstrAddOn* p2);
};
typedef st_InstrAddOn* Pst_InstrAddOn;

struct VAR_ADDON
{
    VAR*	pv;
    st_InstrAddOn* pao; //point add on
    VAR_ADDON()
    {
        pv = NULL;
        pao = NULL;
    }
    ~VAR_ADDON()
    {
        //do not delete pv
        if (pao)
            delete pao;
    }
};

class Instruction
{
public:
    HLType	type;	//	i_Assign ...
    union
    {
        struct
        {
            JxxType	jmp_type;	//JMP_???
            uint32_t   jmpto_off;
            Instruction * target_label;
            Instruction * next_ref_of_this_label;	//Formed a chain here, a label is used to save all the ref
        } jmp;					//for type = i_Jump only

        struct
        {
            Instruction *  ref_instr;	//for type = i_Label only
            ea_t	label_off;
            bool	f_conti;
        } label;

        struct
        {
            Func*  call_func;		// for i_Call
            Api*	papi;			// for i_CallApi
            signed int		esp_level;
            Instruction *      p_callpara;
            Instruction *      p_callret;
        } call;

        struct
        {
            Instruction *      p_thecall;
        } call_addon;   //for i_CallPara and i_CallRet

        struct
        {
            Instruction *		m_end;		// for i_Begin
            enum_COMP	type;		// COMP_if
            Instruction *		m_break;	// 如果允许 break 的话，这是break到的label
            Instruction *		m_conti;	// 如果允许 continue 的话，这是continue到的label
            Instruction *		m_not_conti;	//	尽管这个jmp指向m_conti，它仍然不是continue.
        } begin;						//	这个特例是为了照顾while

        struct
        {
            signed int esp_level;
            UINT howlen;
        } espreport;    // for i_EspReport
    };
    void    setVars(const VAR &v1,const VAR  &v2)
    {
        var_r1 = v1;		// the pointer
        var_r2 = v2;			// the value
    }
    VAR		var_w;
    VAR_ADDON va_r1;    //va=Var AddOn
    VAR_ADDON va_r2;
    VAR		var_r1;
    VAR		var_r2;
    uint32_t	i1;	// =4, 当type == i_Address时，[eax+ebx*4+8888]
    uint32_t	i2;	// =8888

    Instruction(HLType hl_type=i_Unknown,enum_COMP comp_type=COMP_unknown);
    ~Instruction();

    //optim serials functions
    bool	optim_1_();
    bool	optim_4_();
    bool	optim_3_();
    bool	optim_3_1(Instruction * p);
    void	optim_3_2(Instruction * p);
    //-----------------
};

typedef	std::list<Instruction *> INSTR_LIST;
class InstrList
{
    typedef INSTR_LIST::iterator POSITION;
    bool	if_Ly_In(Instruction * p, POSITION firstpos, POSITION endpos);
    bool    IsSwitchCase_multcomp(Instruction * begin);
    bool    IsSwitchCase(Instruction * begin);
    bool	ifOneStatement(Instruction * pNode, POSITION firstpos, POSITION endpos);
    bool	Flow_c(Instruction * pNode);
    void	Flow_b(Instruction * pParentNode, POSITION firstpos, POSITION endpos);
    bool	Flow_aa(Instruction * pNode, POSITION firstpos, POSITION endpos);
    bool	Flow_cc(Instruction * pNode, POSITION firstpos, POSITION endpos);
    void	Add_Begin_End(POSITION firstpos, POSITION endpos, Instruction * begin, Instruction * end);
    void	Add_Begin_End_1(POSITION firstpos, POSITION endpos, Instruction * begin, Instruction * end);
	void	RemoveNops();
    INSTR_LIST &m_list; //要尽量把它private
public:
    InstrList(INSTR_LIST& p) : m_list (p)
    {

    }

    bool	Flow_a(Instruction * pNode);
};

class InstrList_Finger
{
    QString	prt_partern(Instruction * phead);
    static int search_and_add(intptr_t* buf,intptr_t val,int &pn);
    static bool	finger_compare(char * f1,const char* f2);
public:
    INSTR_LIST & m_list; //要尽量把它private
    InstrList_Finger(INSTR_LIST &list) : m_list(list)
    {
    }
    bool	Finger_check_partern(Instruction * p);
    bool	Finger_check_partern_for1(Instruction * p);
};
#endif // INSTR__H
