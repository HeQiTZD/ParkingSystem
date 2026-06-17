#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

public slots:
    void on_closeButton_clicked();
    void on_miniButton_clicked();
    
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
};

#endif // LOGINDIALOG_H
