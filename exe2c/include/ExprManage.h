// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CExprManage.h
#ifndef CExprManage__H
#define CExprManage__H
#include <list>
enum en_MTTYPE
{
    MTT_invalid = 0,
    MTT_tem,
    MTT_reg,
    MTT_var,
    MTT_par,
    MTT_immed,  //对此，namestr和s_off是不用的
    MTT_global,
};
struct M_t
{
	en_MTTYPE type; //MTT_tem ..
	uint32_t s_off;	//start offset
	uint32_t size;

    //H_NAMEID nameid;
    char namestr[80];
    int tem_useno;  //有几个人用我，一般不要管这个变量

    //是否临时变量
    bool bTem;
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

    const char * GetName() const
    {
        return namestr;
    }

    M_t() : tem_useno(0)
    {
        s_off=0;
        size=0;
        namestr[0]=0;
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

class NameMng;

class	ExprManage
{
    M_t* GetVarByName_1(MLIST* list, const char * varname);
public:

    void DeleteUnuse_VarList(MLIST* vlist);

    signed int m_VarRange_L;
    signed int m_VarRange_H;

    MLIST*	vList;

    M_t* CreateNewTemVar(UINT size);


	ExprManage();
	~ExprManage();

	void EspReport(signed int esplevel);
	void AddRef(VAR* pvar);
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

    void ClearUse();
};

extern ExprManage g_GlobalExpr;
#endif // CExprManage__H
