// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#ifndef NameID__H
#define NameID__H
#include <string>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

struct st_NameID
{
	H_NAMEID id;
	char * name;
};

class NameMng
{
private:
    typedef boost::bimap<boost::bimaps::unordered_set_of<std::string>,boost::bimaps::set_of<H_NAMEID>,boost::bimaps::right_based > tBimap;
    tBimap m_map;
public:
	NameMng();
	~NameMng();
	const char * id2name(H_NAMEID id);
	H_NAMEID savname(const char * name);
	H_NAMEID LookupName(const char * name);
	void Rename(H_NAMEID h, const char * newname);
};

#endif // NameID__H
