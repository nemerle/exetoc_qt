#include <QDebug>
#include <QtCore>
#include "functionviewwidget.h"
#include "ui_functionviewwidget.h"
#include "XMLTYPE.h"
FunctionViewWidget::FunctionViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FunctionViewWidget)
{
    ui->setupUi(this);
    //ui->label->setTextFormat(Qt::RichText);
}

FunctionViewWidget::~FunctionViewWidget()
{
    delete ui;
}

void FunctionViewWidget::prtt(const char *s)
{
    collected_text+=s;
    //collected_text+="<br>";
}
void FunctionViewWidget::prtt(const std::string &s)
{
    collected_text+=s.c_str();
    //collected_text+="<br>";
}
void FunctionViewWidget::XMLbegin(XMLTYPE xmltype, void *p)
{
    QColor col= XmlType_2_Color(xmltype);
    switch(xmltype)
    {
        case XT_Function:
            collected_text+="<body style='color: #FFFFFF; background-color: #000000'>";
            break;
        case XT_FuncName:
        case XT_Symbol:
        case XT_Keyword:
        case XT_DataType:
        case XT_Number:
        case XT_AsmOffset:
        case XT_AsmLabel:
            collected_text+="<font color='"+col.name()+"'>";
            break;
        default:
            qDebug()<<"Xml type:"<<xmltype;
    }
}
void FunctionViewWidget::XMLend(XMLTYPE xmltype)
{
    switch(xmltype)
    {
        case XT_Function:
        {
            collected_text+="</body>";
            // TODO: What about attributes with spaces?
            collected_text.replace("  ", "&nbsp;&nbsp;");
            QFile res("result.html");
            res.open(QFile::WriteOnly);
            res.write(collected_text.toUtf8());
            res.close();
            collected_text.replace(QChar('\n'),"<br>");
            ui->textEdit->setHtml(collected_text);
            collected_text.clear();
            break;
        }
        case XT_FuncName:
        case XT_Symbol:
        case XT_Keyword:
        case XT_DataType:
        case XT_Number:
        case XT_AsmOffset:
        case XT_AsmLabel:
            collected_text+="</font>";
            break;
        default:
            qDebug()<<"Xml end:"<<xmltype;
    }
}
