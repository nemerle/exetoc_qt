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

extern enum_EXEType g_EXEType;

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
        WORD sig;
        WORD numbytes,numpages;
        WORD numrelocs,headersize;
        WORD minpara,maxpara;
        WORD initialss,initialsp;
        WORD csum;
        uint32_t csip;
        WORD relocoffs;
        WORD ovlnum;
};

struct neheader
{
        WORD sig;
        WORD linkerver;
        WORD entryoffs,entrylen;
        uint32_t filecrc;
        WORD contentflags;
        WORD dsnum;
        WORD heapsize,stacksize;
        uint32_t csip,sssp;
        WORD numsegs,nummodules;
        WORD nonresnamesize;
        WORD offs_segments,offs_resources,offs_resnames,offs_module,offs_imports;
        uint32_t nonresnametable;
        WORD movableentries;
        WORD shiftcount;
        WORD numresources;
        BYTE targetos,os_info;
        WORD fastloadoffs,fastloadlen;
        WORD mincodeswapareasize,winver;
};

struct nesegtable
{
        WORD sectoroffs;
        WORD seglength;
        WORD segflags;
        WORD minalloc;
};

struct nesegtablereloc
{
        BYTE reloctype,relocsort;
        WORD segm_offs;
        WORD index,indexoffs;
};

struct PEHEADER
{
        uint32_t sigbytes;
        WORD cputype,objects;
        uint32_t timedatestamp;
        uint32_t reserveda[2];
        WORD nt_hdr_size,flags;
        WORD reserved;
        BYTE lmajor,lminor;
        uint32_t reserved1[3];
        uint32_t entrypoint_rva;
        uint32_t reserved2[2];
        uint32_t image_base;
        uint32_t objectalign;
        uint32_t filealign;
        WORD osmajor,osminor;
        WORD usermajor,userminor;
        WORD subsysmajor,subsysminor;
        uint32_t reserved3;
        uint32_t imagesize;
        uint32_t headersize;
        uint32_t filechecksum;
        WORD subsystem,dllflags;
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
        WORD majver,minver;
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
        WORD majver,minver;
        WORD numnames,numids;
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
        enum_EXEFormat exetype;
        FILE *efile;
        BYTE *fbuff;
        BYTE *rawdata;
        // added build 14 bugfix
        uint32_t pdatarva;
    BYTE *	image_buf;
    uint32_t	image_len;

public:
    BYTE *	entry_buf;
    ea_t	entry_offset;
    void GetEntrance(BYTE *& _entry_buf, ea_t& _entry_offset)
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
        void subdirsummary(BYTE *data,char *impname,uint32_t image_base);
        void leaf2summary(BYTE *data,char *name,uint32_t image_base);
        void leafnodesummary(BYTE *data,char *resname,uint32_t image_base);
};
extern	FileLoader* g_FileLoader;

// basic class for lptr's - pointers comprised of a segment and offset.
// the intention is that addresses are well defined within Borg. So comparison
// operators exist in a well defined way. Addresses are not converted to 32 bit
// equivalents or whatever for this.
class lptr				  //Pointer Struct 32-bit.
{
public:
        WORD segm;				 //segment
        uint32_t offs;				 //offset

public:
        lptr(){}
        lptr(WORD seg,uint32_t off);
        ~lptr(){}
        void assign(WORD seg,uint32_t off);
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

bool	if_valid_ea(ea_t ea);

//#define GNAME_MAXLEN 40

#endif	//	FileLoad_H
