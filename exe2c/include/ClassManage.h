// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


#ifndef CClassManage_H
#define CClassManage_H
#include <list>
#include <stdint.h>
typedef uint32_t ea_t;

//#include "../CXmlPrt/CXmlPrt.h"
#include "FuncType.h"

enum enumClassMemberAccess
{
        nm_unknown	=	0,
        nm_private,
        nm_protected,
        nm_public,

        nm_substruc,
        nm_subunion,
        nm_sub_end,
};

struct st_Var_Declare
{	//	是对一个量的定义，包括数据类型和变量名
        //Is a quantitative definition, including data type and variable name
        //	可用于struct_struct中的item定义，和functoin parameter的定义等等
        //Can be used to struct_struct the item definitions, and function parameter definitions, etc.
        VarTypeID	m_vartypeid;
        SIZEOF		m_size;
        uint32_t		m_offset_in_struc;
        char		m_name[80];
        enumClassMemberAccess	m_access;	//	为class预留 //Reserved for the class
        st_Var_Declare():m_vartypeid(0),m_size(0),m_offset_in_struc(0),m_access(nm_unknown)
        {
            m_name[0]=0;
        }
};

//	----------------------------------------------------
//	class

class Class_st
{
public:
        bool	m_TclassFstruc;	//	TRUE means class, FALSE means struct
        char	m_name[80];		//	class名 // class name
        SIZEOF	m_size;			//	class的size,数据部分 //Class's size, the data part of the
        int		m_nDataItem;	//	数据量的个数 //The amount of data the number of
        st_Var_Declare* m_DataItems;	//	sizeof = m_nDataItem 的一个buffer//m_nDataItem of a buffer
        bool	m_Fstruc_Tunion;	//TRUE = union

        ea_t	m_Vftbl;		//	虚表的地址，如果有的话 //Virtual table addresses, if any,
        int		m_nSubFuncs;		//	子函数的个数 //number of subroutines
        FuncType**		m_SubFuncs;	//	各个子函数 //subroutine array

public:
        Class_st();
        ~Class_st();

        FuncType* LookUp_SubFunc(const char * name);
        bool	is_GouZ(FuncType* pft);        //是构造 // Is to construct
        bool	is_GouX(FuncType* pft);        //是构析 //Is a structure analysis
        bool	is_GouZX(FuncType* pft);       //是构造或构析 //Is to construct or conformation analysis
        const char *	getclassitemname(uint32_t off);
        st_Var_Declare* GetClassItem(uint32_t off);
//	void	prtout(CXmlPrt* prt);
        void	set_subfuncs();
        bool	IfThisName(const char * name);
        const char *	getname();
};

typedef	std::list<Class_st *> CLASS_LIST;

class ClassManage
{
        CLASS_LIST	m_classlist;
public:

        ClassManage();
        ~ClassManage();

        FuncType* Get_SubFuncDefine_from_name(const char * classname, const char * funcname);
        void add_class(Class_st* pnew);
        Class_st* LoopUp_class_by_name(const char * name);

        VarTypeID if_StrucName(const char * &p);
        void	new_struc(Class_st* pnew);
};


extern	ClassManage* g_ClassManage;

#endif	//	CClassManage_H
