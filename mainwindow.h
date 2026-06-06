#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>//主窗口基类
#include "databasemanager.h"
#include "registerwindow.h"
#include "utils.h"
#include "initfile.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(InitFile *config, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_loginButton_clicked();//登录按钮点击事件
    void on_registerButton_clicked();//注册按钮点击事件
    void on_DatabaseConnectionChanged(bool connected);//数据库连接状态变化
    void onRegisterSuccess();//注册成功处理
    void onReturnToLogin();//返回登录界面处理

private:
    Ui::MainWindow *ui;
    DatabaseManager *dbManager;
    RegisterWindow *registerWindow = nullptr;
    InitFile *config;

    void initDatabase();//初始化数据库连接
    void updateConnectionStatus();//更新连接状态显示
};
#endif // MAINWINDOW_H
