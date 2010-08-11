// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


//	exe2c project

//#include "stdafx.h"
#include <cstring>
#include <cassert>
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/bind.hpp>
//#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <cstring>

#include "00000.h"
#include "hpp.h"

#include "CClassManage.h"
#include "CEnumMng.h"
#include "CCbuf.h"
#include "strparse.h"
#include "SVarType.h"

/*
#include "../pub/strparse.h"
#include "../CCbuf/CCbuf.h"
#include "../FileLoad/FileLoad.h"

#include "../CEnumMng/CEnumMng.h"
#include "../CVarTypeMng/SVarType.h"
  */

//#include "io.h"
using namespace std;
using namespace boost::lambda;
using namespace boost::filesystem;
class CHpp
{
public:
        FuncTypeList* m_FuncTypeList;	//	保存从.h得来的全局函数定义

        CHpp();
        ~CHpp();
        void newfunc_addlist(CFuncType* pnewfunc);
        void func_define(const char * lbuf, CCInfo *p);
        CFuncType* Get_FuncDefine_from_internal_name_(const std::string & pmyinternalname);
        CFuncType* Get_FuncDefine_from_name_(const std::string &pmyname);
};

CHpp* g_Hpp = NULL;

VarTypeID Get_Var_Declare(const char * &p, char * name);

DefineList* g_DefineList = NULL;


bool LoadIncFile(const std::string &fname);
void prt_defines();
std::string get_define(char * partern);
void define_replace(char * buf);

path g_incpath;
CHpp::CHpp()
{
        m_FuncTypeList = new FuncTypeList;
}
CHpp::~CHpp()
{
        if (m_FuncTypeList)
        {
            for_each(m_FuncTypeList->begin(),m_FuncTypeList->end(), bind(delete_ptr(), _1));
            delete m_FuncTypeList;
            m_FuncTypeList = NULL;
        }
}
path GetMyExePath()
{
    return current_path();
}
bool hpp_init()
{

        g_Hpp = new CHpp;

        g_DefineList = new DefineList;
        g_ClassManage = new CClassManage;
        g_enum_mng = new Enum_mng;
        path current_dir=GetMyExePath();
        current_dir = current_dir/"inc";
        g_incpath = current_dir.native_directory_string();

//	if (g_EXEType == enum_PE_sys)
//		strcat(g_incpath, "\\ntddk\\");

        LoadIncFile("my.h");

        return true;
}

bool hpp_onexit()
{
    delete g_Hpp;
    g_Hpp = NULL;

    delete g_enum_mng;
    if (g_DefineList)
    {
        for_each(g_DefineList->begin(),g_DefineList->end(),bind(delete_ptr(), _1));
        delete g_DefineList;
        g_DefineList = NULL;
    }


    if (g_ClassManage)
    {
        delete g_ClassManage;
        g_ClassManage = NULL;
    }

    return true;
}

//	----------------------------------------------------------
CCInfo::CCInfo()
{
        this->comma1 = 0;
        this->comma2 = 0;
        this->extern_c = 0;
        this->m_default_callc = enum_stdcall;

        this->m_len = 0;
        this->m_buf = NULL;
}

CCInfo::~CCInfo()
{
        if (this->m_buf)
        {
                delete this->m_buf;
                this->m_buf = NULL;
        }
}

