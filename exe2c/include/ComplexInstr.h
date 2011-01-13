// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	ComplexInstr.h
#ifndef ComplexInstr__H
#define ComplexInstr__H
#include <list>
struct OneCase
{
    OneCase(int num,Instruction *lab) : case_n(num),thelabel(lab) {}
    int case_n;
    Instruction *thelabel;
};

typedef std::list<OneCase*> CasePrt_List;



#endif // ComplexInstr__H
