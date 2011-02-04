// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#ifndef	CCbuf_h
#define CCbuf_h
#include <cstdio>
#include <QStringList>
class CCbuf
{
public:
        char f_str;	//因为有'和"两种string形式 Because there 'and "two kinds of string form
        char * m_p;
        size_t	m_len;
        QStringList m_lines;
        CCbuf();
        ~CCbuf(){}

        void LoadFile(FILE *f);
        void OneChar(int c);
        void OneLine(const char * pstr);
};

#endif	//CCbuf_h
