// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#ifndef DASM__H
#define DASM__H

#include <string>
#include <stdint.h>
#include <sstream>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCExpr.h>

#define BIT16	0
#define	BIT32	1
class QString;
enum enum_Register
{
    _EAX_ = 0,
    _ECX_ = 1,
    _EDX_ = 2,
    _EBX_ = 3,
    _ESP_ = 4,
    _EBP_ = 5,
    _ESI_ = 6,
    _EDI_ = 7,
    _AX_ = 0,
    _CX_ = 1,
    _DX_ = 2,
    _BX_ = 3,
    _SP_ = 4,
    _BP_ = 5,
    _SI_ = 6,
    _DI_ = 7,
    _AL_ = 0,
    _CL_ = 1,
    _DL_ = 2,
    _BL_ = 3,
    _AH_ = 4,
    _CH_ = 5,
    _DH_ = 6,
    _BH_ = 7,
    _NOREG_ = 100
};
enum enum_SegmentReg
{
    _ES_ = 0,
    _CS_ = 1,
    _SS_ = 2,
    _DS_ = 3,
    _FS_ = 4,
    _GS_ = 5,
    _NOSEG_ = 100
};

enum enum_ErrorKind
{
    ERR_NOERROR = 0,
    ERR_INVALIDCODE = 1,
};

enum enum_InsnPrefix
{
    NOPREFIX = 0,
    REPZ_PREFIX = 1,
    REPNZ_PREFIX = 2
};
enum enum_OperAccess
{
    OPER_UNKNOWN = 0,
    OPER_READ = 1,
    OPER_WRITE = 2,
    OPER_ACCESS = 3
};
enum OP_TYPE
{
    OP_Invalid	=	0,
    OP_Address	=	1,
    OP_Register	=	2,
    OP_Segment	=	3,
    OP_Immed	=	4,
    OP_Near		=	5,
    OP_Far		=	6

};
// The list of the types of Opcode
enum OPCODETYPE
{
    C_ERROR,		// It'a invalid instruction
    C_GROUP,		// It'a group. If this, InstName specifies the group table.
    C_0FH,			// It's 0FH instruction. If this, InstName specifies the 0FH table.
    C_SEGPREFIX,	// If this, Opdata1 specifies the index of segment prefix
    C_OPRSIZE,		// Operand size change.
    C_ADRSIZE,		// Address size change.
    C_LOCK,			// LOCK prefix
    C_REPZ,			// REPZ prefix
    C_REPNZ,		// REPNZ prefix
    // Ok, prefix define completed.

    C_ADD,
    C_OR,
    C_ADC,
    C_SBB,
    C_AND,
    C_SUB,
    C_XOR,
    C_CMP,
    C_DAA,
    C_DAS,
    C_AAA,
    C_AAS,
    C_INC,
    C_DEC,
    C_PUSH,
    C_POP,
    C_PUSHA,
    C_POPA,
    C_BOUND,
    C_ARPL,
    C_IMUL,
    C_INS,
    C_OUTS,
    C_JO,
    C_JNO,
    C_JB,
    C_JNB,
    C_JZ,
    C_JNZ,
    C_JNA,
    C_JA,
    C_JS,
    C_JNS,
    C_JP,
    C_JNP,
    C_JL,
    C_JNL,
    C_JLE,
    C_JNLE,
    C_TEST,
    C_NOP,
    C_XCHG,
    C_MOV,
    C_LEA,
    C_CBW,
    C_CWD,
    C_CALL,
    C_WAIT,
    C_PUSHF,
    C_POPF,
    C_SAHF,
    C_LAHF,
    C_MOVS,
    C_CMPS,
    C_STOS,
    C_LODS,
    C_SCAS,
    C_RET,
    C_LES,
    C_LDS,
    C_ENTER,
    C_LEAVE,
    C_RETF,
    C_INT3,
    C_INT,
    C_INTO,
    C_IRET,
    C_AAM,
    C_AAD,
    C_XLAT,
    C_LOOPNZ,
    C_LOOPZ,
    C_LOOP,
    C_JCXZ,
    C_IN,
    C_OUT,
    C_JMP,
    C_HLT,
    C_CMC,
    C_CLC,
    C_STC,
    C_CLI,
    C_STI,
    C_CLD,
    C_STD,
    C_CALLFAR,
    C_JMPFAR,

