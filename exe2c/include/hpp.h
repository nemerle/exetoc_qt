// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


//	exe2c project

#ifndef	HPP_H
#define HPP_H
#include <list>
#include <string>
#include "CFuncType.h"
//	--------------------------------------------------------
class define_t
{	//#define src dst
public:
	std::string src;
	std::string dst;
};

typedef std::list<define_t*> DefineList;
	//about this list:
	//	1. allow multi define, that is , it will save both
	//		#define A 1
	//		#define A 2
	//		but if 2 just same define, only once

extern DefineList* g_DefineList;

//	--------------------------------------------------------

class CCInfo
{
	//When we read CPP program line by line, we need a structure to record the current state
public:
	int	comma1;	//	为1表明在()中，期待一个')'。为2表明(())
	int	comma2;	//	为1表明在{}中，期待一个'}'。为2表明{{}}
	int extern_c;	//	表明是在一个extern "C" {}中
	enum_CallC	m_default_callc;

	SIZEOF m_len;	//	是m_buf的长度
	char* m_buf;

	CCInfo();
	~CCInfo();

	void LoadFile(FILE *f);
	void OneLine(const char * lbuf, const char * &pnext);
	void do_typedef(const char * &p);
	void do_typedef_(VarTypeID baseid, const char * &p);
	CFuncType* do_func_proto(const char * pstr);
	CFuncType* do_func_proto_void(const char * pstr);
	void do_class(const char * pleft, const char * &pnext);
};

VarTypeID do_enum(const char * &p);

//	--------------------------------------------------------


bool hpp_init();
bool hpp_onexit();

#endif	//	HPP_H
