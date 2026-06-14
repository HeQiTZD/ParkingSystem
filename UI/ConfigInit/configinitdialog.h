#ifndef CONFIGINITDIALOG_H
#define CONFIGINITDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigInitDialog;
}

class ConfigInitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigInitDialog(QWidget *parent = nullptr);
    ~ConfigInitDialog();

private slots:
    void on_btnSubmit_clicked();
    void on_btnCancel_clicked();

private:
    Ui::ConfigInitDialog *ui;
    bool validateInputs();
    void saveConfig();
};

#endif // CONFIGINITDIALOG_H