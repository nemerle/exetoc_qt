// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

// DataType.cpp: implementation of the CDataTypeMng class.
//
//////////////////////////////////////////////////////////////////////

#include <cstdio>
#include "DataType.h"
#include "ParseHead.h"
#include <QMessageBox>
//#include	"CISC.h"



static bool exe2c_Init()
{

    if (true)//GetFileAttributes("e2c_define.h") != -1)
    {
        //CParseHead ph;
        //ph.ParseHeadFile("e2c_define.h");

//        g_FuncDefineMng.prtall();
    }
    else
    {
        QMessageBox box;
        box.setText("not find e2c_define.h");
        box.setIcon(QMessageBox::Warning);
        box.exec();
    }
        return true;
}


static void exe2c_Exit()
{
}

class CDataType_cpp
{
public:
    CDataType_cpp()
    {
        exe2c_Init();
    }
    ~CDataType_cpp()
    {
        exe2c_Exit();
    }
};

CDataType_cpp g_CDataType_cpp;


int log_prtl(const char * fmt,...);



const char * my_itoa(int i)
{
    static char buf[80];
    sprintf(buf, "%d", i);
    return buf;
}

const char * CallConvToName(enum_CallC ec)
{
    switch (ec)
    {
    case enum_stdcall: return "__stdcall";
    case enum_cdecl:   return "__cdecl";
    case enum_pascal:  return "PASCAL";
    case enum_fastcall:return "__fastcall";
    default:
        return "__unknown";
    }
}
