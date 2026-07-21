#ifndef USERSERVICE_H
#define USERSERVICE_H
#include <QObject>
#include <QList>
#include <QVariantList>
#include "src/utils/messageType.h"
class DatabaseManager;
class UserService : public QObject
{
    Q_OBJECT
public:
    explicit UserService(DatabaseManager& db, QObject* parent = nullptr);
    bool authenticate(const QString& username, const QString& password,
                      QString& userRole, QString& errMsg);
    bool registerUser(const QString& username, const QString& password,
                      const QString& name, const QString& phone,
                      QString& errMsg);
    bool addUser(const QString& username, const QString& password,
                 const QString& telephone, const QString& truename,
                 const QString& role, QString& errMsg);
    bool updateUser(int id, const QString& username, const QString& telephone,
                    const QString& truename, const QString& role, QString& errMsg);
    bool deleteUser(int id, QString& errMsg);
    QList<QVariantList> listUsers(const QString& keyword = QString());
signals:
    void userDataChanged();
    void error(MessageType::Type type, const QString& msg);
private:
    DatabaseManager& m_db;
};
#endif