// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#pragma once

#include <list>
#include <string>
enum en_MTTYPE
{
    MTT_invalid = 0,
    MTT_tem,
    MTT_reg,
    MTT_var,
    MTT_par,
    MTT_immed,  //In this case, namestr and s_off are unused
    MTT_global
};
struct M_t
{
	en_MTTYPE type; //MTT_tem ..
	uint32_t s_off;	//start offset
	uint32_t size;

    std::string namestr;
    int tem_useno;  //how many others, counting me, [do not control this variable]
    bool bTem; //Is temporary variable
    int iThrowned;

    union
    {
        struct
        {
            uint32_t temno;
        }tem;
        struct
        {
            uint32_t d;
        } immed;
    };

    VarTypeID m_DataTypeID;

    const std::string &GetName() const
    {
        return namestr;
    }

    M_t() : tem_useno(0)
    {
        s_off=0;
        size=0;
        bTem = false;
        iThrowned = 0;
        m_DataTypeID = 0;
    }
    uint32_t end_off()
    {
        return s_off + size;
    }
    bool IfInclude(UINT off)
    {
        if (this->s_off <= off &&
            this->s_off + this->size >= off)
            return true;
        return false;
    }
    bool AnyOverlay(M_t* p);
    void Expand(M_t* p);
} ;

typedef std::list<M_t*> MLIST;
    //MLIST is ordered storage of M_t
class	ExprManage
{
    static M_t* GetVarByName_1(const MLIST &list, const char * varname);
    static void DeleteUnuse_VarList(MLIST &vlist);
    friend class FuncOptim;
protected:
    MLIST	vList;
public:
    signed int m_VarRange_L;
    signed int m_VarRange_H;


	ExprManage();
	~ExprManage();

	void EspReport(signed int esplevel);
	void AddRef(VAR* pvar);
    M_t* CreateNewTemVar(UINT size);
	M_t* AddRef_with_name(en_MTTYPE type, uint32_t off, uint32_t size, const char * tj_name);
	M_t* AddRef_tem(uint32_t temno, uint32_t size);
	M_t* AddRef_immed(uint32_t d, uint32_t size);

	M_t* SearchMT(en_MTTYPE type, uint32_t off);

    void 	prt_var(const VAR* var, XmlOutPro* out);
    void 	prt_var_Immed(const VAR* var, XmlOutPro* out);

    //char * VarDataType(const VAR* v);	//	数据类型，比如unsigned long,unsigned long long等uint32_t
    const char * VarName(const VAR* v);
    const char * VarName_Immed(const VAR* v);
    const char * BareVarName(const VAR* v);

    void prt_parameters(XmlOutPro* out);
    void prt_var_declares(XmlOutPro* out);
    M_t* GetVarByName(const char * varname);
    void Enlarge_Var(M_t* p, INSTR_LIST& instr_list);
    void DeleteUnusedVars();

    void ClearUse();
};

extern ExprManage g_GlobalExpr;