void CCInfo::LoadFile(FILE *f)
{
        CCbuf ccbuf;
        ccbuf.LoadFile(f);
        this->m_buf = ccbuf.m_p;
        this->m_len = ccbuf.m_len;
}
bool LoadIncFile(const std::string &fname)
{
    path file_path;
    file_path=g_incpath/fname;
    std::string nativepath=file_path.native_file_string();
    //alert_prtf("begin file %s\n",fname);
    FILE* f = fopen(nativepath.c_str(),"rb");
    if (f == NULL)
    {
        alert_prtf("File open error: %s",nativepath.c_str());
        return false;
    }

    CCInfo *pInfo = new CCInfo;

    pInfo->LoadFile(f);

    fclose(f);

    const char * p = pInfo->m_buf;
    const char * plast = p + pInfo->m_len;
    while (p < plast)
    {
        const char * pnext = NULL;
        pInfo->OneLine(p, pnext);	//	一般情况下，OneLine不会动pnext
        //	如果它是多行，就会把pnext指向最后
//        Under normal circumstances, OneLine does not move pnext
//        If it is more than one line, putting the final point pnext
        if (pnext == NULL)
            p += strlen(p) + 1;
        else
        {
            assert( memcmp(pnext,"E ",2) );
            p = pnext;
        }
    }

    assert(pInfo->comma1 == 0);
    assert(pInfo->comma2 == 0);
    assert(pInfo->extern_c == 0);
    delete [] pInfo->m_buf;
    pInfo->m_buf = NULL;
    delete pInfo;

    //printf("end   file %s\n",fname);
    return true;
}
void LoadIncBuffer(const char * p,char * plast)
{
        CCInfo* pInfo = new CCInfo;

        while (p < plast)
        {
                const char * pnext = NULL;
                pInfo->OneLine(p, pnext);	//	一般情况下，OneLine不会动pnext
                                                                        //	如果它是多行，就会把pnext指向最后
                if (pnext == NULL)
                {
                        p += strlen(p) + 1;
                }
                else
                {
                        p = pnext;
                }
        }

        delete pInfo;
}
void prt_defines()
{
    DefineList::iterator pos = g_DefineList->begin();
    while (pos!=g_DefineList->end())
    {
        define_t* p = *pos;//g_DefineList->;
        ++pos;
        p;
        //printf("#define | %s | %s\n",p->src,p->dst);
    }
}
void do_define(char * p1,const char * p2)
{	//	means #define p1 p2
        //printf("I find #define %s == %s\n",p1,p2);
        DefineList::iterator pos = g_DefineList->begin();
        while (pos!=g_DefineList->end())
        {
                define_t* p = *pos;//g_DefineList->;
                ++pos;
                if (p->src.compare(p1) == 0)
                {
                        if (p->dst.compare(p2) == 0)
                                return;
                        break;
                }
        }
        //	not found
        define_t* pnew = new define_t;
        pnew->src = p1;
        pnew->dst = p2;
        if (pos!=g_DefineList->end())
                g_DefineList->insert(pos,pnew);
        else
                g_DefineList->push_back(pnew);
}
//Handle all lines beginning with #
void One_Line_pre(const char * lbuf)
{
    const char * p = lbuf;
    assert(*p == '#');
    p++;

    if (memcmp(p,"define",6) == 0)
    {	//	#define find
        p+=6;
        skip_space(p);
        assert(*p);	//#define 后面不能什么也没有

        char buf[280];
        get_1part(buf,p);
        skip_space(p);
        do_define(buf,p);
        return;
    }
    if (memcmp(p,"include",7) == 0)
    {
        p += 7;
        skip_space(p);
        if (*p == '\"' || *p == '<')
        {
            char path[80];
            p++;
            int i=0;
            while (*p != '\"' && *p != '>')
            {
                path[i++] = *p++;
            }
            path[i] = '\0';
            log_prtf("Load include file: %s\n",path);
            LoadIncFile(path);
        }
        return;
    }
}

