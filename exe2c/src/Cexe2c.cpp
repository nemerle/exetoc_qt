///////////////////////////////////////////////////////////////
//
// Cexe2c.cpp
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
// Created at 2005.2.1
// Description:	The main cpp file of the component
// History:
//
///////////////////////////////////////////////////////////////

////#include "stdafx.h"
#include <QString>
#include <boost/filesystem/path.hpp>
#include "Cexe2c.h"
#include "CFuncStep1.h"
#include "DataType.h"
#include "ParseHead.h"
#include "LibScanner.h"


//KS_DECLARE_COMPONENT(exe2c, EXE2C)


Cexe2c* g_Cexe2c = NULL;

I_LIBSCANNER* g_LIBSCANNER = NULL;

bool hpp_init();
void lib_init();
void lib_exit();
void CExprManage_cpp_Init();

bool exe2c_Init()
{
    CExprManage_cpp_Init();
    hpp_init();
    return true;
}


void exe2c_Exit()
{
    lib_exit();
}
//FIXME : static initializer
//class CSelfInit
//{
//public:
//    CSelfInit()
//    {
//        exe2c_Init();
//    }
//    ~CSelfInit()
//    {
//        exe2c_Exit();
//    }
//};
//CSelfInit self;


bool Cexe2c::BaseInit()
{
    //KICK_MFC();
    m_E2COut = NULL;
    g_Cexe2c = this;

    //this->m_api_name_manager = new CNameMng;    //new_CNameMng

	// 作一些全局初始化
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

Cexe2c::~Cexe2c()
{
    //KICK_MFC();
    g_Cexe2c = NULL;
    delete this->m_api_name_manager;
    this->m_api_name_manager = NULL;
//    for_each(m_func_list.begin(),m_func_list.end(),remover);
    m_func_list.clear();

    delete m_FileLoader;
    m_FileLoader = NULL;
}


bool Cexe2c::test()
{
	//KICK_MFC();
	return true;
}


void	Cexe2c::Recurse_Analysis()
{
    FUNC_LIST::iterator pos = m_func_list.begin();
    while (pos != m_func_list.end())
    {
        CFunc* p = *pos;
        ++pos;

        log_prtl("Recurse_analysis %x",p->m_head_off);

		if (p->m_nStep != STEP_100)
			continue;

        p->analysis();
    }
}

void	Cexe2c::Recurse_Optim()
{
    FUNC_LIST::iterator pos = m_func_list.begin();
    while (pos != m_func_list.end())
    {
        CFunc* p = *pos;
        ++pos;

        log_prtl("Recurse_Optim %x",p->m_head_off);

		if (p->m_nStep < STEP_6)
			continue;
	}
}
void Cexe2c::exe2c_main(const std::string & fname)
{
	lib_init();
	//MessageBox(0,fname,"file open",0);

    //	文件调入
    // File transferred to
    if (m_FileLoader != NULL)
        delete m_FileLoader;
    m_FileLoader = new FileLoader;  //new_FileLoader
    bool f = m_FileLoader->load(fname.c_str());
    if (!f)
    {
        alert_prtf("File %s load error",fname.c_str());
        return;
    }


    BYTE * entry_buf;
    ea_t entry_offset;
    m_FileLoader->GetEntrance(entry_buf, entry_offset);

    // 因为文件的调入地址与虚拟地址不同，所以要记住这个差值
    // 以后主程序只以offset来访问，不管实际buffer
    //Because the file transferred to a different address and virtual address, so remember this difference
    // After the main program only offset to access, regardless of the actual buffer
    Disassembler_Init_offset(entry_buf, entry_offset);

    //	开始分析
    //start Analysis
    this->do_exe2c(entry_offset);
}


CFunc*	Cexe2c::FindFuncByName(const char * pname)
{
    if (m_func_list.size() == 0)
        return NULL;
    FUNC_LIST::iterator pos = m_func_list.begin();
    while (pos != m_func_list.end())
    {
        CFunc* p = *pos;
        ++pos;
        QString fc(p->m_funcname.c_str());
        if (fc.compare(pname,Qt::CaseInsensitive) == 0)
            return p;
    }
    return NULL;
}

    //start Analysis
void	Cexe2c::do_exe2c(ea_t start)
{
	ea_t pmain = Find_Main(start);

	// 第一步，根据 start，创建一个空的 CFunc
	//The first step, according to start, create an empty CFunc
	CFunc* pfunc = this->func_new(pmain);

	if (pmain == start)
		pfunc->m_funcname="start";
	else
		pfunc->m_funcname="main";
	//Set the current CFunc
	g_Cur_Func = pfunc;	//	设置当前的CFunc

	g_Cur_Func->PrepareFunc();
}

#include "CFuncType.h"


CFunc* Cexe2c::GetFunc(ea_t start)
{
    FUNC_LIST::iterator pos = m_func_list.begin();
    while (pos != m_func_list.end())
    {
        CFunc* p = *pos;
        ++pos;
        if (p->m_head_off == start)
            return p;
    }
    return NULL;
}

//#include "..\..\LibScanner\LibScanner.H"
#include "LibScanner.h"

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

        if (fcname.length()!=0)
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
CFunc* Cexe2c::func_new(ea_t start)
{
    {
        // 检查本func是否已经在func链中了
    //Check whether this func chain of the func
        CFunc* p = GetFunc(start);
        if (p != NULL)
            return p;
    }

	// not find
	log_prtl("New func %x",start);
	if (start == 0x128b1e1)
	{
		start = 0x128b1e1;
	}

	CFunc* p = new CFunc(start);    //new_CFunc

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
        buf[idx]=toupper(buf[idx++]);
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
void Cexe2c::DoCommandLine(const char * cmd)
{
    //if (g_Cur_Func == NULL)
        //return;
    if (memcmp(cmd, "var ", 4) == 0)
    {
        const char * varname = cmd + 4;
        CFuncOptim the(g_Cur_Func);
        the.Prt_Var_Flow(varname);
    }
    else if (strncmp(cmd, "funcinfo", 8) == 0)
    {
        g_Cur_Func->report_info();
    }
    else if (strncmp(cmd, "funcproto ", 10) == 0)
    {
        //Current function of the predefined
        cmd += 10;
        CCInfo * pnew = new CCInfo;
        CFuncType* pfunctype = pnew->do_func_proto(cmd);
        g_Cur_Func->m_functype = pfunctype;
        g_Cur_Func->m_funcname = pfunctype->m_pname;
    }
    else if (strncmp(cmd, "classof ", 8) == 0)
    {
        cmd += 8;
        VarTypeID id = g_VarTypeManage->VarType_Name2ID(cmd);
        Class_st* pclass = g_VarTypeManage->id2_Class(id);
        CFuncType* pfunctype = g_Cur_Func->m_functype;
        if (pfunctype != NULL && pclass != NULL)
        {
            pfunctype->m_class = pclass;
        }
        //this->DoCommandLine("funcproto void __cdecl func1()");
        //this->DoCommandLine("classof CTest1");
    }
    else if (strncmp(cmd, "macro1", 6) == 0)
    {
        this->DoCommandLine("funcproto ATOM __cdecl MyRegisterClass(HINSTANCE hInstance)");
    }
    else if (strncmp(cmd, "macro2", 6) == 0)
    {
        this->DoCommandLine("funcproto bool __cdecl InitInstance(HINSTANCE hInstance, int nCmdShow)");
    }
    else if (strncmp(cmd, "macro3", 6) == 0)
    {
        this->DoCommandLine("funcproto int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)");
    }
    else if (strncmp(cmd, "macro_test", 10) == 0)
    {
        this->DoCommandLine("funcproto void __cdecl CTest1()");
    }
    else if (strncmp(cmd, "macro5", 6) == 0)
    {
        this->DoCommandLine("classof CTest1");
    }
#if 0
    //this->DoCommandLine("funcproto void __cdecl test_class()");
    //this->DoCommandLine("funcproto void __cdecl test_class()");
#endif
    else if (strncmp(cmd, "restart", 7) == 0)
    {
        g_Cur_Func->Restart();
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

void Cexe2c::Change_Array(int colorindex, void* handle, int newarray)
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
        M_t* p = (M_t*)handle;
        //g_Cur_Func->m_exprs->Change_Array(p, newarray);
    }
}
void Cexe2c::LineHotKey(void* hline, char key)
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
        g_Cur_Func->MakeDownInstr(hline);
    }
}
void Cexe2c::HotKey(int colorindex, void* handle, char key)
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
        M_t* p = (M_t*)handle;
        //g_Cur_Func->m_exprs->namemanager->Rename(p->nameid,newname);
    }
}

