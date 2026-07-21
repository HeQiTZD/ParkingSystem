#ifndef DBCONNECTIONPOOL_H
#define DBCONNECTIONPOOL_H
#include <QSqlDatabase>
#include <QHash>
#include <QMutex>
#include "dbconfig.h"
class DbConnectionPool
{
public:
    static DbConnectionPool& instance();

    void setConfig(const DbConfig &config);

    QSqlDatabase connection();

    void closeThreadConnection();

    void closeAll();
private:
    DbConnectionPool();
    ~DbConnectionPool();
    DbConnectionPool(const DbConnectionPool&) = delete;
    DbConnectionPool& operator=(const DbConnectionPool&) = delete;
    QSqlDatabase createConnectionForCurrentThread();
    QMutex m_mutex;
    QHash<Qt::HANDLE, QSqlDatabase> m_conns;
    DbConfig m_config;
    bool m_configured = false;
};
#endif