// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


//	exe2c project

////#include "stdafx.h"
#include <stdint.h>
#include "00000.h"
#include "FileLoad.h"

#define	SEG0	0x1000
#define	Load_Resources	0
#define	Load_Debug	0
#define	Load_Data	1

enum_EXEType g_EXEType = (enum_EXEType)0;

FileLoader* g_FileLoader = NULL;

FileLoader::FileLoader(void)
{
    efile=0;
    exetype=UNKNOWN_EXE;
    fbuff=NULL;
    g_EXEType = (enum_EXEType)0;
    g_FileLoader=this;
}

FileLoader::~FileLoader(void)
{
    delete fbuff;
    fclose(efile);
}

bool	FileLoader::if_valid_ea(ea_t ea)
{
    //FIXME: this should check the actual image for extents.
    switch (g_EXEType)
    {
    case enum_PE_sys:
        return true;
    case enum_PE_exe:
        if (ea < 0x400000)
            return false;
        return true;
    }
    return true;
}
void FileLoader::get_exetype()
{
    char mzhead[2]={0},exthead[2]={0};
    uint32_t num;
    uint32_t pe_offset=0;

    exetype = UNKNOWN_EXE;
    num=fread(mzhead,1,2,efile);
    if (num != 2)
        return;
    if (((mzhead[0]=='M')&&(mzhead[1]=='Z'))||((mzhead[0]=='Z')&&(mzhead[1]=='M')))
    {
        exetype = BIN_EXE;

        fseek(efile,0x3c,SEEK_SET);
        if ( fread(&pe_offset,1,4,efile)==4 )
        {
            fseek(efile,pe_offset,SEEK_SET);
        }
        if ( fread(exthead,1,2,efile)==2 )
        {
            if ( ((short int *)exthead)[0]==0x4550 )
                exetype=PE_EXE;
            else if ( ((short int *)exthead)[0]==0x454e )
                exetype=NE_EXE;
            else if ( ((short int *)exthead)[0]==0x454c )
                exetype=LE_EXE;
            else if ( ((short int *)exthead)[0]==0x584c )
                exetype=OS2_EXE;
            else
                exetype=MZ_EXE;
        }
    }
}
//checks header info, puts up initial loading dialog box and
//selects info routine for file.
bool FileLoader::load(const char * fname)
{
    uint32_t pe_offset;
    uint32_t fsize;
    if ( efile!=0 )return false;

    efile=fopen(fname,"rb");
    if ( efile==0 )
        return false;

    get_exetype();

    if (exetype != PE_EXE)	//only support PE now
        return false;
    fseek(efile,0,SEEK_END);
    fsize=ftell(efile);
    fbuff=new BYTE[fsize];
    fseek(efile,0,SEEK_SET);
    fread(fbuff,1,fsize,efile);

    pe_offset = *(uint32_t *)(fbuff+0x3c);
    //DialogBox(hInst,MAKEINTRESOURCE(D_checktype),mainwindow,(DLGPROC)checktypebox);
    //if(!SEG0)
    //{
    //SEG0=0x1000;
    // MessageBox(mainwindow,"Sorry - Can't use a zero segment base.\nSegment Base has been set to 0x1000"
    //  ,"Borg Message",MB_OK);
    //}
    //dsm.dissettable();
    switch ( exetype )
    {
    case BIN_EXE:
        readbinfile(fsize);
        break;
    case PE_EXE:
        //readpefile(pe_offset);
        LoadPE(pe_offset);
        break;
    case MZ_EXE:
        readmzfile(fsize);
        break;
    case OS2_EXE:
        reados2file();
        fclose(efile);
        efile=0;
        exetype=UNKNOWN_EXE;
        return false; // at the moment;
    case COM_EXE:
        readcomfile(fsize);
        break;
    case SYS_EXE:
        readsysfile(fsize);
        break;
    case LE_EXE:
        readlefile();
        fclose(efile);
        efile=0;
        exetype=UNKNOWN_EXE;
        return false; // at the moment;
    case NE_EXE:
        readnefile(pe_offset);
        break;
    default:
        fclose(efile);
        efile=0;
        exetype=UNKNOWN_EXE;
        return false;
    }
    return true;
}
void FileLoader::readcomfile(uint32_t fsize)
{
}
void FileLoader::readsysfile(uint32_t fsize)
{
}
void FileLoader::readmzfile(uint32_t fsize)
{
}
void FileLoader::readlefile(void)
{
}
void FileLoader::readnefile(uint32_t )
{
}
void FileLoader::reados2file(void)
{
}
void FileLoader::readbinfile(uint32_t fsize)
{
}
bool	IfInWorkSpace(ea_t off)
{	//	check if off lie in our work space
    //Do something about it later, for the time being simple check
        if (off > 0x400000 && off < 0x600000)
                return true;
        return false;
}
