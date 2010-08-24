// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include <cassert>
#include <algorithm>
#include "exe2c.h"
#include "CISC.h"

Func* g_Cur_Func = NULL;

//EXPR_LIST	*g_expr_list = NULL;	// Global variable table


void outstring_in_log(const char * str)
{
    g_Cexe2c->prt_log(str);
}

FUNC_LIST::iterator Cexe2c::GetFirstFuncHandle()
{
    FUNC_LIST::iterator pos = m_func_list.begin();//POSITION pos = begin();
    return pos;
}

FUNC_LIST::iterator Cexe2c::GetCurFuncHandle()
{
    assert(g_Cur_Func!=0);
    FUNC_LIST::iterator pos = std::find(m_func_list.begin(),m_func_list.end(),g_Cur_Func);
    assert(pos!=m_func_list.end());
    return pos;
}

void Cexe2c::SetCurFunc_by_Name(const char * funcname)
{
        Func *p = this->FindFuncByName(funcname);
    if (p == NULL)
        return;
    g_Cur_Func = p;
}
void Cexe2c::GetFuncInfo( FUNC_LIST::iterator h, st_FuncInfo* info )
{
    if (h == m_func_list.end())
        return;

    Func* p = *h;

    strcpy(info->name, p->m_funcname.c_str());
    info->nStep = p->m_nStep;
    info->headoff = p->m_head_off;
    info->endoff = p->m_end_off;
    info->stack_purge = p->m_stack_purge;
    info->m_EBP_base = p->m_EBP_base;
    info->m_args = p->m_args;
}
bool Cexe2c::is_valid_function_handle(const FUNC_LIST::iterator &it) const
{
    return it!=m_func_list.end();
}
FUNC_LIST::iterator Cexe2c::GetNextFuncHandle( FUNC_LIST::iterator h )
{
    if (h == m_func_list.end())
        return h;

    return ++h;
}

bool Cexe2c::RenameCurFuncName(const char * name)
{
    Func *p = g_Cur_Func;
    if (p == NULL)
        return false;
    if (!IfValideFuncName(name))
        return false;	// name not valid
    if (this->FindFuncByName(name) != NULL )
        return false;	// name already use
    p->m_funcname = name;
    return true;
}
