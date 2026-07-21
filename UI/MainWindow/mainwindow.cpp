#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "circleprogress.h"
#include "vehicleentryexitwidget.h"
#include "src/database/databasemanager.h"
#include "src/service/parkingservice.h"
#include "src/service/userservice.h"
#include "src/service/vehicleservice.h"
#include "src/camera/camerathread.h"
#include "src/app/car.h"
#include "src/utils/notification_global.h"
#include "src/utils/initfile.h"
#include "src/camera/framequeue.h"
#include "src/app/recognizethread.h"
#include "src/app/plateconfirmtracker.h"
#include "UI/VehicleInformation/vehicleinformation.h"
#include "UI/UserManager/usermanagement.h"
#include "UI/CameraManagement/cameramanagement.h"
#include "UI/Settings/settingsdialog.h"
#include "src/camera/cameramanager.h"
#include "src/app/platerecognize.h"
#include <QFile>
#include <QButtonGroup>
#include <QDateTime>
#include <QTimer>
#include <QLabel>
#include <QCursor>
#include <QMouseEvent>
MainWindow::MainWindow(QWidget *parent,
                     DatabaseManager *db,
                     ParkingService *parkingSvc,
                     UserService *userSvc,
                     VehicleService *vehicleSvc)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_db(db)
    , m_parkingSvc(parkingSvc)
    , m_userSvc(userSvc)
    , m_vehicleSvc(vehicleSvc)
{
    ui->setupUi(this);
    CameraManager::instance().scanCameras();
    setWindowFlag(Qt::FramelessWindowHint);
    setMinimumSize(1200, 700);
    setMouseTracking(true);
    if (centralWidget())
        centralWidget()->setMouseTracking(true);
    QFile styleFile(":/styles/main.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        setStyleSheet(styleSheet);
        styleFile.close();
    }
    m_timer = new QTimer(this);
    connect(m_timer,&QTimer::timeout,this,&MainWindow::updateTime);
    m_timer->start(1000);
    updateTime();

    ui->userAvatarLabel->setPixmap(QIcon(":/icons/account_circle.svg").pixmap(24, 24));

    QSize iconSize(20, 20);
    ui->dashboardButton->setIcon(QIcon(":/icons/home.svg"));
    ui->dashboardButton->setIconSize(iconSize);
    ui->dashboardButton->setCheckable(true);
    ui->dashboardButton->setChecked(true);
    ui->vehicleInfoButton->setIcon(QIcon(":/icons/directions.svg"));
    ui->vehicleInfoButton->setIconSize(iconSize);
    ui->vehicleInfoButton->setCheckable(true);
    ui->userManagementButton->setIcon(QIcon(":/icons/group.svg"));
    ui->userManagementButton->setIconSize(iconSize);
    ui->userManagementButton->setCheckable(true);
    ui->cameraManagementButton->setIcon(QIcon(":/icons/videocam.svg"));
    ui->cameraManagementButton->setIconSize(iconSize);
    ui->cameraManagementButton->setCheckable(true);
    ui->logoutButton->setIcon(QIcon(":/icons/logout.svg"));
    ui->logoutButton->setIconSize(iconSize);
    ui->logoutButton->setCheckable(true);

    QButtonGroup *navButtonGroup = new QButtonGroup(this);
    navButtonGroup->addButton(ui->dashboardButton);
    navButtonGroup->addButton(ui->vehicleInfoButton);
    navButtonGroup->addButton(ui->userManagementButton);
    navButtonGroup->addButton(ui->cameraManagementButton);
    navButtonGroup->setExclusive(true);

    QSize windowBtnSize(16,16);
    ui->setButton->setIcon(QIcon(":/icons/settings.svg"));
    ui->setButton->setIconSize(windowBtnSize);
    ui->minimizeButton->setIcon(QIcon(":/icons/minied.svg"));
    ui->minimizeButton->setIconSize(windowBtnSize);
    ui->maximizeButton->setIcon(QIcon(":/icons/expand.svg"));
    ui->maximizeButton->setIconSize(windowBtnSize);
    ui->closeButton->setIcon(QIcon(":/icons/close.svg"));
    ui->closeButton->setIconSize(windowBtnSize);

     QSize videoCtrlBtnSize(18, 18);
    ui->cameraCtrlBtn1->setIcon(QIcon(":/icons/upload.svg"));
    ui->cameraCtrlBtn1->setIconSize(videoCtrlBtnSize);
    ui->cameraCtrlBtn2->setIcon(QIcon(":/icons/replay_10.svg"));
    ui->cameraCtrlBtn2->setIconSize(videoCtrlBtnSize);
    ui->cameraCtrlBtn3->setIcon(QIcon(":/icons/play_arrow.svg"));
    ui->cameraCtrlBtn3->setIconSize(videoCtrlBtnSize);
    ui->cameraCtrlBtn4->setIcon(QIcon(":/icons/forward_10.svg"));
    ui->cameraCtrlBtn4->setIconSize(videoCtrlBtnSize);
    ui->cameraCtrlBtn5->setIcon(QIcon(":/icons/photo_camera.svg"));
    ui->cameraCtrlBtn5->setIconSize(videoCtrlBtnSize);

    ui->cameraTitleLabel->setText("<img src=':/icons/videocam.svg' width='16' height='16' style='vertical-align: middle;'/> <span style='font-size: 14px; vertical-align: middle;'>摄像头 - 实时画面</span>");

    ui->parkingStatsTitleLabel->setText("<img src=':/icons/pie_chart.svg' width='16' height='16' style='vertical-align: middle;'/> <span style='font-size: 16px; font-weight: bold; vertical-align: middle;'>车位统计</span>");

    ui->entrySearchLabel->setText("<img src=':/icons/login.svg' width='16' height='16' style='vertical-align: middle;'/> <span style='font-size: 14px; vertical-align: middle;'>出库</span>");
    ui->exitSearchLabel->setText("<img src=':/icons/logout.svg' width='16' height='16' style='vertical-align: middle;'/> <span style='font-size: 14px; vertical-align: middle;'>入库</span>");

    ui->usedSpacesLabel->setText("<span style='color:#4169E1;'>■</span> <span style='font-size: 14px; color: #333333;'>已用车位</span>");
    ui->remainingSpacesLabel->setText("<span style='color:#9a9999;'>■</span> <span style='font-size: 14px; color: #333333;'>剩余车位</span>");
    ui->totalSpacesLabel->setText("<img src=':/icons/directions.svg' width='14' height='14' style='vertical-align: middle;'/> <span style='font-size: 14px; color: #333333;'>总车位</span>");
    m_parkingChart = new CircleProgress(ui->parkingChartWidget);

    m_parkingChart->setGeometry(30, 10, 120, 120);
    m_parkingChart->setProgress(70);
    m_parkingChart->setUsedText("已用空间");

    onUpdateParkingCount();

    connect(m_parkingSvc, &ParkingService::parkingDataChanged,
            this, &MainWindow::onUpdateParkingCount);

    m_videoLabel = new QLabel(ui->cameraViewWidget);
    m_videoLabel->setGeometry(ui->cameraViewWidget->rect());
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setScaledContents(false);

    m_frameQueue = new FrameQueue();
    m_cameraThread = CameraManager::instance().getThread(0);
    if(m_cameraThread){
        connect(m_cameraThread, &CameraThread::newFrameCaptured, this, &MainWindow::updateFrame);
        CameraManager::instance().start(0);
        m_cameraThread->setFrameQueue(m_frameQueue);
    } else {
        delete m_frameQueue;
        m_frameQueue = nullptr;
    }
    ui->cameraViewWidget->installEventFilter(this);
    connect(ui->closeButton,&QPushButton::clicked,this,&MainWindow::onCloseButton);
    connect(ui->minimizeButton,&QPushButton::clicked,this,&MainWindow::onMinButton);
    connect(ui->maximizeButton,&QPushButton::clicked,this,&MainWindow::onMaxButton);
    connect(ui->setButton,&QPushButton::clicked,this,&MainWindow::onSetButton);
    connect(ui->entrySearchButton,&QPushButton::clicked,this,&MainWindow::onEntrySearchButton);
    connect(ui->exitSearchButton,&QPushButton::clicked,this,&MainWindow::onExitSearchButton);
    connect(ui->logoutButton,&QPushButton::clicked,this,&MainWindow::onlogoutButton);

    if(m_frameQueue){
        m_recognizeThread = new RecognizeThread(this);
        m_recognizeThread->setFrameQueue(m_frameQueue);
        m_recognizeThread->setSamplingInterval(1500);
    } else {
        m_recognizeThread = nullptr;
    }

    m_confirmTracker = new PlateConfirmTracker();

    if(m_recognizeThread){
        connect(m_recognizeThread, &RecognizeThread::plateRecognized, this, &MainWindow::onPlateRecognized);
    }

    PlateRecognize& recognizer = PlateRecognize::instance();
    if(!recognizer.isModelsLoaded()){
        QString modelPath = QCoreApplication::applicationDirPath() + "/model";
        recognizer.loadModels(modelPath);
    }

    connect(ui->autoRecognizeCheckBox, &QCheckBox::toggled, this, &MainWindow::onAutoRecognizeToggled);

    m_vehicleInfoPage = new VehicleInformation(this, m_vehicleSvc);
    int vehicleInfoIndex = ui->stackedWidget->addWidget(m_vehicleInfoPage);
    m_userManagementPage = new UserManagement(this, m_userSvc);
    int userMgmtIndex = ui->stackedWidget->addWidget(m_userManagementPage);
    m_cameraManagementPage = new CameraManagement(this);
    int cameraMgmtIndex = ui->stackedWidget->addWidget(m_cameraManagementPage);
    Q_UNUSED(cameraMgmtIndex);

     connect(navButtonGroup, &QButtonGroup::idClicked, this, [this, navButtonGroup](int id) {
        if (id == -1) return;
        QAbstractButton *btn = navButtonGroup->button(id);
        if (!btn) return;

        if (btn == ui->dashboardButton) {
            ui->stackedWidget->setCurrentIndex(0);
        } else if (btn == ui->vehicleInfoButton) {
            ui->stackedWidget->setCurrentIndex(1);
        } else if (btn == ui->userManagementButton) {
            ui->stackedWidget->setCurrentIndex(2);
        } else if (btn == ui->cameraManagementButton) {
            ui->stackedWidget->setCurrentIndex(3);
        }
    });
}
MainWindow::~MainWindow()
{
    if(m_recognizeThread){
        m_recognizeThread->stop();
        m_recognizeThread->wait(3000);
    }

    CameraManager::instance().stop(0);

    if(m_frameQueue){
        delete m_frameQueue;
        m_frameQueue = nullptr;
    }

    delete m_confirmTracker;
    m_confirmTracker = nullptr;
    delete ui;
}
void MainWindow::updateFrame(cv::Mat frame)
{
    if(frame.empty()) return;
    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
    QImage img(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(img);
    m_videoLabel->setPixmap(pixmap.scaled(
        m_videoLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation));}
void MainWindow::onCloseButton()
{

    emit appExitRequested();
}
void MainWindow::onMinButton()
{
    showMinimized();
}
void MainWindow::onMaxButton()
{
    QSize windowBtnSize(16,16);
    ui->maximizeButton->setIconSize(windowBtnSize);
    if(isMaximized()){
        showNormal();
        ui->maximizeButton->setIcon(QIcon(":/icons/expand.svg"));
    }else{
        showMaximized();
        ui->maximizeButton->setIcon(QIcon(":/icons/collapse.svg"));
    }
}
void MainWindow::onSetButton()
{
    auto *dialog = new SettingsDialog(this);
    dialog->setModal(true);
    dialog->exec();
    dialog->deleteLater();
}
void MainWindow::onlogoutButton()
{

    emit logoutRequested();
}
void MainWindow::onEntrySearchButton()
{
    if(!m_db){
        qDebug() << "数据库未连接";
        return;
    }

    QString plate = Car::normalizePlate(ui->entryPlateInput->text());
    if(plate.isEmpty()){
        notifyInfo(this, QStringLiteral("请输入车牌号"));
        return;
    }
    if(!Car::isValidLicensePlate(plate)){
        notifyInfo(this, QStringLiteral("请输入正确的车牌号"));
        ui->entryPlateInput->clear();
        return;
    }
    if(m_db->isVehicleInPark(plate)){
        notifyInfo(this, QStringLiteral("车辆已入库"));
        return;
    }

    QString parkingName = getParkingNameFromConfig();
    if(m_db->checkIn(plate, parkingName)){
        notifySuccess(this, QStringLiteral("%1 入库成功").arg(plate));
        if (ui->vehicleEntryExitWidget) {
            ui->vehicleEntryExitWidget->prependEntry(
                {Car::displayPlate(plate), QDateTime::currentDateTime(), VehicleEntryStatus::In});
        }
        ui->entryPlateInput->clear();
        return;
    }else{
        notifyFailure(this, "入库失败");
        return;
    }
}
void MainWindow::onExitSearchButton()
{
    if(!m_db){
        qDebug() << "数据库未连接";
        return;
    }

    QString plate = Car::normalizePlate(ui->exitPlateInput->text());
    if(plate.isEmpty()){
        notifyInfo(this, QStringLiteral("请输入车牌号"));
        return;
    }
    if(!Car::isValidLicensePlate(plate)){
        notifyInfo(this, QStringLiteral("请输入正确的车牌号"));
        ui->exitPlateInput->clear();
        return;
    }

    if(!m_db->isVehicleInPark(plate)){
        notifyInfo(this, QStringLiteral("%1 车辆未入库").arg(plate));
        return;
    }
    QDateTime intTime = m_db->getVehicleCheckInTime(plate);
    QDateTime outTime = QDateTime::currentDateTime();

    qint64 totalMinutes = intTime.secsTo(outTime) / 60;
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;

    double cost = Car::calculateFee(intTime, outTime,
                                    InitFile::instance().getParkingPrice(),
                                    InitFile::instance().getFreeMinutes());
    QString parkingName = InitFile::instance().getParkingName();

    QString msg = QStringLiteral("停车时长: %1 小时 %2 分钟, 费用 %3 元").arg(hours).arg(minutes).arg(cost, 0, 'f', 2);
    if(!notifyConfirm(this, QStringLiteral("停车时长与费用"), msg)){
        return;
    }
    if(m_db->checkOut(plate, parkingName, cost)){
        notifySuccess(this, QStringLiteral("%1 出库成功").arg(plate));
        if (ui->vehicleEntryExitWidget) {
            ui->vehicleEntryExitWidget->prependEntry(
                {Car::displayPlate(plate), QDateTime::currentDateTime(), VehicleEntryStatus::Out});
        }
        ui->exitPlateInput->clear();
        return;
    }else{
        notifyFailure(this, QStringLiteral("%1 出库失败").arg(plate));
        return;
    }
}
void MainWindow::onUpdateParkingCount()
{
    if(!m_db){
        return;
    }

    QString name = InitFile::instance().getParkingName();
    if(name.isEmpty()){
        qWarning() << "配置文件中停车场名称为空";
        return;
    }
    ParkingStats parkingCount(m_db->getParkingStats(name));
    if(parkingCount.totalSpaces == 0){
        qWarning() << "未找到停车场:" << name;
        return;
    }
    int percentage = (parkingCount.totalSpaces > 0) ? static_cast<int>(static_cast<double>(parkingCount.usedSpaces) / parkingCount.totalSpaces * 100) : 0;
    m_parkingChart->setProgress(percentage);
    ui->usedSpacesValueLabel->setText(QString::number(parkingCount.usedSpaces));
    ui->remainingSpacesValueLabel->setText(QString::number(parkingCount.freeSpaces));
    ui->totalSpacesValueLabel->setText(QString::number(parkingCount.totalSpaces));
}
QString MainWindow::getParkingNameFromConfig() const
{
    return InitFile::instance().getParkingName();
}
void MainWindow::updateTime()
{
    QDateTime currenTime = QDateTime::currentDateTime();
    QString timeStr = currenTime.toString("yyyy-MM-dd HH:mm:ss");
    ui->timeLabel->setText(timeStr);
}
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->cameraViewWidget && event->type() == QEvent::Resize) {
        m_videoLabel->setGeometry(ui->cameraViewWidget->rect());
    }

    if (event->type() == QEvent::MouseMove && m_mouseArea == NoArea) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QWidget *widget = qobject_cast<QWidget*>(obj);
        QPoint windowPos = widget ? widget->mapTo(this, mouseEvent->pos()) : mouseEvent->pos();
        MouseArea hover = getMouseArea(windowPos);
        setCursorShape(hover);
    }
    return QMainWindow::eventFilter(obj, event);
}
void MainWindow::onPlateRecognized(const QString &plate, const cv::Mat &plateImg)
{
    Q_UNUSED(plateImg);

    if(!m_autoRecognizeEnabled){
        return;
    }
    if(!m_db){
        qDebug() << QStringLiteral("自动识别：数据库未连接");
        return;
    }

    if(!m_confirmTracker->report(plate)){
        return;
    }
    QString normalizedPlate = Car::normalizePlate(plate);
    if(normalizedPlate.isEmpty() || !Car::isValidLicensePlate(normalizedPlate)){
        qDebug() << QStringLiteral("自动识别：车牌格式无效") << plate;
        return;
    }
    QString parkingName = getParkingNameFromConfig();
    if(!m_db->isVehicleInPark(normalizedPlate)){
        if(m_db->checkIn(normalizedPlate, parkingName)){
            m_confirmTracker->markActioned(normalizedPlate);
            notifySuccess(this, QStringLiteral("[自动] %1 入库成功").arg(normalizedPlate));
            if(ui->vehicleEntryExitWidget){
                ui->vehicleEntryExitWidget->prependEntry({Car::displayPlate(normalizedPlate), QDateTime::currentDateTime(), VehicleEntryStatus::In});
            }
        }else{
            notifyFailure(this, QStringLiteral("[自动] %1 入库失败").arg(normalizedPlate));
        }
    }else{
        QDateTime inTime = m_db->getVehicleCheckInTime(normalizedPlate);
        QDateTime outTime = QDateTime::currentDateTime();
        double cost = Car::calculateFee(inTime, outTime, InitFile::instance().getParkingPrice(), InitFile::instance().getFreeMinutes());
        if(m_db->checkOut(normalizedPlate, parkingName, cost)){
            m_confirmTracker->markActioned(normalizedPlate);

        qint64 totalMinutes = inTime.secsTo(outTime) / 60;
        int hours = totalMinutes / 60;
        int minutes = totalMinutes % 60;
        notifySuccess(this, QStringLiteral("[自动] %1 出库成功 | %2时%3分 | %4元").arg(normalizedPlate).arg(hours).arg(minutes).arg(cost, 0, 'f', 2));
        if(ui->vehicleEntryExitWidget){
            ui->vehicleEntryExitWidget->prependEntry({Car::displayPlate(normalizedPlate), QDateTime::currentDateTime(), VehicleEntryStatus::Out});
        }
        }else{
            notifyFailure(this, QStringLiteral("[自动] %1 出库失败").arg(normalizedPlate));
        }
    }
}
void MainWindow::onAutoRecognizeToggled(bool checked)
{
    m_autoRecognizeEnabled = checked;
    if(checked){
        if(m_recognizeThread && !m_recognizeThread->isRunning()){
            m_recognizeThread->start();
        }
        notifyInfo(this, QStringLiteral("自动识别已开启"));
    }else{
        if(m_recognizeThread && m_recognizeThread->isRunning()){
            m_recognizeThread->stop();
            m_recognizeThread->wait(3000);
        }
        notifyInfo(this, QStringLiteral("自动识别已关闭"));
    }
}
void MainWindow::startRecognition()
{
    ui->autoRecognizeCheckBox->setChecked(true);
}
void MainWindow::stopRecognition()
{
    ui->autoRecognizeCheckBox->setChecked(false);
}
MainWindow::MouseArea MainWindow::getMouseArea(const QPoint &pos) const
{
    int x = pos.x(), y = pos.y();
    int w = width(), h = height();
    bool top    = y < kResizeBorder;
    bool bottom = y > h - kResizeBorder;
    bool left   = x < kResizeBorder;
    bool right  = x > w - kResizeBorder;
    if (top && left)     return ResizeTopLeft;
    if (top && right)    return ResizeTopRight;
    if (bottom && left)  return ResizeBottomLeft;
    if (bottom && right) return ResizeBottomRight;
    if (top)    return ResizeTop;
    if (bottom) return ResizeBottom;
    if (left)   return ResizeLeft;
    if (right)  return ResizeRight;
    return NoArea;
}
void MainWindow::setCursorShape(MouseArea area)
{
    switch (area) {
    case ResizeTop:
    case ResizeBottom:
        setCursor(Qt::SizeVerCursor); break;
    case ResizeLeft:
    case ResizeRight:
        setCursor(Qt::SizeHorCursor); break;
    case ResizeTopLeft:
    case ResizeBottomRight:
        setCursor(Qt::SizeFDiagCursor); break;
    case ResizeTopRight:
    case ResizeBottomLeft:
        setCursor(Qt::SizeBDiagCursor); break;
    default:
        unsetCursor(); break;
    }
}
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;
    QPoint pos = event->position().toPoint();
    m_mouseArea = getMouseArea(pos);
    if (m_mouseArea == NoArea && pos.y() < m_dragBarHeight) {
        m_mouseArea = DragArea;
    }
    if (m_mouseArea == DragArea && isMaximized()) {
        onMaxButton();
    }
    if (m_mouseArea != NoArea) {
        m_dragPos = event->globalPosition().toPoint();
        m_resizeGeo = geometry();
        event->accept();
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseArea == NoArea) {
        QPoint pos = event->position().toPoint();
        MouseArea hover = getMouseArea(pos);
        setCursorShape(hover);
        return;
    }
    QPoint current = event->globalPosition().toPoint();
    QPoint delta = current - m_dragPos;
    switch (m_mouseArea) {
    case DragArea:
        move(m_resizeGeo.topLeft() + delta);
        break;
    case ResizeTop:
        setGeometry(m_resizeGeo.x(), m_resizeGeo.y() + delta.y(),
                    m_resizeGeo.width(), m_resizeGeo.height() - delta.y());
        break;
    case ResizeBottom:
        setGeometry(m_resizeGeo.x(), m_resizeGeo.y(),
                    m_resizeGeo.width(), m_resizeGeo.height() + delta.y());
        break;
    case ResizeLeft:
        setGeometry(m_resizeGeo.x() + delta.x(), m_resizeGeo.y(),
                    m_resizeGeo.width() - delta.x(), m_resizeGeo.height());
        break;
    case ResizeRight:
        setGeometry(m_resizeGeo.x(), m_resizeGeo.y(),
                    m_resizeGeo.width() + delta.x(), m_resizeGeo.height());
        break;
    case ResizeTopLeft:
        setGeometry(m_resizeGeo.x() + delta.x(), m_resizeGeo.y() + delta.y(),
                    m_resizeGeo.width() - delta.x(), m_resizeGeo.height() - delta.y());
        break;
    case ResizeTopRight:
        setGeometry(m_resizeGeo.x(), m_resizeGeo.y() + delta.y(),
                    m_resizeGeo.width() + delta.x(), m_resizeGeo.height() - delta.y());
        break;
    case ResizeBottomLeft:
        setGeometry(m_resizeGeo.x() + delta.x(), m_resizeGeo.y(),
                    m_resizeGeo.width() - delta.x(), m_resizeGeo.height() + delta.y());
        break;
    case ResizeBottomRight:
        setGeometry(m_resizeGeo.x(), m_resizeGeo.y(),
                    m_resizeGeo.width() + delta.x(), m_resizeGeo.height() + delta.y());
        break;
    default:
        break;
    }
    event->accept();
}
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_mouseArea = NoArea;
    unsetCursor();
    event->accept();
}