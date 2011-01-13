#include <QtGui>
#include "exe2c_mainwindow.h"
#include "ui_exe2c_gui.h"
#include "exe2c_interface.h"
#include "exe2c.h"
#include "functionviewwidget.h"
#include "functionlistdockwidget.h"
I_EXE2C* g_EXE2C = NULL;
extern bool exe2c_Init();
Exe2C_MainWindow::Exe2C_MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Exe2C_MainWindow)
{
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(new QLabel("Test"));
    bool init_result=exe2c_Init();
    assert(init_result);
    g_EXE2C = new Exe2c();
    ((Exe2c *)g_EXE2C)->BaseInit();
    g_EXE2C->Init(this);
    m_last_diplay=g_EXE2C->GetFirstFuncHandle();
    m_functionlist_widget=new FunctionListDockWidget(this);
    m_functionlist_widget->setWindowTitle(QApplication::tr("Function list"));
    connect(m_functionlist_widget,SIGNAL(displayRequested()),
            SLOT(displayCurrentFunction()));
    // we are beeing signalled when display is requested
    connect(this,SIGNAL(functionListChanged()),
            m_functionlist_widget->model(),SLOT(updateFunctionList()));
    this->addDockWidget(Qt::RightDockWidgetArea,m_functionlist_widget);

    m_asm_view = new FunctionViewWidget(this);
    m_asm_view->setWindowTitle(QApplication::tr("Assembly listing"));
    ui->mdiArea->addSubWindow(m_asm_view);
    //m_internal_view = new FunctionViewWidget;
    //m_internal_view->setWindowTitle(QApplication::tr("Internal listing"));
    //ui->mdiArea->addSubWindow(m_internal_view);
    m_c_view = new FunctionViewWidget;
    m_c_view->setWindowTitle(QApplication::tr("Decompiled"));
    ui->mdiArea->addSubWindow(m_c_view);
}

Exe2C_MainWindow::~Exe2C_MainWindow()
{
    delete ui;
}

void Exe2C_MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
void Exe2C_MainWindow::onOptim()
{
    g_EXE2C->analysis_Once();
    emit functionListChanged();
    if(m_last_diplay==g_EXE2C->GetCurFuncHandle())
    {
        displayCurrentFunction();
    }
}
void Exe2C_MainWindow::onOptim10()
{
    for(int i=0; i<10; i++)
        g_EXE2C->analysis_Once();
	emit functionListChanged();
	if(m_last_diplay==g_EXE2C->GetCurFuncHandle())
	{
		displayCurrentFunction();
	}
}
void Exe2C_MainWindow::onOpenFile_Action()
{
    QFileDialog dlg;
    QString name=dlg.getOpenFileName(0,"Select Win32 executable",".","*.exe");
    //if (!CDocument::OnOpenDocument(lpszPathName))
    //        return FALSE;
    g_EXE2C->exe2c_main(name.toStdString());
    //bool m_bSucc = m_xTextBuffer.LoadFromFile(lpszPathName);
    emit functionListChanged();
}

void Exe2C_MainWindow::displayCurrentFunction()
{
    if(m_last_diplay!=g_EXE2C->GetCurFuncHandle())
        m_last_diplay=g_EXE2C->GetCurFuncHandle();
    g_EXE2C->prtout_asm(m_asm_view);
    //g_EXE2C->prtout_itn(m_internal_view);
    g_EXE2C->prtout_cpp(m_c_view);
}
void Exe2C_MainWindow::prt_log(const char *v)
{
    qDebug()<<v;
}
