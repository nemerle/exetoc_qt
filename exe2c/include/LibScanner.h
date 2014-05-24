///////////////////////////////////////////////////////////////
//
// CLibScanner.h
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
//
///////////////////////////////////////////////////////////////

#ifndef	_CLIBSCANNER_H_
#define	_CLIBSCANNER_H_

#include "LibScanner_interface.h"
#include "CISC.h"
#include "vector"

#define IMAGE_REL_I386_ABSOLUTE         0x0000  // Reference is absolute, no relocation is necessary
#define IMAGE_REL_I386_DIR16            0x0001  // Direct 16-bit reference to the symbols virtual address
#define IMAGE_REL_I386_REL16            0x0002  // PC-relative 16-bit reference to the symbols virtual address
#define IMAGE_REL_I386_DIR32            0x0006  // Direct 32-bit reference to the symbols virtual address
#define IMAGE_REL_I386_DIR32NB          0x0007  // Direct 32-bit reference to the symbols virtual address, base not included
#define IMAGE_REL_I386_SEG12            0x0009  // Direct 16-bit reference to the segment-selector bits of a 32-bit virtual address
#define IMAGE_REL_I386_SECTION          0x000A
#define IMAGE_REL_I386_SECREL           0x000B
#define IMAGE_REL_I386_TOKEN            0x000C  // clr token
#define IMAGE_REL_I386_SECREL7          0x000D  // 7 bit offset from base of section containing target
#define IMAGE_REL_I386_REL32            0x0014  // PC-relative 32-bit reference to the symbols virtual address

#define IMAGE_ARCHIVE_START_SIZE             8
#define IMAGE_ARCHIVE_START                  "!<arch>\n"
#define IMAGE_ARCHIVE_END                    "`\n"
#define IMAGE_ARCHIVE_PAD                    "\n"
#define IMAGE_ARCHIVE_LINKER_MEMBER          "/               "
#define IMAGE_ARCHIVE_LONGNAMES_MEMBER       "//              "
typedef struct _IMAGE_ARCHIVE_MEMBER_HEADER {
    uint8_t     Name[16];
    uint8_t     Date[12];
    uint8_t     UserID[6];
    uint8_t     GroupID[6];
    uint8_t     Mode[8];
    uint8_t     Size[10];
    uint8_t     EndHeader[2];
} IMAGE_ARCHIVE_MEMBER_HEADER, *PIMAGE_ARCHIVE_MEMBER_HEADER;

typedef struct
{
        char		ObjName[4096];	//name
        uint32_t		StartOff;   //start address
        uint32_t		Len;        //end address
        uint8_t *		lpBuffer;   //OBJ data
}COFFOBJECT,*PCOFFOBJECT;
typedef struct _IMAGE_FILE_HEADER {
  WORD  Machine;
  WORD  NumberOfSections;
  int32_t TimeDateStamp;
  int32_t PointerToSymbolTable;
  int32_t NumberOfSymbols;
  WORD  SizeOfOptionalHeader;
  WORD  Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;
#define IMAGE_SIZEOF_FILE_HEADER             20
typedef struct
{
        SHORT SYMB_INX;
        SHORT SECT_NUM;
        uint8_t  STOR_CLS;
        uint32_t VALUE;
        WORD  TYPE;
        char  Name[4096];
}COFFSYMBOL,*PCOFFSYMBOL;
#pragma pack(2)
typedef struct _IMAGE_SYMBOL {
    union {
        uint8_t    ShortName[8];
        struct {
            int32_t   Short;     // if 0, use LongName
            int32_t   Long;      // offset into string table
        } Name;
        int32_t   LongName[2];    // PBYTE [2]
    } N;
    int32_t   Value;
    SHORT   SectionNumber;
    WORD    Type;
    uint8_t    StorageClass;
    uint8_t    NumberOfAuxSymbols;
} IMAGE_SYMBOL;
typedef IMAGE_SYMBOL *PIMAGE_SYMBOL;
#pragma pack(2)
typedef struct _IMAGE_SECTION_HEADER {
    uint8_t    Name[8];
    union {
        int32_t   PhysicalAddress;
        int32_t   VirtualSize;
    } Misc;
    int32_t   VirtualAddress;
    int32_t   SizeOfRawData;
    int32_t   PointerToRawData;
    int32_t   PointerToRelocations;
    int32_t   PointerToLinenumbers;
    WORD    NumberOfRelocations;
    WORD    NumberOfLinenumbers;
    int32_t   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;
#pragma pack()
#define IMAGE_SYM_DTYPE_NULL                0       // no derived type.
#define IMAGE_SYM_DTYPE_POINTER             1       // pointer.
#define IMAGE_SYM_DTYPE_FUNCTION            2       // function.
#define IMAGE_SYM_DTYPE_ARRAY               3       // array.

#define N_BTMASK                            0x000F
#define N_TMASK                             0x0030
#define N_TMASK1                            0x00C0
#define N_TMASK2                            0x00F0
#define N_BTSHFT                            4
#define N_TSHIFT                            2

#ifndef ISFCN
#define ISFCN(x) (((x) & N_TMASK) == (IMAGE_SYM_DTYPE_FUNCTION << N_BTSHFT))
#endif
#define MAKEDWORDREVERS(x)  ((((x)&0xFF)<<24) | (((x)&0xFF00) << 8) | (((x)&0xFF0000) >>8) | (((x)&0xFF000000)>>24))
class LibScanner : public I_LIBSCANNER
{
    typedef std::vector<COFFOBJECT>	COFFOBJECT_LIST;
    typedef std::vector<COFFSYMBOL>	COFFSYMBOL_LIST;
    typedef std::vector<PFUNCTION_SYMBOL> FUNCTION_LIST;
public:
    LibScanner(){}
    ~LibScanner();

public:
    virtual bool  ScanLib(const QString & szLib);
    virtual PFUNCTION_SYMBOL  GetFunctionInfo(const char * szFun);
    virtual std::string  CheckIfLibFunc(PCBYTE phead);

private:
    //Add member here
    FUNCTION_LIST m_funs;
    void ClearFunction();
    void ClearCOFFObject(COFFOBJECT_LIST &objs) const;
    const char *COFFGetName(const IMAGE_SYMBOL &coff_sym, const char* coff_strtab);
    bool ScanCOFFObject(COFFOBJECT_LIST &objs,uint8_t * lpBuffer,uint32_t Len);
    PIMAGE_SECTION_HEADER FindSection(PCOFFOBJECT pObj,SHORT SectNumber) const;
    PCOFFSYMBOL FindSymbol(COFFSYMBOL_LIST &syms,int symIndx) const;
    void ScanFunction(FUNCTION_LIST & funs,PCOFFOBJECT pObj);
    bool CheckThisFunc(PFUNCTION_SYMBOL pFun, PCBYTE phead);
    bool CheckSubFunc(PFUNCTION_SYMBOL pFun, PCBYTE phead);
};

#endif	// _CLIBSCANNER_H_
