#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include <QWidget>
#include <QList>
#include <QVariant>

namespace Ui {
class UserManagement;
}

class DatabaseManager;
class QTableWidget;
class QLabel;
class PaginationWidget;

class UserManagement : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagement(QWidget *parent = nullptr, DatabaseManager *db = nullptr);
    ~UserManagement();

private:
    void setupTable();
    void populateTable();
    void showEmptyState();
    void hideEmptyState();
    QString formatPhone(const QString &phone);

private slots:
    void onSearchClicked();
    void onPageChanged(int page);

private:
    Ui::UserManagement *ui;
    QTableWidget *m_tableWidget = nullptr;
    PaginationWidget *m_pagination = nullptr;
    QWidget *m_emptyStateWidget = nullptr;
    QLabel *m_recordCountLabel = nullptr;
    DatabaseManager *m_db = nullptr;
    QList<QVariantList> m_allData;
};

#endif // USERMANAGEMENT_H