    C_ROL,
    C_ROR,
    C_RCL,
    C_RCR,
    C_SHL,
    C_SHR,
    C_SAR,

    C_NOT,
    C_NEG,
    C_MUL,
    C_DIV,
    C_IDIV,
    // One-BYTE opcode table completed.

    C_LAR,
    C_LSL,
    C_LSS,
    C_LFS,
    C_LGS,
    C_MOVZX,
    C_MOVSX,
    C_BT,
    C_BTR,
    C_BTS,
    C_BTC,
    C_BSF,
    C_BSR,
    C_SHLD,
    C_SHRD,
    C_SETO,
    C_SETNO,
    C_SETB,
    C_SETNB,
    C_SETZ,
    C_SETNZ,
    C_SETNA,
    C_SETA,
    C_SETS,
    C_SETNS,
    C_SETP,
    C_SETNP,
    C_SETL,
    C_SETNL,
    C_SETLE,
    C_SETNLE,
    C_CMPXCHG,
    C_XADD,
    C_BSWAP,
    C_CLTS,
    C_CPUID,
    C_WRMSR,
    C_RDTSC,
    C_RDMSR,
    C_RDPMC,
    C_INVD,
    C_WBINVD,
    // Two-BYTE opcode table completed.

    C_JCASE	= 254,
    C_NONE
}   ;

typedef	uint32_t	ea_t;

struct OPERITEM : public llvm::MCOperand
{
    OP_TYPE mode;		//OP_Register, ...
    uint8_t    opersize;	//1:BYTE, 2:WORD, 4:uint32_t, 8:double uint32_t
    union
    {
        struct
        {
            uint8_t    seg_index;	//SegReg Index!!!
            //uint8_t    reg_size;	//2:WORD 4:uint32_t
            uint8_t    base_reg_index;
            uint8_t    off_reg_index;
            uint8_t    off_reg_scale;
            uint32_t   off_value;
        }   addr;		//for OP_Address
        //reg_index -> _ESP_
        struct
        {
            uint32_t   sreg_index;
        }   sreg;		//for OP_Segment
        struct
        {
            ea_t   offset;
        }   nearptr;	//for OP_Near
        struct
        {
            uint32_t   segment;
            uint32_t   offset;
        }   farptr;		//for OP_Far
    };
    bool isRegOp(uint32_t reg_idx); //! returns true if this is a Register operand with requested reg_index
    bool isStaticOffset(); //! return true if this is OP_Address and it does not depend on other regs e.x. [0x11212]
    static OPERITEM createReg(int reg_idx,int width); //! create an Register operand of given width
};

struct XCPUCODE  : public llvm::MCInst
{
    OPCODETYPE  opcode;		//	C_MOV...
    uint8_t     lockflag;	// for LOCK prefix
    uint8_t     repeatflag;	// for REPZ/REPNZ prefix
    OPERITEM    op[3];
    bool    IsJmpMemIndexed() const;
    bool	IsJxx() const;
    bool	IsJmpNear() const;
    bool    IsCallNear() const;
    bool    IsCalculatedJmp() const;
};

// The list of the types of Opdata1, Opdata2, Opdata3
enum OPDATATYPE
{
    D_NONE,			// No any types
    D_EB,           // A ModR/M bytes, specifies the operand size.
    D_EW,
    D_EV,
    D_GB,D_GW,D_GV,	// The reg field of the ModR/M BYTE selects a normal register.
    D_IB,D_IW,D_IV,	// Immediate data.
    D_SB,			// Signed Immediate data.
    D_SW,			// The reg field of the ModR/M BYTE selects a segment register.
    D_MV,D_MP,D_MA,	// The ModR/M BYTE may refer only to memory.
    D_OB,D_OV,		// The offset of the operand is coded as a WORD or d-WORD ( no ModR/M )
    D_JB,D_JV,D_JP,	// The instruction contains a relative offset to be added to EIP.
    D_RD,			// The mod field of the ModR/M BYTE may refer only to a general register.
    D_CD,			// The reg field of the ModR/M BYTE selects a control register.
    D_DD,			// The reg field of the ModR/M BYTE selects a debug register.

    D_1,			// Only used for ( Group2 SHL/SHR... instruction )

