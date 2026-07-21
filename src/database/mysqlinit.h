#ifndef MYSQLINIT_H
#define MYSQLINIT_H
#include <QObject>
class DatabaseManager;
class InitFile;
class MySQLInit : public QObject
{
    Q_OBJECT
public:
    explicit MySQLInit(DatabaseManager* dbManager, InitFile* initFile, QObject* parent = nullptr);
    ~MySQLInit();

    bool initAll();

    bool createUserTable();

    bool createCarTable();

    bool createParkingTable();

    bool initParkingData(const QString &parkingName, int totalSpace, double fee);

    bool initAdminUser();
signals:
    void initProgress(const QString &message, int progress);

    void initFinished(bool success,const QString &message);
private:
    DatabaseManager *m_dbManager;
    InitFile        *m_initFile;

    bool isTableExists(const QString &tableName);

    bool executeSql(const QString &sql, const QString &description);
};
#endif