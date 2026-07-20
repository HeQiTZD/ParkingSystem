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
    DatabaseManager *m_userSvc = nullptr;

    // ── 表格 ──
    QTableWidget *m_tableWidget = nullptr;

    // ── 分页 ──
    PaginationWidget *m_pagination = nullptr;
    QComboBox *m_pageSizeCombo = nullptr;

    // ── 数据缓存 ──
    QList<QVariantList> m_allUsers;  // 最近一次查询的全量结果
};

#endif // USERMANAGEMENT_H
