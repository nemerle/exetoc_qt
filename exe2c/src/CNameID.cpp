// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

////#include "stdafx.h"
#include	"CISC.h"
#include <algorithm>

CNameMng::CNameMng()
{
}
CNameMng::~CNameMng()
{
}
const char * CNameMng::id2name(H_NAMEID id)
{
    tBimap::right_const_iterator iter=m_map.right.find(id);
    if(iter!=m_map.right.end())
        return iter->second.c_str();
    return 0;
}
H_NAMEID CNameMng::savname(const char * name)
{
    assert(false);
    H_NAMEID h = this->LookupName(name);
    if (h != 0)
        return h;   //Do not duplicate deposit
    std::string inserted=name;
    //	p->id = (H_NAMEID)p->name;	//In any case, this is the only //	不管怎样，这也是唯一的
    //m_map.insert(tBimap::value_type(inserted,(H_NAMEID)inserted.c_str()));
    return 0;//(H_NAMEID)inserted.c_str();
}
H_NAMEID CNameMng::LookupName(const char * name)
{
    tBimap::left_const_iterator iter=m_map.left.find(name);
    if(iter!=m_map.left.end())
        return iter->second;
    return 0;
}
void CNameMng::Rename(H_NAMEID h, const char * newname)
{
    tBimap::right_iterator it = m_map.right.find(h);
    if(it!=m_map.right.end())
    {
        bool successful_replace = m_map.right.replace_data( it, newname );
        assert( successful_replace );
    }
}