    D_AL,			// Specifying AL register
    D_CL,			// Specifying CL register
    D_DL,			// Specifying DL register
    D_BL,			// Specifying BL register
    D_AH,			// Specifying AH register
    D_CH,			// Specifying CH register
    D_DH,			// Specifying DH register
    D_BH,			// Specifying BH register

    D_AX,			// Specifying AX register
    D_CX,			// Specifying CX register
    D_DX,			// Specifying DX register
    D_BX,			// Specifying BX register
    D_SP,			// Specifying SP register
    D_BP,			// Specifying BP register
    D_SI,			// Specifying SI register
    D_DI,			// Specifying DI register

    D_AXV,			// Specifying eAX register
    D_CXV,			// Specifying eCX register
    D_DXV,			// Specifying eDX register
    D_BXV,			// Specifying eBX register
    D_SPV,			// Specifying eSP register
    D_BPV,			// Specifying eBP register
    D_SIV,			// Specifying eSI register
    D_DIV,			// Specifying eDI register

    D_ES,			// Specifying ES register
    D_CS,			// Specifying CS register
    D_SS,			// Specifying SS register
    D_DS,			// Specifying DS register
    D_FS,			// Specifying FS register
    D_GS,			// Specifying GS register

    D_V,			// Used for PUSHA/POPA, PUSHF/POPF, SHAF/LAHF
    D_XB,D_XV		// Used for ( MOVS, LODS, OUTS, ... )
};
//Define a structure similar to the IDA of the ASM output
struct INSTRUCTION;
struct st_IDA_OUT
{
    std::string LockName;
    std::string RepName;
    std::string CmdStr;
    std::string Par1Ptr;
    std::string Par1SegPrefix;
    std::string Par1Str;
    std::string Par2Ptr;
    std::string Par2SegPrefix;
    std::string Par2Str;
    std::string Par3Str;

    ea_t linear;

    st_IDA_OUT() : linear(0)
    {
    }
    bool has_param2()
    {
        return !(Par2Ptr.empty() && !Par2SegPrefix.empty() && !Par2Str.empty());
    }
    bool has_param3()
    {
        return !Par3Str.empty();
    }
    void output(QString &buf);
};
enum enum_SizeKind
{
    SIZE_B = 1,
    SIZE_W = 2,
    SIZE_V = 3,
    SIZE_D = 4,
    SIZE_P = 5,
    SIZE_A = 6
};

class Disasm
{
    st_IDA_OUT* m_idaout;

    uint32_t	U_ErrorCode;
    unsigned char *	UasmCode;
    uint32_t	BaseAddress;
    //	!!!Initialized by parameter!!!

    uint32_t	CodeCount;       /*  It's code's number.   */

    uint32_t	OpSize;
    uint32_t	AdrSize;


    uint32_t Global_OFFSET(char * outbuf,unsigned char * codebuf,OPERITEM *op);
    uint32_t Global_MEMORY(char * outbuf,unsigned char * codebuf,OPERITEM *op);
    uint32_t Global_MODRM(char * outbuf,unsigned char * codebuf,OPERITEM *op);
    uint8_t	GetByte();
    WORD	GetWord();
    uint32_t	GetDWord();
    uint8_t	GetByteEx();
    WORD	GetWordEx();
    uint32_t	GetDWordEx();
    uint32_t Global_NEARPTR(char * outbuf,unsigned char * codebuf,OPERITEM *op);
    uint32_t Global_FARPTR(char * outbuf,unsigned char * codebuf,OPERITEM *op);
    void	OpSizePrefix();
    void	AdrSizePrefix();
    uint8_t Global_GetSize(enum_SizeKind srcsize);
    uint32_t	ProcessOpdata(uint32_t opdata,OPERITEM *op,char * outbuf,uint32_t codepos);
    void	SetError(uint32_t errcode);
    void	DisassemblerOne();
    void	ProcessGroup(const INSTRUCTION *pG,const INSTRUCTION &inst);
    void	ProcessInstruction(	uint32_t	opcode,
                                const char *	instname,
                                uint32_t	opdata1,
                                uint32_t	opdata2,
                                uint32_t	opdata3);
    std::string	ProcessSegPrefix(OPERITEM *op);

public:
    uint8_t	Disasm_OneCode(ea_t pos);
    XCPUCODE* get_xcpu();
    uint32_t   Disassembler_X(uint8_t * codebuf, uint32_t eip, st_IDA_OUT* idaout);
};

#endif //DASM__H
