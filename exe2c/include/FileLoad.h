// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
//	exe2c project

#ifndef	FileLoad_H
#define FileLoad_H
#include <cstdio>
#include "types.h"
enum enum_EXEType
{
    enum_PE_exe		= 1,
    enum_PE_dll		= 2,
    enum_PE_sys		= 3
};

enum enum_EXEFormat
{
    UNKNOWN_EXE=0,
    PE_EXE=1,
    MZ_EXE=2,
    OS2_EXE=3,
    COM_EXE=5,
    NE_EXE=6,
    SYS_EXE=7,
    LE_EXE=8,
    BIN_EXE=9
};

struct MZHEADER
{
    uint16_t sig;
    uint16_t numbytes,numpages;
    uint16_t numrelocs,headersize;
    uint16_t minpara,maxpara;
    uint16_t initialss,initialsp;
    uint16_t csum;
    uint32_t csip;
    uint16_t relocoffs;
    uint16_t ovlnum;
};

struct neheader
{
    uint16_t sig;
    uint16_t linkerver;
    uint16_t entryoffs,entrylen;
    uint32_t filecrc;
    uint16_t contentflags;
    uint16_t dsnum;
    uint16_t heapsize,stacksize;
    uint32_t csip,sssp;
    uint16_t numsegs,nummodules;
    uint16_t nonresnamesize;
    uint16_t offs_segments,offs_resources,offs_resnames,offs_module,offs_imports;
    uint32_t nonresnametable;
    uint16_t movableentries;
    uint16_t shiftcount;
    uint16_t numresources;
    uint8_t targetos,os_info;
    uint16_t fastloadoffs,fastloadlen;
    uint16_t mincodeswapareasize,winver;
};

struct nesegtable
{
    uint16_t sectoroffs;
    uint16_t seglength;
    uint16_t segflags;
    uint16_t minalloc;
};

struct nesegtablereloc
{
    uint8_t reloctype,relocsort;
    uint16_t segm_offs;
    uint16_t index,indexoffs;
};

struct PEHEADER
{
    uint32_t sigbytes;
    uint16_t cputype,objects;
    uint32_t timedatestamp;
    uint32_t reserveda[2];
    uint16_t nt_hdr_size,flags;
    uint16_t reserved;
    uint8_t  lmajor,lminor;
    uint32_t reserved1[3];
    uint32_t entrypoint_rva;
    uint32_t reserved2[2];
    uint32_t image_base;
    uint32_t objectalign;
    uint32_t filealign;
    uint16_t osmajor,osminor;
    uint16_t usermajor,userminor;
    uint16_t subsysmajor,subsysminor;
    uint32_t reserved3;
    uint32_t imagesize;
    uint32_t headersize;
    uint32_t filechecksum;
    uint16_t subsystem,dllflags;
    uint32_t stackreserve,stackcommit;
    uint32_t heapreserve,heapcommit;
    uint32_t reserved4;
    uint32_t numintitems;
    uint32_t exporttable_rva,export_datasize;
    uint32_t importtable_rva,import_datasize;
    uint32_t resourcetable_rva,resource_datasize;
    uint32_t exceptiontable_rva,exception_datasize;
    uint32_t securitytable_rva,security_datasize;
    uint32_t fixuptable_rva,fixup_datasize;
    uint32_t debugtable_rva,debug_directory;
    uint32_t imagedesc_rva,imagedesc_datasize;
    uint32_t machspecific_rva,machspecific_datasize;
    uint32_t tls_rva,tls_datasize;
};

struct PEObjData
{
    char name[8];
    uint32_t virt_size,rva;
    uint32_t phys_size,phys_offset;
    uint32_t reserved[3],obj_flags;
};

struct peimportdirentry
{
    uint32_t originalthunkrva;
    uint32_t timedatestamp;
    uint32_t forwarder;
    uint32_t namerva;
    uint32_t firstthunkrva;
};

struct peexportdirentry
{
    uint32_t characteristics;
    uint32_t timedatestamp;
    uint16_t majver,minver;
    uint32_t namerva;
    uint32_t base;
    uint32_t numfunctions;
    uint32_t numnames;
    uint32_t funcaddrrva,nameaddrrva,ordsaddrrva;
};

struct perestable
{
    uint32_t flags;
    uint32_t timedatestamp;
    uint16_t majver,minver;
    uint16_t numnames,numids;
};

struct peleafnode
{
    uint32_t datarva;
    uint32_t size;
    uint32_t codepage;
    uint32_t reserved;
};

struct perestableentry
{
    uint32_t id;
    uint32_t offset;
};

struct perelocheader
{
    uint32_t rva;
    uint32_t len;
};

//bool FAR PASCAL savemessbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);

//loads file and sets up objects using data.cpp
class FileLoader
{
private:
    enum_EXEType g_EXEType;
    enum_EXEFormat exetype;
    FILE *efile;
    uint8_t *fbuff;
    uint8_t *rawdata;
    // added build 14 bugfix
    uint32_t pdatarva;
    uint8_t *	image_buf;
    uint32_t	image_len;

public:
    uint8_t *	entry_buf;
    ea_t	entry_offset;
    void GetEntrance(uint8_t *& _entry_buf, ea_t& _entry_offset)
    {
        _entry_buf = entry_buf;
        _entry_offset = entry_offset;
    }

    FileLoader(void);
    ~FileLoader(void);
    bool load(const char * fname);
    int getexetype(void);
    void setexetype(int etype);
    void savedb(char *fname,char *exename);
    bool loaddb(char *fname,char *exename);

    bool        if_valid_ea(ea_t ea);
private:
    void get_exetype();
    void	LoadPE(uint32_t offs);
    void readcomfile(uint32_t fsize);
    void readsysfile(uint32_t fsize);
    void readpefile(uint32_t offs);
    void readmzfile(uint32_t fsize);
    void readlefile(void);
    void readnefile(uint32_t offs);
    void reados2file(void);
    void readbinfile(uint32_t fsize);
    void subdirsummary(uint8_t *data,char *impname,uint32_t image_base);
    void leaf2summary(uint8_t *data,char *name,uint32_t image_base);
    void leafnodesummary(uint8_t *data,char *resname,uint32_t image_base);
};
extern	FileLoader* g_FileLoader;

// basic class for lptr's - pointers comprised of a segment and offset.
// the intention is that addresses are well defined within Borg. So comparison
// operators exist in a well defined way. Addresses are not converted to 32 bit
// equivalents or whatever for this.
class lptr				  //Pointer Struct 32-bit.
{
public:
    uint16_t segm;				 //segment
    uint32_t offs;				 //offset

public:
    lptr(){}
    lptr(uint16_t seg,uint32_t off);
    ~lptr(){}
    void assign(uint16_t seg,uint32_t off);
    bool operator==(lptr loc2);
    bool operator<=(lptr loc2);
    bool operator>=(lptr loc2);
    bool operator<(lptr loc2);
    bool operator>(lptr loc2);
    bool operator!=(lptr loc2);
    lptr operator+(uint32_t offs2);
    lptr operator++(int x);
    lptr operator+=(uint32_t offs2);
    lptr operator-(uint32_t offs2);
    uint32_t operator-(lptr loc2);
};

// predefined null pointer.
extern const lptr nlptr;

//#define GNAME_MAXLEN 40

#endif	//	FileLoad_H