void Cexe2c::ReType(int colorindex, void* handle, const char * newtype)
{
    if (handle == NULL)
        return;
    else if (colorindex == COLOR_VarH || colorindex == COLOR_type)
    {
        M_t* p = (M_t*)handle;
        g_Cur_Func->ReType(p, newtype);
    }
}
bool Cexe2c::Rename(int arg, void* handle, const char * newname)
{
    XMLTYPE xmltype=(XMLTYPE )arg;
    if (handle == NULL)
        return false;

    if (xmltype == XT_FuncName)
    {
        CFunc* p = (CFunc*)handle;
        p->m_funcname = newname;
        return true;
    }
    else if (xmltype == XT_Symbol)
    {
        M_t* p = (M_t*)handle;
        strcpy(p->namestr, newname);
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
        g_Cur_Func->ReType(p, newname);
    }
    */
    return false;
}
#include "CLibScanner.h"
void lib_init()
{
    //I_LIBSCANNER* pnew = NEW_LIBSCANNER();
    I_LIBSCANNER* pnew = new CLibScanner();
    ((CLibScanner *)pnew)->BaseInit();
    boost::filesystem::path tolibc=GetMyExePath()/"lib"/"libc.lib";
    pnew->ScanLib(tolibc.native_file_string().c_str());

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
