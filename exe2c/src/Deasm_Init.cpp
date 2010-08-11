// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//#include "stdafx.h"
#include	"CISC.h"

intptr_t	g_ea2ptr = 0;
// Because the file transferred to a different address and virtual address, so remember this difference
// Afterward the main program uses only the offset to access, regardless of the actual buffer
void	Disassembler_Init_offset(BYTE * code_buf, ea_t code_offset)
{
    g_ea2ptr = (intptr_t)code_buf - code_offset;
}

BYTE *	ea2ptr(ea_t pos)
{
    return (BYTE *)(g_ea2ptr+pos);
}
ea_t ptr2ea(void* p)
{
    BYTE * p1 = ea2ptr(0);
    return (BYTE *)p - p1;
}

BYTE	Peek_B(ea_t pos)
{
    BYTE * p = ea2ptr(pos);
    return *p;
}
WORD	Peek_W(ea_t pos)
{
    BYTE * p = ea2ptr(pos);
    return *(WORD *)p;
}
uint32_t	Peek_D(ea_t pos)
{
    BYTE * p = ea2ptr(pos);
    return *(uint32_t *)p;
}

bool	XCPUCODE::IsJxx()
{
	BYTE opcode = this->opcode;
	switch (opcode)
	{
	case	C_JO:
	case	C_JNO:
	case	C_JB:
	case	C_JNB:
	case	C_JZ:
	case	C_JNZ:
	case	C_JNA:
	case	C_JA:
	case	C_JS:
	case	C_JNS:
	case	C_JP:
	case	C_JNP:
	case	C_JL:
	case	C_JNL:
	case	C_JLE:
	case	C_JNLE:

	case	C_JCASE:
		return true;

	default:
		return false;
	}
}
bool	XCPUCODE::IsJmpNear()
{
	if (this->opcode == C_JMP && this->op[0].mode == OP_Near)
		return true;
	return false;
}
// To make a disassembly, pos auto-increment, the results remain xcpu
BYTE	 CDisasm::Disasm_OneCode(ea_t &pos)
{
    st_IDA_OUT idaout;
    uint32_t n = this->Disassembler_X(ea2ptr(pos), pos, &idaout);

	pos += n;
	return (BYTE)n;
}
