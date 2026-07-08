#include "src/utils/initfile.h"
#include "UI/Login/logindialog.h"
#include "UI/ConfigInit/configinitdialog.h"
#include "UI/MainWindow/mainwindow.h"
#include "src/database/databasemanager.h"
#include "src/database/mysqlinit.h"
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

    // InitFile initFile;
    // DatabaseManager dbManager;

    // QObject::connect(&initFile,&InitFile::parkingConfigChanged,[&dbManager](const QString &name, double price, int capacity){
    //                                                                         if(dbManager.isConnected()){
    //                                                                             dbManager.updateParkingConfig(name, price, capacity);
    //                                                                         }
    // });

    // if(!loadConfigWithRetry(&initFile)){
    //     return 0;
    // }

    // while(!dbManager.connectDatabase(initFile.getDbHost(), initFile.getDbPort(),
    //                                   initFile.getDbName(), initFile.getDbUsername(),
    //                                   initFile.getDbPassword()))
    // {
    //     QMessageBox::StandardButton btn = QMessageBox::critical(nullptr, "错误", "数据库连接失败! 是否重新配置?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    //     if(btn != QMessageBox::Yes){
    //         return 0;
    //     }

    //     // 数据库连接失败，必须让用户重新配置数据库信息
    //     ConfigInitDialog configDialog;
    //     if(configDialog.exec() != QDialog::Accepted){
    //         return 0;
    //     }

    //     // 重新加载配置
    //     if(!initFile.loadConfig()){
    //         QMessageBox::critical(nullptr, "错误", "配置文件加载失败!");
    //         return 0;
    //     }
    // }

    // //初始化表结构
    // MySQLInit mysqlInit(&dbManager);
    // if(!mysqlInit.initAll()){
    //     QMessageBox::critical(nullptr, "错误", "数据库初始化失败!");
    //     return -1;
    // }

    // LoginDialog loginDialog(nullptr, &dbManager);
    // if(loginDialog.exec() != QDialog::Accepted){
    //     return 0;
    // }

    // QString userRole = loginDialog.getUserRole();

    MainWindow mainWindow;
    mainWindow.show();

    // ─── 自定义通知系统测试（启动后自动演示） ─────────────────────
    // -------------------------------------------------------------------------
    // 测试说明：
    //   启动程序后，mainWindow 显示，随即依次弹出各类通知，
    //   用于验证 ToastWidget（自动消失）和 NotificationDialog（模态确认）的样式、动画、交互。
    // -------------------------------------------------------------------------

    // ① Toast —— 自动消失提示
    notifySuccess(&mainWindow, "停车系统启动成功！");
    notifyInfo(&mainWindow, "当前版本 v1.0.0");
    notifyFailure(&mainWindow, "摄像头未连接（演示）");
    notifyToastWarning(&mainWindow, "车位将满，请及时处理（演示）");

    // ② 模态弹窗 —— 需用户决策
    // 2.1 错误弹窗（单按钮，仅告知）
    QTimer::singleShot(500, &mainWindow, [&mainWindow]() {
        notifyError(&mainWindow, "连接异常", "无法连接到车牌识别服务，已切换至离线模式。");
    });

    // 2.2 警告弹窗（双按钮：确定/取消）
    QTimer::singleShot(1500, &mainWindow, [&mainWindow]() {
        bool ok = notifyWarning(&mainWindow, "操作确认", "出入口道闸未就绪，是否继续运行？");
        // 用户的选择会影响下一条 Toast 的内容
        if (ok) {
            notifySuccess(&mainWindow, "已确认，系统继续运行");
        } else {
            notifyInfo(&mainWindow, "已取消操作");
        }
    });

    // 2.3 询问弹窗（双按钮：是/否）
    QTimer::singleShot(2500, &mainWindow, [&mainWindow]() {
        bool del = notifyConfirm(&mainWindow, "删除确认", "确定要删除该停车记录吗？此操作不可撤销。");
        if (del) {
            notifySuccess(&mainWindow, "记录已删除");
        } else {
            notifyInfo(&mainWindow, "已保留该记录");
        }
    });

    // 2.4 带详情的错误弹窗（双按钮：我知道了 / 复制详情）
    QTimer::singleShot(3500, &mainWindow, [&mainWindow]() {
        NotificationDialog::errorWithDetail(&mainWindow,
                                            "数据库写入失败",
                                            "停车记录未能保存到数据库。",
                                            "MySQL Error 1062: Duplicate entry '京A12345' for key 'plate_number'"
                                            "\n\nFile: databasemanager.cpp:187"
                                            "\nFunction: insertParkingRecord()"
                                            "\nTimestamp: 2026-07-08 14:32:05");
    });

    return app.exec();
}
