///////////////////////////////////////////////////////////////
//
// CLibScanner.cpp
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
//
///////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "types.h"
#include "LibScanner.h"
//#include <io.h>


//KS_DECLARE_COMPONENT(LibScanner, LIBSCANNER)


bool LibScanner_Init()
{
        return true;
}


void LibScanner_Exit()
{
}


bool LibScanner::BaseInit()
{
        //KICK_MFC();
        return true;
}


LibScanner::~LibScanner()
{
        ClearFunction();
    //KICK_MFC();
}


bool LibScanner::test()
{
        //KICK_MFC();
        return true;
}


void LibScanner::ClearFunction()
{
        for(FUNCTION_LIST::iterator it = m_funs.begin();it!=m_funs.end();++it)
        {
                PFUNCTION_SYMBOL pFun= *it;
                delete []pFun->FunRawData;
                delete  []((BYTE*)pFun);
        }
        m_funs.clear();
}

void LibScanner::ClearCOFFObject(COFFOBJECT_LIST &objs)
{
        for(COFFOBJECT_LIST::iterator it = objs.begin();it!=objs.end();++it)
        {
                COFFOBJECT obj;
                obj= *it;
                delete  []obj.lpBuffer;
        }
        objs.clear();
}
const char * LibScanner::COFFGetName(const IMAGE_SYMBOL* coff_sym, const char* coff_strtab)
{
        static	char	namebuff[9];
        const char*		nampnt;

        if (coff_sym->N.Name.Short)
        {
                memcpy(namebuff, coff_sym->N.ShortName, 8);
                namebuff[8] = '\0';
                nampnt = &namebuff[0];
        }
        else
        {
                nampnt = coff_strtab + coff_sym->N.Name.Long;
        }

        return nampnt;
}

PIMAGE_SECTION_HEADER LibScanner::FindSection(PCOFFOBJECT pObj,SHORT SectNumber)
{
        PIMAGE_FILE_HEADER pIFH = (PIMAGE_FILE_HEADER)pObj->lpBuffer;
        PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)(pObj->lpBuffer + IMAGE_SIZEOF_FILE_HEADER);
        for(int j=0;j<pIFH->NumberOfSections;j++)
        {
                if(j+1==SectNumber)
                        return &pISH[j];
        }
        return NULL;
}

void LibScanner::ScanFunction(FUNCTION_LIST & funs,PCOFFOBJECT pObj)
{
    PIMAGE_FILE_HEADER pIFH = (PIMAGE_FILE_HEADER)pObj->lpBuffer;
    PIMAGE_SYMBOL pIS = (PIMAGE_SYMBOL)(pObj->lpBuffer + pIFH->PointerToSymbolTable);
    const char * lpStrTab= (const char*)(pIS + pIFH->NumberOfSymbols);
    assert(sizeof(IMAGE_SYMBOL)==18);
    assert(sizeof(IMAGE_FILE_HEADER)==20);
    COFFSYMBOL_LIST syms;
    for(int k =0;k<pIFH->NumberOfSymbols;k++)
    {
        PIMAGE_SYMBOL pcurIS = pIS+k;

        COFFSYMBOL sym;
        memset(&sym,0,sizeof(COFFSYMBOL));
        strcpy(sym.Name,COFFGetName(pcurIS,lpStrTab));
        sym.SYMB_INX = k;
        sym.SECT_NUM = pcurIS->SectionNumber;
        sym.STOR_CLS = pcurIS->StorageClass;
        sym.TYPE     = pcurIS->Type;
        sym.VALUE    = pcurIS->Value;
        syms.push_back(sym);

        k+=pcurIS->NumberOfAuxSymbols;
    }

    assert(sizeof(IMAGE_RELOCATION)==10);
    for(int i =0;i<pIFH->NumberOfSymbols;i++)
    {
        PIMAGE_SYMBOL pSymb = pIS+i;
        const char * lpName = COFFGetName(pSymb,lpStrTab);
        if( !(ISFCN(pSymb->Type) &&pSymb->SectionNumber>0))
        {
            i+=pSymb->NumberOfAuxSymbols;
            continue;
        }

        PIMAGE_SECTION_HEADER pISH = FindSection(pObj,pSymb->SectionNumber);

        if(!pISH)
            continue;
        PFUNCTION_SYMBOL pFun = (PFUNCTION_SYMBOL) new BYTE[sizeof(FUNCTION_SYMBOL)+pISH->NumberOfRelocations*sizeof(REFSYMBOL)];
        assert(lpName==COFFGetName(pSymb,lpStrTab));
        strcpy(pFun->FunctionName , lpName);

        if(pISH->SizeOfRawData > pSymb->Value )
        {
            pFun->dwFuncLen  = pISH->SizeOfRawData - pSymb->Value;
            pFun->FunRawData = new BYTE[pFun->dwFuncLen];

            memcpy(pFun->FunRawData,(BYTE *)(pObj->lpBuffer + pISH->PointerToRawData) + pSymb->Value,pFun->dwFuncLen);

            PIMAGE_RELOCATION pIR = (PIMAGE_RELOCATION)(pObj->lpBuffer + pISH->PointerToRelocations);

            pFun->RefCount = pISH->NumberOfRelocations;

            for(int i =0;i<pFun->RefCount;i++)
            {
                PCOFFSYMBOL symb = FindSymbol(syms,pIR[i].SymbolTableIndex);
                strcpy(pFun->RefInfo[i].RefSymbol,symb->Name);
                pFun->RefInfo[i].RefOffset = pIR[i].VirtualAddress;
                pFun->RefInfo[i].RefType   = pIR[i].Type;

            }
            strcpy(pFun->ObjName,pObj->ObjName);

            funs.push_back(pFun);
        }
        else
        {
            assert("Error!");
            //TRACE("Error!\r\n");
        }
        i+=pSymb->NumberOfAuxSymbols;
    }
}