void skip_string(char c1, const char * & p)
{
    //c1 is ' or "
    for(;;)
    {
        char c = *p++;
        if (c == '\\')
        {
            p++;
            continue;
        }
        assert(c != '\0');
        if (c == c1)
            return;
    }
}
bool Read_Var_Declare(const char * pstr, st_Var_Declare* pvardcl)
{
        VarTypeID id = Get_Var_Declare(pstr, pvardcl->m_name);
        if (id == 0)
        {
                alert("Read Struct Item error");
                return false;
        }
        pvardcl->m_size = GG_VarType_ID2Size(id);
        pvardcl->m_vartypeid = id;
        return true;
}
void log_display_structure(Class_st* p)
{
        log_prtt("struct ");
        log_prtt(p->m_name);
        log_prtf("\n{    \\\\sizeof = 0x%x\n",p->m_size);
        int nident = 1;
        for (int i=0;i<p->m_nDataItem;i++)
        {
                st_Var_Declare* pv = &p->m_DataItems[i];
                if (pv->m_access == nm_sub_end)
                        nident--;
                for (int j=0;j<nident;j++)
                        log_prtt("    ");
                if (pv->m_access == nm_substruc)
                {
                        log_prtt("struct {\n");
                        nident++;
                        continue;
                }
                if (pv->m_access == nm_subunion)
                {
                        log_prtt("union {\n");
                        nident++;
                        continue;
                }
                if (pv->m_access == nm_sub_end)
                {
                        log_prtt("} ");
                        if (pv->m_name[0])
                                log_prtt(pv->m_name);
                        log_prtt(";\n");
            continue;
                }

                log_prtl("%s\t%s;\t//+%02x",
                                 GG_VarType_ID2Name(pv->m_vartypeid),
                                 pv->m_name,
                                 pv->m_offset_in_struc);
        }
        log_prtt("}\n");
}
VarTypeID do_struct_after_name(const char * strucname, const char * &p, bool Fstruc_Tunion);
VarTypeID do_struct(const char * &p)
{
        const char * savp = p;

        char name[80];
        get_1part(name,p);

        skip_eos(p);

        if (*p == '{')
                return do_struct_after_name(name,p,false);
        VarTypeID id = g_VarTypeManage->NewUnknownStruc(name);
        return id;
}
VarTypeID do_union(const char * &p)
{
        const char * savp = p;

        char name[80];
        get_1part(name,p);

        skip_eos(p);

        if (*p == '{')
                return do_struct_after_name(name,p,true);
        VarTypeID id = g_VarTypeManage->NewUnknownStruc(name);
        return id;
}
VarTypeID do_struct_after_name(const char * strucname, const char * &p, bool Fstruc_Tunion)
{
        assert(*p == '{');
        p++;
        skip_eos(p);

        int n = 0;
        st_Var_Declare items[150];	//50个，够了吗
        SIZEOF size = 0;


        bool f_tbl[20];
        bool f = Fstruc_Tunion;	//false means struct, true means union
        SIZEOF size_tbl[20];
        SIZEOF maxsize_tbl[20];
        SIZEOF maxsize_in_union = 0;
        int	substruc_stack = 0;

        while (*p != '}' || substruc_stack != 0)
        {
                assert(n < 150);
                st_Var_Declare* pvar = &items[n];
                {
                        const char * savp = p;
                        char buf[80];
                        get_1part(buf,p);
                        skip_eos(p);
                        if (strcmp(buf,"struct") == 0 && *p == '{')
                        {	//	这是一个struct 中的struct定义
                                size_tbl[substruc_stack] = size;
                                maxsize_tbl[substruc_stack] = maxsize_in_union;
                                f_tbl[substruc_stack++] = f;
                                assert(substruc_stack<20);
                                f = false;
                                pvar->m_access = nm_substruc;
                                n++;

                                p++;
                                skip_eos(p);
                                continue;
                        }
                        if (strcmp(buf,"union") == 0 && *p == '{')
                        {	//	这是一个struct 中的union定义
                                size_tbl[substruc_stack] = size;
                                maxsize_tbl[substruc_stack] = maxsize_in_union;
                                f_tbl[substruc_stack++] = f;
                                assert(substruc_stack<20);
                                f = true;
                                maxsize_in_union = size;
                                pvar->m_access = nm_subunion;
                                n++;

                                p++;
                                skip_eos(p);
                                continue;
                        }

                        p = savp;
                }
                if (substruc_stack > 0 && *p == '}')
                {
                        if (f)	//union
                        {
                                size = maxsize_in_union;
                        }
                        substruc_stack--;
                        f = f_tbl[substruc_stack];
                        if (f)
                        {
                                maxsize_in_union = maxsize_tbl[substruc_stack];
                                if (size > maxsize_in_union)
                                        maxsize_in_union = size;
                                size = size_tbl[substruc_stack];	//	退回原来的size
                        }
                        pvar->m_access = nm_sub_end;
                        n++;

                        p++;
                        skip_eos(p);
                        if (*p != ';')
                        {	//	说明有名的
                                get_1part(pvar->m_name,p);
                                skip_eos(p);
                                assert(*p == ';');
                        }
                        p++;
                        skip_eos(p);
                        continue;
                }
                Read_Var_Declare(p,pvar);
                pvar->m_offset_in_struc = size;

                //alert_prtf("var_item %s",pvar->m_name);

                if (!f)
                        size += pvar->m_size;
                else
                {	//union
                        if (size + pvar->m_size > maxsize_in_union)
                                maxsize_in_union = size + pvar->m_size;
                }
                n++;
                p += strlen(p)+1;
        }


        p++;
        skip_space(p);
        if (*p == ';')
        {
                p++;
                skip_eos(p);
        }

        //	------------------------------------------
        Class_st* pnew = new Class_st;
        strcpy(pnew->m_name, strucname);
        pnew->m_nDataItem = n;
        pnew->m_DataItems = new st_Var_Declare[n];
        memcpy(pnew->m_DataItems, items, sizeof(st_Var_Declare)*n);
        if (Fstruc_Tunion)
                pnew->m_size = maxsize_in_union;
        else
                pnew->m_size = size;
        pnew->m_Fstruc_Tunion = Fstruc_Tunion;

        log_display_structure(pnew);

        g_ClassManage->new_struc(pnew);

        return g_VarTypeManage->Class2VarID(pnew);
}
void CCInfo::OneLine(const char * lbuf, const char * &pnext)
{
    //	已经把所有的双空格变成了单空格
    //	已经去掉了最后的换行符
    //	已经把\连接的行接起来了
    //Has all the double spaces into single space
    // End of line breaks have been removed
    //Have to \ pick up the line to connect
    //printf("%s\n",lbuf);
    //return;

        const char * p = lbuf;
        assert(p);
        skip_space(p);

        if (*p == '#')
        {	//	它总是单行的
            //It is always a single line
            //FIXME not true
                One_Line_pre(p);
                return;
        }
        //_CRTIMP int __cdecl printf(const char *, ...);
        const char * p1 = p;
        char part1[80];
        get_1part(part1,p1);
        skip_eos(p1);

        if (strcmp(part1,"__inline") == 0)
        {
                pnext = p1;
                OneLine(p1,pnext);
                return;
        }
        if (strcmp(part1,"extern") == 0)
        {
                char part2[80];
                assert(*p1);
                const char * p2 = p1;
                get_1part(part2,p2);
                if (strcmp(part2,"\"C\"") == 0)
                {	//extern "C"
                        //printf("extern C find\n");
                        assert(*p2);
                        if (*p2 == '{')
                        {
                                assert(this->comma1 == 0);
                                assert(this->comma2 == 0);
                                this->extern_c++;
                                this->OneLine(p2+1, pnext);
                                return;
                        }
                        else
                        {	//	just 1 line
                                this->extern_c++;
                                this->OneLine(p2, pnext);
                                this->extern_c--;
                                return;
                        }
                }
        }
        else if (strcmp(part1,"typedef") == 0)
        {
                do_typedef(p1);
                pnext = p1;
                return;
        }
        else if (strcmp(part1,"struct") == 0)
        {
                do_struct(p1);
                pnext = p1;
                return;
        }
        else if (strcmp(part1,"union") == 0)
        {
                do_union(p1);
                pnext = p1;
                return;
        }
        else if (strcmp(part1,"class") == 0)
        {
                do_class(p1,pnext);
                return;
        }
        else if (strcmp(part1,"enum") == 0)
        {
                do_enum(p1);
                pnext = p1;
                return;
        }
        {	//	检查是不是多行的 func define
                if (strcmp(p,"NTSTATUS") == 0)
                        nop();

                const char * pf = p;
                VarTypeID id = get_DataType(pf);
                if (id)	//	第一个量肯定是数据类型
                {
                        char buf1[80];
                        skip_eos(pf);
                        get_1part(buf1,pf);
                        enum_CallC cc = if_CallC(buf1);
                        if (cc == enum_unknown)
                                cc = enum_stdcall;
                        else
                        {
                                skip_eos(pf);
                                get_1part(buf1,pf);
                        }
                        skip_eos(pf);
                        if (*pf == '(')		//	这下，肯定是func define 了
                        {
                                CFuncType* pnewfunc = new CFuncType;
                                if (this->extern_c)
                                        pnewfunc->m_extern_c = true;
                                pnewfunc->m_callc = cc;
                                pnewfunc->m_retdatatype_id = id;
                                pnewfunc->m_pname = buf1;
                                //this->do_func_proto
                                func_define_2(pnewfunc,pf);
                                g_Hpp->newfunc_addlist(pnewfunc);
                                if (*pf == '{')
                                {
                                        int n = 1;
                                        for (;;)
                                        {
                                                pf++;
                                                if (*pf == '{') n++;
                                                if (*pf == '}') n--;
                                                if (n == 0) break;
                                        }
                                }
                                else if (*pf == ';')
                                {
                                }
                                else
                                        assert(0);
                                pf++;
                                skip_eos(pf);
                                pnext = pf;
                                return;
                        }
                }
        }
        int old_comma1 = this->comma1;
//	int old_comma2 = this->comma2;
        for(;;)
        {
                char c = *p++;
                if (c == '\0')
                        break;
                if (c == '\'' || c == '\"')
                {
                        skip_string(c,p);
                        continue;
                }
                if (c == '(')
                {
                        this->comma1++;
                        continue;
                }
                if (c == '{')
                {
                        this->comma2++;
                        continue;
                }
                if (c == ')')
                {
                        this->comma1--;
                        if (old_comma1 == 0	&& this->comma1 == 0)
                        {
                                const char * p1 = p;
                                if (*p1 == ' ')
                                        p1++;
                                if (*p1 == ';')
                                {	//	现在，我认为我已经找到一个函数定义
                                        //printf(":: %s\n",lbuf);
                                        g_Hpp->func_define(lbuf, this);
                                }
                        }
                        continue;
                }
                if (c == '}')
                {
                        if (this->comma2)
                                this->comma2--;
                        else if (this->extern_c != 0)
                                this->extern_c--;
                        else
                                assert(0);	//extra '}' find
                        continue;
                }

        }
}

