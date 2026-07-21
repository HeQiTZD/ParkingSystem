#include "src/utils/initfile.h"
#include "UI/Login/logindialog.h"
#include "UI/ConfigInit/configinitdialog.h"
#include "UI/MainWindow/mainwindow.h"
#include "src/database/databasemanager.h"
#include "src/database/mysqlinit.h"
#include "src/utils/messageType.h"
#include "src/app/ApplicationManager.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include "src/utils/notification_global.h"
bool loadConfigWithRetry(InitFile *initFile){
    if(!initFile->isConfigExists()){
        ConfigInitDialog configDialog;
        if(configDialog.exec() != QDialog::Accepted){
            return false;
        }
        if(!initFile->loadConfig()){
            QMessageBox::critical(nullptr, "错误", "配置文件加载失败!");
            return false;
        }
        return true;
    }
    while(!initFile->loadConfig()){
        QMessageBox::StandardButton btn =  QMessageBox::critical(nullptr, "错误", "配置文件加载失败! 是否重新配置", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(btn != QMessageBox::Yes){
            return false;
        }
        ConfigInitDialog configDialog;
        if(configDialog.exec() != QDialog::Accepted){
            return false;
        }
    }
    return true;
}
int main(int argc, char *argv[])
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    InitFile &initFile = InitFile::instance();
    DatabaseManager dbManager;
    QObject::connect(&initFile,&InitFile::parkingConfigChanged,[&dbManager](const QString &name, double price, int capacity, int freeMinutes){
                                                                            if(dbManager.isConnected()){
                                                                                Q_UNUSED(freeMinutes)
                                                                                dbManager.updateParkingConfig(name, price, capacity);
                                                                            }
    });
    qRegisterMetaType<MessageType::Type>("MessageType::Type");
    if(!loadConfigWithRetry(&initFile)){
        return 0;
    }
    while(!dbManager.connectDatabase(initFile.getDbHost(), initFile.getDbPort(),
                                      initFile.getDbName(), initFile.getDbUsername(),
                                      initFile.getDbPassword()))
    {
        QMessageBox::StandardButton btn = QMessageBox::critical(nullptr, "错误", "数据库连接失败! 是否重新配置?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(btn != QMessageBox::Yes){
            return 0;
        }

        ConfigInitDialog configDialog;
        if(configDialog.exec() != QDialog::Accepted){
            return 0;
        }

        if(!initFile.loadConfig()){
            QMessageBox::critical(nullptr, "错误", "配置文件加载失败!");
            return 0;
        }
    }

    MySQLInit mysqlInit(&dbManager, &initFile);
    if(!mysqlInit.initAll()){
        QMessageBox::critical(nullptr, "错误", "数据库初始化失败!");
        return -1;
    }
    ParkingService  parkingSvc(dbManager);
    UserService     userSvc(dbManager);
    VehicleService  vehicleSvc(dbManager);
    ApplicationManager applicationManager(dbManager, parkingSvc, userSvc, vehicleSvc);
    applicationManager.start();
    return app.exec();
}