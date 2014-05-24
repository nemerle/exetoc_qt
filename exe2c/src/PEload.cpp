// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


//	exe2c project

//#include "stdafx.h"

#include <cassert>
#include <QString>
#include <cstring>
#include <algorithm>
#include <QDebug>
#include "00000.h"
#include "FileLoad.h"
#include "ApiManage.h"

void	Disassembler_Init_offset(const uint8_t * code_buf, ea_t code_offset);
extern std::string DLLDEF_Get_ApiName_from_ord(const char *pDLLname, WORD ord);
uint8_t *	ea2ptr(ea_t pos);
ea_t ptr2ea(void* p);
uint8_t	Peek_B(ea_t pos);
WORD	Peek_W(ea_t pos);
uint32_t	Peek_D(ea_t pos);

//#include "Deasm_Init.h"

static bool	PELoad_isMFC = false;

int RelocationPE(PEHEADER* peh);
int	RelocImportTable(PEHEADER* peh);

void FileLoader::LoadPE(uint32_t peoffs)
{
    uint8_t * pestart = &fbuff[peoffs];
    PEHEADER* peh = (PEHEADER *)pestart;

    if (peh->flags & 0x2000)
    {
        g_EXEType = enum_PE_dll;
    }
    switch (peh->subsystem)
    {
        case 1:		//	native device driver
            g_EXEType = enum_PE_sys;
            break;
        case 2:		//	WINDOWS GUI
        case 3:		//	WINDOWS CUI console
            g_EXEType = enum_PE_exe;
            break;
        default:
            alert_prtf("subsystem = %x",peh->subsystem);
            assert(0);
    }
    //???? hpp_init();

    PEObjData *pdata = (PEObjData *)(pestart+sizeof(PEHEADER)+(peh->numintitems-0x0a)*8);

    uint32_t imagelen = peh->imagesize;

    uint8_t * p0 = (uint8_t *)new uint8_t[imagelen];//VirtualAlloc(0,imagelen,MEM_COMMIT,PAGE_READWRITE);
    if (p0 == NULL)
    {
        //error("VirtualAlloc get NULL\n");
    }
    //uint8_t * p0 = new uint8_t[imagelen];
    memcpy(p0, pestart, peh->headersize);

    for ( int i=0;i<peh->objects;i++ )
    {
        memcpy(p0 + pdata[i].rva,
               fbuff + pdata[i].phys_offset,
               pdata[i].phys_size);
    }

    this->image_buf = p0;
    this->image_len = imagelen;
    this->entry_buf = p0 + peh->entrypoint_rva;
    this->entry_offset = peh->entrypoint_rva+peh->image_base;

    // Because the file was relocate to address different virtual addresses, so remember this difference
    // Afterwards the main program will use only that offset to access data, regardless of the actual buffer
    // Here because later on  RelocImportTable will use it
    Disassembler_Init_offset(this->entry_buf, this->entry_offset);

    //RelocationPE((PEHEADER*)p0);
    RelocImportTable((PEHEADER*)p0);

    //	proc2(p0 + peh->entrypoint_rva,
    //		  peh->entrypoint_rva+peh->image_base);

    //	VirtualFree(p0,imagelen,0);
    //delete p0;

}
/*
    problems left:
    1.	if the module need a private DLL, then maybe we should change
                    GetModuleHandle
            with
                    LoadLibrary
            and do "FreeLibrary" somewhere
    2.	Only do import with name, should add import with ORD
*/
typedef struct
{
    uint32_t	tbl1_rva;	//00
    uint32_t	dummy1;		//04
    uint32_t	dummy2;		//08
    uint32_t	dllname_rva;	//0c
    uint32_t	tbl2_rva;	//10
    //size of = 0x14
}IMP_0;

#include "DLL32DEF.h"