void CCInfo::do_typedef(const char * &p)
{
        const char * savp = p;
        assert(p);
        assert(*p);
        assert(*p != ' ');

        if (memcmp(p,"enum",4) == 0)
                nop();

        VarTypeID id = get_DataType_bare(p);
        if (id == 0)
        {
                alert_prtf("unknown datatype : %s",savp);
                return;
        }

        skip_eos(p);

        do_typedef_(id,p);

        assert(*p == ';');
        p++;
        skip_eos(p);
}

void CCInfo::do_typedef_(VarTypeID baseid, const char * &p)
{
        const char * savp = p;
        //	给定一个 typedef char *LPSTR, *char *;
        //	到这里只剩一个 *LPSTR, *char *;
        VarTypeID id = Get_Additional_id(baseid, p);

        if (*p == '(')
        {	//	it must be a function point
                p++;
                skip_space(p);

                enum_CallC callc = this->m_default_callc;
                if (*p != '*')
                {
                        char part1[80];
                        get_1part(part1,p);
                        callc = if_CallC(part1);
                        assert(*p == '*');
                }
                assert(callc != enum_unknown);
                p++;
                skip_space(p);

                char name[80];
                get_1part(name,p);
                assert(*p == ')');
                p++;
                skip_space(p);
                assert(*p == '(');

                CFuncType* pnewfunc = new CFuncType;
                if (this->extern_c)
                {
                        pnewfunc->m_extern_c = true;
                }

                func_define_2(pnewfunc,p);

                pnewfunc->m_callc = callc;
                pnewfunc->m_retdatatype_id = id;
                //	生成内部函数名
                pnewfunc->create_internal_funcname();

                VarTypeID id_f = g_VarTypeManage->FuncType2VarID(pnewfunc);
                g_VarTypeManage->NewTypeDef(id_f, name);

                return;
        }

        char name[80];
        get_1part(name, p);

        {	//	检查一下是否已经定义过了
                const char * p1 = name;
                VarTypeID id1 = get_DataType_bare(p1);
                if (id1 != 0)
                {
                        if (id1 >= 10)
                                alert_prtf("already typedef : %s", name);
                        //	我们预定义了BYTE,WORD,unsigned lonuint32_t                        return;
                }
        }

        log_prtl("^^ typedef %s === %s",
                         GG_VarType_ID2Name(id),
                         name);
        g_VarTypeManage->NewTypeDef(id, name);

        skip_eos(p);
        if (*p == ';')
                return;
        else if (*p == ',')
        {
                p++;
                skip_eos(p);
                do_typedef_(baseid,p);
                return;
        }
        else
        {
                nop();
                alert_prtf("expect , or ; in typedef: %s",savp);
        }
}
CFuncType* CCInfo::do_func_proto_void(const char * lbuf)
{	//	就是没有函数返回值的情况，比如class的构造函数
    //Function return value is not the case, such as class constructor
        CFuncType* pnewfunc = new CFuncType;
        if (this->extern_c)
        {
                pnewfunc->m_extern_c = true;
        }

        //printf(":: %s\n",lbuf);

        char name[128];
        const char * p = lbuf;

        get_1part(name,p);

        assert(*p == '(');

        //	--------------------------

        pnewfunc->m_pname = name;
        pnewfunc->m_retdatatype_id = id_void;
    pnewfunc->m_callc = enum_stdcall;

        //	下面，处理它的参数 //The following address its argument

        func_define_2(pnewfunc,p);
        //Build the internal function name
        pnewfunc->create_internal_funcname();

        return pnewfunc;
}
CFuncType* CCInfo::do_func_proto(const char * lbuf)
{
        CFuncType* pnewfunc = new CFuncType;
        if (this->extern_c)
        {
                pnewfunc->m_extern_c = true;
        }

        //printf(":: %s\n",lbuf);

        char buf1[128];
        const char * p = lbuf;
        char * p1 = buf1;
        while (*p != '\0' && *p != '(')
                *p1++ = *p++;
        *p1 = '\0';

        assert(*p == '(');

        //	--------------------------

        func_1(pnewfunc, buf1);
        //	下面，处理它的参数 //The following address its argument
        func_define_2(pnewfunc,p);

        //	生成内部函数名
        pnewfunc->create_internal_funcname();

        return pnewfunc;
}
void CHpp::func_define(const char * lbuf, CCInfo* pCCInfo)
{
        CFuncType* pnewfunc = pCCInfo->do_func_proto(lbuf);
        newfunc_addlist(pnewfunc);
}
void CHpp::newfunc_addlist(CFuncType* pnewfunc)
{

        pnewfunc->create_internal_funcname();
        //Add it into g_FuncTypeList
        if (pnewfunc->m_internal_name.size()!=0)
        {
            FuncTypeList::iterator pos=m_FuncTypeList->begin();
            while (pos!=m_FuncTypeList->end())
            {   //Start looking to see if it already is in the list
                    CFuncType* pft = *pos;;
                    ++pos;
                    if (pft->m_internal_name.size()==0)
                        continue;
                    if (pft->m_internal_name.compare(pnewfunc->m_internal_name) == 0)
                    {
                            delete pnewfunc;
                            return; //Found, already have, even if the
                    }
            }
        }
        //alert_prtf("func: %s\ninternal: %s",pnewfunc->m_name,pnewfunc->m_internal_name);

        m_FuncTypeList->push_back(pnewfunc);
}
VarTypeID Get_Var_Declare(const char * &p, char * name)
{	//	就是根据一行
        //	const char * pstr1,
        //	char const * pstr2,
        //	char * const pstr3,
        //	得出这个参数的数据类型和名字

        //	特别要支持 struct _NewStruc * name;


        VarTypeID id = get_DataType(p);
        if (id == 0)
        {
                alert_prtf("unknown datatype : %s",p);
                return 0;
        }

        get_1part(name,p);	//	取得参数名
        if (*p == '[')
        {
                //alert("[ find");
                p++;
                if (*p == ']')
                {
                        p++;
                        id = g_VarTypeManage->NewArray_id_id(id, SIZE_unknown);
                }
                else
                {
                        uint32_t d = Str2Num(p);
                        id = g_VarTypeManage->NewArray_id_id(id, d);
                }
        }
        return id;
}
void func_define_2(CFuncType* pfunc,const char * &p)
{
    //Processing parameters of the definition of func "(int argc, char * argv [])"
    // Finally, point to ')' after ';'
    assert(*p == '(');
    p++;
    skip_space(p);
    skip_eos(p);

    int parnum = 0;
    std::vector<VarTypeID> pars;	//	100 should be enough ?
    std::vector<std::string> parnames;
    parnames.reserve(100);
    pars.reserve(100);
    while (*p && *p != ')')
    {
        if (memcmp(p,"...",3) == 0)
        {
            pfunc->m_varpar = true;
            p += 3;
            skip_space(p);
            skip_eos(p);
            assert(*p == ')');
            break;	 //I know, back then there can be other parameters of the
        }
        char parname[80];
        VarTypeID id = Get_Var_Declare(p,parname);
        if (id == 0)
        {
            alert_prtf("unknown datatype : %s",p);
            break;
        }
        if (id == id_void)
        {
            skip_eos(p);
            assert(*p == ')');
            assert(parnum == 0);
            break;
        }
        parnames.push_back(parname);//[parnum] = parname;
        pars.push_back(id);
        if (*p == ',')
            p++;
        skip_space(p);
        skip_eos(p);
    }
    p++;	//skip ')'
    skip_space(p);
    skip_eos(p);
    pfunc->m_partypes = pars; // vector contents COPIED here
    pfunc->m_parnames = parnames;// vector contents COPIED here
    pfunc->m_args = pfunc->m_parnames.size();
}


