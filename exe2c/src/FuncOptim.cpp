// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include	"CISC.h"
#include "exe2c.h"
#include "Strategy.h"


void CFuncOptim::SimplifyGetAddr(Instruction *p)
{
    p->type = i_Assign;

    st_InstrAddOn* pnew = new st_InstrAddOn;
    pnew->type = IA_GetAddress;
    pnew->pChild = p->va_r1.pao;

    p->va_r1.pao = pnew;
}
void CFuncOptim::SimplifyAddImmed(Instruction *p)
{
    p->type = i_Assign;

    st_InstrAddOn* pnew = new st_InstrAddOn;
    pnew->type = IA_AddImmed;
    pnew->addimmed.iAddon = p->var_r2.d;
    pnew->pChild = p->va_r1.pao;

    p->va_r1.pao = pnew;

    p->var_r2.type = v_Invalid;
}
void CFuncOptim::SimplifyImulImmed(Instruction *p)
{
    p->type = i_Assign;

    st_InstrAddOn* pnew = new st_InstrAddOn;
    pnew->type = IA_MulImmed;
    pnew->addimmed.iAddon = p->var_r2.d;
    pnew->pChild = p->va_r1.pao;

    p->va_r1.pao = pnew;

    p->var_r2.type = v_Invalid;

}
//Simple instructions can look into i_Assign
bool CFuncOptim::Simplify_Instr()
{
    INSTR_LIST& list = this->m_my_func->m_instr_list;
    Instruction * p;
    for(POSITION pos = list.begin(); pos!=list.end(); ++pos )
    {
        p=*pos;
        if (p->type == i_GetAddr)
        {
            SimplifyGetAddr(p);
            return true;
        }
        if (p->type == i_Add && p->var_r2.type == v_Immed)
        {
            SimplifyAddImmed(p);
            return true;
        }
        if (p->type == i_Imul && p->var_r2.type == v_Immed)
        {
            SimplifyImulImmed(p);
            return true;
        }
    }
    return false;
}
//Look if i_Address can become i_Add
bool CFuncOptim::Address_to_Add()
{
    INSTR_LIST& list = this->m_my_func->m_instr_list;
    Instruction * p;
    for(POSITION pos = list.begin(); pos!=list.end(); ++pos )
    {
        p=*pos;
        if (p->type != i_Address)
            continue;
        if (p->var_r2.type == v_Invalid)
        {
            p->i1 = 0;
            if (p->i2 == 0)
            {
                p->type = i_Assign;
                return true;
            }
            //i2 is not zero, add to become
            p->var_r2.type = v_Immed;
            p->var_r2.d = p->i2;
            p->var_r2.thevar = NULL;
            p->i2 = 0;
            p->type = i_Add;
            return true;
        }
        if (p->i2 == 0
                && VAR::IsSame(&p->var_r1, &p->var_r2)
                && st_InstrAddOn::IsSame(p->va_r1.pao, p->va_r2.pao))
        {
            p->var_r2.type = v_Invalid;
            p->va_r2.pao = NULL;

            p->var_r2.type = v_Immed;
            p->var_r2.d = p->i1 + 1;
            p->var_r2.thevar = NULL;

            p->type = i_Imul;

            p->i1 = 0;
            return true;
        }
        if (p->i1 > 1 && p->var_r2.type != v_Invalid)
        {
            st_InstrAddOn* pnew = new st_InstrAddOn;
            pnew->type = IA_MulImmed;
            pnew->addimmed.iAddon = p->i1;
            pnew->pChild = p->va_r2.pao;

            p->va_r2.pao = pnew;

            p->i1 = 1;
            return true;
        }
        if (p->i1 == 1 && p->i2 == 0)
        {
            //This is Add
            p->type = i_Add;
            return true;
        }
    }
    return false;
}
bool	CFuncOptim::pcode_1()
{	//	If cmp and jxx close, the cmp and jxx merger
    INSTR_LIST& list = this->m_my_func->m_instr_list;

    POSITION pos = list.begin();
    for (;pos!=list.end();++pos)
    {
        POSITION nextpos=pos;

        Instruction * pcmp = *pos;
        if (pcmp->type != i_Cmp)
            continue;
        ++nextpos;
        assert(nextpos!=list.end());
        Instruction * pjxx = *nextpos;
        if (pjxx->type == i_Jump && pjxx->jmp.jmp_type != JMP_jmp)
        {
            pjxx->var_r1 = pcmp->var_r1;
            pjxx->va_r1.pao = pcmp->va_r1.pao;
            pjxx->var_r2 = pcmp->var_r2;
            pjxx->va_r2.pao = pcmp->va_r2.pao;
            list.erase(pos);
            return true;
        }	//	Remove the i_Cmp, leaving the jxx, so do not moving label
        else
        {//Solitary cmp, delete forget
            list.erase(pos);
            return true;
        }
    }
    return false;
}

bool	CFuncOptim::ana_Flow()
{
    //	Flow analysis
    Instruction * first = *m_my_func->m_instr_list.begin();
    //	On between the i_Begin to i_End analysis of all the i_Begin and i_End
    InstrList the(m_my_func->m_instr_list);
    return the.Flow_a(first);
}

