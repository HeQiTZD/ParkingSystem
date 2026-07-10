#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

#include "src/database/databasemanager.h"
#include "src/utils/messageType.h"
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr, DatabaseManager* m_db = nullptr);
    ~LoginDialog();

    // 获取登录成功后的用户角色
    QString getUserRole() const { return userRole; }

public slots:
    void on_closeButton_clicked();
    void on_miniButton_clicked();
    void onLoginButton();
    void onRegisterButton();

    // 错误消息接收槽函数
    void onMessageBox(MessageType::Type type, const QString &msg, const QString &title = QString());
    
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;//Qt 框架在需要重绘窗口时自动调用

private:
    Ui::LoginDialog *ui;
    QPoint m_dragPos;
    bool m_dragging = false;
    QRect m_dragArea;

    DatabaseManager *m_dbManager;
    QString userRole;
};

#endif // LOGINDIALOG_H
