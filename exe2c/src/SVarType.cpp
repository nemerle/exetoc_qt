// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


//	exe2c project
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cstdio>

#include	"00000.h"

#include	"SVarType.h"

#include	"strparse.h"

void define_replace(char * buf);
VarTypeID do_struct(const char * &p);
VarTypeID do_union(const char * &p);

CVarTypeMng* g_VarTypeManage = NULL;


void CExprManage_cpp_Init()
{
    g_VarTypeManage = new CVarTypeMng;
}
void CExprManage_cpp_exit()
{
    if (g_VarTypeManage)
    {
        delete g_VarTypeManage;
        g_VarTypeManage = NULL;
    }
}

CVarTypeMng::CVarTypeMng()
{
    list = new VarTypeList;
    nextfreeid = 1;

    //	用这些缺省的类型填小于100的编号
    // Fill with the default type of number is less than 100
    NewBaseVarType(0, "void");	//	id = 1 id_void

    NewBaseVarType(1, "BYTE");	//	id = 2
    NewBaseVarType(2, "WORD");	//	id = 3
    NewBaseVarType(4, "DWORD");	//	id = 4
    NewBaseVarType(4, "double");	//	id = 5
    NewBaseVarType(8, "__int64");	//	id = 6

    NewSignedVarType(id_BYTE, "char");	//	id = 7
    NewSignedVarType(id_WORD, "short");
    NewSignedVarType(id_DWORD, "int" );
    NewSignedVarType(id_DWORD, "long" );

    FuncType2VarID((FuncType*)1);
    //New_p(1, "void *");	//	id = 8

    nextfreeid = 50;	//	from 50


}
struct _var_deleter
{
    void operator()(SVarType *p)
    {
        switch (p->type)
        {
        case vtt_funcpoint:
            //???? if (p->m_funcpoint.pFuncType != (FuncType*)1)	//	1 means unknown func:
            //	delete p->m_funcpoint.pFuncType;
            break;
        case vtt_simple:
            break;

        }
        delete p;
    }
};
CVarTypeMng::~CVarTypeMng()
{
    _var_deleter dl;
    std::for_each(list->begin(),list->end(),dl);
    delete list;
    list = NULL;
}


VarTypeID	CVarTypeMng::New_p(VarTypeID id0)
{	//	新建一个数据类型，它指向id0
    // Create a data type, it points to id0
    assert(id0);

    VarTypeID id = nextfreeid++;

    SVarType* pnew = new SVarType;
    pnew->id = id;
    pnew->type = vtt_point;
    pnew->m_point.id_pointto = id0;

    this->list->push_back(pnew);

    return id;
}
VarTypeID	CVarTypeMng::FuncType2VarID(FuncType* ft)
{
    VarTypeID id = nextfreeid++;

    SVarType* pnew = new SVarType;
    pnew->id = id;
    pnew->type = vtt_funcpoint;
    pnew->m_funcpoint.pFuncType = ft;
    this->list->push_back(pnew);

    return id;
}
VarTypeID	CVarTypeMng::Enum2VarID(enum_st* newenum)
{
    VarTypeID id = nextfreeid++;

    SVarType* pnew = new SVarType;
    pnew->id = id;
    pnew->type = vtt_enum;
    pnew->m_enum.m_penum = newenum;

    this->list->push_back(pnew);

    return id;
}
VarTypeID	CVarTypeMng::NewTypeDef(VarTypeID id0, const char * name)
{
    //typedef: the data types need their own VarTypeID

    assert(id0);

    SVarType* p = this->id2_VarType(id0);
    assert(p);
    if (p->type == vtt_class
            && p->m_class.pClass->m_name[0] == '\0')
    {	//	是一个没名的class或struct或union
        //Is not the name of a class or struct or union
        strcpy(p->m_class.pClass->m_name, name);
        return id0;
    }

    VarTypeID id = nextfreeid++;

    SVarType* pnew = new SVarType;
    pnew->id = id;
    pnew->type = vtt_typedef;
    pnew->name = name;
    pnew->m_typedef.id_base = id0;

    this->list->push_back(pnew);

    return id;
}
VarTypeID	CVarTypeMng::NewSignedVarType(VarTypeID id, const char * name)
{
    VarTypeID id1 = nextfreeid++;

    SVarType* pnew = new SVarType;
    pnew->id = id1;
    pnew->type = vtt_signed;

    pnew->m_signed.id_base = id;
    pnew->name = name;

    this->list->push_back(pnew);

    return id1;
}