bool CFuncOptim::expr_never_use_after_this(VAR* pvar, Instruction * p0, INSTR_LIST* oldroad)
{	//	我必须知道，必须知道！
    POSITION pos = std::find(m_my_func->m_instr_list.begin(),m_my_func->m_instr_list.begin(),p0);
    for(;pos!=this->m_my_func->m_instr_list.end();++pos)
    {
        Instruction * p = *pos;
        if (p->type == i_Label)
        {	//	 Do not consider this
            oldroad->push_front(p);
            continue;
        }
        if (VAR::IsSame(pvar, &p->var_w))
            return true;
        if (VAR::IsSame(pvar, &p->var_r1))
            return false;
        if (VAR::IsSame(pvar, &p->var_r2))
            return false;
        // To i_Jump can also take parameters, so check first
        if (p->type != i_Jump)
            continue;

        //	Should prevent deadlock
        if (std::find(oldroad->begin(),oldroad->end(),p->jmp.target_label)==oldroad->end())
        {
            oldroad->push_front(p->jmp.target_label);
            if (! expr_never_use_after_this(pvar,p->jmp.target_label,oldroad))
                return false;
        }
        if (p->jmp.jmp_type == JMP_jmp)
            return true;
    }
    return true;
}

bool	CFuncOptim::MakeSure_NotRef_in_Range(VAR* pvar, Instruction * p1, Instruction * p2)
{	//	确认从p1到p2没人用过pvar
    //	不包括p1在内,也不包括p2

    POSITION pos = std::find(m_my_func->m_instr_list.begin(),m_my_func->m_instr_list.end(),p1);
    ++pos;		//	skip p1
    for(;pos!=m_my_func->m_instr_list.end();++pos)
    {
        Instruction * p = *pos;
        if (p == p2)
            return true;
        if (VAR::IsSame(pvar, &p->var_w))
            return false;
        if (VAR::IsSame(pvar, &p->var_r1))
            return false;
        if (VAR::IsSame(pvar, &p->var_r2))
            return false;
    }
    error("why here 8u3");
    return false;
}

bool CFuncOptim::ClassSubFuncProcess()
{
    //return false continue analysis
    FuncType* pfctype = this->m_my_func->m_functype;
    if (pfctype == NULL || pfctype->m_class == NULL)
        return false;

    VarTypeID id = g_VarTypeManage->Class2VarID(pfctype->m_class);
    id = g_VarTypeManage->GetAddressOfID(id);

    this->m_my_func->Fill_this_ECX(id);

    return false;
}

bool CFuncOptim::Var_Split()
{
    M_t* pvar;
    MLIST *vList = m_my_func->m_exprs->vList;
    for(MLIST::iterator pos = vList->begin(); pos!=vList->end(); ++pos)
    {
        pvar = *pos;
        if (pvar->type != MTT_reg)
            continue;

        VAROPTM_LIST used_list;
        //Analyzed the instr

        Get_Var_Use_Flow(used_list, pvar);

        if (TryDistinguishVar(used_list, pvar))
        {
            prt_var_uselist(used_list);
            return true;
        }
    }
    return false;
}
bool CFuncOptim::DataType_Flow()
{
    //Analysis of data types flow
    POSITION pos = m_my_func->m_instr_list.begin();
    for (;pos!=m_my_func->m_instr_list.end(); ++pos)
    {
        Instruction * p = *pos;
        if (p->type == i_GetAddr
            && p->var_w.thevar != NULL
            && g_VarTypeManage->GetPointTo(p->var_w.thevar->m_DataTypeID) != 0
            && p->var_r1.thevar != NULL
            && g_VarTypeManage->is_simple(p->var_r1.thevar->m_DataTypeID)
            && GG_VarType_ID2Size(g_VarTypeManage->GetPointTo(p->var_w.thevar->m_DataTypeID))
                >= GG_VarType_ID2Size(p->var_r1.thevar->m_DataTypeID))
        {
            p->var_r1.thevar->m_DataTypeID = g_VarTypeManage->GetPointTo(p->var_w.thevar->m_DataTypeID);
            //Just change it was not enough
            p->var_r1.opsize = GG_VarType_ID2Size(p->var_r1.thevar->m_DataTypeID);
            p->var_r1.thevar->size = GG_VarType_ID2Size(p->var_r1.thevar->m_DataTypeID);
            this->m_my_func->m_exprs->Enlarge_Var(p->var_r1.thevar, this->m_my_func->m_instr_list);
            return true;
        }
    }
    return false;
}
//--------------------------------------------------
bool CFuncOptim::optim_once_new()
{//Notice: at this time, cmp and jxx, has not yet merged

    MLIST::iterator pos = this->m_my_func->m_exprs->vList->begin();
    for (;pos!=this->m_my_func->m_exprs->vList->end();++pos)
    {
        M_t* p = *pos;
        //if (p->type == MTT_reg)
        if (p->type != MTT_var)
        {
            if (Optim_var_NT(p))
                return true;
        }
    }
    return false;
}

static st_VarOptm* used_list_Find(Instruction * pinstr, VAROPTM_LIST& used_list)
{
    VAROPTM_LIST::iterator pos = used_list.begin();

    for(;pos!=used_list.end();++pos)
    {
        if ((*pos)->pInstr == pinstr)
            return *pos;
    }
    return NULL;
}

BYTE GetVarFinger_INSTR(M_t* pvar, Instruction * p)
                //00: nothing with
                //01: read
                //02: write
                //03: read and write
{
    BYTE r = 0;
    if (pvar == p->var_w.thevar)
    {
        r |= 2;
        if (p->var_w.part_flag != 0)
            r|=4;
    }
    if (pvar == p->var_r1.thevar)
    {
        r |= 1;
        if (p->var_r1.part_flag != 0)
            r|=4;
    }
    if (pvar == p->var_r2.thevar)
    {
        r |= 1;
        if (p->var_r2.part_flag != 0)
            r|=4;
    }
    return r;
}

