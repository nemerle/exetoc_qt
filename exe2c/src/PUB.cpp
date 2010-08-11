// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//#include "stdafx.h"
#include	"CISC.h"
#include <QMessageBox>
void outstring_in_log(const char * str);

void	error(const char * msg)
{
    alert(msg);
    exit(0);
}
void	alert(const char * msg)
{

    QMessageBox box;
    box.setText("EXE2C alert");
    box.setInformativeText(msg);
    box.setIcon(QMessageBox::Warning);
    box.exec();
}
int		alert_prtf(const char * fmt,...)
{
    va_list argptr;
    int cnt;
        char buf[280];

    va_start(argptr, fmt);
    cnt = vsprintf(buf, fmt, argptr);
    va_end(argptr);

        alert(buf);

    return(cnt);
}

int log_prtf(const char * fmt,...)
{
    va_list argptr;
    int cnt;
        char buf[280];

    va_start(argptr, fmt);
    cnt = vsprintf(buf, fmt, argptr);
    va_end(argptr);

        outstring_in_log(buf);

    return(cnt);
}
int log_prtl(const char * fmt,...)
{
    va_list argptr;
    int cnt;
        char buf[280];

    va_start(argptr, fmt);
    cnt = vsprintf(buf, fmt, argptr);
    va_end(argptr);

        outstring_in_log(buf);
        outstring_in_log("\n");

    return(cnt);
}



void _warn(char * __cond, char * __file, int __line)
{
        char	buf[280];
        sprintf(buf,"Warn condition %s, in file %s, at line %d",
                        __cond,
                        __file,
                        __line);
        QMessageBox box;
        box.setText("warn");
        box.setInformativeText(buf);
        box.setIcon(QMessageBox::Warning);
        box.exec();
}

//char * new_str(const char * p)
//{
//        char * pnew = new char[strlen(p)+1];
//        strcpy(pnew,p);
//        return pnew;
//}

const char * prt_DWORD(uint32_t d)
{
        static char s[16];
        if (d < 16 || (d % 100) == 0)
                sprintf(s,"%d",d);
        else
                sprintf(s,"0x%x",d);
        return s;
}


