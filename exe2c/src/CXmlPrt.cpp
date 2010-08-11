// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CXmlPrt.cpp

#include <cstdio>
#include	"00000.h"
#include	"CXmlPrt.h"
#include <assert.h>

CXmlPrt::CXmlPrt()
{
    m_xmllist = CXmlList::new_CXmlList(XT_invalid,NULL,0);
    m_str = new CMyString;

    m_curword_type = XT_invalid;
    m_curword_p = NULL;
}

CXmlPrt::~CXmlPrt()
{
    delete m_xmllist;
    m_xmllist=0;
    delete m_str;
    m_str=0;
}
void CXmlPrt::Clear()
{
    this->~CXmlPrt();

    m_xmllist = CXmlList::new_CXmlList(XT_invalid,NULL,0);
    m_str = new CMyString;
}

void CXmlPrt::prtt(const char * str)
{
    this->m_str->strcat(str);
}

void CXmlOutPro::prtslen(const char * s, int len)
{
    char buf[280];
    assert(len<280);
    strncpy(buf,s,len);
    buf[len] = 0;
    this->prtt(buf);
}
void CXmlOutPro::prtf(const char * fmt, ...)
{
    va_list argptr;
    int cnt;
    char buf[280];

    va_start(argptr, fmt);
    cnt = vsprintf(buf, fmt, argptr);
    va_end(argptr);

    this->prtt(buf);

    return;	//(cnt);
}

void CXmlOutPro::prtl(const char * fmt, ...)
{
    va_list argptr;
    int cnt;
    char buf[280];

    va_start(argptr, fmt);
    cnt = vsprintf(buf, fmt, argptr);
    va_end(argptr);

    this->prtt(buf);

    if (!this->m_f_prt_in_1line)
        this->prtt("\n");

    return;	//(cnt);
}
void	CXmlOutPro::ident_add1()
{
    this->m_nIdent++;
}
void	CXmlOutPro::ident_sub1()
{
    assert(this->m_nIdent);
    this->m_nIdent--;
}
void    CXmlOutPro::ident()
{
    if (m_f_prt_in_1line)
        return;
    for (size_t i=0; i<m_nIdent; i++)
        m_out->prtt("    ");
}
void 	CXmlOutPro::prtl_ident(const char * fmt,...)
{
    ident();

    va_list argptr;
    char buf[280];

    va_start(argptr, fmt);
    vsprintf(buf, fmt, argptr);
    va_end(argptr);

    prtl("%s",buf);
}

void 	CXmlOutPro::prtf_ident(const char * fmt,...)
{
    ident();

    va_list argptr;
    char buf[280];

    va_start(argptr, fmt);
    vsprintf(buf, fmt, argptr);
    va_end(argptr);

    prtf("%s",buf);
}

void CXmlOutPro::endline()
{
    if (!this->m_f_prt_in_1line)
        this->prtt("\n");
}
void CXmlOutPro::EOL()
{
    this->nospace();
    if (this->m_f_prt_in_comma)
        prtl(",");
    else
        prtl(";");
}


void CXmlPrt::XMLbegin(XMLTYPE xmltype, void * p0)
{
    long pos = m_str->GetLength();
    this->m_xmllist->XMLbegin(xmltype, p0, pos);
}

void CXmlPrt::XMLend(XMLTYPE xmltype)
{
    long pos = m_str->GetLength();
    XMLTYPE type1 = this->m_xmllist->XMLend(xmltype, pos);
}


//	---------------------------------------------------

