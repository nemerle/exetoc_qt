// Copyright(C) 1999-2005 LiuTaoTao , bookaa@rorsoft.com

////#include "stdafx.h"
//	CFunc_CreateInstrList.cpp
#include <cassert>
#include "CISC.h"



bool Func::Step4_1()
{
    this->Step4_CreateInstrList();  ////	and set STEP_4

    this->Step5_GetArgs();	//	and set STEP_5
    this->ana_RetType();
    this->Func_FillCallParas();	//	Fill Call paras, and Step = 6

    return true;
}
// read on behalf of the sequence
// Read intel on behalf of the sequence transformed into pseudo-generation, generation of sequence of pseudo-structure CFunc list instr_list
// while the internal variable table structure CFunc expr_list,
// If necessary, the global variable table g_expr_list add entries in the
void Func::Step4_CreateInstrList()
{
    // Pseudo-structure initially completed the work substituting Unordered List instr_list
    // However, stack, expr, call, etc. still need to improve
	assert(m_instr_list.size()==0);
	//
	//m_instr_list = new INSTR_LIST;  //new_INSTR_LIST pseudo code table

	CodeList the(m_instr_list);
	the.CreateInstrList_raw(this->ll.m_asmlist, this->m_EBP_base);

    // A improve, perfect of label for each instruction before the label, no one should remove the reference

	Create_Labels_backend();
    // Second call improvement
//	CreateInstrList_welldone_call();   	// find call, and fill its args

}
Instruction * findlabel(INSTR_LIST& list, ea_t off)
{
    assert(off);	//	In general, it is not null
    INSTR_LIST::iterator pos = list.begin();
    while (pos!=list.end())
    {
        Instruction * p = *pos;
        ++pos;
        if (p->type == i_Label && p->label.label_off == off)
            return p;
    }
    assert(!"why here");
    return NULL;
}
void Func::Create_Labels_backend()	// No|Number backend
{
	POSITION pos = m_instr_list.begin();
	while (pos!=m_instr_list.end())
	{
		Instruction * p = *pos;
		++pos;
		if (p->type == i_Jump)
		{
			Instruction * thelabel = findlabel(m_instr_list,p->jmp.jmpto_off);
			if (thelabel->label.ref_instr)
				p->jmp.next_ref_of_this_label = thelabel->label.ref_instr;	// save old ref list
			thelabel->label.ref_instr = p;	// tell the label it was referred
			p->jmp.target_label = thelabel;	// tell the Jxx the label it need
		}
	}
	//std::remove_if(list->begin(),list->end(),)
	pos = m_instr_list.begin();
	while (pos!=m_instr_list.end())
	{// remove all label not referred
		Instruction * p = *pos;
		if (p->type == i_Label && p->label.ref_instr == 0)
			pos=m_instr_list.erase(pos);
		else
			++pos;
	}
}


