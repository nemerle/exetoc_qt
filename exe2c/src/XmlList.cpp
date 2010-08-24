// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include	"00000.h"
#include	"XmlList.h"
#include <assert.h>

XmlList* XmlList::new_CXmlList(XMLTYPE xmltype, void * p, size_t posfrom)
{//static int
    XmlList* pnew = new XmlList(0);
    pnew->m_xmltype = xmltype;
    pnew->m_p = p;
    pnew->m_posfrom = posfrom;
    pnew->m_posto = (size_t)-1;
    pnew->m_sub = 0;
    pnew->m_next = 0;
    return pnew;
}
XmlList::~XmlList()
{
        if (this->m_sub)
                delete m_sub;
        if (this->m_next)
                delete m_next;
        this->m_sub = NULL;
        this->m_next = NULL;
}

XmlList* XmlList::GetLast_willbegin()
{
        XmlList* p = this;
        while (p)
        {
                if (p->m_posto == (size_t)-1)
                {
                        if (p->m_sub == NULL)
                                return p;
                        return p->m_sub->GetLast_willbegin();
                }
                if (p->m_next == NULL)
                        return p;
                p = p->m_next;
        }
        assert(0);
        return NULL;
}
XmlList* XmlList::GetLast_willend()
{
        XmlList* p = this;
        while (p)
        {
                if (p->m_posto == (size_t)-1)
                {
                        if (p->m_sub == NULL)
                                return p;
                        XmlList* p1 = p->m_sub->GetLast_willend();
                        if (p1 != NULL)
                                return p1;
                        return p;
                }
                p = p->m_next;
        }
        return NULL;
}
void XmlList::XMLbegin(XMLTYPE xmltype, void * p0, size_t pos)
{
        XmlList* p = this->GetLast_willbegin();
    if (p->m_xmltype == XT_invalid)
    {
        p->m_xmltype = xmltype;
        p->m_p = p0;
        p->m_posfrom = pos;
        return;
    }

        XmlList* pnew = new_CXmlList(xmltype, p0, pos);

        if (p->m_posto == (size_t)-1)
        {
                assert(p->m_sub == NULL);
                p->m_sub = pnew;
                return;
        }

        assert(p->m_next == NULL);
        p->m_next = pnew;
}
XMLTYPE XmlList::XMLend(XMLTYPE xmltype, size_t pos)
{
        XmlList* p = this->GetLast_willend();
        assert(p);
        assert(p->m_xmltype == xmltype);
        assert(p->m_posto == (size_t)-1);

        p->m_posto = pos;

        p = this->GetLast_willend();
        if (p)
                return p->m_xmltype;
        return XT_invalid;
}

void XmlList::Clicked(long x1, long x2)
{
        long u1,u2;
        XmlList* p = this->GetCurWord(x1,u1,u2);
        if (p == NULL)
                return;
        if (u1 != x1 || u2 != x2)
                return;
        XML_Clicked(p->m_xmltype, p->m_p);

}
XmlList* XmlList::GetCurWord(size_t curpos, long &posfrom, long &posto)
{
        XmlList* p = this;
        while (p)
        {
                if (curpos < p->m_posfrom)
                        return NULL;

                if (p->m_sub)
                {
                        XmlList* p1 = p->m_sub->GetCurWord(curpos,posfrom,posto);
                        if (p1)
                                return p1;
                }
                else if (curpos >= p->m_posfrom
                        && curpos < p->m_posto
                        && p->m_posto != -1)
                {
                        posfrom = p->m_posfrom;
                        posto = p->m_posto;
                        return p;
                }
                p = p->m_next;
        }
        return NULL;
}
bool XmlList::GetLeftWord(long curpos, long &posfrom, long &posto)
{
        for (int i=0;i<2;i++)
        {
                if (NULL != this->GetCurWord(curpos,posfrom,posto))
                        return TRUE;
                if (curpos == 0)
                        return FALSE;
                curpos--;
        }
        return FALSE;
}

bool XmlList::GetRightWord(long curpos, long &posfrom, long &posto)
{
        for (int i=0;i<2;i++)
        {
                if (NULL != this->GetCurWord(curpos,posfrom,posto))
                    return TRUE;
                curpos++;
        }
        return FALSE;
}

#include "XmlPrt.h"
void XmlList::prtprtout(const char * str, XmlOutPro* prt)
{
    prt->XMLbegin(this->m_xmltype, this->m_p);

    size_t k_pos = this->m_posfrom;

    XmlList* p = this->m_sub;
    while (p)
    {
        //	------------------------------------
        if (p->m_posfrom > k_pos)
        {
            long len = p->m_posfrom - k_pos;
            prt->prtslen(str,len);

            str += len;
            k_pos += len;
        }
        //	------------------------------------
        {
            long len = p->m_posto - k_pos;
            p->prtprtout(str, prt);     //递归
            str += len;
            k_pos += len;
        }

        p = p->m_next;
    }
    if (this->m_posto > k_pos)
        prt->prtslen(str, this->m_posto - k_pos);
    else if (this->m_posto == -1)
        prt->prtt(str);

    prt->XMLend(this->m_xmltype);
}
void XmlList::Display(const char * pstr, I_COLOROUT* iColorOut, XMLTYPE curw_xmltype, void * curw_p)
{
    if (curw_xmltype != XT_invalid && curw_p != NULL)
        if (curw_xmltype == this->m_xmltype && curw_p == this->m_p)
        {
            iColorOut->SetBKColor(QColor(255,255,0));
        }
    QColor k_color = XmlType_2_Color(this->m_xmltype);
    size_t k_pos = this->m_posfrom;

    XmlList* p = this->m_sub;
    while (p)
    {
        //	------------------------------------
        if (p->m_posfrom > k_pos)
        {
            long len = p->m_posfrom - k_pos;
            iColorOut->ColorOut(pstr, len, k_color);
            pstr += len;
            k_pos += len;
        }
        //	------------------------------------
        {
            assert(k_pos == p->m_posfrom);
            assert(p->m_posto != -1);
            long len = p->m_posto - p->m_posfrom;
            p->Display(pstr, iColorOut, curw_xmltype, curw_p);
            pstr += len;
            k_pos += len;
        }

        p = p->m_next;
    }
    if (this->m_posto > k_pos)
    {
        long len = this->m_posto - k_pos;
        iColorOut->ColorOut(pstr, len, k_color);
    }
    else
    {
        iColorOut->ColorOut(pstr, strlen(pstr), k_color);
    }
    if (curw_xmltype != XT_invalid && curw_p != NULL)
        if (curw_xmltype == this->m_xmltype && curw_p == this->m_p)
        {
            iColorOut->SetBKColor(QColor(0,0,0));
        }
}
