// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
#pragma once

#include <list>
#include <stdint.h>
struct OneCase
{
    OneCase(uint32_t num,Instruction *lab) : case_n(num),thelabel(lab) {}
    uint32_t case_n;
    Instruction *thelabel;
};

typedef std::list<OneCase*> CasePrt_List;
