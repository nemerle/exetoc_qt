// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


//#include "stdafx.h"
#include <cstring>
#include <cassert>
#include <cstdio>

#include "00000.h"
#include "CFuncType.h"
#include "SVarType.h"

CFuncType::CFuncType() : m_callc(),m_retdatatype_id(0),m_extern_c(false),m_varpar(0),m_args(0),
    m_partypes(0),m_class(0)
{
}
CFuncType::~CFuncType()
{
    this->m_partypes.clear();
    this->m_parnames.clear();
}

CFuncType* CFuncType::ft_clone()
{
    //Copy the current information
    CFuncType* pnew = new CFuncType;
    *pnew = *this;

    pnew->m_pname = m_pname;
    pnew->m_internal_name = m_internal_name;

    pnew->m_partypes = m_partypes; // vector contents COPIED here
    if (this->m_args)
    {
        //pnew->m_parnames.resize( = new char *[this->m_args];
        //memcpy(pnew->m_parnames, this->m_parnames,sizeof(char *) * this->m_args);
        pnew->m_parnames = m_parnames; // vector contents copied here
        assert(m_args==pnew->m_parnames.size());
    }

    return pnew;
}

void CFuncType::create_internal_funcname()
{
        if (this->m_internal_name.size()!=0)
                return;

        char buf[80];
        if (this->m_extern_c)
        {
                m_internal_name="_"+this->m_pname;
                return;
        }
        if (this->m_callc == enum_stdcall) //	stdcall is alos simple
                sprintf(buf,"%s@%d",this->m_pname.c_str(),this->m_args * 4);
        else if (this->m_callc == enum_fastcall) //	不知道怎么变，随便吧 //Do not know how to change it easily
                sprintf(buf,"%s@@%d",this->m_pname.c_str(),this->m_args * 4);
        else if (this->m_callc == enum_cdecl)
                sprintf(buf,"%s@%d",this->m_pname.c_str(),this->m_args * 4);
        else
            assert(("I do not know how to convert it to internal funcname",0));
        this->m_internal_name = buf;
}


VarTypeID CFuncType::SearchPara(SIZEOF off)
{
    size_t retn = 0;
    for (int i=0; i<m_args; i++)
    {
        if (retn == off)
            return m_partypes[i];

        retn += GG_VarType_ID2Size(m_partypes[i]);
    }
    return 0;
}
SIZEOF CFuncType::para_total_size()
{
    SIZEOF retn = 0;
    for (int i=0; i<m_args; i++)
    {
        retn += GG_VarType_ID2Size(m_partypes[i]);
    }
    return retn;
}

unsigned char CFuncType::get_stack_purge()
{
    //	根据m_ftype，算出 m_stack_purge
    if (m_varpar)
    {
        assert(m_callc == enum_cdecl);	//	似乎只有__cdecl才会变参
        return 0;	//	变参数肯定是不改堆栈的
    }
    switch (m_callc)
    {
    case enum_cdecl:
        return 0;

    case enum_fastcall:
        if (m_args <= 2)
            return 0;
        return (m_args-2) * 4;

    case enum_stdcall:
        return m_args * 4;

    default:
        assert(0);
        return 0;
    }
}
