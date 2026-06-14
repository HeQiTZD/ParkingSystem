#ifndef MANAGEPAGE_H
#define MANAGEPAGE_H

#include <QWidget>

namespace Ui {
class ManagePage;
}

class ManagePage : public QWidget
{
    Q_OBJECT

public:
    explicit ManagePage(QWidget *parent = nullptr);
    ~ManagePage();

private slots:
    void on_btnAdd_clicked();
    void on_btnDelete_clicked();
    void on_btnModify_clicked();

private:
    Ui::ManagePage *ui;
    void loadUserData();
};

#endif // MANAGEPAGE_H