void CFuncOptim::prt_var_uselist(VAROPTM_LIST& used_list)
{
    log_prtl("%s","===========");
    //INSTR_LIST &list = this->Q->m_instr_list;
    VAROPTM_LIST::iterator pos = used_list.begin();
    for (;pos!=used_list.end(); ++pos)
    {
        st_VarOptm* the = *pos;
        Instruction * p = the->pInstr;
        if (p->type == i_Jump)
        {
            if (p->jmp.jmp_type == JMP_jmp)
                log_prtl("    jmp %x", p->jmp.jmpto_off);
            else
                log_prtl("    jxx %x", p->jmp.jmpto_off);
        }
        else if (p->type == i_Label)
            log_prtl("    %x:", p->label.label_off);
        else if (p->type == i_Return)
            log_prtl("    ret");
        else
        {
            CFunc_Prt theprt(this->m_my_func);
            std::string s = theprt.prt_the_instr(p);
            log_prtl("    %s", (const char *)s.c_str());
        }
    }
    log_prtl("%s","===========");
}


bool RemoveOneInstr_1(VAROPTM_LIST& used_list, Instruction * p)
{
    assert(p->type == i_Jump);
    for(VAROPTM_LIST::iterator pos = used_list.begin(); pos!=used_list.end(); ++pos)
    {
        if (p == (*pos)->pInstr && (*pos)->bJxx)
        {
            used_list.erase(pos);
            return true;
        }
    }
    //std::remove_if()
    return false;
}
bool RemoveOneInstr(VAROPTM_LIST& used_list, Instruction * p)
{
    bool rtn = false;
    rtn |= RemoveOneInstr_1(used_list, p);
    while (p->jmp.next_ref_of_this_label)
    {
        p = p->jmp.next_ref_of_this_label;
        rtn |= RemoveOneInstr_1(used_list, p);
    }
    return rtn;
}
bool CFuncOptim::Optim_Var_Flow_3(VAROPTM_LIST& used_list)
{
    //For two consecutive EE, should remove one
    bool FindE = false;

    for(VAROPTM_LIST::iterator pos = used_list.begin(); pos!=used_list.end(); ++pos)
    {
        st_VarOptm* the = *pos;
        if (the->pInstr->type == i_End || the->pInstr->type == i_Return)
        {
            if (the->rw == 0)
            {
                if (FindE)
                {
                    used_list.erase(pos);
                    return true;
                }
                FindE = true;
                continue;
            }
        }
        FindE = false;
    }
    return false;
}
bool CFuncOptim::Optim_Var_Flow_2(VAROPTM_LIST& used_list)
{
    //If there is a LE, then LE points to out of this the jmp
    //If there is a L6, then L6 points to out of this the jmp
    Instruction * lastlabelinstr = NULL;
    for (VAROPTM_LIST::iterator pos = used_list.begin(); pos!=used_list.end(); ++pos)
    {
        st_VarOptm* the = *pos;
        if (lastlabelinstr != NULL)
        {
            bool f = false;
            if (the->pInstr->type == i_End || the->pInstr->type == i_Return)
            {
                if (the->rw == 0)
                {//Found a LE
                    f=true;
                }
            }
            if (the->rw == 2)   //rw == 2 means write
            {
                f=true;
            }
            if (f)
            {
                Instruction * p = lastlabelinstr->label.ref_instr;
                if (RemoveOneInstr(used_list, p))   // This points to out the jmp LE
                    return true;
            }
        }

        lastlabelinstr = NULL;
        if (the->pInstr->type == i_Label)
            lastlabelinstr = the->pInstr;
    }

    return false;
}
bool CFuncOptim::Optim_Var_Flow_1(VAROPTM_LIST& used_list)
{
    //第一条就是条件跳，可去掉
    //跳到第一条了，可去掉
    VAROPTM_LIST::iterator headpos = used_list.begin();
    Instruction * head_label_instr = NULL;
    for (VAROPTM_LIST::iterator pos = used_list.begin(); pos!=used_list.end(); ++pos)
    {
        st_VarOptm* the = *pos;
        Instruction * p = the->pInstr;
        if (headpos == pos)
        {
            if (p->type == i_Return)
            {
                used_list.erase(pos); //remove me
                return true;    //The first is Return, can be removed
            }
            if (the->IsJump())
            {
                used_list.erase(pos); //remove me
                return true;    //The first is jump, can be removed
            }
            if (p->type == i_Label)
            {
                head_label_instr = p;
                continue;
            }
            break;
        }

        if (the->IsJump())
        {
            if (p->jmp.target_label == head_label_instr)
            {
                used_list.erase(pos); //remove me
                return true;    //跳到第一条了，可去掉
            }
        }
    }
    return false;
}
bool CFuncOptim::Optim_Var_Flow(VAROPTM_LIST& used_list)
{
    //这些函数，都只允许 used_list中的项，不允许 instr_list中的项
    if (this->Optim_Var_Flow_1(used_list))
        return true;
    if (this->Optim_Var_Flow_2(used_list))
        return true;
    if (this->Optim_Var_Flow_3(used_list))
        return true;
    if (this->Optim_Var_Flow_4(used_list))
        return true;
    if (this->Optim_Var_Flow_5(used_list))
        return true;
    return false;
}


