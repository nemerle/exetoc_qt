///////////////////////////////////////////////////////////////
//
// LibScanner.h
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
//
///////////////////////////////////////////////////////////////


#ifndef	_LIBSCANNER_H_
#define	_LIBSCANNER_H_
#include <cstdlib>
#include <string>
#include <stdint.h>
#include <QtCore/QString>

typedef struct REFSYMBOL
{
    char        RefSymbol[4096];	//Referenced symbol
    uint16_t    RefType;    //such as IMAGE_REL_I386_REL32
    uint32_t    RefOffset;				//Offset in the referenced function
} *PREFSYMBOL;

typedef struct tagFUNCTION_SYMBOL
{
    uint32_t    dwFuncLen;
    uint8_t *   FunRawData;
    char        ObjName[4096];
    char        FunctionName[4096]; // was MAX_PATH
    uint32_t    RefCount;			//Number of referenced symbols
    REFSYMBOL   RefInfo[];			//Reference Information
}FUNCTION_SYMBOL,* PFUNCTION_SYMBOL;
#pragma pack(2)
typedef struct _IMAGE_RELOCATION {
    union {
        int32_t   VirtualAddress;
        int32_t   RelocCount;             // Set to the real count when IMAGE_SCN_LNK_NRELOC_OVFL is set
    };
    int32_t   SymbolTableIndex;
    uint16_t    Type;
} IMAGE_RELOCATION;
#pragma pack()
typedef IMAGE_RELOCATION *PIMAGE_RELOCATION;

typedef const uint8_t * PCBYTE;

class I_LIBSCANNER
{
public:
    virtual ~I_LIBSCANNER() {}
    virtual bool ScanLib(const QString& szLib)=0;
    //virtual PFUNCTION_SYMBOL GetFunctionInfo(const char * szFun)=0;
    virtual std::string CheckIfLibFunc(PCBYTE phead) = 0;
};

//KS_DECLARE_INTERFACE(LibScanner, LIBSCANNER)

#endif	// _LIBSCANNER_H_

/*
 Put detailed explanation of interface functions here.
*/
