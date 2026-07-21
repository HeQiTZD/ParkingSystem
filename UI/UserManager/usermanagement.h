#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QList>
#include <QVariantList>
namespace Ui {
class UserManagement;
}
class UserService;
class PaginationWidget;
class UserManagement : public QWidget
{
    Q_OBJECT
public:
    explicit UserManagement(QWidget *parent = nullptr, UserService *userSvc = nullptr);
    ~UserManagement();
private slots:
    void onSearchClicked();
    void onPageChanged(int page);
    void onRegisterClicked();
    void onEditUser(const QVariantList &userData);
    void onDeleteUser(const QVariantList &userData);
private:
    void setupTable();
    void populateTable();
    Ui::UserManagement *ui;
    UserService *m_userSvc = nullptr;

    QTableWidget *m_tableWidget = nullptr;

    PaginationWidget *m_pagination = nullptr;
    QComboBox *m_pageSizeCombo = nullptr;

    QList<QVariantList> m_allUsers;
};
#endif