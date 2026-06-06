#include "mainwindow.h"
#include "initfile.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //初始化配置文件
    InitFile config;
    if(!config.loadConfig()){
        qDebug() << QStringLiteral("配置文件加载失败");
    }

    //将配置传递给MainWindow
    MainWindow *w = new MainWindow(&config); // 传递配置指针
    w->setAttribute(Qt::WA_DeleteOnClose);//关闭窗口时自动释放内存
    w->show();
    return QApplication::exec();
}
