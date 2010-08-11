// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CApiManage.cpp

#include <QString>
#include "CISC.h"
#include "Cexe2c.h"

CApiManage* g_ApiManage = NULL;

class CApiManage_cpp
{
public:
    CApiManage_cpp();
    ~CApiManage_cpp(){}
};

CApiManage_cpp myself;

CApiManage_cpp::CApiManage_cpp()
{
    g_ApiManage = new CApiManage;   //new_CApiManage
}
//	--------------------------------------------------


bool CApiManage::new_api(ea_t address,int stacksub)
{
    CApi *p = new CApi;     //new_CApi
    p->address = address;
    p->m_stack_purge = stacksub;

    sprintf(p->name,"api_%x",address);

    this->apilist->push_front(p);
    return true;
}
CApi*	CApiManage::get_api(ea_t address)
{
    //    uint32_t ptr = (uint32_t)ea2ptr(address);
    API_LIST::iterator pos = this->apilist->begin();
    for (;pos!=apilist->end(); ++pos)
    {
        CApi* p = *pos;
        if (p->address == address)
            return p;
    }
    //assert(0);
    return NULL;
}

void CApiManage::New_ImportAPI(const std::string &pstr, uint32_t apiaddr)
{
    // Note that this time the apiaddr is actually a ptr rather than ea_t
    // Because then the function can not work properly ea2ptr
    QString frst(pstr.c_str());
    if (!frst.compare("RegisterClassExA",Qt::CaseInsensitive))
        log_prtl("New_ImportAPI %s 0x%x", pstr.c_str(), apiaddr);


    CFuncType* pf = Get_FuncDefine_from_name(pstr);
    if (pf == NULL)
        return;
    CApi *p = new CApi;     //new_CApi
    p->address = apiaddr;
    assert(pf);
    p->m_functype = pf;
    p->m_stack_purge = pf->get_stack_purge(); //g_FuncDefineMng.API_stack(pstr);

    strcpy(p->name, pstr.c_str()	);

    this->apilist->push_front(p);
}
typedef const BYTE* PCBYTE;
const char * check_if_jmp_api(PCBYTE phead)
{
    if (*(WORD *)phead != 0x25ff)
        return NULL;
    phead += 2;

    uint32_t d = *(uint32_t *)phead;

    CApi* papi = g_ApiManage->get_api((ea_t)d);
    if (papi == NULL)
    {
        alert_prtf("error!!! %x", d);
        return NULL;
    }

    char * name = papi->name;
    //alert_prtf("I find jmp api %s",name);
    return name;
}

