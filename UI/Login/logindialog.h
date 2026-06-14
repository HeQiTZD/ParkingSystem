#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QPoint>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_btnMinimize_clicked();
    void on_btnClose_clicked();

private:
    Ui::LoginDialog *ui;
    bool m_dragging;
    QPoint m_dragPosition;

    void setupWindowFlags();
    void setupShadow();
    void setupTitleBar();
    void updateBrandPanelBackground();
};

#endif // LOGINDIALOG_H