st_VarOptm* Find_Instr(VAROPTM_LIST& used_list, Instruction * pinstr)
{
    for (VAROPTM_LIST::iterator pos = used_list.begin(); pos!=used_list.end(); ++pos)
    {
        st_VarOptm* the = *pos;
        if (the->pInstr == pinstr)
            return the;
    }
    return NULL;
}
bool CFuncOptim::SureNotUse_1(VAROPTM_LIST& used_list, st_VarOptm* the)
{
    if (the->tem_1)
        return true;
    the->tem_1 = true;

    if (the->rw == 2)
        return true;
    if (the->rw & 1)
        return false;
    if (the->pInstr->type == i_Return)
        return true;

    if (the->bJxx)
    {
        if (!SureNotUse_1(used_list, Find_Instr(used_list, the->pInstr->jmp.target_label)))
            return false;
    }

    VAROPTM_LIST::iterator pos = std::find(used_list.begin(),used_list.end(),the);
    assert(pos!=used_list.end());
    ++pos;
    if (pos==used_list.end())
        return true;
    return SureNotUse_1(used_list,*pos);
}
bool CFuncOptim::SureNotUse(VAROPTM_LIST& used_list, st_VarOptm* j)
{
    assert(j->bJxx);
    VAROPTM_LIST::iterator pos = used_list.begin();
    for (;pos!=used_list.end();++pos)
    {
        (*pos)->tem_1 = false;
    }
    return SureNotUse_1(used_list,j);
}
bool CFuncOptim::Optim_Var_Flow_5(VAROPTM_LIST& used_list)
{//这是最难的
    for (VAROPTM_LIST::iterator pos = used_list.begin(); pos!=used_list.end(); ++pos)
    {
        st_VarOptm* the = *pos;
        if (the->bJxx)
        {
            if (this->SureNotUse(used_list, the))
            {
                used_list.erase(pos);
                return true;
            }
        }
    }
    return false;
}
bool CFuncOptim::Optim_Var_Flow_4(VAROPTM_LIST& used_list)
{
    //空的下跳，要
    //空的上跳，要
    //没人用的Label，要

    VAROPTM_LIST::iterator last_pos;
    Instruction * last_jmp = NULL;
    Instruction * last_label[256];
    int n_label = 0;

    for (VAROPTM_LIST::iterator pos = used_list.begin(); pos!=used_list.end(); ++pos)
    {
        st_VarOptm* the = *pos;
        Instruction * p = the->pInstr;
        if (the->IsJump())
        {
            for (int i=0; i<n_label; i++)
            {
                if (last_label[i] != NULL && p->jmp.target_label == last_label[i])
                {
                    used_list.erase(pos); //remove me
                    return true;
                }
            }
            last_jmp = p;
            last_pos = pos;
            n_label = 0;
        }
        else if (p->type == i_Label)
        {
            if (last_jmp != NULL && last_jmp->jmp.target_label == p)
            {
                used_list.erase(last_pos);
                return true;
            }
            last_label[n_label++] = p;
        }
        else
        {
            last_jmp = NULL;
            n_label = 0;
        }
    }
    //没人用的空的label要 掉
    for (VAROPTM_LIST::iterator pos = used_list.begin(); pos!=used_list.end(); ++pos)
    {
        st_VarOptm* the = *pos;
        Instruction * p = the->pInstr;
        if (p->type == i_Label)
        {
            bool used = false;
            for (VAROPTM_LIST::iterator pos1 = used_list.begin(); pos1!=used_list.end(); ++pos1)
            {
                st_VarOptm* the1 = *pos1;
                Instruction * p1 = the1->pInstr;
                if (!the1->IsJump())
                    continue;

                if (p1->jmp.target_label == p)
                {
                    used = true;
                    break;
                }
            }
            if (!used)
            {
                used_list.erase(pos);
                return true;
            }
        }
    }

    return false;
}
char HowVarUse_Char(st_VarOptm* the);
std::string CFuncOptim::Get_var_finger_NT(VAROPTM_LIST& volist, M_t* pvar)
{
    std::string tbl_c;
    VAROPTM_LIST::iterator pos = volist.begin();
    for(;pos!=volist.end();++pos)
    {
        tbl_c += HowVarUse_Char(*pos);
    }
    return tbl_c;
}
char HowVarUse_Char(st_VarOptm* the)
{
    if (the->bJxx)
        return 'J';

    Instruction * pinstr = the->pInstr;
    BYTE rw = the->rw;
    if (pinstr->type == i_Label) return 'L';
    if (pinstr->type == i_Return) return 'E';
    if (pinstr->type == i_Assign)
    {
        if (rw == 1) return '5';    //Read
        if (rw == 2) return '6';    //Write
        if (rw == 3) return '7';    //Read and Write
    }
    else
    {
        if (rw == 1) return '1';    //Read
        if (rw == 2) return '2';    //Write
        if (rw == 3) return '3';    //Read and Write
    }
    if (rw >= 4) return 'u';
    assert(0);
    return 'U'; //Should not reach here
}
//SuperC_func: used only in <CFuncOptim::Optim_var_NT>
bool CFuncOptim::Optim_var_flow_NT(VAROPTM_LIST& volist, M_t* pvar)
{
    int n = 0;
    char tbl_c[256];
    Instruction * tbl_pinstr[256];

    VAROPTM_LIST::iterator pos = volist.begin();
    while (pos!=volist.end())
    {
        st_VarOptm* the = *pos;
        ++pos;
        tbl_c[n] = HowVarUse_Char(the);
        tbl_c[n+1] = '\0';
        tbl_pinstr[n] = the->pInstr;
        n++;

        if (n >= 1)
        {
            const char* pb = tbl_c + n - 1;
            Instruction ** pi = tbl_pinstr + n - 1;
            if (strcmp(pb, "7") == 0 && pi[0]->va_r1.pao == NULL) // the is Assign[Read and Write]
            {//自己assign自己
                g_CStrategy.AddOne_CanDelete(pvar, pi[0], "assiang self, delete it");
                return true;
            }
        }
        if (n >= 2)
        {
            const char* pb = tbl_c + n - 2;
            Instruction ** pi = tbl_pinstr + n - 2;
            if (strcmp(pb, "2E") == 0 || strcmp(pb, "3E") == 0 || strcmp(pb, "6E") == 0
                || strcmp(pb, "7E") == 0)
            {
                g_CStrategy.AddOne_CanDelete(pvar, pi[0], "write and end, delete it");
                return true;
            }
            if (strcmp(pb, "63") == 0)
            {
                g_CStrategy.AddOne_CanEliminate_63(pvar, pi[0], pi[1], "63");
                return true;
            }
            if (strcmp(pb, "22") == 0 || strcmp(pb, "26") == 0
                || strcmp(pb, "32") == 0 || strcmp(pb, "36") == 0
                || strcmp(pb, "62") == 0 || strcmp(pb, "66") == 0)
            {
                g_CStrategy.AddOne_CanDelete(pvar, pi[0], "Write and Write, delete first");
                return true;
            }
            if (strcmp(pb, "23") == 0)
            {
                if (pvar->type != MTT_tem)
                {
                    g_CStrategy.AddOne_CanEliminate_21E(pvar,pi[0],pi[1],"23");
                    return true;
                }
            }
        }
        if (n >= 3)
        {
            const char* pb = tbl_c + n - 3;
            char fpt[4];    //finger point
            strcpy(fpt,pb);
            assert(strlen(fpt) == 3);
            if (fpt[2] == '6' || fpt[2] == '2')
                fpt[2] = 'E';

            Instruction ** pi = tbl_pinstr + n - 3;
            if (strcmp(fpt, "25E") == 0 && pi[1]->va_r1.pao == NULL)
            {
                g_CStrategy.AddOne_CanEliminate_25E(pvar,pi[0],pi[1],"25E");
                return true;
            }
            if (strcmp(fpt, "61E") == 0)
            {
                g_CStrategy.AddOne_CanEliminate_63(pvar, pi[0], pi[1], "61E");
                return true;
            }
            if (strcmp(fpt, "65E") == 0)
            {
                if (pi[1]->va_r1.pao == NULL || pi[1]->va_r1.pao->type != IA_GetAddress)
                {
                    //If A = 3, B = & A obviously can not become A = & 3
                    g_CStrategy.AddOne_CanEliminate_63(pvar,pi[0],pi[1],"65E");
                    return true;
                }
            }
            if (strcmp(fpt, "31E") == 0)
            {
                if (pi[0]->type == i_Sub || pi[0]->type == i_Add)
                if (pi[0]->var_w.thevar == pi[0]->var_r1.thevar)
                if (pi[1]->type == i_Cmp && pi[0]->var_w.thevar == pi[0]->var_r1.thevar)
                {
                    //log_prtl("31E find");
                    g_CStrategy.AddOne_CanEliminate_31E(pvar,pi[0],pi[1],"31E");
                    return true;
                }
            }
            if (strcmp(fpt, "21E") == 0)
            {
                if (pvar->type != MTT_tem)
                {
                    g_CStrategy.AddOne_CanEliminate_21E(pvar,pi[0],pi[1],"21E");
                    return true;
                }
            }
        }
        if (n >= 4)
        {
            const char* pb = tbl_c + n - 4;
            Instruction ** pi = tbl_pinstr + n - 4;

            char fpt[5];    //finger point
            strcpy(fpt,pb);
            assert(strlen(fpt) == 4);
            if (fpt[3] == '6' || fpt[3] == '2')
                fpt[3] = 'E';

            if (strcmp(fpt, "311E") == 0)
            {
                if (pi[0]->type == i_Sub || pi[0]->type == i_Add)
                if (pi[0]->var_w.thevar == pi[0]->var_r1.thevar)
                if (pi[1]->type == i_Cmp && pi[0]->var_w.thevar == pi[1]->var_r1.thevar)
                if (pi[2]->type == i_Cmp && pi[0]->var_w.thevar == pi[2]->var_r1.thevar)
                {
                    log_prtl("311E find");
                    signed int d = pi[0]->var_r2.d;
                    if (pi[0]->type == i_Add)
                        d = -d;
                    pi[1]->var_r2.d += d;
                    pi[2]->var_r2.d += d;
                    INSTR_LIST::iterator to_erase=std::find(m_my_func->m_instr_list.begin(),m_my_func->m_instr_list.end(),pi[0]);
                    assert(to_erase!=m_my_func->m_instr_list.end());
                    m_my_func->m_instr_list.erase(to_erase);
                    return true;
                }
            }
        }
    }
    return false;
}
//SuperC_func: Used only in <CFuncOptim::optim_once_new>
//Attempt to optimize the var
bool CFuncOptim::Optim_var_NT(M_t* pvar)
{
    if (pvar == NULL)
        return false;

    VAROPTM_LIST used_list;
    //Parsed instr

    Get_Var_Use_Flow(used_list, pvar);
    //Variables used by the first graph
    //First optimized, and then divided
    if (Optim_var_flow_NT(used_list, pvar))
    {
        prt_var_uselist(used_list);
        return true;
    }

    return false;
}