void func_1(CFuncType* pft,const char * p)
{
    //    p point is the definition of the function, including function name, but does not include parameters
    //    p may be "int ","__ cdecl", "uint32_t ",....
    //    P instructions should fill in the information and going to the pfunc
    assert(pft->m_retdatatype_id == 0);

    char part[80];

    pft->m_callc = enum_stdcall;	//	给它一个缺省值

    skip_space(p);
    //alert(p);

    while (*p)
    {
        if (pft->m_retdatatype_id == 0)
        {
            VarTypeID id = get_DataType(p);
            if (id != 0)
            {
                pft->m_retdatatype_id = id;
                continue;
            }
        }

        get_1part(part,p);
        assert(part[0]);
        if (*p == 0)
        {	//The last part is the function name
            //printf("func name = %s\n",part);
            pft->m_pname = part;
            break;
        }
        std::string p1 = get_define(part);
        if (p1.size()==0)
            p1 = part;	//If it is a defin, transforming what //	如果它是一个de fin，转化一下

        enum_CallC callc = if_CallC(p1.c_str());
        if (callc != 0)
        {
            pft->m_callc = callc;
            continue;
        }
        alert_prtf("-> %s",p1.c_str());
        assert(("I do not know this part",0));
    }
}

enum_CallC if_CallC(const char * p)
{
        if (strcmp(p,"__cdecl") == 0)
                return enum_cdecl;
        if (strcmp(p,"__stdcall") == 0)
                return enum_stdcall;
        if (strcmp(p,"__pascal") == 0)
                return enum_pascal;
        if (strcmp(p,"__fastcall") == 0)
                return enum_fastcall;
        if (strcmp(p,"_fastcall") == 0)
                return enum_fastcall;
        return enum_unknown;
}
void define_replace(char * buf)
{
        std::string p = get_define(buf);
        if (p.size()>0)
            strcpy(buf,p.c_str());
}
std::string get_define(char * partern)
{
        if (*partern == '\0')
                return "";
        std::string compared_with=partern;
        DefineList::iterator iter;
        iter = std::find_if(g_DefineList->begin(),
                            g_DefineList->end(),
                            bind<const string &>(&define_t::src,_1)==compared_with);
        if(iter!=g_DefineList->end())
            return (*iter)->dst;
        return "";
}
CFuncType* Get_FuncDefine_from_internal_name(const std::string & pmyinternalname)
{
        return g_Hpp->Get_FuncDefine_from_internal_name_(pmyinternalname);
}
CFuncType* CHpp::Get_FuncDefine_from_internal_name_(const std::string & pmyinternalname)
{
        assert(m_FuncTypeList);
//        POSITION pos = m_FuncTypeList->begin();

        FuncTypeList::iterator iter;
        iter = std::find_if(m_FuncTypeList->begin(), m_FuncTypeList->end(),
                    bind<const string &>(&CFuncType::m_internal_name,_1)==pmyinternalname);

        if(iter==m_FuncTypeList->end())
            return NULL;
        return *iter;

}
CFuncType* Get_FuncDefine_from_name(const std::string &pmyname)
{
        return g_Hpp->Get_FuncDefine_from_name_(pmyname);
}
struct _name_search
{
    const char *m_search_for;
    _name_search(const char *n_to_find) : m_search_for(n_to_find)
    {}
    _name_search(const std::string &n_to_find) : m_search_for(n_to_find.c_str())
    {}
    bool operator()(CFuncType *p)
    {
        if (p->m_pname.size()==0)
            return false;
        if (p->m_pname.compare(m_search_for) == 0 || p->m_internal_name.compare(m_search_for) == 0)
            return true;
        return false;
    }
};
CFuncType* CHpp::Get_FuncDefine_from_name_(const std::string & pmyname)
{
    //	?AfxWinMain@@YGHPAUHINSTANCE__@@0PADH@Z
    std::string name;
    {
        //if (*pmyname == '?')
        //    pmyname++;
        std::string::size_type loc=pmyname.find_first_of('?');
        if(loc==0)
            loc+=1;
        else
            loc=0;
        name=pmyname.substr(loc,pmyname.size()-loc);
        //assert(name[127] == '\0');
        loc = name.find_first_of('@');
        if(loc!=std::string::npos)
        {
            name=name.substr(0,loc);
        }
    }
    assert(m_FuncTypeList);

    FuncTypeList::iterator pos = find_if(m_FuncTypeList->begin(),
                                         m_FuncTypeList->end(),
                                         _name_search(name));
    if(pos!=m_FuncTypeList->end())
        return *pos;
    return NULL;
}
VarTypeID define_enum(const char * &p)
{
        char name[80];
        get_1part(name,p);
        skip_eos(p);
        if (*p == '{')
        {
                p++;
                skip_space(p);
                skip_eos(p);

                enum_st* pnew = new enum_st;
                strcpy(pnew->m_name,name);
                pnew->m_pfirst = NULL;
                int nextnnn = 0;		//	first enum = 0
                while (*p != '}')
                {
                        char itemname[80];
                        get_1part(itemname,p);
                        if (*p == '=')
                        {
                                p++;
                                skip_space(p);
                                nextnnn = Str2Num(p);
                                while (*p != '\0' && *p != ',' && *p != '}')
                                        p++;

                        }
                        NumStr_st* newitem = new NumStr_st;
                        newitem->n = nextnnn++;
                        newitem->name = itemname;
                        newitem->next = pnew->m_pfirst;
                        pnew->m_pfirst = newitem;

                        skip_space(p);
                        skip_eos(p);
                        if (*p == ',')
                        {
                                p++;
                                skip_eos(p);
                        }
                        else if (*p == '}')
                                break;
                        else
                                assert(0);
                }
                p++;
                g_enum_mng->Add_New_Enum(pnew);
                return g_VarTypeManage->Enum2VarID(pnew);
        }
        return 0;
}

