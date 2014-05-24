// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include "00000.h"
#include "ClassManage.h"

#include	"hpp.h"
#include	"SVarType.h"
//#include	"Func.h"
#include "strparse.h"

#include <cassert>
#include <cstring>
#include <cstdio>
#include <algorithm>

#include <QtCore/QtAlgorithms>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/bind.hpp>

using namespace boost::lambda;

ClassManage* ClassManage::s_ClassManage = NULL;
ClassManage* ClassManage::get()
{
    if(0==s_ClassManage)
        s_ClassManage=new ClassManage;
    return s_ClassManage;
}
VarTypeID ClassManage::if_StrucName(const char * &pstr)
{
    CLASS_LIST::iterator pos = this->m_classlist.begin();
    for (;pos!=m_classlist.end();++pos)
    {
        Class_st* p = *pos;
        int n = strlen(p->getname());
        if (memcmp(p->getname(),pstr,n))
            continue;
        if (if_split_char(pstr[n]))
        {
            pstr += n;
            return VarTypeMng::get()->Class2VarID(p);
        }
    }
    return 0;
}
//	----------------------------------

ClassManage::ClassManage()
{
}
ClassManage::~ClassManage()
{
    qDeleteAll(m_classlist.begin(),m_classlist.end());
    m_classlist.clear();
}
void ClassManage::add_class(Class_st* pnew)
{
    m_classlist.push_back(pnew);
}

Class_st* ClassManage::LoopUp_class_by_name(const char * name)
{
    CLASS_LIST::iterator pos = this->m_classlist.begin();
    for (;pos!=m_classlist.end();++pos)
    {
        Class_st* p = *pos;
        if (p->IfThisName(name))
            return p;
    }
    return NULL;
}
FuncType* ClassManage::Get_SubFuncDefine_from_name(const char * classname, const char * funcname)
{
    Class_st* pclass = this->LoopUp_class_by_name(classname);
    if (pclass == NULL)
        return NULL;

    return pclass->LookUp_SubFunc(funcname);
}


Class_st::Class_st() : m_TclassFstruc(false),
    m_size(0),
    m_Fstruc_Tunion(false),
    m_Vftbl(0)
{
    m_name[0]=0;
}

Class_st::~Class_st()
{
    m_DataItems.clear();
    qDeleteAll(m_SubFuncs.begin(),m_SubFuncs.end());
    m_SubFuncs.clear();
}

void Class_st::set_subfuncs()
{
    std::for_each(m_SubFuncs.begin(),m_SubFuncs.end(), boost::lambda::bind(&FuncType::setClass,_1, this));
    for (size_t i=0; i<m_SubFuncs.size(); i++)
    {
        assert(m_SubFuncs[i]->m_class == this);
    }
}

bool Class_st::IfThisName(const char * name) const
{
    if (strcmp(m_name, name) == 0)
        return true;
    return false;
}

FuncType* Class_st::LookUp_SubFunc(const char * name)
{
    FuncType* p;
    for (size_t i=0; i<m_SubFuncs.size(); i++)
    {
        p = m_SubFuncs[i];
        if(p->m_pname.compare(name) == 0)
            return p;
    }
    return NULL;
}
bool	Class_st::is_Constructor(const FuncType* pft) const //is a constructor
{
    return pft->m_pname.compare(this->m_name)==0;
}
bool	Class_st::is_Destructor(const FuncType* pft) const //destructor
{
    return (pft->m_pname[0] == '~');
}
//constructor or destructor
bool	Class_st::is_ConstructOrDestruct(const FuncType* pft) const
{
    return is_Constructor(pft) || is_Destructor(pft);
}

/*
void CFunc::ClassSubFuncProcess()
{
    if (m_ftype == NULL)
        return;
    if (m_ftype->m_class == NULL)
        return;

    VarTypeID id = CVarTypeMng::get()->Class2VarID(m_ftype->m_class);
    id = CVarTypeMng::get()->GetAddressOfID(id);

    this->m_instrs->Fill_this_ECX(id);
}*/

