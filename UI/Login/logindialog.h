#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include <QDialog>
#include "src/utils/messageType.h"
class UserService;
namespace Ui {
class LoginDialog;
}
class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr, UserService* userSvc = nullptr);
    ~LoginDialog();

    QString getUserRole() const { return userRole; }

    void reset();
public slots:
    void on_closeButton_clicked();
    void on_minimizeButton_clicked();
    void onLoginButton();
    void onRegisterButton();

    void onMessageBox(MessageType::Type type, const QString &msg, const QString &title = QString());
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    void saveCredentials();
    void loadCredentials();
private:
    Ui::LoginDialog *ui;
    QPoint m_dragPos;
    bool m_dragging = false;
    QRect m_dragArea;
    UserService *m_userSvc = nullptr;
    QString userRole;
};
#endif