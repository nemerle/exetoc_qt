// Copyright(C) 1999-2005 LiuTaoTaobookaa@rorsoft.com
#ifndef CCodeList__H
#define CCodeList__H

class	CodeList
{
    friend class CodeList_Maker;
private:
    void			InstrAddTail(Instruction * p);

    AsmCodeList *	m_asmlist;
    int				m_EBP_base;
    INSTR_LIST &	m_instr_list;
public:
    CodeList(INSTR_LIST &list):m_instr_list(list)
    {}

    void		CreateInstrList_raw(AsmCodeList* asmlist, int EBP_base);
};

enum
{
    enum_00 = 0,    //there is no operands such as ret nop
    enum_RR = 1,    //both operands are read, such as cmp
    enum_WR = 2,    //2 operands, a write, a read
    enum_AR = 3     //like add
};

class CodeList_Maker
{
    AsmCode*	cur;	//temp used by instrlist maker
    CodeList* Q;
    uint32_t		m_tem_var_no;

    void	Code_Jxx(JxxType t);
    Instruction *	Code_general(int type, HLType t);
    void	TransVar(VAR &pvar,int no);
    void	TransVar_(VAR &pvar,int no);
    void    TransVar_op_addr(VAR &pvar, OPERITEM *op);
    void	VarRead(VAR_ADDON& va);
    void	WriteToAddress(Instruction * p);
    void	new_temp(VAR* pvar);
public:
    CodeList_Maker(CodeList* p_owner, AsmCode* p_cur)
    {
        Q = p_owner;
        cur = p_cur;
        m_tem_var_no = 0;
    }
    void AddTail_Cur_Opcode();
};


#endif // CCodeList__H
