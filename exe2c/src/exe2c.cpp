///////////////////////////////////////////////////////////////
//
// Exe2c.cpp
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
// Created at 2005.2.1
// Description:	The main cpp file of the component
// History:
//
///////////////////////////////////////////////////////////////

////#include "stdafx.h"
#include <QString>
#include <boost/filesystem/path.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/bind.hpp>

#include "exe2c.h"
#include "FuncStep1.h"
#include "DataType.h"
#include "LibScanner.h"

using namespace boost::lambda;
//KS_DECLARE_COMPONENT(exe2c, EXE2C)


I_LIBSCANNER* g_LIBSCANNER = NULL;

bool hpp_init(void);
void lib_init(void);
void lib_exit(void);

bool exe2c_Init()
{
    hpp_init();
    return true;
}


void exe2c_Exit()
{
    lib_exit();
}
Exe2c* Exe2c::s_Cexe2c=0;
Exe2c *Exe2c::get()
{
    if(0==s_Cexe2c)
        s_Cexe2c = new Exe2c;
    return s_Cexe2c;
}
Exe2c::Exe2c()
{
    m_Cur_Func = NULL;
}
bool Exe2c::BaseInit()
{
    m_E2COut = NULL;
    //this->m_api_name_manager = new CNameMng;    //new_CNameMng
    // Make some global initializations
        m_FileLoader = NULL;
        return true;
}
struct remover
{
    void operator()(FUNC_LIST::value_type v)
    {
        delete v;
    }
};

Exe2c::~Exe2c()
{
    //KICK_MFC();
    s_Cexe2c = NULL;
//    for_each(m_func_list.begin(),m_func_list.end(),remover);
    m_func_list.clear();

    delete m_FileLoader;
    m_FileLoader = NULL;
}


bool Exe2c::test()
{
        //KICK_MFC();
        return true;
}


void	Exe2c::Recurse_Analysis()
{
    Func* p;
    FUNC_LIST::iterator pos = m_func_list.begin();
    FUNC_LIST::iterator end = m_func_list.end();
    for( ; pos != end; ++pos)
    {
        p = *pos;
        log_prtl("Recurse_analysis %x",p->m_head_off);
        if (p->m_nStep != STEP_100)
            continue;
        p->analysis();
    }
}

void	Exe2c::Recurse_Optim()
{
    Func* p;
    FUNC_LIST::iterator pos = m_func_list.begin();
    FUNC_LIST::iterator end = m_func_list.end();
    for( ; pos != end; ++pos)
    {
        p = *pos;
        log_prtl("Recurse_Optim %x",p->m_head_off);
        if (p->m_nStep < STEP_6)
            continue;
    }
}
void Exe2c::exe2c_main(const std::string & fname)
{
        lib_init();

    if (m_FileLoader != NULL)
        delete m_FileLoader;
    m_FileLoader = new FileLoader;  //new_FileLoader
    // File to decompile
    bool f = m_FileLoader->load(fname.c_str());
    if (!f)
    {
        alert_prtf("File %s load error",fname.c_str());
        return;
    }


    uint8_t * entry_buf;
    ea_t entry_offset;
    m_FileLoader->GetEntrance(entry_buf, entry_offset);

    // 因为文件的调入地址与虚拟地址不同，所以要记住这个差值
    // 以后主程序只以offset来访问，不管实际buffer
    //Because the file transferred to a different address and virtual address, so remember this difference
    // After the main program only offset to access, regardless of the actual buffer
    Disassembler_Init_offset(entry_buf, entry_offset);

    //start analysis
    this->do_exe2c(entry_offset);
}


Func*	Exe2c::FindFuncByName(const char * pname)
{
    if (m_func_list.size() == 0)
        return NULL;
    Func* p;
    FUNC_LIST::iterator pos = m_func_list.begin();
    FUNC_LIST::iterator end = m_func_list.end();
    for( ; pos != end; ++pos)
    {
        p = *pos;
        QString fc(p->m_funcname.c_str());
        if (fc.compare(pname,Qt::CaseInsensitive) == 0)
            return p;
    }
    return NULL;
}

    //start Analysis