int	RelocImportTable(PEHEADER* peh)
{
    uint8_t * pestart = (uint8_t *)peh;
    uint8_t * pimp = pestart + peh->importtable_rva;

    //	uint32_t	impsize = peh->import_datasize;

    for (IMP_0*	pimp0 = (IMP_0*)pimp; pimp0->tbl1_rva != 0; pimp0++)
    {
        char *	pDLLname = (char *)pestart+pimp0->dllname_rva;
        log_prtl("pDLLname is %s ",pDLLname);
        //HMODULE hModule = GetModuleHandle(pDLLname);	//should I use Load ?
        int32_t *p2 = (int32_t *)(pestart+pimp0->tbl2_rva);
        uint32_t d;
        while ((d = *p2) != 0)
        {
            std::string apiname;
            //uint32_t apiaddr = (uint32_t)GetProcAddress(hModule,apiname);
            static uint32_t ggdd = 0xACBC0000;
            uint32_t apiaddr = ggdd++;

            if ((d & 0xffff0000) == 0x80000000)
            {	//Input by ord
                apiname = DLLDEF_Get_ApiName_from_ord(pDLLname,(WORD)d);
                if (apiname.size() == 0)
                {
                    QString bufname;
                    bufname = QString("ord_%1_%2").arg((WORD)d,16).arg(pDLLname);
                    bufname.replace(".","_");
                    apiname = bufname.toStdString();
                }
                //assert(apiname.size() < 80);
            }
            else
            {
                uint8_t * pimpitem = pestart+d;
                apiname = (char *)pimpitem+2;
                assert(apiname.size() < 80);
            }
            log_prtl("impapi is %s , %x",apiname,apiaddr);	// + 2byte

            *p2 = apiaddr;	//fill imp table with api address
            ApiManage::get()->New_ImportAPI(apiname, ptr2ea((uint8_t *)p2));
            p2++;
        }
    }


    return 0;
}

#define KSPE_IMAGE_SIZEOF_BASE_RELOCATION          8    // Because exclude the first TypeOffset

typedef struct _KSPE_IMAGE_BASE_RELOCATION {
    uint32_t   VirtualAddress;
    uint32_t   SizeOfBlock;
    WORD    TypeOffset[1];
} KSPE_IMAGE_BASE_RELOCATION, *PKSPE_IMAGE_BASE_RELOCATION;

#define KSPE_IMAGE_REL_BASED_ABSOLUTE              0
#define KSPE_IMAGE_REL_BASED_HIGH                  1
#define KSPE_IMAGE_REL_BASED_LOW                   2
#define KSPE_IMAGE_REL_BASED_HIGHLOW               3
#define KSPE_IMAGE_REL_BASED_HIGHADJ               4
#define KSPE_IMAGE_REL_BASED_MIPS_JMPADDR          5




ea_t	Find_Main(ea_t start)
{
    ea_t p = start;
    if (Peek_W(p) == 0x8B55
            && Peek_W(p+3) == 0xFF6A
            && Peek_W(p+0x1d) == 0xEC83
            && Peek_B(p+0xaf) == 0xE8		//	401780 - 4016d1 = af
            )
    {
        p += 0xaf;
        uint32_t d = Peek_D(p+1);
        //alert_prtf("p = %x, d = %x",p,d);
        //alert_prtf(" I get main = %x",p+5+d);
        return p+5+d;
    }
    if (Peek_W(p) == 0x8B55
            && Peek_W(p+3) == 0xFF6A
            && Peek_W(p+0x1d) == 0xEC83
            && Peek_B(p+0xc9) == 0xE8		//	00401149 - 00401080 = c9
            )
    {
        p += 0xc9;
        uint32_t d = Peek_D(p+1);
        //alert_prtf("p = %x, d = %x",p,d);
        //alert_prtf(" I get main = %x",p+5+d);
        return p+5+d;
    }

    if (Peek_W(p) == 0xa164
            && Peek_W(p+0x16) == 0x8964
            && Peek_W(p+0x2f) == 0x15ff
            && Peek_B(p+0x152) == 0xE8		//	1A42 - 18f0=152
            )
    {
        p += 0x152;
        uint32_t d = Peek_D(p+1);
        //alert_prtf("p = %x, d = %x",p,d);
        //alert_prtf(" I get main = %x",p+5+d);
        return p+5+d;   //This is the WinMain
    }
    return start;
}