PCOFFSYMBOL LibScanner::FindSymbol(COFFSYMBOL_LIST &syms,int symIndx)
{
        for(COFFSYMBOL_LIST::iterator it = syms.begin();it!=syms.end();++it)
        {
                if((*it).SYMB_INX == symIndx)
                        return &(*it);
        }
        return NULL;
}


bool LibScanner::ScanCOFFObject(COFFOBJECT_LIST &objs,BYTE * lpBuffer,uint32_t Len)
{
        if(memcmp(lpBuffer,IMAGE_ARCHIVE_START,IMAGE_ARCHIVE_START_SIZE)!=0)
                return FALSE;

        PIMAGE_ARCHIVE_MEMBER_HEADER pSect = (PIMAGE_ARCHIVE_MEMBER_HEADER)(lpBuffer+IMAGE_ARCHIVE_START_SIZE);
        bool bFirst = FALSE;

        BYTE * lpNewPtr = (BYTE *)pSect;
        char *lpLongTable = NULL;
        while(lpNewPtr <lpBuffer + Len)
        {
                if(memcmp(pSect->Name,IMAGE_ARCHIVE_LINKER_MEMBER,16)==0)
                {
                        //Nothing
                }
                else if(memcmp(pSect->Name,IMAGE_ARCHIVE_LONGNAMES_MEMBER,16)==0)//LONG Name
                {
                        uint32_t theSecSize = atol((char *)pSect->Size);
                        lpLongTable  = ((char *)pSect) + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER);
                }
                else //Obj Section
                {
                        ptrdiff_t ObjOff = ((BYTE *)pSect) - lpBuffer;
                        char * lpEnd = (char *)&pSect->Name[15];

                        while(*lpEnd!='/')
                        {
                                lpEnd--;
                        }

                        char * lpObjName = NULL;
                        if(lpEnd == (char *)pSect->Name )
                        {//Long Name
                                int longNameOff = atol((char *)&pSect->Name[1]);
                                lpObjName = lpLongTable+longNameOff;
                        }
                        else
                        {
                                lpEnd++;
                                *lpEnd = 0;
                                lpObjName = (char *)pSect->Name;
                        }

                        COFFOBJECT Obj;
                        strcpy(Obj.ObjName,lpObjName);
                        Obj.Len		 = atol((char *)pSect->Size);
                        Obj.lpBuffer = new BYTE[Obj.Len];
                        Obj.StartOff = ObjOff;

                        BYTE * lpData = ((BYTE *)pSect) + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER);
                        memcpy(Obj.lpBuffer,lpData,Obj.Len);
                        objs.push_back(Obj);
                }
                lpNewPtr = (BYTE *)pSect;
                lpNewPtr += atol((char *)pSect->Size) + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER);
                if(*lpNewPtr=='\n')
                        lpNewPtr++;
                pSect = (PIMAGE_ARCHIVE_MEMBER_HEADER) lpNewPtr;
        }
        return TRUE;
}


