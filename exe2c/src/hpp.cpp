// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


#include <cassert>
#include <algorithm>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/bind.hpp>
//#include <boost/bind.hpp>
#include <QtCore/QDir>

#include <cstring>

#include "00000.h"
#include "hpp.h"

#include "ClassManage.h"
#include "EnumMng.h"
#include "Cbuf.h"
#include "strparse.h"
#include "SVarType.h"

using namespace std;
using namespace boost::lambda;

class CHpp
{
public:
    FuncTypeList* m_FuncTypeList;	//	Saved from the '.h' derived global function definition

    CHpp();
    ~CHpp();
    void newfunc_addlist(FuncType* pnewfunc);
    void func_define(const char * lbuf, CCInfo *p);
    FuncType* Get_FuncDefine_from_internal_name_(const std::string & pmyinternalname);
    FuncType* Get_FuncDefine_from_name_(const std::string &pmyname);
};

static CHpp* g_Hpp = NULL;
static QDir g_incpath;

struct define_t
{
    std::string src; //#define src dst
    std::string dst;
};

typedef std::list<define_t*> DefineList;
//about this list:
//	1. allow multi define, that is , it will save both
//		#define A 1
//		#define A 2
//		but if 2 just same define, only once

static DefineList* g_DefineList = NULL;

VarTypeID Get_Var_Declare(const char * &p, char * name);
static void prt_defines();
static std::string get_define(char * partern);
void define_replace(char * buf);

CHpp::CHpp()
{
    m_FuncTypeList = new FuncTypeList;
}
CHpp::~CHpp()
{
    if (m_FuncTypeList)
    {
        for_each(m_FuncTypeList->begin(),m_FuncTypeList->end(), boost::lambda::bind(delete_ptr(), _1));
        delete m_FuncTypeList;
        m_FuncTypeList = NULL;
    }
}
QDir getInstallDir()
{
    // TODO: this is working directory for now, one day it might actually do what it advertises :)
    return QDir(".");
}
QDir getInstallSubdir(QString name) {
    QDir path(getInstallDir());
    path.cd(name);
    return path;
}
bool hpp_init()
{

    g_Hpp = new CHpp;

    g_DefineList = new DefineList;
    g_incpath = getInstallSubdir("inc");

    //	if (g_EXEType == enum_PE_sys)
    //		strcat(g_incpath, "\\ntddk\\");

    CCInfo::LoadIncFile("my.h");

    return true;
}

bool hpp_onexit()
{
    delete g_Hpp;
    g_Hpp = NULL;
    if (g_DefineList)
    {
        for_each(g_DefineList->begin(),g_DefineList->end(),boost::lambda::bind(delete_ptr(), _1));
        delete g_DefineList;
        g_DefineList = NULL;
    }
    return true;
}

//	----------------------------------------------------------
CCInfo::CCInfo()
{
    this->m_parentheses_level = 0;
    this->m_curly_level = 0;
    this->m_extern_c = 0;
    m_default_callc = enum_stdcall;

    m_len = 0;
    m_buf = NULL;
}

CCInfo::~CCInfo()
{
    delete m_buf;
    m_buf = NULL;
}