void	Exe2c::do_exe2c(ea_t start)
{
        ea_t pmain = Find_Main(start);

        //The first step, according to start, create an empty CFunc
        Func* pfunc = this->func_new(pmain);

        if (pmain == start)
                pfunc->m_funcname="start";
        else
                pfunc->m_funcname="main";
        //Set the current CFunc
    m_Cur_Func = pfunc;
    m_Cur_Func->PrepareFunc();
}

#include "FuncType.h"

Func* Exe2c::GetFunc(ea_t start)
{
    FUNC_LIST::iterator iter=std::find_if(m_func_list.begin(),m_func_list.end(),
        boost::lambda::bind<ea_t>(&Func::m_head_off,_1)==start);
    if(iter==m_func_list.end())
        return NULL;
    return *iter;
}

//#include "..\..\LibScanner\LibScanner.H"
#include "LibScanner_interface.h"

const char * check_if_jmp_api(PCBYTE phead);
extern boost::filesystem::path GetMyExePath();
static std::string CheckIf_libfunc(PCBYTE phead)
{
    const char * apiname = check_if_jmp_api(phead);
    if (apiname)
        return apiname;

    if (1)
    {
        std::string fcname = g_LIBSCANNER->CheckIfLibFunc(phead);

        if (!fcname.empty())
            return fcname;
    }
    return "";
}
//	根据 start，创建一个空的 CFunc
//	并加入 m_func_list
//	如果该地址的 CFunc 已经存在，则直接返回它
// According to start, create an empty CFunc
// And join the m_func_list
// If the address CFunc already exists, then return it directly
Func* Exe2c::func_new(ea_t start)
{
    {
        // 检查本func是否已经在func链中了
    //Check whether this func chain of the func
        Func* p = GetFunc(start);
        if (p != NULL)
            return p;
    }

        // not find
        log_prtl("New func %x",start);
        if (start == 0x128b1e1)
        {
                start = 0x128b1e1;
        }

        Func* p = new Func(start);    //new_CFunc

        //	填入 CFunc 的一些其它信息
        //Fill in some of the other information CFunc
        fill_func_info(start, p);

        std::string pname = CheckIf_libfunc(ea2ptr(p->m_head_off));

    if (pname.length()!=0)
    {
        p->m_IfLibFunc = true;
        p->m_functype = Get_FuncDefine_from_internal_name(pname);
        if (p->m_functype)
            p->m_funcname=p->m_functype->m_pname;
        else
            p->m_funcname=pname;
    }

    m_func_list.push_back(p);	//insert cur Func to m_func_list

    return p;
}

static uint32_t str_to_dword(const char * cmd)
{
    char buf[80];
    strncpy(buf, cmd, 70);
    buf[70] = 0;
    size_t idx=0;
    while(buf[idx]!=0)
    {
        buf[idx]=toupper(buf[idx]);
        ++idx;
    }
    if (buf[0] == '0' && buf[1] == 'X')
    {
        uint32_t d;
        sscanf(buf+2,"%X", &d);
        return d;
    }
    uint32_t d;
    sscanf(buf,"%d", &d);
    return d;
}
#include "hpp.h"
const char * my_itoa(int i);
void Exe2c::DoCommandLine(const char * cmd)
{
    //if (m_Cur_Func == NULL)
        //return;
    if (memcmp(cmd, "var ", 4) == 0)
    {
        const char * varname = cmd + 4;
        FuncOptim the(m_Cur_Func);
        the.Prt_Var_Flow(varname);
    }
    else if (strncmp(cmd, "funcinfo", 8) == 0)
    {
        m_Cur_Func->report_info();
    }
    else if (strncmp(cmd, "funcproto ", 10) == 0)
    {
        //Current function of the predefined
        cmd += 10;
        CCInfo * pnew = new CCInfo;
        FuncType* pfunctype = pnew->do_func_proto(cmd);
        m_Cur_Func->m_functype = pfunctype;
        m_Cur_Func->m_funcname = pfunctype->m_pname;
    }
    else if (strncmp(cmd, "classof ", 8) == 0)
    {
        cmd += 8;
        VarTypeID id = VarTypeMng::get()->VarType_Name2ID(cmd);
        Class_st* pclass = VarTypeMng::get()->id2_Class(id);
        FuncType* pfunctype = m_Cur_Func->m_functype;
        if (pfunctype != NULL && pclass != NULL)
        {
            pfunctype->m_class = pclass;
        }
    }
    else if (strncmp(cmd, "restart", 7) == 0)
    {
        m_Cur_Func->Restart();
    }
    else if (strncmp(cmd, "optim", 5) == 0)
    {
        this->analysis_All();
    }
    else if (strncmp(cmd, "funcnew ", 8) == 0)
    {
        cmd += 8;
        uint32_t d = str_to_dword(cmd);
        this->func_new(d);
    }
}

