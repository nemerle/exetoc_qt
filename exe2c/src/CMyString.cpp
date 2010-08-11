// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CMyString.cpp

#include <cstring>
#include "00000.h"
#include "CMyString.h"

CMyString::CMyString()
{
        this->m_bufptr = 0;
        this->m_curlen = 0;
        this->m_maxlen = 0;
}

CMyString::~CMyString()
{
    this->Clear();
}

void CMyString::Clear()
{
    delete [] this->m_bufptr;
    this->m_bufptr = 0;
    this->m_curlen = 0;
    this->m_maxlen = 0;
}

void CMyString::strcat(const char * str)
{
    size_t len = strlen(str);
    while (m_curlen + len + 1 > m_maxlen)
    {
        char * pnew = new char[m_maxlen + MEMORY_STEP];
        memcpy(pnew, m_bufptr, m_maxlen);
        delete m_bufptr;
        m_bufptr = pnew;
        m_maxlen += MEMORY_STEP;
    }
    memcpy(m_bufptr + m_curlen, str, len+1);	//	include last EOS
    m_curlen += len;
}

const char *CMyString::GetString()
{
    static const char* p = "";
    if (m_bufptr != 0)
        return m_bufptr;
    return p;
}

char *	CMyString::GetWritableString()
{
    return m_bufptr;
}

size_t	CMyString::GetLength()
{
    return this->m_curlen;
}
