#include "dbconnectionpool.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>
#include <QDebug>

DbConnectionPool& DbConnectionPool::instance()
{
    static DbConnectionPool pool;  // Meyer's
    return pool;
}

DbConnectionPool::DbConnectionPool() {}

DbConnectionPool::~DbConnectionPool()
{
    closeAll();
}

void DbConnectionPool::setConfig(const DbConfig &config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
    m_configured = true;
}

QSqlDatabase DbConnectionPool::connection()
{
    QMutexLocker locker(&m_mutex);
    if(!m_configured){
        qWarning() << "DbConnectionPool: 配置未设置, 返回无效连接";
        return QSqlDatabase();  // 默认构造的无效连接
    }

    Qt::HANDLE tid = QThread::currentThreadId();
    auto it = m_conns.find(tid);
    if(it != m_conns.end() && it.value().isOpen()){
        return it.value();
    }

    // 不在锁内创建连接(QSqlDatabase::addDatabase 不重入)
    locker.unlock();
    QSqlDatabase conn = createConnectionForCurrentThread();
    return conn;
}

QSqlDatabase DbConnectionPool::createConnectionForCurrentThread()
{
    // 唯一连接名, 与线程 ID 绑定
    QString connName = QString("conn_%1")
        .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    // 若连接名已存在(残留未清理), 先移除
    if(QSqlDatabase::contains(connName)){
        QSqlDatabase existing = QSqlDatabase::database(connName, false);
        if(existing.isOpen()){
            QMutexLocker locker(&m_mutex);
            m_conns[QThread::currentThreadId()] = existing;
            return existing;
        }
        QSqlDatabase::removeDatabase(connName);
    }

    QSqlDatabase conn = QSqlDatabase::addDatabase(m_config.driverName, connName);
    conn.setHostName(m_config.host);
    conn.setPort(m_config.port);
    conn.setDatabaseName(m_config.dbName);
    conn.setUserName(m_config.username);
    conn.setPassword(m_config.password);

    if(!conn.open()){
        qCritical() << "DbConnectionPool: 线程" << QThread::currentThreadId()
                    << "连接失败:" << conn.lastError().text();
    }

    QMutexLocker locker(&m_mutex);
    m_conns[QThread::currentThreadId()] = conn;
    return conn;
}

void DbConnectionPool::closeThreadConnection()
{
    QMutexLocker locker(&m_mutex);
    Qt::HANDLE tid = QThread::currentThreadId();
    auto it = m_conns.find(tid);
    if(it != m_conns.end()){
        QString connName = it.value().connectionName();
        it.value().close();
        m_conns.erase(it);
        // 在无引用时 removeDatabase
        QSqlDatabase::removeDatabase(connName);
    }
}

void DbConnectionPool::closeAll()
{
    QMutexLocker locker(&m_mutex);
    QStringList connNames;
    for(auto it = m_conns.begin(); it != m_conns.end(); ++it){
        connNames << it.value().connectionName();
        it.value().close();
    }
    m_conns.clear();
    locker.unlock();
    for(const QString &name : connNames){
        QSqlDatabase::removeDatabase(name);
    }
}
