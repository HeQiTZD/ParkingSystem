#include "ApplicationManager.h"
#include "UI/Login/logindialog.h"
#include "UI/MainWindow/mainwindow.h"
#include "src/utils/pthreadpool.h"
#include "src/camera/cameramanager.h"
#include "src/app/platerecognize.h"
#include "src/utils/initfile.h"
#include <QApplication>
#include <QDebug>

ApplicationManager::ApplicationManager(DatabaseManager &dbMgr, QObject *parent)
    : QObject(parent)
    , m_dbMgr(dbMgr)
{
}

ApplicationManager::~ApplicationManager()
{
    // 先停识别
    if(m_mainWindow) m_mainWindow->stopRecognition();

    // 逆序依赖: ThreadPool→Camera→Recognize→InitFile
    ThreadPoolManager::instance().shutdown();
    CameraManager::instance().shutdown();
    PlateRecognize::instance().shutdown();
    InitFile::instance().shutdown();
}

void ApplicationManager::start()
{
    // 两个窗口都作为本对象的子对象，长生命周期常驻。
    // 析构时由 Qt 父子关系自动 delete，无泄漏。
    m_loginDialog = new LoginDialog(nullptr, &m_dbMgr);
    m_mainWindow  = new MainWindow(nullptr, &m_dbMgr);

    // —— 登录框 → 主窗口 ——
    // QDialog::accepted 在 accept() 时发射（登录成功）。
    connect(m_loginDialog, &LoginDialog::accepted,
            this, &ApplicationManager::onLoginAccepted);

    // —— 登录框右上角 X：关闭即退出整个应用 ——
    // setQuitOnLastWindowClosed=false 后，关闭登录框不会自动退出，
    // 这里手动接管：登录框被关闭(rejected)→退出应用。
    connect(m_loginDialog, &LoginDialog::rejected,
            this, &ApplicationManager::onAppExitRequested);

    // —— 主窗口 → 登录框 / 退出 ——
    connect(m_mainWindow, &MainWindow::logoutRequested,
            this, &ApplicationManager::onLogoutRequested);
    connect(m_mainWindow, &MainWindow::appExitRequested,
            this, &ApplicationManager::onAppExitRequested);

    // 启动：显示登录框
    m_loginDialog->show();
}

void ApplicationManager::onLoginAccepted()
{
    m_loginDialog->hide();
    m_mainWindow->show();
    m_mainWindow->startRecognition();
}

void ApplicationManager::onLogoutRequested()
{
    m_mainWindow->stopRecognition();
    m_mainWindow->hide();
    // 复用登录框：清空输入，重新弹出
    m_loginDialog->reset();
    m_loginDialog->show();
    m_loginDialog->raise();
    m_loginDialog->activateWindow();
}

void ApplicationManager::onAppExitRequested()
{
    // 主动退出，不依赖 quitOnLastWindowClosed
    qApp->quit();
}