VarTypeID do_enum(const char * &p)
{
        VarTypeID id = define_enum(p);
        skip_space(p);
        if (*p == ';')
                p++;
    skip_eos(p);
        return id;
}
void CCInfo::do_class(const char * p, const char * &pnext)
{
        Class_st theclass;
        get_1part(theclass.m_name,p);

        if (theclass.m_name[0] == '\0')
                return;
    if (!strcmp(theclass.m_name, "CTest1"))
    {
        strcmp(theclass.m_name, "CTest1");
    }

        skip_eos(p);

        if (*p != '{')
                return;
    p++;
        skip_eos(p);


        enumClassMemberAccess cma = nm_private;

        int n = 0;
        st_Var_Declare items[50];	//50个，够了吗
        SIZEOF size = 0;

        int nfunc = 0;
        CFuncType* funcs[50];
        memset(funcs,0,50 * sizeof(CFuncType*));

        while (*p && *p != '}')
        {
                if (strcmp(p,"private:") == 0)
                {
                        cma = nm_private;
                        p += 9;
                }
                if (strcmp(p,"protected:") == 0)
                {
                        cma = nm_private;
                        p += 11;
                }
                if (strcmp(p,"public:") == 0)
                {
                        cma = nm_public;
                        p += 8;
                }
                if (strchr(p,'('))	//	是函数
                {
                        CFuncType* pft = NULL;
                        int sz = strlen(theclass.m_name);
                        if (memcmp(p,theclass.m_name, sz) == 0
                                && if_split_char(p[sz]))	//	是构造函数
                        {
                                pft = do_func_proto_void(p);
                        }
                        else if (p[0] == '~')
                        {
                                assert(memcmp(p+1,theclass.m_name, sz) == 0);
                                assert(if_split_char(p[sz+1]));	//	是构析函数
                                pft = do_func_proto_void(p);
                        }
                        else
                        {
                                pft = do_func_proto(p);
                        }
                        assert(pft);
                        funcs[nfunc++] = pft;
                        assert(nfunc < 50);
                        p+= strlen(p);	//	先跳过
                        p++;
                }
                else
                {	//	变量定义
                        assert(n < 50);
                        st_Var_Declare* pvar = &items[n];
                        Read_Var_Declare(p, pvar);
                        pvar->m_offset_in_struc = size;
                        pvar->m_access = cma;

                        size += pvar->m_size;
                        n++;
                        p += strlen(p)+1;
                }
        }
        theclass.m_nDataItem = n;
        theclass.m_size = size;
        theclass.m_DataItems = new st_Var_Declare[n];
        memcpy(theclass.m_DataItems, items, n * sizeof(st_Var_Declare));

        theclass.m_nSubFuncs = nfunc;
        theclass.m_SubFuncs = new CFuncType *[nfunc];
        memcpy(theclass.m_SubFuncs, funcs, nfunc * sizeof(CFuncType *));

        Class_st* pnew = new Class_st;
        *pnew = theclass;

        //ZeroMemory(&theclass, sizeof(Class_st)); //防止theclass析构时删东西

        pnew->set_subfuncs();
        g_ClassManage->add_class(pnew);

        //	现在 p 应该指向 "};"
        p++;
        skip_space(p);
        if (*p == ';')
                p++;
        p++;
        pnext = p;
}