void CCInfo::LoadFile(FILE *f)
{
    CCbuf ccbuf;
    ccbuf.LoadFile(f);
    m_buf = ccbuf.m_p;
    m_len = ccbuf.m_len;
}
bool CCInfo::LoadIncFile(const std::string &fname)
{
    QString nativepath = g_incpath.filePath(fname.c_str());
    //alert_prtf("begin file %s\n",fname);
    FILE* f = fopen(qPrintable(nativepath),"rb");
    if (f == NULL)
    {
        alert_prtf("File open error: %s",qPrintable(nativepath));
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
        pInfo->OneLine(p, pnext);	//	Under normal circumstances, OneLine will not move pnext
        //        If it is more than one line, putting the final point pnext
        if (pnext == NULL)
            p += strlen(p) + 1;
        else
        {
            assert( memcmp(pnext,"E ",2) );
            p = pnext;
        }
    }

    assert(pInfo->m_parentheses_level == 0);
    assert(pInfo->m_curly_level == 0);
    assert(pInfo->m_extern_c == 0);
    delete [] pInfo->m_buf;
    pInfo->m_buf = NULL;
    delete pInfo;

    //printf("end   file %s\n",fname);
    return true;
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
static void onDefine(const char *p)
{
    skip_space(p);
    assert(*p);	// #define can not be followed by nothing

    char buf[280];
    get_1part(buf,p);
    skip_space(p);
    do_define(buf,p);
    return;
}
static void onInclude(const char *p)
{
    skip_space(p);
    if (*p == '\"' || *p == '<')
    {
        char inc_path[80];
        p++;
        int i=0;
        while (*p != '\"' && *p != '>')
        {
            inc_path[i++] = *p++;
        }
        inc_path[i] = '\0';
        log_prtf("Load include file: %s\n",inc_path);
        CCInfo::LoadIncFile(inc_path);
    }
}
//Handle all lines beginning with #
static void onPreprocessorDirective(const char * lbuf)
{
    const char * p = lbuf;
    assert(*p == '#');
    p++;

    if (memcmp(p,"define",6) == 0)
        onDefine(p+6);
    if (memcmp(p,"include",7) == 0)
        onInclude(p+7);
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
void Class_st::log_display_structure()
{
    log_prtt("struct ");
    log_prtt(m_name);
    log_prtf("\n{    \\\\sizeof = 0x%x\n",m_size);
    int nident = 1;
    size_t count=m_DataItems.size();
    for (size_t i=0;i<count;i++)
    {
        st_Var_Declare &pv(m_DataItems[i]);
        if (pv.m_access == nm_sub_end)
            nident--;
        for (int j=0;j<nident;j++)
            log_prtt("    ");
        if (pv.m_access == nm_substruc)
        {
            log_prtt("struct {\n");
            nident++;
            continue;
        }
        if (pv.m_access == nm_subunion)
        {
            log_prtt("union {\n");
            nident++;
            continue;
        }
        if (pv.m_access == nm_sub_end)
        {
            log_prtt("} ");
            if (pv.m_name[0])
                log_prtt(pv.m_name);
            log_prtt(";\n");
            continue;
        }

        log_prtl("%s\t%s;\t//+%02x", GG_VarType_ID2Name(pv.m_vartypeid),pv.m_name,pv.m_offset_in_struc);
    }
    log_prtt("}\n");
}
VarTypeID do_struct_after_name(const char * strucname, const char * &p, bool Fstruc_Tunion);
VarTypeID do_struct(const char * &p)
{
    char name[80];
    get_1part(name,p);

    skip_eos(p);

    if (*p == '{')
        return do_struct_after_name(name,p,false);
    VarTypeID id = VarTypeMng::get()->NewUnknownStruc(name);
    return id;
}
VarTypeID do_union(const char * &p)
{
    char name[80];
    get_1part(name,p);

    skip_eos(p);

    if (*p == '{')
        return do_struct_after_name(name,p,true);
    VarTypeID id = VarTypeMng::get()->NewUnknownStruc(name);
    return id;
}
VarTypeID do_struct_after_name(const char * strucname, const char * &p, bool Fstruc_Tunion)
{
    assert(*p == '{');
    p++;
    skip_eos(p);

    int n = 0;
    st_Var_Declare items[150];	// 'x' should be enough
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
            {	//	This is a struct defined in the struct
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
            {	//	This is a struct defined in an union
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
                size = size_tbl[substruc_stack];	//	Returned to its original size
            }
            pvar->m_access = nm_sub_end;
            n++;

            p++;
            skip_eos(p);
            if (*p != ';')
            {	//	Description known
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
    pnew->m_DataItems.reserve(n);
    pnew->m_DataItems.assign(items,items+n);
    if (Fstruc_Tunion)
        pnew->m_size = maxsize_in_union;
    else
        pnew->m_size = size;
    pnew->m_Fstruc_Tunion = Fstruc_Tunion;

    pnew->log_display_structure();

    ClassManage::get()->new_struc(pnew);

    return VarTypeMng::get()->Class2VarID(pnew);
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
    {

        onPreprocessorDirective(p); //It is always a single line
        return;
    }
    const char * p1 = p;
    char part1[80];
    get_1part(part1,p1); // next token
    skip_eos(p1); // skip spaces, also embeded nulls

    if (strcmp(part1,"__inline") == 0) // skip __inline
    {
        pnext = p1;
        OneLine(p1,pnext); //recurse
        return;
    }
    if (strcmp(part1,"extern") == 0)
    {
        char part2[80];
        assert(*p1);
        const char * p2 = p1;
        get_1part(part2,p2);
        if (strcmp(part2,"\"C\"") == 0)
        {
            assert(*p2);
            if (*p2 == '{')
            {
                assert(m_parentheses_level == 0);
                assert(m_curly_level == 0);
                m_extern_c++;
                OneLine(p2+1, pnext);
                return;
            }
            else
            {	//	just 1 line
                m_extern_c++;
                OneLine(p2, pnext);
                m_extern_c--;
                return;
            }
        }
    }
    else if (strcmp(part1,"typedef") == 0)
    {
        onTypedef(p1);
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
    {	//	Check is not the func define multi-line
        const char * pf = p;
        VarTypeID id = get_DataType(pf);
        if (id)	//The first part is definitely the data type
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
            if (*pf == '(')		//	This, is certainly a func define
            {
                FuncType* pnewfunc = new FuncType;
                if (this->m_extern_c)
                    pnewfunc->m_extern_c = true;
                pnewfunc->m_callc = cc;
                pnewfunc->m_retdatatype_id = id;
                pnewfunc->m_pname = buf1;
                //this->do_func_proto
                func_define_2(pnewfunc,pf);
                g_Hpp->newfunc_addlist(pnewfunc);
                if (*pf == '{') // function body
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
                else if (*pf != ';')
                {
                    assert(0);
                }
                pf++;
                skip_eos(pf);
                pnext = pf;
                return;
            }
        }
    }
    int old_comma1 = this->m_parentheses_level;
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
            m_parentheses_level++;
            continue;
        }
        if (c == '{')
        {
            m_curly_level++;
            continue;
        }
        if (c == ')')
        {
            m_parentheses_level--;
            if (old_comma1 == 0	&& this->m_parentheses_level == 0)
            {
                const char * p1 = p;
                if (*p1 == ' ') // skip ' '
                    p1++;
                if (*p1 == ';')
                {	//	Now, I think I have to find a function definition
                    //printf(":: %s\n",lbuf);
                    g_Hpp->func_define(lbuf, this);
                }
            }
            continue;
        }
        if (c == '}')
        {
            if (m_curly_level)
                m_curly_level--;
            else if (m_extern_c != 0)
                m_extern_c--;
            else
                assert(0);	//extra '}' found
            continue;
        }

    }
}

void CCInfo::onTypedef(const char * &p)
{
    const char * savp = p;
    assert(p);
    assert(*p);
    assert(*p != ' ');

    if (memcmp(p,"enum",4) == 0)
    {
        //       nop();
    }

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

        FuncType* pnewfunc = new FuncType;
        if (this->m_extern_c)
        {
            pnewfunc->m_extern_c = true;
        }

        func_define_2(pnewfunc,p);

        pnewfunc->m_callc = callc;
        pnewfunc->m_retdatatype_id = id;
        //	Generate the internal function name
        pnewfunc->create_internal_funcname();

        VarTypeID id_f = VarTypeMng::get()->FuncType2VarID(pnewfunc);
        VarTypeMng::get()->NewTypeDef(id_f, name);

        return;
    }

    char name[80];
    get_1part(name, p);

    {	//	Checking to see if it's already defined
        const char * p1 = name;
        VarTypeID id1 = get_DataType_bare(p1);
        if (id1 != 0)
        {
            if (id1 >= 10)
            {
                alert_prtf("already typedef : %s", name);
                //	我们预定义了BYTE,WORD,unsigned long uint32_t                        return;
            }
        }
    }

    log_prtl("^^ typedef %s === %s", GG_VarType_ID2Name(id), name);
    VarTypeMng::get()->NewTypeDef(id, name);

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
        //TODO: nop();
        alert_prtf("expect , or ; in typedef: %s",savp);
    }
}
FuncType* CCInfo::do_func_proto_void(const char * lbuf)
{
    //Function return value is not defined ex. class constructor
    FuncType* pnewfunc = new FuncType;
    if (this->m_extern_c)
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


    func_define_2(pnewfunc,p); //	Handle arguments
    pnewfunc->create_internal_funcname(); //Generate the internal function name

    return pnewfunc;
}
FuncType* CCInfo::do_func_proto(const char * lbuf)
{
    FuncType* pnewfunc = new FuncType;
    if (this->m_extern_c)
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
    func_define_2(pnewfunc,p); //Handle arguments
    pnewfunc->create_internal_funcname(); //	Generate the internal function name

    return pnewfunc;
}
void CHpp::func_define(const char * lbuf, CCInfo* pCCInfo)
{
    FuncType* pnewfunc = pCCInfo->do_func_proto(lbuf);
    newfunc_addlist(pnewfunc);
}

