// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

////#include "stdafx.h"
//	CFunc_CreateInstrList.cpp
#include <cassert>
#include "CISC.h"



bool CFunc::Step4_1()
{
    this->Step4_CreateInstrList();  ////	and set STEP_4

    this->Step5_GetArgs();	//	and set STEP_5
    this->ana_RetType();
    this->Func_FillCallParas();	//	Fill Call paras, and Step = 6

    return true;
}
void CFunc::Step4_CreateInstrList()
// 读取代码序列
//	读取intel代码序列，转变为伪代码，构造CFunc的伪代码序列表instr_list
//  同时构造CFunc的内部变量表 expr_list，
//  如果需要的话，在全局变量表 g_expr_list 中添加条目
{
	// 初步完成构造伪代码序列表instr_list的工作
	// 但在 stack, expr, call 等方面尚需完善
	assert(m_instr_list.size()==0);
	//
	//m_instr_list = new INSTR_LIST;  //new_INSTR_LIST pseudo code table

	CCodeList the(m_instr_list);
	the.CreateInstrList_raw(this->ll.m_asmlist, this->m_EBP_base);

	// 完善一，对 label 的完善,因为在每条指令前加了label，要去掉没人引用的

	Create_Labels_backend();
	// 完善二，对 call 的完善
//	CreateInstrList_welldone_call();   	// find call, and fill its args

}
PINSTR findlabel(INSTR_LIST& list, ea_t off)
{
    assert(off);	//	一般情况下，这是不为零的
    INSTR_LIST::iterator pos = list.begin();
    while (pos!=list.end())
    {
        PINSTR p = *pos;
        ++pos;
        if (p->type == i_Label && p->label.label_off == off)
            return p;
    }
    assert(!"why here");
    return NULL;
}
void CFunc::Create_Labels_backend()	// 标号后端
{
	POSITION pos = m_instr_list.begin();
	while (pos!=m_instr_list.end())
	{
		PINSTR p = *pos;
		++pos;
		if (p->type == i_Jump)
		{
			PINSTR thelabel = findlabel(m_instr_list,p->jmp.jmpto_off);
			if (thelabel->label.ref_instr)
				p->jmp.next_ref_of_this_label = thelabel->label.ref_instr;	// save old ref list
			thelabel->label.ref_instr = p;	// tell the label it was referred
			p->jmp.the_label = thelabel;	// tell the Jxx the label it need
		}
	}
	//std::remove_if(list->begin(),list->end(),)
	pos = m_instr_list.begin();
	while (pos!=m_instr_list.end())
	{// remove all label not referred
		PINSTR p = *pos;
		if (p->type == i_Label && p->label.ref_instr == 0)
			pos=m_instr_list.erase(pos);
		else
			++pos;
	}
}


