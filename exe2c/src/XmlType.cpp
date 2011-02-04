// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	XmlType.cpp

#include <QDebug>
#include	"00000.h"
#include "XMLTYPE.h"

/*
enum XMLTYPE
{
        XT_invalid = 0,
        XT_blank,
        XT_Symbol,
        XT_Function,
        XT_Keyword,		//Keywords, such as struct, union, for, while
        XT_Class,		//Is a class or struct or union name
        XT_K1,			//{} []
        XT_Comment,		//
        XT_DataType,	//
        XT_Number,		//
        XT_AsmStack,	//Stack value
        XT_AsmOffset,	//Display, seg: offset
        XT_AsmLabel,	//label name

};
*/

tColorPair tbl_color[] =
{
        {QColor(255,255,255),	QColor(0,0,0)},			//0
        {QColor(255,255,255),	QColor(0,0,0)},			//blank
        {QColor(155,255,25),	QColor(0,0,0)},		//symbol
        {QColor(255,0,255),	QColor(0,0,0)},		//function
        {QColor(255,255,0),	QColor(0,0,0)},		//keyword
        {QColor(255,255,0),	QColor(0,0,0)},		//Class
        {QColor(163,70,255),	QColor(0,0,0)},		//K1 brace
        {QColor(0,245,255),	QColor(0,0,0)},		//comment
        {QColor(100,222,192),	QColor(0,0,0)},		//datatype
        {QColor(0,255,0),		QColor(0,0,0)},		//number
        {QColor(0,70,255),		QColor(0,0,0)},		//AsmStack
        {QColor(70,180,70),	QColor(0,0,0)},		//AsmOffset
        {QColor(255,180,70),	QColor(0,0,0)},		//AsmLabel


        {QColor(192,192,192),	QColor(0,0,0)},			//1
        {QColor(0,	255,0),	QColor(0,0,0)},			//2
        {QColor(0,	0,	255),	QColor(0,0,0)},		//3
        {QColor(255,255,0),	QColor(0,0,0)},		//preprocessor
        {QColor(0,	255,0),	QColor(0,0,0)},			//string
        {QColor(255,0,0),	QColor(0,0,0)},			//red
        {QColor(0,	0,	255),	QColor(0,0,0)},		//symbol name in ASM out

};

QColor XmlType_2_Color(XMLTYPE xmltype)
{
    switch (xmltype)
    {
    case XT_invalid  : return QColor(255,255,255);
    case XT_blank    : return QColor(255,255,255);
    case XT_Symbol   : return QColor(57,109,165);
    case XT_Function : return QColor(255,255,255);
    case XT_Keyword  : return QColor(255,255,0);
    case XT_Class    : return QColor(255,255,0);
    case XT_K1       : return QColor(163,70,255);
    case XT_Comment  : return QColor(0,245,255);
    case XT_DataType : return QColor(100,222,192);
    case XT_Number   : return QColor(0,255,0);
    case XT_AsmStack : return QColor(0,70,255);
    case XT_AsmOffset: return QColor(70,180,70);
    case XT_AsmLabel : return QColor(255,180,70);
    case XT_FuncName : return QColor(255,0,255);
    }
    return QColor(255,255,255);
}

size_t PopUpKeys(const char * msgtbl[])
{
    return 0;
}

const char * tbl_Key_Function[] =
        {
    "N: rename",
    "H: help",
    NULL
};
void XML_Clicked(XMLTYPE xmltype, void * p)
{
    size_t key;
    switch (xmltype)
    {
    case XT_Function:
        key = PopUpKeys(tbl_Key_Function);
        break;
    default:
        qDebug()<<"Unhandled xml click";
    }
}

