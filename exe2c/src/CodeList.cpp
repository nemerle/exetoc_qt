// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//#include "stdafx.h"
#include	"CISC.h"
#include "exe2c.h"

//Initial construction instr_list and expr_list
void CodeList::CreateInstrList_raw(AsmCodeList* asmlist, int EBP_base)
{
    this->m_asmlist = asmlist;
    this->m_EBP_base = EBP_base;

    Instruction * p_begin = new Instruction(i_Begin);     //new_INSTR
    Instruction * p_end = new Instruction(i_End);		//new_INSTR	总是同时new //new_INSTR	 is always the same time, new
    p_begin->begin.m_end = p_end;			//	point at it

    InstrAddTail(p_begin);	// preceded by i_Begin

    signed int esp_level = 3;
    AsmCodeList::iterator pos = this->m_asmlist->begin();
    for (;pos!=this->m_asmlist->end();++pos)
    {
        AsmCode* cur = *pos;
        assert(cur);
        assert(cur->linear);

        CodeList_Maker the(this,cur);

        if (cur->xcpu.opcode != C_JCASE)
        {	//The only exception, C_JCASE label without prior
            Instruction * p = new Instruction(i_Label);   //new_INSTR
            p->label.label_off = cur->linear;
            //In front of each instruction preceded by a label
            InstrAddTail(p);
        }

        if (esp_level != 3 && esp_level < cur->esp_level)
        {
            //This is a pop
            cur->xcpu.opcode;
            Instruction *	p = new Instruction(i_EspReport);  //new_INSTR
            p->espreport.esp_level = cur->esp_level;
            p->espreport.howlen = cur->esp_level - esp_level;
            InstrAddTail(p);
        }

        the.AddTail_Cur_Opcode();

        esp_level = cur->esp_level;
    }

    InstrAddTail(p_end);	//	i_End - last instruction
}

void CodeList::InstrAddTail(Instruction * p)
{
    if (p->var_w.type)
            assert(p->var_w.opsize);
    if (p->var_r1.type)
            assert(p->var_r1.opsize);
    if (p->var_r2.type)
            assert(p->var_r2.opsize);

    m_instr_list.push_back(p);
}

void	set_address(OPERITEM* op,Instruction * p)
{
    if (op->addr.base_reg_index != _NOREG_)
    {
        p->var_r1.type = v_Reg;
        p->var_r1.opsize = BIT32_is_4;
        p->var_r1.reg = regindex_2_regoff(op->addr.base_reg_index);
    }

    if (op->addr.off_reg_index != _NOREG_)
    {
        p->var_r2.type = v_Reg;
        p->var_r2.opsize = BIT32_is_4;
        p->var_r2.reg = regindex_2_regoff(op->addr.off_reg_index);
    }

    p->i1 = op->addr.off_reg_scale;
    p->i2 = op->addr.off_value;
}
//-------------------------------------------------------
void	set_address(OPERITEM* op,Instruction * p);

ea_t FindApiAddress_Reg(uint32_t regindex, XCPUCODE* pxcpu1, AsmCodeList* asmlist);