VarTypeID	CVarTypeMng::NewSimpleVarType(SIZEOF opsize)
{
    VarTypeList::iterator pos = this->list->begin();
    while (pos!=this->list->end())
    {
        SVarType* p = *pos;
        ++pos;
        if (p->type == vtt_simple && p->m_simple.opsize == opsize)
            return p->id;
    }
    //not found, new it
    VarTypeID id = nextfreeid++;

    SVarType* pnew = new SVarType;
    pnew->id = id;
    pnew->type = vtt_simple;

    pnew->m_simple.opsize = opsize;

    this->list->push_back(pnew);

    return id;
}
VarTypeID	CVarTypeMng::NewBaseVarType(SIZEOF opsize, const char * name)
{
    VarTypeID id = nextfreeid++;

    if (id >= 10)
        nop();
    assert(id < 10);
    //  In addition to my own definition of these base classes, the other can only use
    //		New_p
    //		NewTypeDef
    //  To define, impossible to use this

    SVarType* pnew = new SVarType;
    pnew->id = id;
    pnew->type = vtt_base;

    pnew->name = name;
    pnew->m_base.opsize = opsize;

    this->list->push_back(pnew);

    return id;
}
VarTypeID CVarTypeMng::GetBaseID(bool fsign, SIZEOF size)
{
    VarTypeList::iterator pos = this->list->begin();
    while (pos!=list->end())
    {
        SVarType* p = *pos;//list->;
        ++pos;
        if (p->type == vtt_base
                && p->m_base.opsize == size)
        {
            if (fsign)
                return this->Get_signed_id(p->id);
            return p->id;
        }
    }
    //assert(0);
    return 0;
}

void CVarTypeMng::VarType_ID2Name(VarTypeID id, char * namebuf)
{
    SVarType* p = id2_VarType(id);
    switch (p->type)
    {
    case vtt_base:
    case vtt_signed:
    case vtt_typedef:
        strcpy(namebuf,p->name.c_str());
        return;
    case vtt_simple:
        sprintf(namebuf,"bit%d",p->m_simple.opsize * 8);
        return;
    case vtt_point:
        VarType_ID2Name(p->m_point.id_pointto,namebuf);
        strcat(namebuf,"*");
        return;
    case vtt_array:
        VarType_ID2Name(p->m_array.id_arrayitem, namebuf);
        return;
    case vtt_class:
        if (p->m_class.pClass == NULL)
            strcpy(namebuf,p->name.c_str());
        else
            strcpy(namebuf,p->m_class.pClass->m_name);
        return;
    case vtt_enum:
        if (p->m_enum.m_penum == NULL)
            strcpy(namebuf,p->name.c_str());
        else
            strcpy(namebuf,p->m_enum.m_penum->m_name);
        return;
    case vtt_const:
        strcpy(namebuf,"const ");
        VarType_ID2Name(p->m_const.id_base, namebuf+6);
        return;
    default:
        assert(0);
    }
}


VarTypeID CVarTypeMng::NoConst(VarTypeID id)
{
    SVarType* p = id2_VarType(id);
    assert(p);
    if (p->type == vtt_const)
        return p->m_const.id_base;
    return id;
}
VarTypeID CVarTypeMng::GetPointTo(VarTypeID id)
{
    //  If the id is a pointer type, return the type pointed to
    SVarType* p = id2_VarType(id);
    assert(p);
    switch (p->type)
    {
    case vtt_point:
        return NoConst(p->m_point.id_pointto);
        //As const char * refers to char, not const char
    case vtt_const:
        return GetPointTo(p->m_const.id_base);
    case vtt_funcpoint:
    case vtt_signed:
    case vtt_base:
    case vtt_simple:
    case vtt_class:
    case vtt_array:
    case vtt_enum:
        return 0;
    case vtt_typedef:
        return GetPointTo(p->m_typedef.id_base);
    default:
        assert(0);
    }

    return 0;
}
SVarType*	CVarTypeMng::id2_VarType(VarTypeID id)
{
    assert(id);
    assert(id < 2500);
    VarTypeList::iterator pos = list->begin();
    while (pos!=list->end())
    {
        SVarType* p = *pos;//list->;
        ++pos;
        if (p->id == id)
            return p;
    }
    alert_prtf("not find Var id = %d", id);
    assert(0);
    return NULL;	//	why here
}

