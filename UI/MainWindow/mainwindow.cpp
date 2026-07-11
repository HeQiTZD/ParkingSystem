#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "circleprogress.h"
#include "src/app/car.h"
#include "src/utils/notification_global.h"
#include "src/utils/initfile.h"
#include <QFile>
#include <QButtonGroup>
#include <QDateTime>
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent, DatabaseManager *db)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_db(db)
{
    ui->setupUi(this);
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


    // 设置用户头像
    ui->userAvatarLabel->setPixmap(QIcon(":/icons/account_circle.svg").pixmap(24, 24));

    // 设置导航栏按钮图标和属性
    QSize iconSize(20, 20);
    ui->dashboardButton->setIcon(QIcon(":/icons/home.svg"));
    ui->dashboardButton->setIconSize(iconSize);
    ui->dashboardButton->setCheckable(true);
    ui->dashboardButton->setChecked(true);  // 默认选中仪表盘

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

    // 创建按钮组实现互斥选中
    QButtonGroup *navButtonGroup = new QButtonGroup(this);
    navButtonGroup->addButton(ui->dashboardButton);
    navButtonGroup->addButton(ui->vehicleInfoButton);
    navButtonGroup->addButton(ui->userManagementButton);
    navButtonGroup->addButton(ui->cameraManagementButton);
    navButtonGroup->setExclusive(true);  // 互斥模式

    // 设置窗口控制按钮图标
    QSize windowBtnSize(16,16);

    ui->setButton->setIcon(QIcon(":/icons/settings.svg"));
    ui->setButton->setIconSize(windowBtnSize);

    ui->minimizeButton->setIcon(QIcon(":/icons/minied.svg"));
    ui->minimizeButton->setIconSize(windowBtnSize);

    ui->maximizeButton->setIcon(QIcon(":/icons/expand.svg"));
    ui->maximizeButton->setIconSize(windowBtnSize);

    ui->closeButton->setIcon(QIcon(":/icons/close.svg"));
    ui->closeButton->setIconSize(windowBtnSize);

    // 设置视频控制按钮图标
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

    // 设置摄像头标题标签图标（使用富文本）
    ui->cameraTitleLabel->setText("<img src=':/icons/videocam.svg' width='16' height='16' style='vertical-align: middle;'/> <span style='font-size: 14px; vertical-align: middle;'>摄像头 - 实时画面</span>");

    // 设置车位统计标题标签图标（使用富文本）
    ui->parkingStatsTitleLabel->setText("<img src=':/icons/pie_chart.svg' width='16' height='16' style='vertical-align: middle;'/> <span style='font-size: 16px; font-weight: bold; vertical-align: middle;'>车位统计</span>");

    // 设置出入库标签图标（使用富文本）
    ui->entrySearchLabel->setText("<img src=':/icons/login.svg' width='16' height='16' style='vertical-align: middle;'/> <span style='font-size: 14px; vertical-align: middle;'>出库</span>");
    ui->exitSearchLabel->setText("<img src=':/icons/logout.svg' width='16' height='16' style='vertical-align: middle;'/> <span style='font-size: 14px; vertical-align: middle;'>入库</span>");

    // 设置车位统计标签图标（使用富文本）
    ui->usedSpacesLabel->setText("<span style='color:#4169E1;'>■</span> <span style='font-size: 14px; color: #333333;'>已用车位</span>");
    ui->remainingSpacesLabel->setText("<span style='color:#9a9999;'>■</span> <span style='font-size: 14px; color: #333333;'>剩余车位</span>");
    ui->totalSpacesLabel->setText("<img src=':/icons/directions.svg' width='14' height='14' style='vertical-align: middle;'/> <span style='font-size: 14px; color: #333333;'>总车位</span>");

    m_parkingChart = new CircleProgress(ui->parkingChartWidget);

    // 手动设置固定位置和大小（x, y, width, height）
    m_parkingChart->setGeometry(30, 10, 120, 120);

    m_parkingChart->setProgress(70);
    m_parkingChart->setUsedText("已用空间");

    // 初始化更新车位情况
    onUpdateParkingCount();

    // 摄像头初始化
    m_videoLabel = new QLabel(ui->cameraViewWidget);
    m_videoLabel->setGeometry(ui->cameraViewWidget->rect());
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setScaledContents(false);

    m_cameraThread = new CameraThread(0, this);
    connect(m_cameraThread,&CameraThread::newFrameCaptured,this,&MainWindow::updateFrame);
    m_cameraThread->start();

    ui->cameraViewWidget->installEventFilter(this);

    connect(ui->closeButton,&QPushButton::clicked,this,&MainWindow::onCloseButton);
    connect(ui->minimizeButton,&QPushButton::clicked,this,&MainWindow::onMinButton);
    connect(ui->maximizeButton,&QPushButton::clicked,this,&MainWindow::onMaxButton);
    connect(ui->setButton,&QPushButton::clicked,this,&MainWindow::onSetButton);
    connect(ui->entrySearchButton,&QPushButton::clicked,this,&MainWindow::onEntrySearchButton);
    connect(ui->exitSearchButton,&QPushButton::clicked,this,&MainWindow::onExitSearchButton);
    connect(ui->logoutButton,&QPushButton::clicked,this,&MainWindow::onlogoutButton);
}