//	把当前的xcpu转化为伪码，同时在intr_list中addtail
//Put the current xcpu into pseudo-code, while intr_list in addtail
void	CodeList_Maker::AddTail_Cur_Opcode()
{
    XCPUCODE* pxcpu = &cur->xcpu;

    switch (pxcpu->opcode)
    {
    case C_INC:
        {
            OPERITEM* op0 = &pxcpu->op[0];
            OPERITEM* op1 = &pxcpu->op[1];
//            OPERITEM* op2 = &this->cur->xcpu.op[2];
            op1->mode = OP_Immed;
            op1->opersize = op0->opersize;
            op1->immed.immed_value = 1;
        }

        Code_general(enum_AR, i_Add);    // Added 2005.2.1
        break;
    case C_DEC:
        {
            OPERITEM* op0 = &pxcpu->op[0];
            OPERITEM* op1 = &pxcpu->op[1];
//            OPERITEM* op2 = &this->cur->xcpu.op[2];
            op1->mode = OP_Immed;
            op1->opersize = op0->opersize;
            op1->immed.immed_value = 1;
        }

        Code_general(enum_AR, i_Sub);    //2005.2.1加
        {
            OPERITEM* op0 = &pxcpu->op[0];
            OPERITEM* op1 = &pxcpu->op[1];
//            OPERITEM* op2 = &this->cur->xcpu.op[2];
            op1->mode = OP_Immed;
            op1->opersize = op0->opersize;
            op1->immed.immed_value = 0;
            Code_general(enum_RR, i_Cmp);
        }
        break;
    case C_JCASE:
        {
            //alert("C_JCASE find ");
            Instruction * p = new Instruction(i_Jump);   //new_INSTR
            p->jmp.jmp_type = JMP_case;
            p->jmp.jmpto_off = pxcpu->op[0].nearptr.offset;
            //We take a look at what it is before a
            Instruction * plast = *Q->m_instr_list.rbegin(); //We take a look at what it is before a
            if (plast->type == i_JmpAddr)
            {	//	means this is case 0
                p->var_r1 = plast->var_r2;	// index reg
                p->var_r2.type = v_Immed;
                p->var_r2.d = 0;
            }
            else if (plast->type == i_Jump && plast->jmp.jmp_type == JMP_case)
            {
                p->var_r1 = plast->var_r1;
                p->var_r2.type = v_Immed;
                p->var_r2.d = plast->var_r2.d + 1;	//	next case
            }
            else
            {
                alert_prtf("type is %s", hlcode_name(plast->type));
                assert(0);
            }
            Q->InstrAddTail(p);
        }
        break;
    case C_LEA:     Code_general(enum_WR, i_Lea);          break;
    case C_MOV:     Code_general(enum_WR, i_Assign);       break;
    case C_MOVZX:   Code_general(enum_WR, i_NosignExpand); break;
    case C_MOVSX:   Code_general(enum_WR, i_SignExpand);   break;
    case C_ADD:     Code_general(enum_AR, i_Add);    break;
    case C_IMUL:    Code_general(enum_AR, i_Imul);    break;
    case C_SUB:
        Code_general(enum_AR, i_Sub);
        {
            OPERITEM* op0 = &pxcpu->op[0];
            OPERITEM* op1 = &pxcpu->op[1];
//            OPERITEM* op2 = &this->cur->xcpu.op[2];
            op1->mode = OP_Immed;
            op1->opersize = op0->opersize;
            op1->immed.immed_value = 0;
            Code_general(enum_RR, i_Cmp);
        }
        break;
    case C_SAR: Code_general(enum_AR, i_Sar);    break;
    case C_SHL: Code_general(enum_AR, i_Shl);    break;
    case C_SHR: Code_general(enum_AR, i_Shr);    break;
    case C_AND: Code_general(enum_AR, i_And);    break;
    case C_XOR:
        {
            Instruction * p = Code_general(enum_AR, i_Xor);
            if (VAR::IsSame(&p->var_r1,&p->var_r2))
            {	//	xor eax,eax means mov eax,0
                p->type = i_Assign;
                p->var_r1.type = v_Immed;
                p->var_r1.d = 0;
                p->var_r2.type = v_Invalid;
                //opsize not change
            }
            break;
        }

    case C_TEST:
        {
            Instruction * p = Code_general(enum_RR, i_Test);
            if (VAR::IsSame(&p->var_r1,&p->var_r2))
            {	//	test eax,eax means cmp eax,0
                p->type = i_Cmp;
                p->var_r2.type = v_Immed;
                p->var_r2.d = 0;
                //opsize not change
            }
        }
        break;
    case C_CMP:
        Code_general(enum_RR, i_Cmp);
        break;
    case C_PUSH:
        {
            Instruction *	p = new Instruction(i_Assign);  //new_INSTR

            p->var_w.type = v_Var;
            p->var_w.opsize = BIT32_is_4;
            p->var_w.var_off = stack2varoff(cur->esp_level - 4);	// or esp_level_next

            TransVar(p->var_r1, 0);	//	0 means	xcpu.op[0]
            Q->InstrAddTail(p);
        }
        break;
    case C_POP:
        {
            Instruction *	p = new Instruction(i_Assign);  //new_INSTR
            p->var_r1.type = v_Var;
            p->var_r1.opsize = BIT32_is_4;
            p->var_r1.var_off = stack2varoff(cur->esp_level);

            TransVar(p->var_w, 0);	//	0 means	xcpu.op[0]
            Q->InstrAddTail(p);
        }
        break;
    case C_LEAVE:
        break;


    case C_JO: 	Code_Jxx(JMP_jo);	break;
    case C_JNO: Code_Jxx(JMP_jno);	break;
    case C_JB: 	Code_Jxx(JMP_jb);	break;
    case C_JNB: Code_Jxx(JMP_jnb);	break;
    case C_JZ:	Code_Jxx(JMP_jz);	break;
    case C_JNZ: Code_Jxx(JMP_jnz);	break;
    case C_JNA: Code_Jxx(JMP_jna);	break;
    case C_JA:	Code_Jxx(JMP_ja);	break;
    case C_JS: 	Code_Jxx(JMP_js);	break;
    case C_JNS:	Code_Jxx(JMP_jns);	break;
    case C_JP: 	Code_Jxx(JMP_jp);	break;
    case C_JNP: Code_Jxx(JMP_jnp);	break;
    case C_JL: 	Code_Jxx(JMP_jl);	break;
    case C_JNL: Code_Jxx(JMP_jnl);	break;
    case C_JLE:	Code_Jxx(JMP_jle);	break;
    case C_JNLE:Code_Jxx(JMP_jnle);	break;
    case C_JMP:
        if (pxcpu->op[0].mode == OP_Near)
            Code_Jxx(JMP_jmp);
        else
        {
            if (pxcpu->op[0].mode == OP_Address)
            {
                Instruction * p = new Instruction(i_JmpAddr);   //new_INSTR
                //That said, I should first of this jmp [edx * 4 +402000] write it down
                 //And look forward to the back of C_JCASE
                set_address(&pxcpu->op[0], p);
                Q->InstrAddTail(p);
            }
        }
        break;
    case C_CALL:
        if (pxcpu->op[0].mode == OP_Near)
        {
            Instruction * p = new Instruction(i_Call);  //new_INSTR
            p->call.esp_level = cur->esp_level;
            p->call.call_func = g_Cexe2c->GetFunc(pxcpu->op[0].nearptr.offset);
            p->call.p_callpara = NULL;
            p->call.p_callret = NULL;
            Q->InstrAddTail(p);
        }
        else if (pxcpu->op[0].mode == OP_Address)
        {
            if (pxcpu->op[0].addr.base_reg_index == _NOREG_ &&
                pxcpu->op[0].addr.off_reg_index == _NOREG_)
            {
                ea_t address = pxcpu->op[0].addr.off_value;
                Api* papi = ApiManage::get()->get_api(address);	//find it
                if (papi)
                {
                    Instruction *	p = new Instruction(i_CallApi);  //new_INSTR
                    p->call.papi = papi;
                    p->call.esp_level = cur->esp_level;
                    p->call.p_callpara = NULL;
                    p->call.p_callret = NULL;
                    Q->InstrAddTail(p);
                }
                else
                    Code_general(0, i_Unknown);
            }
            else
                Code_general(0, i_Unknown);
        }
        else if (pxcpu->op[0].mode == OP_Register)
        {
            ea_t address = FindApiAddress_Reg(pxcpu->op[0].reg.reg_index, pxcpu, this->Q->m_asmlist);
            Api* papi = ApiManage::get()->get_api(address);	//find it
            if (papi)
            {
                Instruction *	p = new Instruction(i_CallApi);  //new_INSTR
                p->call.papi = papi;
                p->call.esp_level = cur->esp_level;
                p->call.p_callpara = NULL;
                p->call.p_callret = NULL;
                Q->InstrAddTail(p);
            }
            else
                Code_general(0, i_Unknown);
        }
        else
            Code_general(0, i_Unknown);
        break;
    case C_RET:	Code_general(0, i_Return);	break;
    default:	Code_general(0, i_Unknown);	break;

    }
}

