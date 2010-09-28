// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

// CFunc.h
#ifndef CFunc__H
#define CFunc__H
#include <list>
#include <string>
//#include "../exe2c_interface.h"
#include "DataType.h"
#include "XmlPrt.h"

class	CodeList;
class	ExprManage;
typedef unsigned int uint;
struct st_FuncInfo
{
    char name[256];
    int nStep;
    uint headoff;
    uint endoff;
    int stack_purge;
    int m_EBP_base;
    int m_args;
};

struct st_LINE_SYNTAX
{
    //with 0 starting position in the string can not be greater than the string length
    int pos;
    int len;
    int colorindex; // displayed color
    void* handle;   // This is something, for something unknown, or nonsense, this is zero
};
//ESP_IGNORE 只用于esp_level_next，表明上下stack无关

struct AsmCode
{
        ea_t		linear;
        BYTE		opsize;	//length in byte of this opcode
        signed int	esp_level;	//init to ESP_UNKNOWN, usually negative
        signed int	esp_level_next;	//esp level next instr should be
        XCPUCODE	xcpu;

    int iAddRemove; //0是原来的，1是删掉的，2是新加的

    struct t_h
    {
        t_h() : type(i_Nothing)
        {
            jmp.jmp_type=JMP_none;
            jmp.the_label=0;
            jmp.next_ref_of_this_label=0;
            label.ref_j=0;
            label.label_name[0]=0;
        }
        HLType	type;	//	i_Var ...
        union
        {
            struct
            {
                JxxType	jmp_type;	//JMP_???
                AsmCode* the_label;
                AsmCode* next_ref_of_this_label;	//这里组成个链，用来保存对一个label的所有ref
            } jmp;					//for type = i_Jump only
        };
        struct
        {
            AsmCode* ref_j;
            char label_name[40];
        } label;
    };
    t_h h;
    static AsmCode* new_AsmCode()
    {//SlickEdit does not support the list of all the new ways of using this method on
        AsmCode* p = new AsmCode();
        p->esp_level = ESP_UNKNOWN;
        p->esp_level_next = ESP_UNKNOWN;
        p->iAddRemove = 0;
        return p;
    }
private:
    AsmCode():linear(0),opsize(0),esp_level(0),esp_level_next(0),iAddRemove(0)
    {}
};

typedef	std::list<AsmCode*>	AsmCodeList;
//typedef	CList<ea_t,ea_t>	EALIST;

enum REF_ENUM
{
        enum_NOREF	= 0,
        enum_REF	= 1,
        enum_REUSE	= 2,	//	居然有人重用我！改它
};



enum em_STEP
{
        STEP_Init = 0,
        STEP_IDA_1 = 1,
        STEP_IDA_2 = 2,
        STEP_IDA_3 = 3,
        STEP_IDA_4 = 4,
        STEP_4 = 5,
        STEP_5 = 6,
        STEP_6 = 7,
        STEP_7 = 8,

    STEP_100 = 100,
};

struct st_VarLL
{
    int off;    //总是>=0的
    int size;
    int array;  //default to 1
    char Name[40];

    st_VarLL()
    {
        array = 1;
    }
};
typedef std::list<st_VarLL*> VarLL_LIST;

class VarLL
{
    VarLL_LIST m_varll_list;

    std::string size_to_ptr_name(int size);
public:
    signed int m_VarRange_L;
    signed int m_VarRange_H;
    VarLL()
    {
        m_VarRange_L = m_VarRange_H = 0;
    }
    void Init(signed int VarRange_L, signed int VarRange_H)
    {
        m_VarRange_L = VarRange_L;
        m_VarRange_H = VarRange_H;
    }
    void AddRef(signed int level, int opersize);
    st_VarLL* LookUp_VarLL(int off);
    void prtout(XmlOutPro* out);
};
class Func
{
    typedef INSTR_LIST::iterator POSITION;
    friend class CFuncPrt;
    friend class CFuncOptim;
private:
    UINT GetVaryParaSize(POSITION pos);

public:
    void Restart();
    bool Var_analysis();
    void DeleteUnusedVar();

    bool m_IfLibFunc;
    FuncType* m_functype;

    int   m_EBP_base;
    signed int m_VarRange_L;
    signed int m_VarRange_H;
    class VarLL m_varll;
    struct
    {
        AsmCodeList	*m_asmlist;
    }ll;
    ea_t	m_head_off;	// = 0x100 , offset of head
    ea_t	m_end_off;

    int		m_nStep;	//STEP_IDA_1,...
    bool    m_prepareTrue_analysisFalse;

    int     m_stack_purge;
    int     m_args;		// 3 for 3 args

