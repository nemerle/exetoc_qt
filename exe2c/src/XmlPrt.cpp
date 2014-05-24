// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CXmlPrt.cpp

#include <cstdio>
#include	"00000.h"
#include	"XmlPrt.h"
#include <assert.h>

XmlPrt::XmlPrt()
{
    m_xmllist = XmlList::new_CXmlList(XT_invalid,NULL,0);

    m_curword_type = XT_invalid;
    m_curword_p = NULL;
}

XmlPrt::~XmlPrt()
{
    delete m_xmllist;
    m_xmllist=0;
}
void XmlPrt::Clear()
{
    this->~XmlPrt(); //TODO: fix this hack

    m_xmllist = XmlList::new_CXmlList(XT_invalid,NULL,0);
    m_str.clear();
}

void XmlPrt::prtt(const char * str)
{
    m_str+=str;
}

void XmlOutPro::prtslen(const char * s, int len)
{
    char buf[280];
    assert(len<280);
    strncpy(buf,s,len);
    buf[len] = 0;
    this->prtt(buf);
}
void XmlOutPro::prtf(const char * fmt, ...)
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

void XmlOutPro::prtl(const char * fmt, ...)
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
void	XmlOutPro::ident_add1()
{
    this->m_nIdent++;
}
void	XmlOutPro::ident_sub1()
{
    assert(this->m_nIdent);
    this->m_nIdent--;
}
void    XmlOutPro::ident()
{
    if (m_f_prt_in_1line)
        return;
    for (size_t i=0; i<m_nIdent; i++)
        m_out->prtt("    ");
}
void 	XmlOutPro::prtl_ident(const char * fmt,...)
{
    ident();

    va_list argptr;
    char buf[280];

    va_start(argptr, fmt);
    vsprintf(buf, fmt, argptr);
    va_end(argptr);

    prtl("%s",buf);
}

void 	XmlOutPro::prtf_ident(const char * fmt,...)
{
    ident();

    va_list argptr;
    char buf[280];

    va_start(argptr, fmt);
    vsprintf(buf, fmt, argptr);
    va_end(argptr);

    prtf("%s",buf);
}

void XmlOutPro::endline()
{
    if (!this->m_f_prt_in_1line)
        this->prtt("\n");
}
void XmlOutPro::EOL()
{
    this->nospace();
    if (this->m_f_prt_in_comma)
        prtl(",");
    else
        prtl(";");
}


void XmlPrt::XMLbegin(XMLTYPE xmltype, void * p0)
{
    long pos = m_str.length();
    m_xmllist->XMLbegin(xmltype, p0, pos);
}

void XmlPrt::XMLend(XMLTYPE xmltype)
{
    long pos = m_str.length();
    XMLTYPE type1 = m_xmllist->XMLend(xmltype, pos);
}


//	---------------------------------------------------

void XmlPrt::Clicked(long x1, long x2)
{
    m_xmllist->Clicked(x1,x2);

}
bool XmlPrt::GetCurWord(long curpos, long &posfrom, long &posto)
{
    if (NULL != m_xmllist->GetCurWord(curpos, posfrom, posto))
        return true;
    posfrom = curpos;
    posto = curpos+1;
    return false;
}
bool XmlPrt::GetLeftWord(long curpos, long &posfrom, long &posto)
{
    if (this->m_xmllist->GetLeftWord(curpos, posfrom, posto))
        return true;
    posfrom = curpos;
    posto = curpos+1;
    return false;
}
bool XmlPrt::GetRightWord(long curpos, long &posfrom, long &posto)
{
    if (this->m_xmllist->GetRightWord(curpos, posfrom, posto))
        return true;
    posfrom = curpos;
    posto = curpos+1;
    return false;
}

void	XmlPrt::prtprtout(XmlOutPro* prt)
{
    const char * pstr = m_str.toLatin1().constData();

    if (m_xmllist == NULL || m_xmllist->m_xmltype == XT_invalid)
    {
        prt->prtt(m_str);
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

void XmlPrt::Display(I_COLOROUT* iColorOut)
{

    const char * pstr = m_str.toLatin1().constData();

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



std::string XmlPrt::GetString()
{
    return m_str.toStdString();
}


void XmlPrt::CommaLast()
{//the last , replaced with ;
    int idx=m_str.lastIndexOf(',');
    if(idx==-1)
        return;
    if(idx>=m_str.length()-2) // last or next to last char
    {
        m_str[idx]=';';
    }
}

int XmlPrt::MoveHome(int nLine)
{
    std::string str = this->GetLine(nLine);
    for (size_t i=0; i<str.size(); i++)
    {
        if (str[i] != ' ')
            return i;
    }
    return 0;
}

int XmlPrt::MoveLeftWord(int x, int y)
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
int XmlPrt::MoveRightWord(int x, int y)
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
int XmlPrt::WordToLeft(size_t x, int y)
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
int XmlPrt::WordToRight(int x, int y)
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
int XmlPrt::GetLineLength(int nLine)
{
    std::string str = this->GetLine(nLine);
    return str.size();
}

std::string XmlPrt::GetLine(int nLine)
{
    if (m_str.isEmpty())
        return "";
    int n = 0;
    int current_index=0;
    int line_end_idx=-1;
    for (;;)
    {
        line_end_idx=m_str.indexOf('\n',current_index);

        if (n == nLine)
            return m_str.mid(current_index,line_end_idx).toStdString();

        if(line_end_idx==-1)
            break;
        current_index=line_end_idx+1;
        n++;
    }
    return "";
}

int XmlPrt::GetPosXY(int x, int y)
{
    //Enter the ranks of the second only, a only back to the pos
    const char * pstr_org = m_str.toStdString().c_str();
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

void XmlPrt::GetItem(int x, int y, enum XMLTYPE& xmltype, void *& h)
{
    int pos = GetPosXY(x,y);
    long posfrom, posto;
    XmlList* p = this->m_xmllist->GetCurWord(pos,posfrom,posto);
    if (p != NULL)
    {
        xmltype = p->m_xmltype;
        h = p->m_p;
    }
}

std::string XmlPrt::GetText(int y1, int x1, int y2, int x2)
{
    std::string retn;
    if (y1 > y2)
        return "";

    const char * pstr = m_str.toStdString().c_str();
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
int XmlPrt::GetLineCount()
{
    const char * pstr = m_str.toStdString().c_str();
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

static size_t GetPosFromXY(const char * p0, int x, int y)
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
bool XmlPrt::SetCurWord(int x, int y)
{
    this->m_curword_type = XT_invalid;
    this->m_curword_p = 0;

    size_t pos = GetPosFromXY(m_str.toStdString().c_str(), x, y);
    long posfrom, posto;
    XmlList* p1 = this->m_xmllist->GetCurWord(pos, posfrom, posto);
    if (p1 == NULL)
        return true;

    this->m_curword_type = p1->m_xmltype;
    this->m_curword_p = p1->m_p;
    return true;
}
