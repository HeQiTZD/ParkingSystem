#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "src/database/databasemanager.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr, DatabaseManager *db = nullptr);
    ~RegisterDialog();

private slots:
    void on_btnRegister_clicked();
    void on_btnBack_clicked();

private:
    Ui::RegisterDialog *ui;
    bool validateInputs();

private:
    DatabaseManager *m_db;
};

#endif // REGISTERDIALOG_H
