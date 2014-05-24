#ifndef EXE2C_MAINWINDOW_H
#define EXE2C_MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractTableModel>
#include <QVariant>
#include <vector>
#include "exe2c.h"
class FunctionViewWidget;
class FunctionListDockWidget;
namespace Ui {
class Exe2C_MainWindow;
}

class Exe2C_MainWindow : public QMainWindow,public I_E2COUT {
    Q_OBJECT
public:
    explicit Exe2C_MainWindow(QWidget *parent = 0);
    ~Exe2C_MainWindow();
    void prt_log(const char * str);
public slots:
    void onOptim();
    void onOptim10();
    void onOpenFile_Action();
    void displayCurrentFunction();
signals:
    void functionListChanged();
protected:
    void changeEvent(QEvent *e);
private slots:
    void on_actionExit_triggered();

private:
    FunctionViewWidget *m_asm_view;
    //  FunctionViewWidget *m_internal_view;
    FunctionViewWidget *m_c_view;
    FunctionListDockWidget *m_functionlist_widget;
    Ui::Exe2C_MainWindow *ui;
    FUNC_LIST::iterator m_last_diplay;
};

#endif // EXE2C_MAINWINDOW_H
