// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#ifndef proto___H
#define proto___H
#include <list>
#include "../exe2c.h"
#include "enum.h"

typedef	std::list<ea_t>	EALIST;
//---------------------------------------------
//	unit1.cpp
void	Set_Cur_CFunc(CFunc* pfunc);
void	Redraw_CurFunc();
void	Redraw();
//---------------------------------------------

void _warn(char * __cond, char * __file, int __line);


#ifdef NDEBUG
#define assert(p)   ((void)0)
#define assert_msg(p,msg)   ((void)0)
#define warn(p)   ((void)0)
#define warn_msg(p,msg)   ((void)0)
#else
//Assert error process will be aborted, but with warn it will not
//#define assert(p)   ((p) ? (void)0 : _assert(#p, __FILE__, __LINE__))
#define assert_msg(p,msg)   ((p) ? (void)0 : assert(msg##" -- "#p))
#define warn(p)   ((p) ? (void)0 : _warn(#p, __FILE__, __LINE__))
#define warn_msg(p,msg)   ((p) ? (void)0 : _warn(msg##" -- "#p, __FILE__, __LINE__))
#endif

//	--------------------------
//	main.cpp


extern	CFunc	*g_Cur_Func;		// 全局保存当前 CFunc

//extern	EXPR_LIST	*g_expr_list;	// 全局变量表


//---------------------------------------------
// pub.cpp
const char *	prt_DWORD(uint32_t d);
ea_t	Find_Main(ea_t start);

char *	new_str(const char * p);

int log_prtf(const char * fmt,...);
int log_prtl(const char * fmt,...);
int		alert_prtf(const char * fmt, ...);
void	error(const char * msg);
void	alert(const char * msg);


//---------------------------------------------
// Deasm_Init.cpp
void	Disassembler_Init_offset(BYTE * code_buf, ea_t code_offset);
BYTE *	ea2ptr(ea_t pos);
ea_t ptr2ea(void* p);
BYTE	Peek_B(ea_t pos);
WORD	Peek_W(ea_t pos);
uint32_t	Peek_D(ea_t pos);


//---------------------------------------------
// CFunc.cpp

void	fill_func_info( ea_t pos,CFunc* pfnc);
bool	IfValideFuncName(const char * pname);


const char *	hlcode_name(HLType t);
//---------------------------------------------
uint32_t	regindex_2_regoff(uint32_t regindex);

//---------------------------------------------
//	CFunc_CreateInstrList.cpp
PINSTR	instr_next(const INSTR_LIST& list,const INSTR *p);
PINSTR	instr_prev(const INSTR_LIST& list,const INSTR *p);

//---------------------------------------------
//	FileLoad.cpp
bool	IfInWorkSpace(ea_t off);	//	check if off lye in our work space



#endif // proto___H
