// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

// INSTR.h
#ifndef INSTR__H
#define INSTR__H
#include <list>
#include "enum.h"
#include "EXPR.h"
class	Func;

class INSTR;
enum em_InstrAddOn
{
    IA_Nothing = 0,
    IA_AddImmed,    //simply add the number immediately
    IA_MulImmed,    //simply multiply the number immediately
    IA_ReadPointTo, //*
    IA_GetAddress,  //&
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

class INSTR
{
public:
    HLType	type;	//	i_Assign ...
    union
    {
        struct
        {
            JxxType	jmp_type;	//JMP_???
            uint32_t   jmpto_off;
            INSTR * the_label;
            INSTR * next_ref_of_this_label;	//这里组成个链，用来保存对一个label的所有ref
        } jmp;					//for type = i_Jump only

		struct
		{
			INSTR *  ref_instr;	//for type = i_Label only
			ea_t	label_off;
			bool	f_conti;
		} label;

		struct
		{
			Func*  call_func;		// for i_Call
			Api*	papi;			// for i_CallApi
			signed int		esp_level;
			INSTR *      p_callpara;
			INSTR *      p_callret;
		} call;

        struct
        {
            INSTR *      p_thecall;
        } call_addon;   //for i_CallPara and i_CallRet

		struct
		{
			INSTR *		m_end;		// for i_Begin
			enum_COMP	type;		// COMP_if
			INSTR *		m_break;	// 如果允许 break 的话，这是break到的label
			INSTR *		m_conti;	// 如果允许 continue 的话，这是continue到的label
			INSTR *		m_not_conti;	//	尽管这个jmp指向m_conti，它仍然不是continue.
		} begin;						//	这个特例是为了照顾while

        struct
        {
            signed int esp_level;
            UINT howlen;
        } espreport;    // for i_EspReport
    };

    VAR		var_w;
    VAR_ADDON va_r1;    //va=Var AddOn
    VAR_ADDON va_r2;
    VAR		var_r1;
    VAR		var_r2;
    uint32_t	i1;	// =4, 当type == i_Address时，[eax+ebx*4+8888]
    uint32_t	i2;	// =8888

	INSTR();
	~INSTR();

	//optim serials functions
	bool	optim_1_();
	bool	optim_4_();
	bool	optim_3_();
	bool	optim_3_1(INSTR * p);
	void	optim_3_2(INSTR * p);
	//-----------------
};

typedef	std::list<INSTR *> INSTR_LIST;
class InstrList
{
    typedef INSTR_LIST::iterator POSITION;
    bool	if_Ly_In(INSTR * p, POSITION firstpos, POSITION endpos);
    bool    IsSwitchCase_multcomp(INSTR * begin);
    bool    IsSwitchCase(INSTR * begin);
    bool	ifOneStatement(INSTR * pNode, POSITION firstpos, POSITION endpos);
    bool	Flow_c(INSTR * pNode);
    void	Flow_b(INSTR * pParentNode, POSITION firstpos, POSITION endpos);
    bool	Flow_aa(INSTR * pNode, POSITION firstpos, POSITION endpos);
    bool	Flow_cc(INSTR * pNode, POSITION firstpos, POSITION endpos);
    void	Add_Begin_End(POSITION firstpos, POSITION &endpos, INSTR * begin, INSTR * end);
    void	Add_Begin_End_1(POSITION firstpos, POSITION endpos, INSTR * begin, INSTR * end);

    INSTR_LIST &m_list; //要尽量把它private
public:
    InstrList(INSTR_LIST& p) : m_list (p)
    {

    }

    bool	Flow_a(INSTR * pNode);
};

class InstrList_Finger
{
    void	prt_partern(INSTR * phead, char * partern_buf);
    static int search_and_add(uint32_t* buf,uint32_t val,int* pn);
    static bool	finger_compare(char * f1,const char* f2);
public:
    INSTR_LIST & m_list; //要尽量把它private
    InstrList_Finger(INSTR_LIST &list) : m_list(list)
    {
    }
    bool	Finger_check_partern(INSTR * p);
    bool	Finger_check_partern_for1(INSTR * p);
};
#endif // INSTR__H