MainWindow::~MainWindow()
{
    if(m_cameraThread){
        m_cameraThread->quit();
        m_cameraThread->wait();
    }
    delete ui;
}

void MainWindow::updateFrame(cv::Mat frame)
{
    if(frame.empty()) return;

    // BGR->RGB
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);

    QPixmap pixmap = QPixmap::fromImage(img);
    m_videoLabel->setPixmap(pixmap.scaled(
        m_videoLabel->size(), 
        Qt::KeepAspectRatio, 
        Qt::SmoothTransformation));}

void MainWindow::onCloseButton()
{
    // 通知 ApplicationManager 结束整个应用。
    // 不在这里 close()：主窗口是常驻对象，由 ApplicationManager
    // 通过 qApp->quit() 退出后随父对象一起析构。
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
    return;
}

void MainWindow::onlogoutButton()
{
    // 通知 ApplicationManager 切回登录框。
    // 不在这里 close()：主窗口是常驻对象，由 ApplicationManager 隐藏后复用。
    emit logoutRequested();
}

void MainWindow::onEntrySearchButton()
{
    if(!m_db){
        qDebug() << "数据库未连接";
        return;
    }

    // 车牌号验证
    QString plate = ui->entryPlateInput->text().trimmed();
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

    if(m_db->checkIn(plate)){
        notifySuccess(this, QStringLiteral("%1 入库成功").arg(plate));
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

    // 车牌号验证
    QString plate = ui->exitPlateInput->text().trimmed();
    if(plate.isEmpty()){
        notifyInfo(this, QStringLiteral("请输入车牌号"));
        return;
    }
    if(!Car::isValidLicensePlate(plate)){
        notifyInfo(this, QStringLiteral("请输入正确的车牌号"));
        ui->exitPlateInput->clear();
        return;
    }

    //检查车辆是否已在库中
    if(!m_db->isVehicleInPark(plate)){
        notifyInfo(this, QStringLiteral("%1 车辆未入库").arg(plate));
        return;
    }

    QDateTime intTime = m_db->getVehicleCheckInTime(plate);
    QDateTime outTime = QDateTime::currentDateTime();

    // 计算总时长
    qint64 totalMinutes = intTime.secsTo(outTime) / 60;
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;

    double cost = Car::calculateFee(intTime, outTime);// 计算费用

    // 构造消息
    QString msg = QStringLiteral("停车时长: %1 小时 %2 分钟, 费用 %3 元").arg(hours).arg(minutes).arg(cost, 0, 'f', 2);
    if(!notifyConfirm(this, QStringLiteral("停车时长与费用"), msg)){
        return;
    }

    if(m_db->checkOut(plate, cost)){
        notifySuccess(this, QStringLiteral("%1 出库成功").arg(plate));
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

    InitFile initFile;
    if(!initFile.loadConfig()){
        qWarning() << "配置文件加载失败";
        return;
    }
    QString name = initFile.getParkingName();
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

    // 在所有子控件上检测鼠标移动，更新拉伸光标
    if (event->type() == QEvent::MouseMove && m_mouseArea == NoArea) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        // 将子控件坐标转换为窗口坐标
        QWidget *widget = qobject_cast<QWidget*>(obj);
        QPoint windowPos = widget ? widget->mapTo(this, mouseEvent->pos()) : mouseEvent->pos();
        MouseArea hover = getMouseArea(windowPos);
        setCursorShape(hover);
    }

    return QMainWindow::eventFilter(obj, event);
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
