// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CApiManage.h
#ifndef __ApiManage__H
#define __ApiManage__H
#pragma once
#include <list>
#include "DataType.h"

class Api
{
public:
	BYTE	m_stack_purge;	//7 for invalid
	ea_t	address;
	char name[80];

	FuncType* m_functype;

    Api();
    ~Api(){}
};

typedef	std::list<Api*> API_LIST;

class ApiManage
{
	static ApiManage *s_self;
protected:
	ApiManage(){ this->apilist = new API_LIST;}    //new_API_LIST
	~ApiManage(){ delete this->apilist; }
public:
	API_LIST*	apilist;


	bool	new_api(ea_t address,int stacksub);
	Api*	get_api(ea_t address);

	void New_ImportAPI(const std::string & pstr, uint32_t apiaddr);
	static ApiManage *get();
};

#endif // __ApiManage__H
