// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


//	exe2c project

#include <cassert>
#include <cstdio>
#include "00000.h"
//#include <io.h>
#include "strparse.h"

void skip_space(const char * &p)
{
        while (*p == ' ')
                p++;
}
void skip_eos(const char * &p)
{
        if (*p == ' ')
                p++;
        if (*p == '\0')
                p++;
}
void get_1part(char * buf,const char * &p)
{	// P referred to, is not part of the storage space buf
    //	Separator can be '' , ',' , '\0' , '[' , ']' , ';' , '{' , '}'
    assert(p);
    assert(*p != ' ');
    *buf = 0;
    for(;;)
    {
        char c = *p;
        if (if_split_char(c))
        {
            *buf = '\0';
            if (c == ' ')	//	This is a special case, I hope to make more points
                p++;
            return;
        }
        *buf++ = c;
        p++;
    }
}

bool if_split_char(char c)
{
        switch( c )
        {
        case ' ':
        case '\t':
        case ',':
        case ';':
        case '\0':
        case '(':
        case ')':
        case '*':
        case '[':
        case ']':
        case '{':
        case '}':
        case '+':
        case '\x0a':
        case '\x0d':
                return true;
        }
        return false;
}
uint32_t Str2Num(const char * p)
{
    uint32_t d = 0;
    if (*p == '0' && (p[1] | 0x20) == 'x')
    {
        sscanf(p+2,"%x",&d);
        return d;
    }
    sscanf(p,"%d",&d);
    return d;
}

signed int Str2Int(const char * p)
{
    if (*p == '-')
    {
        return -Str2Int(p+1);
    }
    return (signed int)Str2Num(p);
}


