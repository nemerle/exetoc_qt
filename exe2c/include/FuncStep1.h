// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#ifndef CFuncStep1__H
#define CFuncStep1__H
#include <list>
#include <set>
#include <string>
#include "Func.h"

class CFuncStep1
{
    AsmCodeList* m_asmlist;

    ea_t	m_end_off;

    typedef	std::list<ea_t>	EALIST;
    struct CASE_t
    {
        ea_t	jxx_opcode;
        EALIST*	caselist;
    };
    typedef std::list<CASE_t*> CaseList;

    void CheckIfJustSwitchCase(CaseList& caselist, ea_t ea);
    static void    check_if_switch_case(ea_t cur_off, CaseList* pcaselist,std::set<ea_t> &orderd_ea_set,EALIST* pjxxlist, XCPUCODE* pxcpu);
public:
    CFuncStep1(AsmCodeList* asmlist)
    {
        this->m_asmlist = asmlist;
    }
    bool	Step_1(ea_t head_off);	// The end of address to find the functions
    ea_t Get_end_off()
    {
        return this->m_end_off;
    }
    void CreateNewFunc_if_CallNear();
};

class FuncLL
{
        int m_EBP_base;
    AsmCodeList* m_asmlist;
        bool	Asm_Code_Change_ESP(int &esp, XCPUCODE* pxcpu);
    bool	Fill_Stack_1();
        bool	Check_Stack();
        bool	stack_stack(AsmCode* p0, AsmCode* p1);
        AsmCode* ea2pasm(ea_t ea);
    std::string GetLabelName(ea_t ea);
    void    VarLL_Analysis_1(VarLL* pvarll, OPERITEM* op, AsmCode* pasm) const;
        void	prtout_asm_1(VarLL* pvarll, XmlOutPro* out);
public:
    FuncLL(AsmCodeList* asmlist)
    {
        this->m_asmlist = asmlist;
                m_EBP_base = Not_EBP_based;   //invalid
    }
    int Get_Ret_Purge();    //return -1 for fail
        bool	Fill_Stack_Info();
        int Get_EBP_base()
        {
                return this->m_EBP_base;
        }
        void prtout_asm(Func* pfunc, VarLL* pvarll, XmlOutPro* out);
    void GetVarRange(signed int& VarRange_L, signed int& VarRange_H);
    void VarLL_Analysis(VarLL* pvarll);
    void Prepare_CallFunc();
    void AddRemoveSomeInstr();
};

class CJxxLabel
{
    AsmCodeList* m_asmlist;
public:
    CJxxLabel(AsmCodeList* asmlist)
    {
        this->m_asmlist = asmlist;
    }

    void Label_Analysis();
};


#endif // CFuncStep1__H
