///////////////////////////////////////////////////////////////
//
// Cexe2c.h
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
// Created at 2005.2.1
// Description:	The standard header file of the component implement
// History:
//
///////////////////////////////////////////////////////////////

#ifndef	_CEXE2C_H_
#define	_CEXE2C_H_

#include "../exe2c_interface.h"
#include "CISC.h"
#include "proto.h"
class Exe2c : public I_EXE2C
{
private:
    static Exe2c* s_Cexe2c;
    Exe2c();
	~Exe2c();
public:
    static Exe2c* get();
///////////// DO NOT EDIT THIS //////////////
	virtual bool	BaseInit();	//override the origin function, it's a class creator!
///////////// DO NOT EDIT THIS //////////////

	//Add interface here
	virtual bool	test();		//Test interface
	void  exe2c_main(const std::string &fname);
    void  prtout_asm(I_XmlOut* iOut);
    void  prtout_itn(I_XmlOut* iOut)
    {
        XmlOutPro out(iOut);
        m_Cur_Func->prtout_internal(&out);
    }
    void  prtout_cpp(I_XmlOut* iOut)
    {
        if (m_Cur_Func != NULL)
        {
            XmlOutPro out(iOut);
            CFunc_Prt prt(m_Cur_Func);
            prt.prtout_cpp(&out);
        }
    }
    void  Init(I_E2COUT* i_E2COut)
    {
        m_E2COut = i_E2COut;
    }
    void prt_log(const char * str)
    {
        if (m_E2COut != NULL)
        {
            m_E2COut->prt_log(str);
        }
    }


    FUNC_LIST::iterator GetFirstFuncHandle();
    virtual size_t  GetFuncCount() const {return m_func_list.size();}

    void GetFuncInfo( FUNC_LIST::iterator h, st_FuncInfo* info);
    FUNC_LIST::iterator GetNextFuncHandle(FUNC_LIST::iterator h);
    FUNC_LIST::iterator  GetCurFuncHandle();
    bool is_valid_function_handle(const FUNC_LIST::iterator &it) const;
    void  SetCurFunc_by_Name(const char * funcname);

	bool  analysis_Once()
	{
        if (m_Cur_Func != NULL)
            return (m_Cur_Func->analysis_step_by_step());
		return false;
	}
	void  analysis_All() {
        m_Cur_Func->analysis();
	}
	bool  RenameCurFuncName(const char * name);
	virtual void  DoCommandLine(const char * cmd);
	virtual bool  Rename(int xmltype, void* handle, const char * newname);
	virtual void  ReType(int colorindex, void* handle, const char * newtype);
	virtual void  Change_Array(int colorindex, void* handle, int newarray);
	virtual void  HotKey(int colorindex, void* handle, char key);
	virtual void  LineHotKey(void* hline, char nChar);
	//Add interface here

    void Recurse_Analysis();
    void Recurse_Optim();
    Func* func_new(ea_t start);
    Func* GetFunc(ea_t start);
    Func *current_func() {return m_Cur_Func;}
    void current_func(Func *x) {m_Cur_Func=x;}

private:
    //Add member here
    I_E2COUT* m_E2COut;
    FileLoader* m_FileLoader;
    FUNC_LIST m_func_list;	// Global function list
    //Add member here

    Func*	FindFuncByName(const char * pname);
    void	do_exe2c(ea_t entry);
    Func	*m_Cur_Func;		// Stores the current global CFunc

};
#endif	// _CEXE2C_H_