FuncType* CVarTypeMng::get_funcptr(VarTypeID id)
{
    assert(id);
    SVarType* p = id2_VarType(id);
    assert(p);
    switch (p->type)
    {
    case vtt_const:
        return get_funcptr(p->m_const.id_base);
    case vtt_typedef:
        return get_funcptr(p->m_typedef.id_base);
    case vtt_funcpoint:
        return p->m_funcpoint.pFuncType;
    default:
        return NULL;
    }
}


bool    CVarTypeMng::is_simple(VarTypeID id)
{
    if (id == 0)
        return false;
    SVarType* p = id2_VarType(id);
    assert(p);
    if (p->type == vtt_simple)
        return true;
    return false;
}

Class_st*	CVarTypeMng::is_classpoint(VarTypeID id)
{
    assert(id);
    SVarType* p = id2_VarType(id);
    assert(p);
    switch (p->type)
    {
    case vtt_const:
        return is_classpoint(p->m_const.id_base);
    case vtt_typedef:
        return is_classpoint(p->m_typedef.id_base);
    case vtt_point:
        return id2_Class(p->m_point.id_pointto);
    default:
        return NULL;
    }
}
enum_st*	CVarTypeMng::id2_enum(VarTypeID id)
{
    assert(id);
    SVarType* p = id2_VarType(id);
    assert(p);
    switch (p->type)
    {
    case vtt_const:
        return id2_enum(p->m_const.id_base);
    case vtt_typedef:
        return id2_enum(p->m_typedef.id_base);
    case vtt_enum:
        return p->m_enum.m_penum;
    default:
        return NULL;
    }
}
Class_st*	CVarTypeMng::is_class(VarTypeID id)
{
    if (id == 0)
        return NULL;
    return id2_Class(id);
}

Class_st*	CVarTypeMng::id2_Class(VarTypeID id)
{
    assert(id);
    SVarType* p = id2_VarType(id);
    assert(p);
    switch (p->type)
    {
    case vtt_const:
        return id2_Class(p->m_const.id_base);
    case vtt_typedef:
        return id2_Class(p->m_typedef.id_base);
    case vtt_class:
        return p->m_class.pClass;
    default:
        return NULL;
    }
}
bool	CVarTypeMng::is_funcptr(VarTypeID id)
{
    //	if a pointer to function
    assert(id);
    SVarType* p = id2_VarType(id);
    assert(p);
    switch (p->type)
    {
    case vtt_const:
        return is_funcptr(p->m_const.id_base);
    case vtt_typedef:
        return is_funcptr(p->m_typedef.id_base);
    case vtt_funcpoint:
        return true;
    default:
        return false;
    }
}
SIZEOF CVarTypeMng::VarType_ID2Size(VarTypeID id)
{
    assert(id);
    SVarType* p = id2_VarType(id);
    assert(p);
    switch (p->type)
    {
    case vtt_base:
        return p->m_base.opsize;
    case vtt_simple:
        return p->m_simple.opsize;
    case vtt_point:
    case vtt_funcpoint:
        return BIT32_is_4;
    case vtt_typedef:
        return VarType_ID2Size(p->m_typedef.id_base);
    case vtt_array:
        return VarType_ID2Size(p->m_array.id_arrayitem) * p->m_array.arraynum;
    case vtt_class:
        if (p->m_class.pClass == NULL)
            alert_prtf("need struc size: %s",p->name.c_str());
        return p->m_class.pClass->m_size;
    case vtt_signed:
        return VarType_ID2Size(p->m_signed.id_base);
    case vtt_enum:
        return BIT32_is_4;
    case vtt_const:
        return VarType_ID2Size(p->m_const.id_base);
    default:
        assert(0);
    }
    return 0;
}

