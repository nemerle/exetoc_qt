// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CApiManage.cpp

#include "CISC.h"
#include "exe2c.h"

#include <QString>
#include <QDebug>
#include <algorithm>

ApiManage *ApiManage::s_self=0;
ApiManage * ApiManage::get()
{
    //WARN: Poor man's singleton, not thread safe
    if(!s_self)
        s_self = new ApiManage();
    return s_self;
}

//	--------------------------------------------------
ApiManage::ApiManage()
{
}
ApiManage::~ApiManage()
{
    qDeleteAll(m_apilist.begin(),m_apilist.end());
    m_apilist.clear();
}
bool ApiManage::new_api(ea_t address,int stacksub)
{
    Api *p = new Api;     //new_CApi
    p->m_address = address;
    p->m_stack_purge = stacksub;

    sprintf(p->name,"api_%x",address);
    p->m_functype = new FuncType; //TODO: this is just a placeholder, and should actually be left to the user ?
    p->m_functype->m_retdatatype_id = id_void;
    m_apilist.push_front(p);
    return true;
}
Api*	ApiManage::get_api(ea_t address)
{
    //    uint32_t ptr = (uint32_t)ea2ptr(address);
    lApi::iterator pos = this->m_apilist.begin();
    for (;pos!=m_apilist.end(); ++pos)
    {
        Api* p = *pos;
        if (p->m_address == address)
            return p;
    }
    //assert(0);
    return NULL;
}

void ApiManage::New_ImportAPI(const std::string &pstr, uint32_t apiaddr)
{
    // Note that this time the apiaddr is actually a ptr rather than ea_t
    // Because then the function can not work properly ea2ptr

    QString frst(pstr.c_str());
    if (!frst.compare("RegisterClassExA",Qt::CaseInsensitive))
        log_prtl("New_ImportAPI %s 0x%x", pstr.c_str(), apiaddr);


    FuncType* pf = Get_FuncDefine_from_name(pstr);
    qDebug()<<QString("Searching for imported function %1 at %2 - %3 :").arg(pstr.c_str())
              .arg(apiaddr,0,16).arg(pf ? "found":"not found");
    if (pf == NULL) {
        return;
    }
    Api *p = new Api;     //new_CApi
    p->m_address = apiaddr;
    assert(pf);
    p->m_functype = pf;
    p->m_stack_purge = pf->get_stack_purge(); //g_FuncDefineMng.API_stack(pstr);

    strcpy(p->name, pstr.c_str()	);

    m_apilist.push_front(p);
}
const char * check_if_jmp_api(const uint8_t* phead)
{
    if (*reinterpret_cast<const uint16_t *>(phead) != 0x25ff)
        return NULL;
    phead += 2;

    uint32_t d = *(const uint32_t *)phead;

    Api* papi = ApiManage::get()->get_api((ea_t)d);
    if (papi == NULL)
    {
        alert_prtf("error!!! %x", d);
        return NULL;
    }

    char * name = papi->name;
    //alert_prtf("I find jmp api %s",name);
    return name;
}

Api::Api() : m_stack_purge(INVALID_STACK),m_address(~0),m_functype(0)
{
    name[0]=0;
}