static int static_n = 0;
static int static_tbl[80];


void CFuncOptim::TryDistinguishVar_1( VAROPTM_LIST& volist, M_t* pvar, VAROPTM_LIST::iterator pos, int* pglobalstep )
{
    int step = *pglobalstep;

    //之所以要有一个pglobalstep，是为了得到下一个step
    for(;pos!=volist.end();++pos)
    {
        st_VarOptm* the = *pos;
        if (the->bJxx)
        {
            st_VarOptm* plabel = used_list_Find(the->pInstr->jmp.target_label, volist);
            assert(plabel != 0);
            VAROPTM_LIST::iterator it_found=std::find(volist.begin(),volist.end(),plabel);
            assert(it_found!=volist.end());
            TryDistinguishVar_1(volist, pvar, it_found, pglobalstep);

            if (the->pInstr->jmp.jmp_type == JMP_jmp)
            {
                step = *pglobalstep;
                *pglobalstep = step+1;
            }
            continue;
        }
        if (the->pInstr->type == i_Label)
        {
            continue;   //不需要做什么事情
        }
        if (the->pInstr->type == i_Return)
        {
            step = *pglobalstep;
            *pglobalstep = step+1;
            continue;
        }
        //
        assert(the->rw != 0);
        if (the->rw & 1)    //read
        {
            if (the->varstep_r == step)
                return;

            if (the->varstep_r != 0 && the->varstep_r != step)
            {//撞车了，怎么办？
                int i1 = std::min(the->varstep_r, step);
                int i2 = std::max(the->varstep_r, step);
                assert(static_n < 80);
                static_tbl[static_n++] = i1;
                static_tbl[static_n++] = i2;
                return;
            }
            the->varstep_r = step;
        }
        if (the->rw & 2)    //write
        {
            if (the->varstep_w != 0)
                return; //有人做过了，不必再做

            step = *pglobalstep;
            step++;
            *pglobalstep = step;
            the->varstep_w = step;
        }
    }
}

