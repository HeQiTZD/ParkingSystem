#include "src/utils/initfile.h"
#include "UI/Login/logindialog.h"
#include "UI/ConfigInit/configinitdialog.h"
#include "UI/MainWindow/mainwindow.h"
#include "src/database/databasemanager.h"
#include "src/database/mysqlinit.h"
#include "src/utils/messageType.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>

// ─── 自定义通知系统（Toast + 模态弹窗） ─────────────────────
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
    QApplication app(argc, argv);

    InitFile initFile;
    DatabaseManager dbManager;

    QObject::connect(&initFile,&InitFile::parkingConfigChanged,[&dbManager](const QString &name, double price, int capacity, int freeMinutes){
                                                                            if(dbManager.isConnected()){
                                                                                // 免费时长暂存于配置文件，数据库保留扩展 TODO: 后续设置页面接入后可同步DB
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

        // 数据库连接失败，必须让用户重新配置数据库信息
        ConfigInitDialog configDialog;
        if(configDialog.exec() != QDialog::Accepted){
            return 0;
        }

        // 重新加载配置
        if(!initFile.loadConfig()){
            QMessageBox::critical(nullptr, "错误", "配置文件加载失败!");
            return 0;
        }
    }

    //初始化表结构
    MySQLInit mysqlInit(&dbManager);
    if(!mysqlInit.initAll()){
        QMessageBox::critical(nullptr, "错误", "数据库初始化失败!");
        return -1;
    }

    LoginDialog loginDialog(nullptr, &dbManager);
    if(loginDialog.exec() != QDialog::Accepted){
        return 0;
    }

    QString userRole = loginDialog.getUserRole();

    MainWindow mainWindow(nullptr, &dbManager);
    mainWindow.show();

    return app.exec();
}
