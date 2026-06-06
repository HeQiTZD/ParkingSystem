#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class RegisterWindow;
}
QT_END_NAMESPACE

class RegisterWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterWindow(QWidget *parent = nullptr,DatabaseManager *dbManager = nullptr);
    ~RegisterWindow();

signals:
    //注册成功信号
    void registerSuccess();
    //返回登录信号
    void returnToLogin();

private slots:
    void on_registerButton_clicked();
    void on_returnToLogin_clicked();

private:
    Ui::RegisterWindow *ui;
    DatabaseManager *dbManager;

    //验证输入
    bool validateInput();
    //清除输入
    void clearInput();
};

#endif // REGISTERWINDOW_H
