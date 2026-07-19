#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QList>
#include <QVariantList>

namespace Ui {
class UserManagement;
}

class DatabaseManager;
class PaginationWidget;

class UserManagement : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagement(QWidget *parent = nullptr, DatabaseManager *db = nullptr);
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
    DatabaseManager *m_db = nullptr;

    // ── 表格 ──
    QTableWidget *m_tableWidget = nullptr;

    // ── 分页 ──
    PaginationWidget *m_pagination = nullptr;
    QLabel *m_recordLabel = nullptr;

    // ── 数据缓存 ──
    QList<QVariantList> m_allUsers;  // 最近一次查询的全量结果
};

#endif // USERMANAGEMENT_H
