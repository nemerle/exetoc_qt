// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

////#include "stdafx.h"
#include <QString>
#include <QStringList>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include "Cbuf.h"

//#include <io.h>

//	----------------------------------------------------------
CCbuf::CCbuf()
{
    this->f_str = 0;
}

// split into lines
// remove comments
// remove line continuations
void CCbuf::LoadFile(FILE *f)
{
    QString v;
    fseek(f,0,SEEK_END);
    size_t flen= ftell(f);
    char *buf = new char[flen];
    fseek(f,0,SEEK_SET);
    fread(buf,1,flen,f);
    v=QString::fromAscii(buf,flen);
	m_p = new char[flen+1];	//thats enough
	m_len = 0;

    for (size_t i = 0; i < flen; i++)
        OneChar(buf[i]);
    OneChar(EOF);
    v.replace(QRegExp("/\\*.*\\*/"),""); // remove /* */, does not handle nested comments
    v.replace(QRegExp("//[^\n]*\\n"),"\n"); // replace comments //
    v.replace(QRegExp("[\\n|\\r]+"),"\n");  // replace multiple newlines
    v.replace(QRegExp("\\\\\\s*\\n"),"\n"); // replace line continuations
    m_lines = v.split('\n',QString::SkipEmptyParts);
    for(size_t i=m_lines.size(); i>0; --i)
    {
        m_lines[i-1]=m_lines[i-1].simplified();
    }
    delete [] buf;
    assert(m_len <= flen+1);
}

void CCbuf::OneLine(const char * pstr)
{
	while (*pstr)
	{
		OneChar(*pstr++);
	}
	OneChar(0);
}
void CCbuf::OneChar(int c3)
{
    int i = m_len;
    char * p = m_p;

	if (c3 == '\r')
		c3 = '\n';

	// c3 is the input char, or EOF
	// If so, put the c3 added to the p referred to buf, the same time, len ++

	if (c3 == EOF)
	{
		assert(f_str == 0);
		OneChar(0);
		return;
	}
	char c = (char)c3;

    if ( ( f_str == 0) &&  ( i >= 2) )
	{
        if (memcmp(p+i-2,"//",2)==0)
		{
			if (c == '\n' || c == 0)
			{
				m_len -= 2;
				OneChar(0);
				return;
			}
			else
				return;	//do nothing
		}
        if (memcmp(p+i-2,"/*",2)==0)
		{
			static char c1 = 0;
			if (c1 == '*' && c == '/')
			{
				m_len -= 2;
				return;
			}
			c1 = c;
			return;
		}
	}

	if ( f_str )
	{
		static bool f = false;	//	for '\\'
		if (c == 0 || c == '\n')
            assert(!"error");	//error !
		if (f)
			f = false;
		else
		{
			if (c == '\\')
				f = true;
			if (c == f_str)
				f_str = 0;
		}
	}
    else if (c == '\'' || c == '\"')
			f_str = c;


	switch (c)
	{
	case 0:
	case '\n':
		p[i] = 0;
		m_len++;
		if (i > 0)
		{
			// Get rid of the spaces behind the
			// Blank lines removed
			if (p[i-1] == ' ' || p[i-1] == '\0')
			{
				p[i-1] = 0;
				m_len--;
				return;
			}
			// Check is not the Po line breaks
			if ( p[i-1] == '\\')
			{
				m_len -= 2;
                return;
			}
		}
		break;
	case '\t':
	case ' ':
		if (i == 0 || p[i-1] == '\0')
            break;	//The first character is a space is not allowed
		if (f_str == 0 && p[i-1] == ' ')
            break;	//If you already have a space in front, even if the

		c = ' ';
        //fallthrough
	default:
		p[i] = c;
		m_len++;
		break;
	}
}