void CXmlPrt::Clicked(long x1, long x2)
{
    this->m_xmllist->Clicked(x1,x2);

}
bool CXmlPrt::GetCurWord(long curpos, long &posfrom, long &posto)
{
    if (NULL != this->m_xmllist->GetCurWord(curpos, posfrom, posto))
        return TRUE;
    posfrom = curpos;
    posto = curpos+1;
    return FALSE;
}
bool CXmlPrt::GetLeftWord(long curpos, long &posfrom, long &posto)
{
    if (this->m_xmllist->GetLeftWord(curpos, posfrom, posto))
        return TRUE;
    posfrom = curpos;
    posto = curpos+1;
    return FALSE;
}
bool CXmlPrt::GetRightWord(long curpos, long &posfrom, long &posto)
{
    if (this->m_xmllist->GetRightWord(curpos, posfrom, posto))
        return TRUE;
    posfrom = curpos;
    posto = curpos+1;
    return FALSE;
}

void	CXmlPrt::prtprtout(CXmlOutPro* prt)
{
    const char * pstr = this->m_str->GetString();

    if (this->m_xmllist == NULL || this->m_xmllist->m_xmltype == XT_invalid)
    {
        prt->prtt(pstr);
        return;
    }

    if (m_xmllist->m_posfrom != 0)
    {
        prt->prtslen(pstr, m_xmllist->m_posfrom);
        pstr += m_xmllist->m_posfrom;
    }
    m_xmllist->prtprtout(pstr, prt);

    int len = m_xmllist->m_posto - m_xmllist->m_posfrom;

    pstr += len;
    if (pstr[0] != 0)
    {
        prt->prtt(pstr);
    }
}

void CXmlPrt::Display(I_COLOROUT* iColorOut)
{

    const char * pstr = this->m_str->GetString();

    if (this->m_xmllist == NULL || this->m_xmllist->m_xmltype == XT_invalid)
    {
        iColorOut->ColorOut(pstr,strlen(pstr),QColor(255,255,255));
        return;
    }
    if (m_xmllist->m_posfrom != 0)
    {
        iColorOut->ColorOut(pstr, m_xmllist->m_posfrom, QColor(255,255,255));
        pstr += m_xmllist->m_posfrom;
    }
    m_xmllist->Display(pstr, iColorOut, this->m_curword_type, this->m_curword_p);

    int len = m_xmllist->m_posto - m_xmllist->m_posfrom;

    pstr += len;
    if (pstr[0] != 0)
    {
        iColorOut->ColorOut(pstr,strlen(pstr),QColor(255,255,255));
    }
}



std::string CXmlPrt::GetString()
{
    return this->m_str->GetString();
}


void CXmlPrt::CommaLast()
{//the last , replaced with ;
    char * p = this->m_str->GetWritableString();
    int len = strlen(p);
    len -= 2;
    if (len < 0)
        len = 0;
    p+=len;
    if (*p == ',')
    {
        *p = ';';
        return;
    }
    p++;
    if (*p == ',')
    {
        *p = ';';
        return;
    }
}

int CXmlPrt::MoveHome(int nLine)
{
    std::string str = this->GetLine(nLine);
    for (int i=0; i<str.size(); i++)
    {
        if (str[i] != ' ')
            return i;
    }
    return 0;
}

int CXmlPrt::MoveLeftWord(int x, int y)
{
    if (x > 0)
        x--;
    std::string str = this->GetLine(y);
    while (x > 0)
    {
        char c = str[x];
        if (c != ' ')
            break;
        x--;
    }
    while (x > 0)
    {
        char c = str[x];
        if (c == ' ')
            break;
        x--;
    }
    return x+1;
}
int CXmlPrt::MoveRightWord(int x, int y)
{
    std::string str = this->GetLine(y);
    int len = str.size();
    while (x < len)
    {
        char c = str[x];
        if (c == ' ')
            break;
        x++;
    }
    while (x < len)
    {
        char c = str[x];
        if (c != ' ')
            break;
        x++;
    }
    return x;
}
int CXmlPrt::WordToLeft(int x, int y)
{
    std::string str = this->GetLine(y);
    if (x >= str.size())
    {
        return str.size();
    }
    while (x > 0)
    {
        char c = str[x];
        if (c == ' ')
            break;
        x--;
    }
    return x+1;
}
int CXmlPrt::WordToRight(int x, int y)
{
    std::string str = this->GetLine(y);
    int len = str.size();
    while (x < len)
    {
        char c = str[x];
        if (c == ' ')
            break;
        x++;
    }
    return x;
}
int CXmlPrt::GetLineLength(int nLine)
{
    std::string str = this->GetLine(nLine);
    return str.size();
}

