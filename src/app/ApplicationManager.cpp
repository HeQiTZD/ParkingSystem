#include "ApplicationManager.h"
#include "UI/Login/logindialog.h"
#include "UI/MainWindow/mainwindow.h"
#include "src/utils/pthreadpool.h"
#include "src/camera/cameramanager.h"
#include "src/app/platerecognize.h"
#include "src/utils/initfile.h"
#include <QApplication>
#include <QDebug>
ApplicationManager::ApplicationManager(DatabaseManager &dbMgr,
                                     ParkingService &parkingSvc,
                                     UserService &userSvc,
                                     VehicleService &vehicleSvc,
                                     QObject *parent)
    : QObject(parent)
    , m_dbMgr(dbMgr)
    , m_parkingSvc(parkingSvc)
    , m_userSvc(userSvc)
    , m_vehicleSvc(vehicleSvc)
{
}
ApplicationManager::~ApplicationManager()
{
    if(m_mainWindow) m_mainWindow->stopRecognition();

    ThreadPoolManager::instance().shutdown();
    CameraManager::instance().shutdown();
    PlateRecognize::instance().shutdown();
    InitFile::instance().shutdown();
}
void ApplicationManager::start()
{

    m_loginDialog = new LoginDialog(nullptr, &m_userSvc);
    m_mainWindow  = new MainWindow(nullptr, &m_dbMgr, &m_parkingSvc, &m_userSvc, &m_vehicleSvc);

    connect(m_loginDialog, &LoginDialog::accepted,
            this, &ApplicationManager::onLoginAccepted);

    connect(m_loginDialog, &LoginDialog::rejected,
            this, &ApplicationManager::onAppExitRequested);

    connect(m_mainWindow, &MainWindow::logoutRequested,
            this, &ApplicationManager::onLogoutRequested);
    connect(m_mainWindow, &MainWindow::appExitRequested,
            this, &ApplicationManager::onAppExitRequested);

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
    m_loginDialog->reset();
    m_loginDialog->show();
    m_loginDialog->raise();
    m_loginDialog->activateWindow();
}
void ApplicationManager::onAppExitRequested()
{
    qApp->quit();
}