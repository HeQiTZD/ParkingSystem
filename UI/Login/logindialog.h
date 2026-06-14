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

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::LoginDialog *ui;
    void updateBrandPanelBackground();
};

#endif // LOGINDIALOG_H
