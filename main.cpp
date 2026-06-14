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

    // 检查是否需要初始化配置
    // TODO: 检查配置文件是否存在

    // 显示登录对话框
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        MainWindow mainWindow;
        mainWindow.show();
        return app.exec();
    }

    return 0;
}
