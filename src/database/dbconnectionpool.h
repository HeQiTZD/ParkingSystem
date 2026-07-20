#ifndef DBCONNECTIONPOOL_H
#define DBCONNECTIONPOOL_H

#include <QSqlDatabase>
#include <QHash>
#include <QMutex>
#include "dbconfig.h"

// per-thread QSqlDatabase 连接池。
// Qt 文档明确: 同一 QSqlDatabase 实例跨线程 exec 不安全。
// 本类为每个线程克隆独立连接, 共享同一连接配置。
// Meyer's Singleton, 析构时关闭所有线程连接。
class DbConnectionPool
{
public:
    static DbConnectionPool& instance();

    // 初始化模板配置(主线程调用一次)。
    // 后续各线程调用 connection() 时, 会基于此配置 cloneDatabase。
    void setConfig(const DbConfig &config);

    // 取当前线程的连接;
    // 若不存在则创建新连接, 打开并缓存。
    // 返回的 QSqlDatabase 仅供当前线程使用。
    QSqlDatabase connection();

    // 关闭并移除当前线程的连接。
    // 线程退出前调用, 防止 MySQL 连接泄漏。
    void closeThreadConnection();

    // 关闭所有线程连接(shutdown 时调用)。
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

#endif // DBCONNECTIONPOOL_H