ea_t	Find_WinMain(ea_t start)
{
    ea_t p = start;
    if (Peek_W(p) == 0x8B55
            && Peek_W(p+3) == 0xFF6A
            && Peek_W(p+0x1d) == 0xEC83
            && Peek_B(p+0xc9) == 0xE8
            )
    {
        p += 0xc9;
        uint32_t d = Peek_D(p+1);
        //alert_prtf("p = %x, d = %x",p,d);
        //alert_prtf(" I get WinMain = %x",p+5+d);
        log_prtl(" I get WinMain = %x",p+5+d);
        return p+5+d;
    }
    if (Peek_W(p) == 0x8B55
            && Peek_W(p+3) == 0xFF6A
            && Peek_W(p+0x1d) == 0xEC83
            && Peek_B(p+0x12f) == 0xE8
            )	//	This seems to be used by MFC
    {
        p += 0x12f;
        uint32_t d = Peek_D(p+1);
        //alert_prtf("p = %x, d = %x",p,d);
        //alert_prtf(" I get main = %x",p+5+d);
        log_prtl(" I get MFC WinMain = %x",p+5+d);
        PELoad_isMFC = true;
        return p+5+d;
    }
    //alert_prtf("not find WinMain");
    log_prtl("not find WinMain");
    return start;
}
bool	Valid_ea(ea_t ea)
{
    if (ea >= 0x400000 && ea < 0x80000000)
        return true;
    return false;
}
void OneItem_Init(ea_t ea);
void	SomeOther_about_MFC_load()
{
    if (! PELoad_isMFC)
        return;
    ea_t start = g_FileLoader->entry_offset;

    uint8_t * p = ea2ptr(start);

    p += 0xbb;
    if (p[0] != 0x68 || p[5] != 0x68 || p[10] != 0xe8)
        return;		//	Here are two push immed

    ea_t s_init = *(uint32_t *)(p+6);
    ea_t e_init = *(uint32_t *)(p+1);

    p = ea2ptr(s_init);

    ea_t ea = *(uint32_t *)(p+4);
    if ( ! Valid_ea(ea))
        return;

    ea = *(uint32_t *)(p+8);
    if ( ! Valid_ea(ea))
        return;

    OneItem_Init(ea);

    //alert_prtf("here init start = %x, end = %x, useful = %x",s_init, e_init, ea);
}

void	WinApp_vftbl(ea_t vftbl);

void OneItem_Init(ea_t ea)
{
    uint8_t * p = ea2ptr(ea);
    if (*p != 0xe8)
        return;

    p += *(uint32_t *)(p+1);
    p += 5;

    if (p[0] != 0xb9 || p[5] != 0xe9)
        return;

    //ea_t theapp = *(uint32_t *)(p+1);

    //alert_prtf("theapp = %x",theapp);

    p += *(uint32_t *)(p+6);
    p += 10;

    if (p[5] != 0xe8)
        return;
    if (*(WORD*)(p+10) != 0x06c7)
        return;

    ea_t vftbl = *(uint32_t *)(p+12);

    //alert_prtf("vftbl = %x",vftbl);

    WinApp_vftbl(vftbl);
}

#define	WinApp_InitInstance	0x16

void	WinApp_vftbl(ea_t vftbl)
{
    alert_prtf("vftbl = %x",vftbl);
    uint32_t* p = (uint32_t*)ea2ptr(vftbl);

    ea_t ea_InitInstance = p[WinApp_InitInstance];
    alert_prtf("ea_InitInstance = %x",ea_InitInstance);
}
