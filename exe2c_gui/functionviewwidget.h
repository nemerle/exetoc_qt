#ifndef FUNCTIONVIEWWIDGET_H
#define FUNCTIONVIEWWIDGET_H

#include <QWidget>
#include <CXmlPrt.h>
namespace Ui {
    class FunctionViewWidget;
}
class FunctionViewWidget : public QWidget,public I_XmlOut
{
    Q_OBJECT

public:
    explicit FunctionViewWidget(QWidget *parent = 0);
    ~FunctionViewWidget();
    void  prtt(const char * s);
    void  prtt(const std::string &s);
    void  XMLbegin(enum XMLTYPE xmltype, void * p);
    void  XMLend(enum XMLTYPE xmltype);
private:
    Ui::FunctionViewWidget *ui;
    QString collected_text;
};

#endif // FUNCTIONVIEWWIDGET_H