VarTypeID CVarTypeMng::NewArray(char * item_name,SIZEOF n)
{
    VarTypeID id = VarType_Name2ID(item_name);
    return NewArray_id_id(id,n);
    /*
        VarTypeID id = if_DataType(item_name);
        if (*item_name == 0)
        {
                SVarType* pv = id2_VarType(id);
                SVarType* pnew = new SVarType;
                *pnew = *pv;
                pnew->id = nextfreeid++;
                pnew->arraynum = n;
                this->list->AddTail(pnew);
                return pnew->id;
        }
        return 0;
        */
}
VarTypeID CVarTypeMng::VarType_Name2ID(const char * name)
{
    //  Borrow the following procedures before it
    VarTypeID id = get_DataType(name);
    if (*name == 0)
        return id;
    return 0;
}

VarTypeID get_DataType_bare(const char * &p)
{	//	with "unsigned char *", only parse "unsigned char"

    if (memcmp(p,"unsigned __int64",16) == 0)
        nop();

    const char * savp = p;
    char buf[80];
    buf[0] = '\0';
    while (*p && buf[0] == '\0')
    {
        get_1part(buf,p);
        if (buf[0] == '\0')
            break;
        define_replace(buf);
    }
    if (strcmp(buf,"__declspec") == 0)
    {	//	skip this
        assert(*p == '(');
        while (*p != ')')
            p++;
        p++;
        skip_eos(p);
        return get_DataType_bare(p);
    }
    if (strcmp(buf,"volatile") == 0)
        return get_DataType_bare(p);


    //  Special to support the struct _NewStruc * name;
    if (strcmp(buf,"struct") == 0)
    {
        VarTypeID id = do_struct(p);
        return id;
    }
    if (strcmp(buf,"union") == 0)
    {
        VarTypeID id = do_union(p);
        return id;
    }
    if (strcmp(buf,"enum") == 0)
    {
        VarTypeID id = 0;	//???? do_enum(p);
        return id;
    }
    if (strcmp(buf,"unsigned") == 0 )
    {
        VarTypeID id = get_DataType_bare(p);
        return g_VarTypeManage->Get_unsigned_id(id);
    }
    else if (strcmp(buf,"signed") == 0)
    {
        VarTypeID id = get_DataType_bare(p);
        return g_VarTypeManage->Get_signed_id(id);
    }
    else if (strcmp(buf,"int") == 0 )
    {
        return id_int;
    }
    else if (strcmp(buf,"long") == 0)
    {
        return id_long;
    }
    else if (strcmp(buf,"short") == 0)
    {
        return id_short;
    }
    else if (strcmp(buf,"__int64") == 0)
    {
        return id_int64;
    }
    else if (strcmp(buf,"double") == 0)
    {
        return id_double;
    }
    else if (strcmp(buf,"const") == 0)
    {
        VarTypeID id = get_DataType_bare(p);
        assert(id);
        return g_VarTypeManage->GetConstOfID(id);
    }
    const char * p1 = buf;
    VarTypeID id = g_VarTypeManage->FirstDataType(p1);
    if (id == 0)
        id = g_ClassManage->if_StrucName(p1);
    if (id == 0)
        id = g_enum_mng->if_EnumName(p1);
    if (id == 0)
        p = savp;

    return id;
}
VarTypeID Get_Additional_id(VarTypeID baseid, const char * &p)
{
    //	对 "unsigned char *", 已经知道 "unsigned char" 的id,
    //	期望 p 指的是一个 "*" 或 "**" 之类
    //  On the "unsigned char *", have known "unsigned char" in the id,
    //  p refers to expect a "*" or "**" and the like
    VarTypeID id = baseid;
    for (;;)
    {
        if (*p == ' ')
            p++;
        else if (*p == '*')
        {
            p++;
            id = g_VarTypeManage->GetAddressOfID(id);
        }
        else if (memcmp(p,"far ",4) == 0)
            p += 4;
        else if (memcmp(p,"FAR ",4) == 0)
            p += 4;
        else if (memcmp(p,"near ",5) == 0)
            p += 5;
        else if (memcmp(p,"NEAR ",5) == 0)
            p += 5;
        else
        {
            skip_eos(p);
            return id;
        }
    }
}
VarTypeID get_DataType(const char * &p)
{
    VarTypeID id = get_DataType_bare(p);
    if (id == 0)
        return 0;

    for (;;)
    {
        if (*p == ' ')
            p++;
        else if (*p == '*')
        {
            p++;
            id = g_VarTypeManage->GetAddressOfID(id);
        }
        else if (memcmp(p,"const",5) == 0 && if_split_char(p[5]))
        {
            id = g_VarTypeManage->GetConstOfID(id);
            p += 5;
        }
        else
            break;
    }
    return id;
}