void Exe2c::Change_Array(int colorindex, void* handle, int newarray)
{
    if (handle == NULL)
        return;

    if (colorindex == COLOR_Var)
    {
        st_VarLL* p = (st_VarLL*)handle;
        if (p->array < newarray)
        {
        }
        p->array = newarray;
    }
    if (colorindex == COLOR_VarH)
    {
        //M_t* p = (M_t*)handle;
        //m_Cur_Func->m_exprs->Change_Array(p, newarray);
    }
}
void Exe2c::LineHotKey(void* hline, char key)
{
    if (key == 'i' || key == 'I')
    {//i for internal
        static bool flag = false;
        flag = !flag;
        if (flag)
        {
            //prtout_itn();
        }
        else
        {
            //prtout_cpp();
        }
    }
    if (hline == NULL)
        return;

    if (key == 'd' || key == 'D')
    {
        m_Cur_Func->MakeDownInstr(hline);
    }
}
void Exe2c::HotKey(int colorindex, void* handle, char key)
{
    if (handle == NULL)
        return;
    if (key == 'p' || key == 'P')
    {
    }
    else
        return;

    if (colorindex == COLOR_VarH)
    {
        //M_t* p = (M_t*)handle;
        //m_Cur_Func->m_exprs->namemanager->Rename(p->nameid,newname);
    }
}

void Exe2c::ReType(int colorindex, void* handle, const char * newtype)
{
    if (handle == NULL)
        return;
    else if (colorindex == COLOR_VarH || colorindex == COLOR_type)
    {
        M_t* p = (M_t*)handle;
        m_Cur_Func->ReType(p, newtype);
    }
}
bool Exe2c::Rename(int arg, void* handle, const char * newname)
{
    XMLTYPE xmltype=(XMLTYPE )arg;
    if (handle == NULL)
        return false;

    if (xmltype == XT_FuncName)
    {
        Func* p = (Func*)handle;
        p->m_funcname = newname;
        return true;
    }
    else if (xmltype == XT_Symbol)
    {
        M_t* p = (M_t*)handle;
        p->namestr=newname;
        return true;
    }
    /*
    else if (colorindex == COLOR_Var)
    {
        st_VarLL* p = (st_VarLL*)handle;
        strcpy(p->Name, newname);
    }
    else if (colorindex == COLOR_type)
    {
        M_t* p = (M_t*)handle;
        m_Cur_Func->ReType(p, newname);
    }
    */
    return false;
}
void Exe2c::prtout_asm(I_XmlOut* iOut)
{
    if (m_Cur_Func->m_nStep == 0)
        return;

    XmlOutPro out(iOut);
    FuncLL the(m_Cur_Func->ll.m_asmlist);
    the.prtout_asm(m_Cur_Func, &m_Cur_Func->m_varll, &out);
}

#include "LibScanner.h"
void lib_init()
{
    //I_LIBSCANNER* pnew = NEW_LIBSCANNER();
    I_LIBSCANNER* pnew = new LibScanner();
    boost::filesystem::path tolibc=GetMyExePath()/"lib"/"libc.lib";
    pnew->ScanLib(tolibc.string().c_str());

    g_LIBSCANNER = pnew;
}

void lib_exit()
{
    if (g_LIBSCANNER != NULL)
    {
        delete g_LIBSCANNER;//g_LIBSCANNER->Release();
        g_LIBSCANNER = NULL;
    }
}
