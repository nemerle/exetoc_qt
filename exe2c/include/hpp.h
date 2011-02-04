// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


//	exe2c project

#ifndef	HPP_H
#define HPP_H
#include <list>
#include <string>
#include "FuncType.h"
//	--------------------------------------------------------

class CCInfo
{
	//When we read CPP program line by line, we need a structure to record the current state
    SIZEOF m_len;	//	The length of m_buf
    char* m_buf;
public:
    int	m_parentheses_level;	//	1 indicates that (), the look forward for a ')'. 2 shows that (())
    int	m_curly_level;	//	1 indicates that (), the look forward for a ')'. 2 shows that (())
    int m_extern_c;	//	indicates that we're inside extern "C" {
	enum_CallC	m_default_callc;


	CCInfo();
	~CCInfo();

	void LoadFile(FILE *f);
	void OneLine(const char * lbuf, const char * &pnext);
    void onTypedef(const char * &p);
	void do_typedef_(VarTypeID baseid, const char * &p);
	FuncType* do_func_proto(const char * pstr);
	FuncType* do_func_proto_void(const char * pstr);
	void do_class(const char * pleft, const char * &pnext);
    static bool LoadIncFile(const std::string &fname);
};

VarTypeID do_enum(const char * &p);

//	--------------------------------------------------------


bool hpp_init();
bool hpp_onexit();

#endif	//	HPP_H
