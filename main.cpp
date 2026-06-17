#include "initfile.h"
#include "UI/MainWindow/mainwindow.h"
#include "UI/Login/logindialog.h"
#include <QApplication>
#include <QStyleFactory>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LoginDialog loginDialog;
    //loginDialog.setWindowFlags(Qt::FramelessWindowHint);
    loginDialog.exec();

    return app.exec();
}
