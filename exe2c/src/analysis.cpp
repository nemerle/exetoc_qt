// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	analysis.cpp
#include <QDebug>
#include <list>
#include "CISC.h"
#include "Strategy.h"
extern bool g_f_Step_by_Step;
extern bool g_any1_return_TRUE;
bool Step_by_Step();

bool	Func::analysis_step_by_step()
{
    // Let us show points of information
    static Func* lastfunc = 0;
    static int n = 0;
    if (this != lastfunc)
    {
        lastfunc = this;
        n = 0;
    }

    g_f_Step_by_Step = true;
    g_any1_return_TRUE = false;
    bool f = analysis_once();
    g_f_Step_by_Step = false;
    f = f || g_any1_return_TRUE;

    if (f)
        log_prtl("step %d",n++);
    return f;
}

typedef bool (FuncOptim::*OPTIMFUNC)(void);
typedef bool (Func::*PROGFUNC)(void);

static const PROGFUNC tbl_Progress[] =
{
    &Func::Step_1,              // 0
    &Func::Step2_GetRetPurge,   // 1
    &Func::AddRemoveSomeInstr,  // 2
    &Func::Step3_FillStackInfo, // 3
    &Func::Step_Label_Analysis, // 4
    &Func::Step4_1,             // 5
    &Func::Var_analysis,        // 6
    0
};

static const OPTIMFUNC tbl_Ana[] =
{
    &FuncOptim::Address_to_Add,                 //0 See if i_Address can become i_Add
    &FuncOptim::Simplify_Instr,                 //1 to see if a simple command can become i_Assign
    &FuncOptim::Var_Split,					    //2 variable split
    &FuncOptim::ClassSubFuncProcess,            //If it is a class of Functions
    &FuncOptim::VarDataType_analysis_mydefine,	//3
    &FuncOptim::VarDataType_analysis,	        //4
    &FuncOptim::DataType_Flow,	                //5 Analysis of data types flow
    &FuncOptim::optim_once_new,                 //6
    &FuncOptim::pcode_1,	                    //7 If cmp and jxx are close, then merge cmp and jxx
    &FuncOptim::ana_Flow,	                    //8
    0
};

bool	Func::analysis_once_1()
{
    m_prepareTrue_analysisFalse = false;
    if (m_nStep < STEP_7)
    {
        PROGFUNC fn = tbl_Progress[m_nStep];
        if (fn != NULL)
        {
            if ((this->*fn)())
            {
                m_nStep++;
                return true;
            }
            return false;
        }
        m_nStep = STEP_100;   //Force the next analysis step
    }

    assert(m_nStep == STEP_100);

    FuncOptim the(this);
    for (int i=0; ;i++)
    {
        OPTIMFUNC pfunc = tbl_Ana[i];
        if (pfunc == 0)
            break;
        if ((the.*pfunc)())
            return true;
    }


    return false;
}
bool	Func::analysis_once()
{
	if(false==analysis_once_1())
		return false;
	if (g_CStrategy.IfAny())
	{
		g_CStrategy.PrintIt(this->m_instr_list, this);
		g_CStrategy.DoIt(this->m_instr_list, this->m_exprs);
	}
	DeleteUnusedVar();
	return true;
}
void Func::analysis()
{
    for (;;)
    {
        g_f_Step_by_Step = true;
        g_any1_return_TRUE = false;
        if (!this->analysis_once())
            break;
    }
}


void	Func::ana_RetType()
{	//	检查函数的返回值
    VAR v;
    v.type = v_Reg;
    v.reg = enum_EAX;   //	enum_EAX = 0 = enum_AL = enum_AX
    v.opsize = BIT32_is_4;

    if (m_functype != NULL)
    {
        int n = GG_VarType_ID2Size(this->m_functype->m_retdatatype_id);
        if (n == 0)
            return;
        if (n == 2 || n == 4)
            v.opsize = n;
    }


    INSTR_LIST::iterator pos = m_instr_list.begin();
    for (;pos!=m_instr_list.end();++pos)
    {
        const Instruction * p = *pos;
		if (p->type != i_Return)
			continue;

		Instruction * pnew = new Instruction(i_RetPar);// For the time being that each is ret uint32_t func
		pnew->var_r1 = v;
        m_instr_list.insert(pos,pnew);
    }
}
//Prepare is to add new functions, but does not call the new function prepare
void Func::PrepareFunc()
{
    m_prepareTrue_analysisFalse = true;
    while (m_nStep < STEP_IDA_3)
    {
        PROGFUNC fn = tbl_Progress[m_nStep];
        if (fn != NULL)
        {
            if ((this->*fn)())
            {
                m_nStep++;
                continue;
            }
            break;
        }
        else
            m_nStep = STEP_100;
    }
}
