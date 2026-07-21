#ifndef APPLICATIONMANAGER
#define APPLICATIONMANAGER
#include <QObject>
#include "src/database/databasemanager.h"
#include "src/service/parkingservice.h"
#include "src/service/userservice.h"
#include "src/service/vehicleservice.h"
class LoginDialog;
class MainWindow;
class ApplicationManager : public QObject {
    Q_OBJECT
public:
    explicit ApplicationManager(DatabaseManager &dbMgr,
                                ParkingService &parkingSvc,
                                UserService &userSvc,
                                VehicleService &vehicleSvc,
                                QObject *parent = nullptr);
    ~ApplicationManager();
    void start();
private slots:
    void onLoginAccepted();
    void onLogoutRequested();
    void onAppExitRequested();
private:
    DatabaseManager &m_dbMgr;
    ParkingService  &m_parkingSvc;
    UserService     &m_userSvc;
    VehicleService  &m_vehicleSvc;
    LoginDialog *m_loginDialog = nullptr;
    MainWindow  *m_mainWindow  = nullptr;
};
#endif