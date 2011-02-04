// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#ifndef	CFuncType_H
#define CFuncType_H
#include <list>
#include <vector>
#include <string>
enum enum_CallC
{	//	0 for invalid or unknown
	enum_unknown=	0,
	enum_cdecl	=	1,
	enum_stdcall,
	enum_pascal,
	enum_fastcall,
};	//	calling convention

enum_CallC if_CallC(const char* p);
class Class_st;
class FuncType
{
public:
	enum_CallC	m_callc;
	VarTypeID	m_retdatatype_id;	//	返回值的数据类型
	bool		m_extern_c;
	bool		m_varpar;			//	为TRUE表明它参数可变
	int			m_args;				//	参数个数。如果m_varpar==TRUE，则这是最少个数
									//	not include last "..."
	//	Point to a m_args size of the array is the data type of each parameter
	//	VarTypeID[m_args] *
	std::vector<VarTypeID> m_partypes;
	//Point to a m_args array size is of each parameter name, such as argc, argv
	//	指向一个m_args大小的数组，是各个参数的name,比如argc,argv
	//	char *[m_args] *
	std::vector<std::string> m_parnames;
	std::string	m_pname;
	std::string	m_internal_name;	//can be "_printf", "printf$CRS"
	Class_st*	m_class;			//	if not null, means this func is a subfunc of this class

	FuncType();
	~FuncType();
	void setClass(Class_st* v) {m_class=v;}
	void create_internal_funcname();
	FuncType* ft_clone();

    unsigned char get_stack_purge();
    SIZEOF para_total_size();
    VarTypeID SearchPara(SIZEOF off);
};


typedef std::list<FuncType*> FuncTypeList;


void func_1(FuncType* pfunc,const char* p);
void func_define_2(FuncType* pfunc,const char* &p);


//	--------------------------------------------------------

FuncType* Get_FuncDefine_from_internal_name(const std::string &pmyinternalname);
FuncType* Get_FuncDefine_from_name(const std::string &pmyinternalname);
//	对库函数，是用internal_name，对api，则是用func_name


#endif	//	CFuncType_H
