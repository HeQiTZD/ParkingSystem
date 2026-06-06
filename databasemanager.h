#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    //连接数据库，地址，端口，数据库名称，用户名，密码
    bool connectDatabase(const QString &host, int port,
                         const QString &dbName,
                         const QString &username,
                         const QString &password);

    //断开数据库连接
    void disconnectDatabase();

    //检查连接状态
    bool isConnected() const;

    //执行查询
    QSqlQuery executeQuery(const QString &sql);

    //验证用户登录
    bool validateUser(const QString &username, const QString &password, QString &userRole);
    //查询用户是否已存在
    bool isUsernameExists(const QString &username);
    //用户注册
    bool registerUser(const QString &username, const QString &password, const QString &phone);

signals:
    void connectionStatusChanged(bool connected);

private:
    QSqlDatabase db;
    bool connected;
};

#endif // DATABASEMANAGER_H