VarTypeID CVarTypeMng::GetConstOfID(VarTypeID id)
{
    VarTypeList::iterator pos = list->begin();
    while (pos!=list->end())
    {
        SVarType* p = *pos;//list->;
        ++pos;
        if (p->type == vtt_const && p->m_const.id_base == id)
            return p->id;
    }
    //	not find, create one

    assert(id);

    VarTypeID idnew = nextfreeid++;

    SVarType* pnew = new SVarType;

    pnew->id = idnew;	//
    pnew->type = vtt_const;
    pnew->m_const.id_base = id;

    this->list->push_back(pnew);

    return idnew;
}

VarTypeID CVarTypeMng::FirstDataType(const char * &pattern)
{
    assert(list);

    assert(pattern);

    VarTypeList::iterator pos = list->begin();
    while (pos!=list->end())
    {
        SVarType* pv = *pos;//list->;
        ++pos;
        std::string name = "";
        if (pv->type == vtt_base)
            name = pv->name;
        else if (pv->type == vtt_typedef)
            name = pv->name;
        else if (pv->type == vtt_signed)
            name = pv->name;
        else
            continue;

        int n = name.size();
        if (memcmp(name.c_str(),pattern,n))
            continue;
        // Translated
        // v1:Only the first part of the same issues can not explain
        // v2:Is the same as the previous section, can not explain the problem
        if (if_split_char(pattern[n]))
        {
            pattern += n;
            return pv->id;
        }
    }
    return 0;
}

bool	CVarTypeMng::If_Based_on_idid(VarTypeID id, VarTypeID id0)
{
    //	check if id is based on id0

    if (id == id0)
        return true;
    SVarType* p = id2_VarType(id);
    assert(p);
    switch (p->type)
    {
    case vtt_const:
        return If_Based_on_idid(p->m_const.id_base, id0);
    case vtt_typedef:
        return If_Based_on_idid(p->m_typedef.id_base, id0);
    default:
        return false;
    }
}
bool	CVarTypeMng::If_Based_on(VarTypeID id, char * basename)
{
    VarTypeID id0 = this->VarType_Name2ID(basename);
    assert(id0);

    return this->If_Based_on_idid(id, id0);
}

VarTypeID CVarTypeMng::Get_unsigned_id(VarTypeID id)
{
    SVarType* p = id2_VarType(id);
    assert(p);
    switch (p->type)
    {
    case vtt_base:
        return id;
    case vtt_signed:
        return p->m_signed.id_base;
    default:
        assert(0);
    }
    return 0;
}

