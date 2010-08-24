// Copyright(C) 1999-2005 LiuTaoTaobookaa@rorsoft.com

//	EnumMng.h

////#include "stdafx.h"
#include "CISC.h"
#include "00000.h"
#include "EnumMng.h"

#include "strparse.h"
#include "SVarType.h"

Enum_mng * g_enum_mng = NULL;

Enum_mng::~Enum_mng()
{
	Enum_List::iterator pos = m_list.begin();
	for (;pos!=m_list.end(); ++pos)
	{
		enum_st* p = *pos;
		NumStr_st* p1 = p->m_pfirst;
		while (p1)
		{
			NumStr_st* p2 = p1->next;
			delete p1->name;
			delete p1;
			p1 = p2;
		}
		delete p;
	}
	m_list.clear();
}
void Enum_mng::Add_New_Enum(enum_st* pnew)
{
	m_list.push_front(pnew);
}

char * enum_st::lookup_itemname(uint32_t n)
{
	NumStr_st* p = this->m_pfirst;
	while (p)
	{
		if (p->n == n)
			return p->name;
		p = p->next;
	}
	return NULL;
}
VarTypeID Enum_mng::if_EnumName(const char * &pstr)
{
    Enum_List::iterator pos = m_list.begin();
    for (;pos!=m_list.end();++pos)
    {
        enum_st* p = *pos;
        int n = strlen(p->m_name);
        if (memcmp(p->m_name,pstr,n))
            continue;
        if (if_split_char(pstr[n]))
        {
            pstr += n;
            return g_VarTypeManage->Enum2VarID(p);
        }
    }
    return 0;
}