void	CodeList_Maker::Code_Jxx(JxxType t)
{
    Instruction * p = new Instruction(i_Jump);   //new_INSTR
    p->jmp.jmp_type = t;
    p->jmp.jmpto_off = cur->xcpu.op[0].nearptr.offset;
    Q->InstrAddTail(p);
}

Instruction *	CodeList_Maker::Code_general(int type, HLType t)
{
  //Only type == enum_RR, the return value to be useful
    if (t == i_Unknown)
    {
        t=i_Unknown;
    }
    Instruction *	p = new Instruction(t);  //new_INSTR
    switch (type)
    {
    case enum_00:
        Q->InstrAddTail(p);
        return p;
    case enum_RR:
        {
            TransVar(p->var_r1, 0);	//	0 means	xcpu.op[0]
            TransVar(p->var_r2, 1);	//	1 means	xcpu.op[1]
            VarRead(p->va_r1);
            VarRead(p->va_r2);
        }
        Q->InstrAddTail(p);
        return p;
    case enum_WR:
        {
            TransVar(p->var_w, 0);	//	0 means	xcpu.op[0]
            TransVar(p->var_r1, 1);	//	1 means	xcpu.op[1]
            if (t == i_Lea)
            {
                p->type = i_Assign;
                if (p->var_r1.type != v_Tem)
                {
                    //For example, as lea eax, [ebp]
                    p->type = i_GetAddr;
                }
            }
            else
            {
                VarRead(p->va_r1);
                //VarWrite(&p->var_w);
                if (p->var_w.type == v_Tem)
                {
                    WriteToAddress(p);
                    return NULL;//Because here no one will use the return value
                }
            }
        }
        Q->InstrAddTail(p);
        return p;
    case enum_AR:
        {
            VAR v;
            TransVar(v, 0);	//	0 means	xcpu.op[0]
            TransVar(p->var_r2, 1);	//	1 means	xcpu.op[1]
            p->var_r1 = v;
            p->var_w = v;
            VarRead(p->va_r2);
            VarRead(p->va_r1);
            //VarWrite(&p->var_w);
            if (p->var_w.type == v_Tem)
            {
                WriteToAddress(p);
                //Because here no one will use the return value
                return NULL;
            }
        }
        Q->InstrAddTail(p);
        return p;
    default:
        alert("why here 325426");
        return NULL;
    }
    //return NULL;
}
// translate asm operand to var
void	CodeList_Maker::TransVar(VAR &pvar,int no)
{
    TransVar_(pvar,no);
    if (pvar.type)
    {
        assert(pvar.opsize);
    }
}
//SuperC_func: Use only in <CCodeList_Maker::TransVar>
void    CodeList_Maker::TransVar_op_addr(VAR &pvar,OPERITEM *op)
{
    if (op->addr.base_reg_index == _NOREG_ && op->addr.off_reg_index == _NOREG_)
    {
        if (op->addr.off_value == 0 && op->addr.seg_index == _FS_)
        {
            //determine fs: [0]
            pvar.type = v_Volatile;    //Now only used for fs: 0
            pvar.opsize = op->opersize;
            pvar.temno = 222;  //As long as even on the list
            return;
        }
        pvar.type = v_Global;
        pvar.opsize = op->opersize;
        pvar.off = op->addr.off_value;
        return;
    }
    if (op->addr.base_reg_index == _ESP_ && op->addr.off_reg_index == _NOREG_) // [esp+x]
    {
        pvar.opsize = op->opersize;
        signed long l = this->cur->esp_level + (signed int)op->addr.off_value;
        if (l >= 0)
        {
            pvar.par_off = l;
            pvar.type = v_Par;
        }
        else
        {
            pvar.var_off = stack2varoff(l);
            pvar.type = v_Var;
        }
        return;
    }
    if (op->addr.base_reg_index == _EBP_ && this->Q->m_EBP_base != Not_EBP_based && op->addr.off_reg_index == _NOREG_)
    {
        pvar.opsize = op->opersize;
        signed long l = this->Q->m_EBP_base + (signed int)op->addr.off_value;
        if (l >= 0)
        {
            pvar.par_off = l;
            pvar.type = v_Par;
        }
        else
        {
            pvar.var_off = stack2varoff(l);
            pvar.type = v_Var;
        }
        return;
    }
    //	now, really stuff
    VAR v;
    new_temp(&v);

    Instruction * p = new Instruction(i_Address);   //new_INSTR

    p->var_w = v;

    set_address(op, p);

    Q->InstrAddTail(p);

    pvar = v;

}
void	CodeList_Maker::TransVar_(VAR &pvar,int no)
{
    OPERITEM* op = &this->cur->xcpu.op[no];
    switch (op->mode)
    {
    case OP_Register:
        pvar.type = v_Reg;
        pvar.opsize = op->opersize;
        pvar.reg = regindex_2_regoff(op->reg.reg_index);
        return;
    case OP_Immed:
        pvar.type = v_Immed;
        pvar.opsize = op->opersize;
        pvar.d = op->immed.immed_value;
        return;
    case OP_Address:
        TransVar_op_addr(pvar,op);
        return;
    default:
        warn("op mode unknown");
        break;
    }
}

