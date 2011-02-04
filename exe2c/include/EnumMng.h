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
    char * lookup_itemname(uint32_t n) const;
};

class Enum_mng
{
private:
    typedef std::list<enum_st*> Enum_List;
	Enum_List m_list;
    static Enum_mng* s_enum_mng;
	Enum_mng(){}
	~Enum_mng();
public:
    static Enum_mng *get();
	void Add_New_Enum(enum_st * pnew);
	VarTypeID if_EnumName(const char * &pstr);
};



#endif	//	CEnumMng_H
