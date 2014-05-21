// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CXmlPrt.h

#ifndef	CXmlPrt_H
#define CXmlPrt_H
#include <string>
#include <QString>
#include "XmlList.h"
class I_XmlOut
{
public:
    virtual void  prtt(const char * s) = 0;
    virtual void  prtt(const std::string &s) = 0;
    virtual void  XMLbegin(enum XMLTYPE xmltype, void * p) = 0;
    virtual void  XMLend(enum XMLTYPE xmltype) = 0;
};
class XmlPrt : public I_XmlOut
{
        XmlList* 	m_xmllist;
        QString	m_str;

        XMLTYPE m_curword_type;
        void *	m_curword_p;

        std::string GetLine(int nLine);
    int GetPosXY(int x, int y);
public:

        XmlPrt();
        ~XmlPrt();

        virtual void prtt(const char * s);
        virtual void prtt(const std::string &s) {prtt(s.c_str());}
        virtual void XMLbegin(enum XMLTYPE xmltype, void * p);
        virtual void XMLend(enum XMLTYPE xmltype);

        void	prtprtout(class XmlOutPro* prt);

        void Clear();
        void Clicked(long x1,long x2);
        bool GetRightWord(long curpos, long &posfrom, long &posto);
        bool GetLeftWord(long curpos, long &posfrom, long &posto);
        bool GetCurWord(long curpos, long &posfrom, long &posto);
        int MoveHome(int nLine);
        int MoveLeftWord(int x, int y);
        int MoveRightWord(int x, int y);
        int WordToLeft(size_t x, int y);
        int WordToRight(int x, int y);
    std::string GetText(int y1, int x1, int y2, int x2);

    void GetItem(int x, int y, enum XMLTYPE& xmltype, void *& p);
    void Display(I_COLOROUT* iColorOut);
    int GetLineCount();
    int GetLineLength(int nLine);
    std::string GetString();
    void CommaLast();
    bool SetCurWord(int x, int y);
};

class XmlOutPro
{
    I_XmlOut* m_out;
    size_t    m_nIdent;
    bool b_OneLine;
    bool fHasSpace;
public:
    //Is true when requested the output to one line without a \n
    bool	m_f_prt_in_1line;
    bool	m_f_prt_in_comma;	//1 - use ',' instead of ';'
    XmlOutPro(I_XmlOut* out)
    {
        fHasSpace = false;
        b_OneLine = false;
        m_out = out;
        m_f_prt_in_1line = false;
        m_f_prt_in_comma = false;
        m_nIdent = 0;
    }
    void prtt(const char * s)
    {
        if (fHasSpace)
            this->m_out->prtt(" ");
        fHasSpace = false;

        this->m_out->prtt(s);
    }
    void prtt(const std::string &s)
    {
        if (fHasSpace)
            this->m_out->prtt(" ");
        fHasSpace = false;

        m_out->prtt(s);
    }
    void prtt(const QString &s)
    {
        if (fHasSpace)
            m_out->prtt(" ");
        fHasSpace = false;

        m_out->prtt(s.toStdString());
    }
    void prtslen(const char * s, int len);
    void XMLbegin(enum XMLTYPE xmltype, void * p)
    {
        if (fHasSpace)
            this->m_out->prtt(" ");
        fHasSpace = false;
        this->m_out->XMLbegin(xmltype,p);
    }
    void XMLend(enum XMLTYPE xmltype)
    {
        this->m_out->XMLend(xmltype);
        this->fHasSpace = true;
    }
    void prtspace(int n = 1)
    {
        for (int i=0; i<n; i++) prtt(" ");
    }
    void prtf(const char * fmt,...);
    void prtl(const char * fmt,...);
    void EOL();		//Under normal circumstances, is ";\n"
    void endline();
    void SetOneLine(bool b)
    {
        b_OneLine = b;
        m_f_prt_in_1line = b;
        m_f_prt_in_comma = b;
    }


    void	ident_add1();
    void	ident_sub1();
    void    ident();
    void 	prtl_ident(const char * fmt,...);
    void 	prtf_ident(const char * fmt,...);
    void    nospace()
    {
        fHasSpace = false;
    }

    // This class is intended, the interface provides us with a I_XmlOut, inconvenient to use
    // Need to be extended about
};

#endif