    std::string    m_funcname;

    ExprManage* m_exprs;
    INSTR_LIST m_instr_list;   //new in Step4_CreateInstrList


    Func(ea_t start);
    ~Func();


    // Analysis methods
    bool    Step_1();
    bool    Step_Label_Analysis();
    bool	Step2_GetRetPurge();
    bool	Step5_GetArgs();
    bool	Func_FillCallParas();
    bool	Fill_Stack_2();
    bool	Step3_FillStackInfo();
    bool    AddRemoveSomeInstr();

    void	Finger_it();
    // Analysis methods end


    // create methods
    bool Step4_1();
    void Step4_CreateInstrList();
    void CreateInstrList_welldone_call();

    void Create_Labels_backend();   // label back-end processing
    // create methods end

    bool expr_only_use_in_this(VAR* pvar, INSTR *phead);

    void    PrepareFunc();
    void    analysis();
    bool	analysis_once();
    bool	analysis_once_1();
    bool	analysis_step_by_step();

    //	Step by step analysis methods
    void	ana_RetType();

    void Fill_this_ECX(VarTypeID id);
    //	Step by step analysis methods end

    int  nParas();

    //	Output methods

    void	prtout_internal(XmlOutPro* out);		//	把CFunc按internal打印
    INSTR *	Get_no_Statement(INSTR * phead,int no);
    void    report_info();
    //	Output methods end

    void MakeDownInstr(void* hline);
    void ReType(M_t* p, const char * newtype);

    std::string Instr_prt_simple(INSTR * p);
};

typedef std::list<Func*>	FUNC_LIST;

//#define THIS this->p
class CFunc_Prt
{
    typedef INSTR_LIST::iterator POSITION;
    Func* Q;
    bool m_flag_prt_var_delare;

    const char *	BareVarName(const VAR* v);
    void        	prt_case(const INSTR *phead, const INSTR *plabel, XmlOutPro* out);
    void          	prt_instr(const INSTR *p,POSITION &nextpos, XmlOutPro* out);
    void            prt_instr_call(const INSTR *p, XmlOutPro* out);
    void            prt_parameter(const INSTR *p, XmlOutPro* out);
    void            prt_para_1(M_t* thevar, XmlOutPro* out);
    bool            prt_instr_callret(POSITION nextpos, XmlOutPro* out);
    void            prt_instr_callthis(POSITION nextpos, XmlOutPro* out);
    void           	prt_add(const INSTR *p, const char * s, XmlOutPro* out);
    void            prt_var(const VAR* var, XmlOutPro* out);
    void         	prt_va(const VAR_ADDON& va, XmlOutPro* out);
    void            prt_va_1(const st_InstrAddOn* pa,const VAR* pv, XmlOutPro* out);
    void            out_PointTo(st_InstrAddOn* pa,const VAR* pv, XmlOutPro* out);
    void            prt_jxx_compare_false(INSTR * &pjxx, XmlOutPro* out);
    void            prt_one_statement(const INSTR *phead, XmlOutPro* out);
    void            prt_switch_case(CasePrt_List* list, const INSTR* phead, XmlOutPro* out);
    void            prt_var_declares(XmlOutPro* out);
    void            prt_statement_in_1_line(INSTR * &p, XmlOutPro* out);
    void            prt_jxx_compare_true(INSTR * &spjxx, XmlOutPro* out);
    void            prt_compare(const INSTR *phead, XmlOutPro* out);
    void            prt_one_statement_mainbody(const INSTR *phead, XmlOutPro* out);
    void            prt_sub(const INSTR *p, const char * s, XmlOutPro* out);
    void            prt_func_head(XmlOutPro* out);		//打印 CFunc 的头, 如 void func1(int a1,int a2)
    const char *	prt_iAddress(const INSTR *p);
    void            prt_iAddress_out(const INSTR *p, XmlOutPro* out);

static void     add_default_entry(CasePrt_List* list, INSTR * thelabel);
static void	Add_case_entry(CasePrt_List* list, int case_n, INSTR * thelabel);
    void	prt_the_instr_1(const INSTR *p, XmlOutPro* out);
public:

    CFunc_Prt(Func* p)
    {
        Q = p;
    }
        void	prtout_cpp(XmlOutPro* out);			//	把CFunc按CPP打印
        std::string	prt_the_instr(const INSTR *p);

};

struct st_VarOptm
{
    INSTR * pInstr;
    BYTE rw;
    bool bJxx;
    int varstep_r;	//May read a variable, and then write into another variable
    int varstep_w;

    bool tem_1; //Temporary local use, not to worry

