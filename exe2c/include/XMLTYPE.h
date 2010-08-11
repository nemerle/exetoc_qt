// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	XmlType.h

#ifndef	XmlType_H
#define	XmlType_H
#include <qcolor.h>
enum XMLTYPE
{
	XT_invalid = 0,
	XT_blank,
	XT_Symbol,
	XT_Function,
	XT_Keyword,		//Keywords, such as struct, union, for, while
	XT_Class,		//Is a class or struct or union name
	XT_K1,			//{} []
	XT_Comment,		//Comments
	XT_DataType,	//
	XT_Number,		//
	XT_AsmStack,	//stack values
	XT_AsmOffset,	//seg:offset
	XT_AsmLabel,	//label name
	XT_FuncName,
};
struct tColorPair
{
        QColor color1;
        QColor color2;
};
extern tColorPair tbl_color[];

QColor XmlType_2_Color(XMLTYPE xmltype);

void XML_Clicked(XMLTYPE xmltype, void * p);

#endif	//	XmlType_H
