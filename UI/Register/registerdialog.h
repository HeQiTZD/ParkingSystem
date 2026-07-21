#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H
#include <QDialog>
class UserService;
namespace Ui {
class RegisterDialog;
}
class RegisterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RegisterDialog(QWidget *parent = nullptr, UserService *userSvc = nullptr);
    ~RegisterDialog();
    QString getUserName() const;
    QString getPassword() const;
private slots:
    void on_btnRegister_clicked();
    void on_btnBack_clicked();
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    Ui::RegisterDialog *ui;
    bool validateInputs();
private:
    UserService *m_userSvc = nullptr;
    bool m_dragging = false;
    QPoint m_dragPos;
    QRect  m_dragArea;
};
#endif