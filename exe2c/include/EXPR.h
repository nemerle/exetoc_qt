// Copyright(C) 1999-2005 LiuTaoTaobookaa@rorsoft.com

// EXPR.h
#ifndef EXPR__H
#define EXPR__H
/*
class EXPR;
typedef	EXPR*	PEXPR;

class EXPR
{
public:
	BYTE    type;		// EXPR_???

	BYTE    len;		// var size in BYTE
	BYTE    no1;
	BYTE    f_G;		// 0: local 1: global

	BYTE	fUserDefineName;	//if it already has a user defined name, then we should not optim it
	BYTE	f_part;		// this expr is part of another, AL to EAX
	BYTE	no2;
	BYTE	no3;

	char	str_type[20];	//string of EXPR type, "uint32_t"


	union
	{
		uint32_t d;
		struct
		{
			uint32_t ESP_off;	//-04 for first push
		} v1;
		struct
		{
			BYTE REG_index;
		} v2;
		struct
		{
			uint32_t MEM_off;
		} v3;
		struct
		{
			//BYTE    base_reg;
			//BYTE    index_reg;
			PEXPR   expr_base;
			PEXPR   expr_index;
			uint32_t   off;
		} reg_ptr;
	};
	//VALUEE   value;
	char    name[20];

	EXPR();		//  CFunc::expr_new() һ EXPR
	~EXPR();

	char * Get_TypeDefStr();	//  EXPR Ķ壬 "unsigned lonuint32_tf
	CList<PEXPR,PEXPR>   EXPR_LIST;
*/
enum VARType
{
	v_Invalid = 0,
	v_Immed,    //
	v_Reg,		//r_
	//v_Stack, ջֱΪڲģԷֿ
	v_Par,		//v_
	v_Var,		//a_
	v_Global,	//g_ ȫֱ
	v_Tem,		//t_ ʱ
	v_Volatile, //ֻ fs:0
};

struct VAR
{
    VARType	type;	//can be v_Reg,v_Stack,v_Global,v_Immed

    UINT	opsize;	//0:void,1:BYTE,2:WORD,4:uint32_t,...
    struct M_t* thevar;
    int part_flag;  //show var thevar
                    //part_flag-1 potential var thevar offset

	union
	{
		uint32_t	reg;	//for v_Reg, 0:eax,1:ah,4:ecx....
		uint32_t	par_off;	//	for v_Par
		uint32_t	var_off;	//  for v_Var
		ea_t	off;	//for v_Global
		BYTE	b;
		WORD	w;
		uint32_t	d;		//for v_Immed
		uint32_t	temno;	//for v_Tem
	};

    static bool IsSame(const VAR* v1,const VAR* v2);
    static int	VarCompare(const VAR* v1,const VAR* v2);
        //	0:	no relationship
        //	1:	same
        //	2:	v1 include v2
        //	3:	v2 include v1
    VAR()
    {
        type = v_Invalid;
        opsize=0;
        thevar = NULL;
        part_flag = 0;
        reg=0;
    }
    bool IfTemVar() const;
    bool IfSameTemVar(const VAR* v2) const;
};


uint32_t	stack2varoff(int32_t stackoff);

#endif // EXPR__H
