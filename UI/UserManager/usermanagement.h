#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include <QWidget>

namespace Ui {
class UserManagement;
}

class DatabaseManager;

class UserManagement : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagement(QWidget *parent = nullptr, DatabaseManager *db = nullptr);
    ~UserManagement();

private:
    Ui::UserManagement *ui;
    DatabaseManager *m_db = nullptr;
};

#endif // USERMANAGEMENT_H
