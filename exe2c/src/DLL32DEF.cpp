// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
//	exe2c project

#include <list>
#include <cstring>
#include <string>
#include <cassert>
#include <boost/filesystem.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/bind.hpp>
#include "00000.h"
#include "DLL32DEF.h"
#include "strparse.h"

using namespace boost::lambda;
using namespace std;
struct ord_st
{
        WORD ord;
        std::string name;
};

typedef std::list<ord_st> ORD_LIST;

struct deffile_st
{
    ~deffile_st()
    {
        delete m_list;
        m_list=0;
    }
    std::string m_fname;
    ORD_LIST* m_list;
};

ORD_LIST* DefName_2_list(char * fname);
typedef std::list<deffile_st> lDefFile;
lDefFile g_FirstDef;


void onExit_DLL32DEF()
{
    g_FirstDef.clear();
}
extern boost::filesystem::path GetMyExePath();
extern	char g_mypath[];
CCbuf* ReadDefFile(const std::string & fname)
{
        boost::filesystem::path path=GetMyExePath()/"def"/(fname+".def");
        FILE* f = fopen(path.native_file_string().c_str(),"rb");
        if (f == NULL)
        {
                alert_prtf("Cannot load def file:\n%s",fname.c_str());
                return NULL;
        }
        CCbuf *pInfo = new CCbuf;
        pInfo->LoadFile(f);
        fclose(f);
        return pInfo;
}
std::string DLLDEF_Get_ApiName_from_ord(char * pDLLname, WORD ord)
{
    if (strcmp(pDLLname,"MFC42.DLL") == 0)
    {
        if (ord == 0x628)
            ; //TODO: nop();
        //return "AfxWinMain";
        //return NULL;
    }
    char fname[80];
    {
        strcpy(fname,pDLLname);
        int len = strlen(fname);
        if (len < 5)
            return NULL;
        if (fname[len-4] != '.')
            return "";
        fname[len-4] = '\0';

    }

    ORD_LIST* olist = DefName_2_list(fname);
    if (olist == NULL)
        return "";

    ORD_LIST::iterator pos = olist->begin();
    while (pos!=olist->end())
    {
        ord_st& p = *pos;//olist->;
        ++pos;
        if (p.ord == ord)
        {
            assert(p.name.size() < 130);
            return p.name;
        }
    }
    return "";
}
char * * ppit = NULL;
ORD_LIST* DefName_createlist(char * fname);
ORD_LIST* DefName_2_list(char * fname)
{
    string search_for=fname;
    lDefFile::iterator iter;
    iter = std::find_if(g_FirstDef.begin(),
                        g_FirstDef.end(),
                        boost::lambda::bind(&deffile_st::m_fname,_1)==search_for);

    if(iter!=g_FirstDef.end())
        return (*iter).m_list;
    //	not find, lets create one
    return DefName_createlist(fname);
}

void Def_BuildList(ORD_LIST & list, char * buf, SIZEOF len);
ORD_LIST* DefName_createlist(char * fname)
{
    CCbuf* info = ReadDefFile(fname);

    if (info == NULL)
        return NULL;

    ORD_LIST *result=new ORD_LIST;
    deffile_st newlist;
    newlist.m_fname = fname;
    newlist.m_list = result;
    g_FirstDef.push_back(newlist);
    newlist.m_list=0; // detaching from newlist
    Def_BuildList(*result, info->m_p, info->m_len);
    delete [] info->m_p;
    delete info;
    return result;
}
#include <QString>
void Def_BuildList(ORD_LIST & list, char * pbuf, SIZEOF len)
{
    const char * p = pbuf;

    while (*p == ';')
        p += strlen(p)+1;
    QString str(p);
    if (str.startsWith("LIBRARY",Qt::CaseInsensitive)==false)
        return;
    p += strlen(p)+1;
    QString str2(p);
    if (str2.startsWith("EXPORTS")==false)
        return;
    p += strlen(p)+1;
    //	now, really
    while (p < pbuf+len)
    {
        char name[256];
        name[255] = '\0';
        get_1part(name,p);
        assert(name[255] == '\0');
        if (*p != '@')
        {
            p += strlen(p)+1;
            continue;
        }
        p++;
        skip_space(p);
        uint32_t d = 0;
        sscanf(p,"%d",&d);

        ord_st pnew;
        pnew.ord = (WORD)d;
        if (d == 0x9d0)
            ; //TODO: nop();
        pnew.name = name;
        int llen = pnew.name.size();
        assert(llen<130);
        list.push_front(pnew);
        pnew.name.clear();	//
        p += strlen(p)+1;
    }
}
