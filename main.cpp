#include "initfile.h"
#include "UI/MainWindow/mainwindow.h"
#include "UI/Login/logindialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LoginDialog loginDialog;
    loginDialog.exec();

    return app.exec();
}