bool CFuncOptim::IfAnyThisStep(int i, VAROPTM_LIST& volist)
{
    VAROPTM_LIST::iterator pos = volist.begin();
    for (;pos!=volist.end();++pos)
    {
        st_VarOptm* the = *pos;
        if (the->varstep_r == i)
            return true;
        if (the->varstep_w == i)
            return true;
    }
    return false;
}

bool Get_pair(int& out_i1, int& out_i2)
{
//找i2最大的那个
    int savi;
    int max_i2 = 0;
    for (int i=0; i<static_n; i+= 2)
    {
        int i1 = static_tbl[i];
        int i2 = static_tbl[i+1];
        if (i2 > max_i2)
        {
            max_i2 = i2;
            out_i2 = i2;
            out_i1 = i1;
            savi = i;
        }
    }
    if (max_i2 == 0)
        return false;
    static_tbl[savi] = 0;  //清掉，以免重复使用
    static_tbl[savi+1] = 0;
    return true;
}

void Replace_i2_with_i1(VAROPTM_LIST& volist, int i1, int i2)
{
    VAROPTM_LIST::iterator pos = volist.begin();
    for (;pos!=volist.end();++pos)
    {
        st_VarOptm* the = *pos;
        if (the->varstep_r == i2)
            the->varstep_r = i1;
        if (the->varstep_w == i2)
            the->varstep_w = i1;
    }
}


void CFuncOptim::Replace_Var_vo(VAROPTM_LIST& volist, int step, M_t* pvar, M_t* pnew)
{
    VAROPTM_LIST::iterator pos = volist.begin();
    for (;pos!=volist.end();++pos)
    {
        st_VarOptm* p = *pos;
        if (p->varstep_r == step)
        {
            if (p->pInstr->var_r1.thevar == pvar)
                p->pInstr->var_r1.thevar = pnew;
            if (p->pInstr->var_r2.thevar == pvar)
                p->pInstr->var_r2.thevar = pnew;
        }
        if (p->varstep_w == step)
        {
            if (p->pInstr->var_w.thevar == pvar)
                p->pInstr->var_w.thevar = pnew;
        }
    }
}
const char * my_itoa(int i);
//看看能不能把一个var从逻辑上分为两个
//开始写写varstep_r和varstep_w
bool CFuncOptim::TryDistinguishVar(VAROPTM_LIST& volist, M_t* pvar)
{
    if (pvar->type != MTT_reg)
        return false;   //Only the first type of processing reg

    if (pvar->GetName().compare("ESI") == 0)
        pvar->GetName();

    static_n = 0;
    int step = 1;
    TryDistinguishVar_1(volist, pvar,
                        volist.begin(),
                        &step);

    for (;;)
    {
        int i1, i2;
        if (!Get_pair(i1, i2))
            break;
        Replace_i2_with_i1(volist, i1, i2);
    }

    int n = 0;
    int tblstep[120];
    for (int i=1; i<step+1; i++)
    {
        if (IfAnyThisStep(i, volist))
            tblstep[n++] = i;
        }
    if (n > 1)
    {
        log_prtl("find var distinguish");
        for (int i=0; i<n; i++)
        {
            if (i == 0)
                continue;   //keep the first one

            M_t* pnew = new M_t;
            *pnew = *pvar;
            pnew->namestr +="_";
            pnew->namestr +=my_itoa(i);
            this->m_my_func->m_exprs->vList->push_back(pnew);
            Replace_Var_vo(volist, tblstep[i], pvar, pnew);
        }
        return true;
    }

    return false;
}

