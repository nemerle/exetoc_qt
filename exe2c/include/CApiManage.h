// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CApiManage.h
#ifndef CApiManage__H
#define CApiManage__H
#include <list>
#include "DataType.h"
#define	INVALID_STACK	11

typedef	uint32_t	H_NAMEID;
class CApi
{
public:
	BYTE	m_stack_purge;	//7 for invalid
	ea_t	address;
	char name[80];

	CFuncType* m_functype;

    CApi()
    {
        m_stack_purge = INVALID_STACK;
    }

    ~CApi(){}
};

typedef	std::list<CApi*> API_LIST;

class CApiManage
{
public:
	API_LIST*	apilist;

	CApiManage(){ this->apilist = new API_LIST;}    //new_API_LIST
	~CApiManage(){ delete this->apilist; }

	bool	new_api(ea_t address,int stacksub);
	CApi*	get_api(ea_t address);

	void New_ImportAPI(const std::string & pstr, uint32_t apiaddr);
};

extern	CApiManage* g_ApiManage;


#endif // CApiManage__H


