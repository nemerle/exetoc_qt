// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	XML.h

#ifndef	XML_H
#define XML_H
#include <QColor>
#include "XMLTYPE.h"

class I_COLOROUT
{
public:
    virtual void SetBKColor(QColor textcolor) = 0;
    virtual void ColorOut(const char * str, long len, QColor textcolor) = 0;
    // str is a very long string, this only shows part of the previous length of len
    //color for foreground character
    //if you see '\n' then wrap
};
typedef void (*Func_ColorOut)(const char * str, long len, QColor color, long pos);

class CXmlList
{
    CXmlList();
    CXmlList(int){}
    friend class CXmlPrt;

    XMLTYPE m_xmltype;
    void *	m_p;
    size_t	m_posfrom;
    size_t	m_posto;

    CXmlList* m_sub;
    CXmlList* m_next;
    CXmlList* GetLast_willbegin();
    CXmlList* GetLast_willend();
public:
    static CXmlList* new_CXmlList(XMLTYPE xmltype, void * p, size_t posfrom);
    void Clicked(long x1,long x2);
    bool GetRightWord(long curpos, long &posfrom, long &posto);
    bool GetLeftWord(long curpos, long &posfrom, long &posto);
    CXmlList* GetCurWord(size_t curpos, long &posfrom, long &posto);
    ~CXmlList();
    XMLTYPE XMLend(XMLTYPE xmltype, size_t pos);
    void XMLbegin(XMLTYPE xmltype, void * p, size_t pos);

    void prtprtout(const char * str, class CXmlOutPro* prt);
    void Display(const char * pstr, I_COLOROUT* iColorOut, XMLTYPE curw_xmltype, void * curw_p);
};

//void Del_XmlRec_List(XMLREC* pfirst);

#endif	//	XML_H