std::string CXmlPrt::GetLine(int nLine)
{
    const char * pstr = this->m_str->GetString();
    if (pstr == NULL)
        return "";

    int n = 0;
    for (;;)
    {
        const char * p = strchr(pstr,'\n');
        if (p != NULL)
        {
            if (n == nLine)
            {
                return std::string(pstr, p-pstr);
            }
            n++;
            pstr = p+1;
            continue;
        }
        break;
    }
    if (n == nLine)
        return pstr;
    return "";
}

int CXmlPrt::GetPosXY(int x, int y)
{
    //Enter the ranks of the second only, a only back to the pos
    const char * pstr_org = this->m_str->GetString();
    const char * pstr = pstr_org;
    if (pstr == NULL)
        return 0;

    int n = 0;
    for (;;)
    {
        if (n == y)
        {
            return (pstr - pstr_org)+x;
        }
        const char * p = strchr(pstr,'\n');
        if (p != NULL)
        {
            n++;
            pstr = p+1;
            continue;
        }
        break;
    }
    return 0;
}

void CXmlPrt::GetItem(int x, int y, enum XMLTYPE& xmltype, void *& h)
{
    int pos = GetPosXY(x,y);
    long posfrom, posto;
    CXmlList* p = this->m_xmllist->GetCurWord(pos,posfrom,posto);
    if (p != NULL)
    {
        xmltype = p->m_xmltype;
        h = p->m_p;
    }
}

std::string CXmlPrt::GetText(int y1, int x1, int y2, int x2)
{
    std::string retn;
    if (y1 > y2)
        return "";

    const char * pstr = this->m_str->GetString();
    if (pstr == NULL)
        return "";

    int n = 0;
    for (;;)
    {
        const char * p = strchr(pstr,'\n');
        if (p != NULL)
        {
            if (n == y1)
            {
                if (y1 == y2)
                {
                    return std::string(pstr+x1, x2-x1);
                }
                retn += std::string(pstr+x1, p-pstr-x1);
                retn += '\n';
            }
            else if (n > y1 && n < y2)
            {
                retn += std::string(pstr, p-pstr);
                retn += '\n';
            }
            else if (n == y2)
            {
                retn += std::string(pstr, x2);
                return retn;
            }
            n++;
            pstr = p+1;
            continue;
        }
        break;
    }
    if (n == y2)
    {
        retn += std::string(pstr, x2);
        return retn;
    }
    return "";
}
int CXmlPrt::GetLineCount()
{
    const char * pstr = this->m_str->GetString();
    if (pstr == NULL)
        return 0;

    int n = 0;
    for (;;)
    {
        const char * p = strchr(pstr,'\n');
        if (p != NULL)
        {
            n++;
            pstr = p+1;
            continue;
        }
        if (*pstr != '\0')
            n++;
        break;
    }

    return n;
}

size_t GetPosFromXY(const char * p0, int x, int y)
{
    const char * p = p0;
    while (y)
    {
        const char * p1 = strchr(p,'\n');
        if (p1 == NULL)
        {
            return p-p0;
        }
        p = p1+1;
        y--;
    }

    return (p-p0)+x;
}
bool CXmlPrt::SetCurWord(int x, int y)
{
    this->m_curword_type = XT_invalid;
    this->m_curword_p = 0;

    size_t pos = GetPosFromXY(this->m_str->GetString(), x, y);
    long posfrom, posto;
    CXmlList* p1 = this->m_xmllist->GetCurWord(pos, posfrom, posto);
    if (p1 == NULL)
        return true;

    this->m_curword_type = p1->m_xmltype;
    this->m_curword_p = p1->m_p;
    return true;
}
