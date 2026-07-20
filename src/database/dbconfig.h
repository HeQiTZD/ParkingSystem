#ifndef DBCONFIG_H
#define DBCONFIG_H

#include <QString>

// 数据库连接配置(纯数据,无连接)
// 用于在 DbConnectionPool 中为每个线程克隆独立的 QSqlDatabase。
struct DbConfig {
    QString driverName = "QMYSQL";
    QString host;
    int     port = 3306;
    QString dbName;
    QString username;
    QString password;

    bool isValid() const {
        return !host.isEmpty() && !dbName.isEmpty();
    }
};

#endif // DBCONFIG_H