void CFuncOptim::Get_Var_Use_Flow(VAROPTM_LIST& used_list, M_t* pvar)
{
    //Write only used_list.pInstr the used_list.rw, do not write varstep_r and varstep_w
    Instruction * tbl_label[728];
    int num = 0;

    INSTR_LIST &list = this->m_my_func->m_instr_list;
    POSITION pos = list.begin();
    for( ;pos!=list.end();++pos)
    {
        Instruction * p = *pos;
        BYTE c = GetVarFinger_INSTR(pvar, p);
        if (c != 0 || p->type == i_Label || p->type == i_Return)
        {//These are my only concern

            st_VarOptm* the = new st_VarOptm;

            the->pInstr = p;
            the->rw = c;

            used_list.push_back(the);
        }
        if (p->type == i_Jump)
        {
            tbl_label[num++] = p->jmp.target_label;

            st_VarOptm* the = new st_VarOptm;

            the->pInstr = p;
            the->rw = c;
            the->bJxx = true;

            used_list.push_back(the);

            // In this way, a jump may be added twice,
            // the first one is that it uses the variable cmp (v,?)
            // After one of its jxx
        }
    }
    //----------------------------------
    //To optimize the sequence and the label to get rid of useless jmp

    while (Optim_Var_Flow(used_list))
        ;
}

