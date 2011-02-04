// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

////#include "stdafx.h"
//	optim.cpp
#include <algorithm>
#include "CISC.h"


//bool Func::expr_only_use_in_this(const VAR * const pvar, const Instruction * const phead)
//{
//    assert( phead->type == i_Begin || phead->type == i_CplxBegin);

//    INSTR_LIST::iterator pos = m_instr_list.begin();
//    for (;pos!=m_instr_list.end();++pos)
//    {
//        Instruction * p = *pos;
//        if (p == phead)
//        {
//            p = p->begin.m_end;
//            pos = std::find(m_instr_list.begin(),m_instr_list.end(),p);
//            continue;
//        }
//        if (VAR::IsSame(pvar, &p->var_w))
//            return false;
//        if (VAR::IsSame(pvar, &p->var_r1))
//            return false;
//        if (VAR::IsSame(pvar, &p->var_r2))
//            return false;
//    }
//    return true;
//}