VarTypeID CVarTypeMng::Get_signed_id(VarTypeID id)
{
    SVarType* p = id2_VarType(id);
    assert(p);
    if (p->type == vtt_signed)
        return id;

    VarTypeList::iterator pos = list->begin();
    while (pos!=list->end())
    {
        SVarType* pv = *pos;//list->;
        ++pos;
        if (pv->type == vtt_signed && pv->m_signed.id_base == id)
            return pv->id;
    }
    //assert(0);
    return id;
}
VarTypeID CVarTypeMng::Class2VarID(Class_st *pclass)
{
    VarTypeList::iterator pos = list->begin();
    while (pos!=list->end())
    {
        SVarType* p = *pos;//list->;
        ++pos;
        if (p->type == vtt_class)
        {
            if (p->m_class.pClass == pclass)
                return p->id;
            if (p->m_class.pClass == NULL
                    && p->name.compare(pclass->m_name) == 0)
            {	//	for unknown struc
                p->m_class.pClass = pclass;
                return p->id;
            }
        }
    }

    SVarType* pnew = new SVarType;
    pnew->id = nextfreeid++;
    pnew->type = vtt_class;
    pnew->m_class.pClass = pclass;

    this->list->push_back(pnew);
    return pnew->id;
}
VarTypeID CVarTypeMng::NewArray_id_id(VarTypeID id0, SIZEOF n)
{
    // Is already char's id, generate a char kkk [] for id
    VarTypeList::iterator pos = list->begin();
    while (pos!=list->end())
    {
        SVarType* p = *pos;//list->;
        ++pos;
        if (p->type == vtt_array
                && p->m_array.id_arrayitem == id0
                && p->m_array.arraynum == n)
            return p->id;
    }
    //	not find, create one

    assert(id0);

    VarTypeID id = nextfreeid++;

    SVarType* pnew = new SVarType;
    pnew->id = id;
    pnew->type = vtt_array;
    pnew->m_array.id_arrayitem = id0;
    pnew->m_array.arraynum = n;

    this->list->push_back(pnew);
    return id;
}
VarTypeID CVarTypeMng::GetArrayItemID(VarTypeID id)
{
    //	if it is a array, return item datatype
    SVarType* p = id2_VarType(id);
    if (p->type == vtt_array)
        return p->m_array.id_arrayitem;
    return id;
}
VarTypeID CVarTypeMng::NewUnknownStruc(const char * strucname)
{
    VarTypeList::iterator pos = list->begin();
    while (pos!=list->end())
    {
        SVarType* p = *pos;//list->;
        ++pos;
        if (p->type == vtt_class)
        {
            if (p->name==strucname)
                return p->id;
            if (p->m_class.pClass != NULL)
                if (strcmp(p->m_class.pClass->m_name,strucname) == 0)
                    return p->id;
        }
        if (p->type == vtt_typedef && p->name==strucname)
        {
            Class_st* cls = id2_Class(p->m_typedef.id_base);
            if (cls)
                return p->id;
        }
    }


    SVarType* pnew = new SVarType;
    pnew->type = vtt_class;
    pnew->id = nextfreeid++;
    pnew->name=strucname;
    pnew->m_class.pClass = NULL;

    this->list->push_back(pnew);
    return pnew->id;
}
VarTypeID CVarTypeMng::GetAddressOfID(VarTypeID id)
{
	//It returns an type id of pointer to type 'id'
	// so for unsigned long it returns unsigned long *
	VarTypeList::iterator pos = list->begin();
	while (pos!=list->end())
	{
		SVarType* p = *pos;//list->;
		++pos;
		if (p->type == vtt_point && p->m_point.id_pointto == id)
			return p->id;
	}
	//	not found, create one

	return this->New_p(id);
}

//	-------------------------------------------------------


SVarType* GG_id2_VarType(VarTypeID id)
{
    return g_VarTypeManage->id2_VarType(id);
}

std::string	GG_VarType_ID2Name(VarTypeID id)
{
    char buf[128];
    g_VarTypeManage->VarType_ID2Name(id,buf);
    return (std::string)buf;
}

SIZEOF	GG_VarType_ID2Size(VarTypeID id)
{
    return g_VarTypeManage->VarType_ID2Size(id);
}

bool	GG_is_funcpoint(VarTypeID id)
{
    return g_VarTypeManage->is_funcptr(id);
}
FuncType* GG_get_funcpoint(VarTypeID id)
{
    return g_VarTypeManage->get_funcptr(id);
}

Class_st*	GG_id2_Class(VarTypeID id)
{
    return g_VarTypeManage->id2_Class(id);
}