void log_prtl(const char * fmt,...);
bool LibScanner::ScanLib(const char * szLib)
{
    log_prtl("Loading %s", szLib);
    FILE * pFile = fopen(szLib,"rb");
    if(pFile==NULL)
    {
        log_prtl("Load error: file not find %s", szLib);
        return FALSE;
    }
    fseek(pFile,0,SEEK_END);
    long fsize = ftell(pFile);
    fseek(pFile,0,SEEK_SET);

    BYTE * fbuf = new BYTE[fsize];
    if (fbuf == NULL)
    {
        fclose(pFile);
        return FALSE;
    }
    fread(fbuf,fsize,1,pFile);

    fclose(pFile);
    COFFOBJECT_LIST objs;
    if(!ScanCOFFObject(objs,fbuf,fsize))
    {
        delete []fbuf;
        return FALSE;
    }
    delete []fbuf;

    for(COFFOBJECT_LIST::iterator it = objs.begin();it!=objs.end();++it)
    {
        ScanFunction(m_funs,&(*it));
    }
    ClearCOFFObject(objs);

    log_prtl("%d function loaded.", this->m_funs.size());
    return TRUE;
}
PFUNCTION_SYMBOL LibScanner::GetFunctionInfo(const char * szFun)
{
        for(FUNCTION_LIST::iterator it = m_funs.begin();it!=m_funs.end();++it)
        {
                PFUNCTION_SYMBOL pFun= *it;

                if(strcmp(pFun->FunctionName,szFun)==0)
                        return pFun;
        }
        return NULL;
}
bool IfReAlloc(PFUNCTION_SYMBOL pFun, int offset)
{
    for (int i=0; i<pFun->RefCount; i++)
    {
        REFSYMBOL* p = &pFun->RefInfo[i];
        if (p->RefOffset == offset)
            return true;
    }
    return false;
}
void nop(int)
{
}
void nop(const void*)
{
}

//call    ??2@YAPAXI@Z    ; operator new(uint)
bool LibScanner::CheckSubFunc(PFUNCTION_SYMBOL pFun, PCBYTE phead)
{
    for (int i=0; i<pFun->RefCount; i++)
    {
        REFSYMBOL* p = &pFun->RefInfo[i];
        nop(p->RefType);
                PCBYTE p1 = phead + p->RefOffset;
                nop(p1);
                if (p->RefType == IMAGE_REL_I386_REL32	//==0x14
            && *(p1-1) == 0xe8) //e8是个call
        {
            signed int off = *(signed int*)p1;
            PCBYTE new_phead = p1 + off + 4;
            if (!strcmp(p->RefSymbol, this->CheckIfLibFunc(new_phead).c_str()))
            {//有一个就足够了
                return true;
            }
                        else
                                return false;
        }
    }
    return true;
}
bool LibScanner::CheckThisFunc(PFUNCTION_SYMBOL pFun, PCBYTE phead)
{
    PCBYTE phead1 = phead;

    PCBYTE psrc = pFun->FunRawData;
    int len = pFun->dwFuncLen;
    int offset = 0;
    while (len > 0)
    {
        if (IfReAlloc(pFun, offset))
        {
            offset += 4;
            len -= 4;
            psrc+=4;
            phead1+=4;
            continue;
        }
        if (*psrc != *phead1)
            return false;
        offset++;
        len--;
        psrc++;
        phead1++;
    }
    if (pFun->dwFuncLen < 0x200 && pFun->RefCount > 0)
    {//太短的函数，最好再检查一下它call的函数
        return CheckSubFunc(pFun, phead);
    }
    return true;
}
std::string LibScanner::CheckIfLibFunc(PCBYTE phead)
{
    for(FUNCTION_LIST::iterator it = m_funs.begin();it!=m_funs.end();++it)
    {
        PFUNCTION_SYMBOL pFun= *it;

        if (this->CheckThisFunc(pFun, phead))
        {
            return pFun->FunctionName;
        }
    }
    return "";
}