void CHpp::newfunc_addlist(FuncType* pnewfunc)
{

    pnewfunc->create_internal_funcname();
    //Add it into g_FuncTypeList
    if (pnewfunc->m_internal_name.size()!=0)
    {
        FuncTypeList::iterator pos=m_FuncTypeList->begin();
        while (pos!=m_FuncTypeList->end())
        {   //Start looking to see if it already is in the list
            FuncType* pft = *pos;;
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
            id = VarTypeMng::get()->NewArray_id_id(id, SIZE_unknown);
        }
        else
        {
            uint32_t d = Str2Num(p);
            id = VarTypeMng::get()->NewArray_id_id(id, d);
        }
    }
    return id;
}
void func_define_2(FuncType* pfunc,const char * &p)
{
    //Processing parameters of the definition of func "(int argc, char * argv [])"
    // Finally, point to ')' after ';'
    assert(*p == '(');
    p++;
    skip_space(p);
    skip_eos(p);

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
            assert(pars.size()==0);
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


void func_1(FuncType* pft,const char * p)
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
                        boost::lambda::bind<const string &>(&define_t::src,_1)==compared_with);
    if(iter!=g_DefineList->end())
        return (*iter)->dst;
    return "";
}
FuncType* Get_FuncDefine_from_internal_name(const std::string & pmyinternalname)
{
    return g_Hpp->Get_FuncDefine_from_internal_name_(pmyinternalname);
}
FuncType* CHpp::Get_FuncDefine_from_internal_name_(const std::string & pmyinternalname)
{
    assert(m_FuncTypeList);
    //        POSITION pos = m_FuncTypeList->begin();

    FuncTypeList::iterator iter;
    iter = std::find_if(m_FuncTypeList->begin(), m_FuncTypeList->end(),
                        boost::lambda::bind<const string &>(&FuncType::m_internal_name,_1)==pmyinternalname);

    if(iter==m_FuncTypeList->end())
        return NULL;
    return *iter;

}
FuncType* Get_FuncDefine_from_name(const std::string &pmyname)
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
    bool operator()(FuncType *p)
    {
        if (p->m_pname.size()==0)
            return false;
        if (p->m_pname.compare(m_search_for) == 0 || p->m_internal_name.compare(m_search_for) == 0)
            return true;
        return false;
    }
};
FuncType* CHpp::Get_FuncDefine_from_name_(const std::string & pmyname)
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
        Enum_mng::get()->Add_New_Enum(pnew);
        return VarTypeMng::get()->Enum2VarID(pnew);
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
    FuncType* funcs[50];
    memset(funcs,0,50 * sizeof(FuncType*));

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
            FuncType* pft = NULL;
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
    theclass.m_size = size;
    theclass.m_DataItems.reserve(n);

    theclass.m_DataItems.assign(items, items+n);

    theclass.m_SubFuncs.reserve(nfunc);
    theclass.m_SubFuncs.assign(funcs,funcs+nfunc);

    Class_st* pnew = new Class_st;
    *pnew = theclass;

    pnew->set_subfuncs();
    ClassManage::get()->add_class(pnew);

    //	Now p should point to "}; "
    p++;
    skip_space(p);
    if (*p == ';')
        p++;
    p++;
    pnext = p;
}
