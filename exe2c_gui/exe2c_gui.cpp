#include "exe2c_gui.h"

#include <QApplication>
#include "exe2c_mainwindow.h"
int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Exe2C_MainWindow frame;
    frame.show();

    return app.exec();
}
