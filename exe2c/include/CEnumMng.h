// Copyright(C) 1999-2005 LiuTaoTaobookaa@rorsoft.com

//	exe2c project
//	EnumMng.h
#ifndef	CEnumMng_H
#define CEnumMng_H
#include <list>
typedef uint32_t	VarTypeID;

struct NumStr_st
{
	NumStr_st* next;
	uint32_t	n;
	char *	name;
};

struct enum_st
{
	char	m_name[80];		//	enum  //enum name
	NumStr_st*	m_pfirst;
	char * lookup_itemname(uint32_t n);
};
typedef std::list<enum_st*> Enum_List;

class Enum_mng
{
public:
	Enum_List m_list;
	Enum_mng(){}
	~Enum_mng();

	void Add_New_Enum(enum_st * pnew);
	VarTypeID if_EnumName(const char * &pstr);
};


extern Enum_mng* g_enum_mng;

#endif	//	CEnumMng_H
