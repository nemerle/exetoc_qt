///////////////////////////////////////////////////////////////
//
// LibScanner.h
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
//
///////////////////////////////////////////////////////////////
//#include "..\..\LibScanner\LibScanner.H"

#ifndef	_LIBSCANNER_H_
#define	_LIBSCANNER_H_
#include <cstdlib>
#include <string>
typedef unsigned char BYTE;
typedef signed char CHAR;
//#include "..\I_KSUNKNOWN\KsFrame.h"

#define	IID_LIBSCANNER				0x00003a91
#define	LIBSCANNER_INITORDER		0x80000000
#define	LIBSCANNER_PARENT_IID		NULL
#define	LIBSCANNER_PRIORITY			0x80000000
#pragma warning(disable:4200)

typedef struct REFSYMBOL
{
        char    RefSymbol[4096];	//引用符号
        WORD	RefType;    //such as IMAGE_REL_I386_REL32
        unsigned long	RefOffset;				//引用在函数中的偏移
} *PREFSYMBOL;

typedef struct tagFUNCTION_SYMBOL
{
        unsigned long       dwFuncLen;
        BYTE*		FunRawData;
        char		ObjName[4096];
        char		FunctionName[4096]; // was MAX_PATH
        unsigned long		RefCount;			//引用的符号次数
        REFSYMBOL	RefInfo[];			//引用信息
}FUNCTION_SYMBOL,* PFUNCTION_SYMBOL;
#pragma pack(2)
typedef struct _IMAGE_RELOCATION {
    union {
        DWORD   VirtualAddress;
        DWORD   RelocCount;             // Set to the real count when IMAGE_SCN_LNK_NRELOC_OVFL is set
    };
    DWORD   SymbolTableIndex;
    WORD    Type;
} IMAGE_RELOCATION;
#pragma pack()
typedef IMAGE_RELOCATION *PIMAGE_RELOCATION;
#pragma warning(default:4200)

typedef const BYTE* PCBYTE;

class I_LIBSCANNER //: public I_KSUNKNOWN
{
public:
        //Add interface here
        virtual bool	test() = 0;	//Test interface
        //Add interface here

        virtual bool ScanLib(const char * szLib)=0;
        //virtual PFUNCTION_SYMBOL GetFunctionInfo(const char * szFun)=0;
    virtual std::string CheckIfLibFunc(PCBYTE phead) = 0;
};

//KS_DECLARE_INTERFACE(LibScanner, LIBSCANNER)

#endif	// _LIBSCANNER_H_

/*	这里放接口函数的详细解释

*/