//This function means that, if this is a v_Tem, then add one i_Readpointto
void	CodeList_Maker::VarRead(VAR_ADDON& va)
{
    VAR* pvar = va.pv;
    Pst_InstrAddOn &pAddOn = va.pao;
    if (pvar->type != v_Tem)
        return;

    Pst_InstrAddOn pnew = new st_InstrAddOn;
    pnew->type = IA_ReadPointTo;
    pnew->pChild = pAddOn;
    pAddOn = pnew;

    return;
}
//This function means that, if it is to an address to write, then add a i_Writepointto
void	CodeList_Maker::WriteToAddress(Instruction * p)
{
    // For the add [ebx +4], 6, becomes
    //		tem_1 = i_addr(ebx,4);
    //		tem_2 = i_readpointto(tem_1);
    //		tem_3 = tem_2 + 6;
    //		i_writepointto(tem_1, tem_3);


    // 	The current situation is:
    //		tem1 addr eax,ebx*4,401000
    //		tem1 = ????
    // 	We wanted to change
    //		tem1 addr eax,ebx*4,401000
    //		tem2 = ????
    //		Writepointto(tem1, tem2);

    if (p->var_w.type != v_Tem)
    {
        //	Will not actually enter here
        Q->InstrAddTail(p);
        return;
    }

    VAR tem1 = p->var_w;	//	sav it
    VAR tem2;

    new_temp(&tem2);

    p->var_w = tem2;
    Q->InstrAddTail(p);	//	add this

    Instruction * pnew = new Instruction(i_Writepointto);    //new_INSTR
    pnew->setVars(tem1,tem2);// the pointer,the value
    //Right i_Writepointto, yes var_r1 is a pointer, var_r2 is the value of
    Q->InstrAddTail(pnew);

}

extern int g_newtemno;
void	CodeList_Maker::new_temp(VAR* pvar)
{
    pvar->type = v_Tem;
    pvar->temno = g_newtemno;
    pvar->opsize = BIT32_is_4;	//	temp var always uint32_t
    g_newtemno += 2;
}
