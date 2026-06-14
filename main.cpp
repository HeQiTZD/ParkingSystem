#include "initfile.h"
#include "UI/Login/logindialog.h"
#include "UI/MainWindow/mainwindow.h"
#include "UI/ConfigInit/configinitdialog.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 加载全局样式
    QFile styleFile(":/styles/global.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        app.setStyleSheet(style);
        styleFile.close();
    }

    // // 创建InitFile实例检查配置文件
    // InitFile initfile;

    // // 检查配置文件是否存在
    // if(initfile.isConfigExists()){
    //     LoginDialog loginDialog;
    //     if(loginDialog.exec() == QDialog::Accepted){
    //         MainWindow mainWindow;
    //         mainWindow.show();
    //         return app.exec();
    //     }
    // }else{
    //     ConfigInitDialog configDialog;
    //     if(configDialog.exec() == QDialog::Accepted){
    //         LoginDialog loginDialog;
    //         if(loginDialog.exec() == QDialog::Accepted){
    //             MainWindow mainWindow;
    //             mainWindow.show();
    //             return app.exec();
    //         }
    //     }else{
    //         return 0;
    //     }
    // }

    LoginDialog loginDialog;
    loginDialog.exec();

    return 0;
}