void CFuncOptim::Prt_Var_Flow(const char * varname)
{
    M_t* pvar = this->m_my_func->m_exprs->GetVarByName(varname);
    if (pvar == NULL)
    {
        log_prtl("var: %s not found ", varname);
        return;
    }
    log_prtl("find var: %s", varname);
    VAROPTM_LIST used_list; //parsed instr

    Get_Var_Use_Flow(used_list, pvar);
    prt_var_uselist(used_list);
    std::string s = Get_var_finger_NT(used_list, pvar);

    log_prtl("%s", s.c_str());
}
bool CFuncOptim::DataType_Check(VAR_ADDON* pva, FuncType* pftype)
{
    M_t* pvar = pva->pv->thevar;
    if (pvar->m_DataTypeID == 0)
        return false;
    assert(pvar);
    if (!pftype->m_varpar)
    {
        int i1 = GG_VarType_ID2Size(pvar->m_DataTypeID);
        int i2 = pftype->para_total_size();
        if (pva->pao == NULL)
        {
            assert(i1 == i2);
        }
        else if (pva->pao->type == IA_GetAddress)
        {
            assert(i2 == 4);
        }
    }
    //这是把i_CallPara的参数的数据类型改了
    for(POSITION pos=m_my_func->m_instr_list.begin(); pos != m_my_func->m_instr_list.end(); ++pos)
    {
        Instruction * p = *pos;
        if (p->var_w.thevar == pvar && p->type == i_Assign)
        {
            if (p->var_r1.thevar != NULL
                    && g_VarTypeManage->is_simple(p->var_r1.thevar->m_DataTypeID)
                    )
            {
                VarTypeID id;
                if (p->var_w.part_flag == 0)
                {//说明只有一个参数
                    assert(pftype->m_args == 1);
                    id = pftype->m_partypes[0];
                }
                else
                {
                    id = pftype->SearchPara(p->var_w.part_flag - 1);
                }
                if (id != 0)
                {
                    if (p->va_r1.pao == NULL)
                    {
                        if (p->var_r1.thevar->type != MTT_immed)
                        {
                            int size1 = p->var_r1.thevar->size;
                            int size2 = GG_VarType_ID2Size(id);
                            assert(size1 == size2);
                        }
                        VarTypeID oldid = p->var_r1.thevar->m_DataTypeID;
                        p->var_r1.thevar->m_DataTypeID = id;

                        log_prtl("1: %s datatype %s -> %s",
                                 p->var_r1.thevar->GetName().c_str(),
                                 ::GG_VarType_ID2Name(oldid).c_str(),
                                 ::GG_VarType_ID2Name(id).c_str()
                                 );

                        return true;
                    }
                    else if (p->va_r1.pao->type == IA_GetAddress)
                    {
                        VarTypeID oldid = p->var_r1.thevar->m_DataTypeID;
                        VarTypeID id2 = g_VarTypeManage->GetPointTo(id);

                        p->var_r1.thevar->m_DataTypeID = id2;

                        UINT size1 = p->var_r1.thevar->size;    //原来的size
                        UINT size2 = GG_VarType_ID2Size(id2);   //新的size
                        if (size1 < size2)
                        {//变大了
                            p->var_r1.thevar->size = size2;
                            this->m_my_func->m_exprs->Enlarge_Var(p->var_r1.thevar, m_my_func->m_instr_list);
                        }

                        log_prtl(this->m_my_func->Instr_prt_simple(p).c_str());
                        log_prtl("& X = TYPE *, so X = TYPE");

                        log_prtl("2: %s datatype %s -> %s",
                                 p->var_r1.thevar->GetName().c_str(),
                                 ::GG_VarType_ID2Name(oldid).c_str(),
                                 ::GG_VarType_ID2Name(id2).c_str());

                        return true;
                    }
                }
            }
        }
    }
    return false;
}
bool CFuncOptim::SetParaType(UINT offset, UINT sizepara, enum_CallC conv,const std::string & paraname, VarTypeID paraid)
{
    if (conv == enum_cdecl || conv == enum_stdcall)
    {
        UINT par_off = offset + 4;  //对不对呢
        M_t* pmt = this->m_my_func->m_exprs->SearchMT(MTT_par, par_off);
        if (pmt == NULL)
            return false;
        if (pmt->m_DataTypeID != 0 && !g_VarTypeManage->is_simple(pmt->m_DataTypeID))
            return false;   //Already have the type, do reason to do it again
        pmt->namestr = paraname;
        //Now, make pmt-> m_DataType for the paratype
        if (pmt->size == GG_VarType_ID2Size(paraid))
        {
            pmt->m_DataTypeID = paraid;
        }
        else
        {
            M_t* pnew = new M_t;    //new_M_t
            *pnew = *pmt;
            pnew->m_DataTypeID = paraid;

            m_my_func->m_exprs->vList->push_back(pnew);

            this->m_my_func->m_exprs->Enlarge_Var(pnew, m_my_func->m_instr_list);

            assert(pnew->size == GG_VarType_ID2Size(paraid));
        }
        return true;
    }
    else
        assert(0);
    return false;
}
bool CFuncOptim::VarDataType_analysis_mydefine()
{//I already have a function definition, parameter names and types from which to take
    if (this->m_my_func->m_functype == NULL)
        return false;
    FuncType* pftype = this->m_my_func->m_functype;

    UINT sizepara = pftype->para_total_size();
    int offset = 0;
    for (int i=0; i<pftype->m_args; i++)
    {
        if (this->SetParaType(offset,sizepara, this->m_my_func->m_functype->m_callc,pftype->m_parnames[i],pftype->m_partypes[i]))
            return true;
        offset += GG_VarType_ID2Size(pftype->m_partypes[i]);
        while (offset % 4)
            offset++;
    }

    return false;
}
bool CFuncOptim::VarDataType_analysis()
{
    for(POSITION pos=m_my_func->m_instr_list.begin(); pos != m_my_func->m_instr_list.end(); ++pos)
    {
        Instruction * p = *pos;
        if (p->type == i_CallThis)
        {
            Instruction * pcall = p->call_addon.p_thecall;
            assert(pcall);
            if (pcall->type == i_Call)
            {
                FuncType* pft = pcall->call.call_func->m_functype;
                assert(pft != NULL);
                assert(pft->m_class != NULL);

                if (p->var_r1.thevar != NULL && g_VarTypeManage->is_simple(p->var_r1.thevar->m_DataTypeID) )
                {
                    VarTypeID id = g_VarTypeManage->Class2VarID(pft->m_class);
                    VarTypeID oldid = p->var_r1.thevar->m_DataTypeID;
                    if (p->va_r1.pao == NULL)
                    {
                        id = g_VarTypeManage->GetAddressOfID(id);
                        p->var_r1.thevar->m_DataTypeID = id;
                    }
                    else if (p->va_r1.pao->type == IA_GetAddress)
                    {
                        p->var_r1.thevar->m_DataTypeID = id;
                    }
                    log_prtl("3: %s datatype %s -> %s",
                             p->var_r1.thevar->GetName().c_str(),
                             ::GG_VarType_ID2Name(oldid).c_str(),
                             ::GG_VarType_ID2Name(id).c_str());

                    return true;
                }
            }
        }
        if (p->type == i_CallPara)
        {
            Instruction * pcall = p->call_addon.p_thecall;
            assert(pcall);
            if (pcall->type == i_CallApi)
            {
                FuncType* pft = pcall->call.papi->m_functype;
                if (pft != NULL)
                    if (this->DataType_Check(&p->va_r1, pft))
                        return true;
            }
            if (pcall->type == i_Call)
            {
                FuncType* pft = pcall->call.call_func->m_functype;
                if (pft != NULL)
                    if (this->DataType_Check(&p->va_r1, pft))
                        return true;
            }
        }
        if (p->type == i_CallRet)
        {
            Instruction * pcall = p->call_addon.p_thecall;
            assert(pcall);
            if (pcall->type == i_Call)
            {
                FuncType* pdf = pcall->call.call_func->m_functype;
                if (pdf != NULL
                        && p->var_w.thevar != NULL
                        && g_VarTypeManage->is_simple(p->var_w.thevar->m_DataTypeID)
                        )
                {
                    VarTypeID oldid = p->var_w.thevar->m_DataTypeID;
                    VarTypeID id = pdf->m_retdatatype_id;
                    p->var_w.thevar->m_DataTypeID = pdf->m_retdatatype_id;

                    log_prtl("4: %s datatype %s -> %s",
                             p->var_w.thevar->GetName().c_str(),
                             ::GG_VarType_ID2Name(oldid).c_str(),
                             ::GG_VarType_ID2Name(id).c_str());

                    return true;
                }
            }
            if (pcall->type == i_CallApi)
            {
                Api* papi = pcall->call.papi;
                VarTypeID retid = papi->m_functype->m_retdatatype_id;
                assert(retid);
                if (p->var_w.thevar != NULL
                        && g_VarTypeManage->is_simple(p->var_w.thevar->m_DataTypeID)
                        )
                {
                    VarTypeID oldid = p->var_w.thevar->m_DataTypeID;
                    p->var_w.thevar->m_DataTypeID = retid;

                    log_prtl("5: %s datatype %s -> %s",
                             p->var_w.thevar->GetName().c_str(),
                             ::GG_VarType_ID2Name(oldid).c_str(),
                             ::GG_VarType_ID2Name(retid).c_str());

                    return true;
                }
            }
        }
    }
    return false;
}