    st_VarOptm() : pInstr(0),rw(0),bJxx(false),varstep_r(0),varstep_w(0)
    {
    }
    bool IsJump()
    {
        return (bJxx && pInstr->type == i_Jump);
    }
};
typedef std::list<st_VarOptm*> VAROPTM_LIST;
typedef std::list< INSTR_LIST::iterator > POS_LIST;

class CFuncOptim
{
    typedef INSTR_LIST::iterator POSITION;
    Func* Q;
    bool MakeSure_NotRef_in_Range(VAR* pvar, INSTR * p1, INSTR * p2);
    bool expr_never_use_after_this(VAR* pvar, INSTR * pend, INSTR_LIST* oldroad);

    void prt_var_uselist(VAROPTM_LIST& used_list);
    bool Optim_Var_Flow(VAROPTM_LIST& used_list);
    bool Optim_Var_Flow_1(VAROPTM_LIST& used_list);
    bool Optim_Var_Flow_2(VAROPTM_LIST& used_list);
    bool Optim_Var_Flow_3(VAROPTM_LIST& used_list);
    bool Optim_Var_Flow_4(VAROPTM_LIST& used_list);
    bool Optim_Var_Flow_5(VAROPTM_LIST& used_list);
    bool SureNotUse(VAROPTM_LIST& used_list, st_VarOptm* j);
    bool SureNotUse_1(VAROPTM_LIST& used_list, st_VarOptm* j);

    void Get_Var_Use_Flow(VAROPTM_LIST& volist, M_t* pvar);
    void TryDistinguishVar_1(VAROPTM_LIST& volist, M_t* pvar, VAROPTM_LIST::iterator pos, int* pglobalstep);
    bool IfAnyThisStep(int i, VAROPTM_LIST& volist);
    bool Optim_var_flow_NT(VAROPTM_LIST& volist, M_t* pvar);
    std::string Get_var_finger_NT(VAROPTM_LIST& volist, M_t* pvar);

    bool SetParaType(UINT offset, UINT sizepara, enum_CallC conv,
                     const std::string & paraname, VarTypeID paraid);
public:
    CFuncOptim(Func* p)
    {
        Q = p;
    }
    bool Var_Split();
    bool ClassSubFuncProcess();
    bool DataType_Flow();
    bool optim_once_new();
    bool pcode_1();
    bool Address_to_Add();
    bool Simplify_Instr();
    bool ana_Flow();
    bool Optim_var_NT(M_t* pvar);
    bool TryDistinguishVar(VAROPTM_LIST& volist, M_t* pvar);
    void Replace_Var_vo(VAROPTM_LIST& volist, int step, M_t* pvar, M_t* pnew);
    void Prt_Var_Flow(const char * varname);
    bool VarDataType_analysis_mydefine();
    bool VarDataType_analysis();
    bool DataType_Check(VAR_ADDON* pvar, FuncType* pftype);
};

enum em_PRT_COLOR
{
    COLOR_0 = 0,    //default color
    COLOR_Func = 1,
    COLOR_Var,
    COLOR_VarH,     //Advanced, used in cpp
    COLOR_Immed,
    COLOR_ea,
    COLOR_type,
    COLOR_label,
    COLOR_last
};
struct st_OutBuf
{
    char strbuf[8000];
    st_LINE_SYNTAX linesyntax[240];
};
class CPrtOut
{
    bool b_Indent;
    bool b_Endl;
    bool b_OneLine;
    const void* hline;    //this is INSTR *
    st_OutBuf m_buf;
public:
    CPrtOut();

    void SetHline(const void* h_line)
    {
        this->hline = h_line;
    }
    void SetOneLine(bool b)
    {
        b_OneLine = b;
    }
    bool SetIndent(bool bindent)
    {
        bool b = b_Indent;
        b_Indent = bindent;
        return b;
    }
    bool SetEndl(bool bendl)
    {
        bool b = b_Endl;
        b_Endl = bendl;
        return b;
    }
    std::string GetCurStringBuf()
    {
        return (std::string)m_buf.strbuf;
    }
    void Clear()
    {
        strcpy(m_buf.strbuf, "");
        m_buf.linesyntax[0].pos = -1;
    }
};
extern CPrtOut g_PrtOut;

// holds reference to some function's instruction list
// life time must be limited by 'parent' function
class CFunc_InstrList
{
    const INSTR_LIST& m_instr_list;
public:
    CFunc_InstrList(const INSTR_LIST& list) : m_instr_list(list)
    {

    }
    INSTR * instr_next_in_func(const INSTR * p);
    INSTR * instr_prev_in_func(const INSTR * p);
    INSTR * skip_compl(const INSTR * p);
};


#endif // CFunc__H