/*
void	Class_st::prtout(CXmlPrt* prt)
{
    prt->XMLbegin(XT_Keyword, 0);
    if (this->m_Fstruc_Tunion)
        prt->prtt("union");
    else
        prt->prtt("struct");
    prt->XMLend(XT_Keyword);

    prt->XMLbegin(XT_Class, this);
    prt->prtt(this->m_name);
    prt->XMLend(XT_Class);

    prt->prtt("\n");

    prt->XMLbegin(XT_K1, 0);
    prt->prtt("{");
    prt->XMLend(XT_K1);

    prt->prtt("    ");

    prt->XMLbegin(XT_Comment, this);
    prt->prtf("//sizeof = 0x%x",this->m_size);
    prt->XMLend(XT_Comment);

    prt->prtt("\n");

    prt->ident_add1();
    for (int i=0;i<this->m_nDataItem;i++)
    {
        st_Var_Declare* pv = &this->m_DataItems[i];
        if (pv->m_access == nm_sub_end)
            prt->ident_sub1();
        prt->ident();

        if (pv->m_access == nm_substruc)
        {
            prt->XMLbegin(XT_Keyword, 0);
            prt->prtt("struct");
            prt->XMLend(XT_Keyword);

            prt->XMLbegin(XT_K1, 0);
            prt->prtt("{");
            prt->XMLend(XT_K1);

            prt->prtt("\n");

            prt->ident_add1();
            continue;
        }
        if (pv->m_access == nm_subunion)
        {
            prt->XMLbegin(XT_Keyword, 0);
            prt->prtt("union");
            prt->XMLend(XT_Keyword);

            prt->XMLbegin(XT_K1, 0);
            prt->prtt("{");
            prt->XMLend(XT_K1);

            prt->prtt("\n");

            prt->ident_add1();
            continue;
        }
        if (pv->m_access == nm_sub_end)
        {
            prt->XMLbegin(XT_K1, 0);
            prt->prtt("}");
            prt->XMLend(XT_K1);
            if (pv->m_name[0])
            {
                //any XmlType ?
                prt->prtt(pv->m_name);
            }
            prt->prtt(";\n");
            continue;
        }

        prt->XMLbegin(XT_DataType, 0);
        prt->prtt(GG_VarType_ID2Name(pv->m_vartypeid));
        prt->XMLend(XT_DataType);

        prt->prtt("\t");

        prt->prtt(pv->m_name);

        SVarType* pvt = GG_id2_VarType(pv->m_vartypeid);
        if (pvt && pvt->type == vtt_array)
        {
            prt->XMLbegin(XT_K1, 0);
            prt->prtt("[");
            prt->XMLend(XT_K1);

            prt->XMLbegin(XT_Number, 0);
            prt->prtf("%d", pvt->m_array.arraynum);
            prt->XMLend(XT_Number);

            prt->XMLbegin(XT_K1, 0);
            prt->prtt("]");
            prt->XMLend(XT_K1);
        }
        prt->prtt(";\t");

        prt->XMLbegin(XT_Comment, this);
        prt->prtf("//+%02x",pv->m_offset_in_struc);
        prt->XMLend(XT_Comment);

        prt->prtt("\n");

    }
    prt->XMLbegin(XT_K1, 0);
    prt->prtt("}");
    prt->XMLend(XT_K1);

    prt->prtt("\n");

}*/
void	ClassManage::new_struc(Class_st* pnew)
{
    this->m_classlist.push_front(pnew);
}

const char *	Class_st::getclassitemname(uint32_t off)
{
    st_Var_Declare* p = this->GetClassItem(off);
    if (p == NULL)
        return "??a??";
    SVarType* psvt = GG_id2_VarType(p->m_vartypeid);
    if (psvt->type == vtt_array)
    {
        SIZEOF sz = GG_VarType_ID2Size(psvt->m_array.id_arrayitem);
        int n = (off - p->m_offset_in_struc) / sz;
        static char buf[128];
        sprintf(buf,"%s[%d]",p->m_name,n);
        return buf;
    }
    return p->m_name;
}
st_Var_Declare* Class_st::GetClassItem(uint32_t off)
{
    assert(m_DataItems.size() < 0x1000);
    for (size_t i=0; i<m_DataItems.size(); i++)
    {
        st_Var_Declare &p(m_DataItems[i]);
        if (p.m_offset_in_struc == off)
            return &p;
        if (p.m_offset_in_struc <= off && (p.m_offset_in_struc + p.m_size) > off)
            return &p;
    }
    return 0;
}

const char *	Class_st::getname() const
{
    return m_name;
}
