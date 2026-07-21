#ifndef DBCONFIG_H
#define DBCONFIG_H
#include <QString>
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
#endif