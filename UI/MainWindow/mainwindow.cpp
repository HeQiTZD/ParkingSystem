#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "circleprogress.h"

#include <QFile>
#include <QButtonGroup>
#include <QDateTime>

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

#ifdef Q_OS_WIN
/**
 * @brief 根据鼠标在窗口中的位置，返回 Windows 命中区域值
 * @param pos   鼠标在窗口本地坐标系中的位置
 * @param rect  窗口矩形（0,0 为原点）
 * @param borderWidth  边缘拉伸感应宽度（像素）
 * @param titleBarHeight  标题栏高度（像素）
 * @return Windows HT* 命中区域值
 *
 * 优先级：角 > 边 > 标题栏 > 客户区
 */
static int getHitArea(const QPoint &pos, const QRect &rect,
                      int borderWidth, int titleBarHeight)
{
    int x = pos.x();
    int y = pos.y();
    int w = rect.width();
    int h = rect.height();

    bool onLeft   = (x >= 0 && x < borderWidth);
    bool onRight  = (x >= w - borderWidth && x < w);
    bool onTop    = (y >= 0 && y < borderWidth);
    bool onBottom = (y >= h - borderWidth && y < h);

    // 角部优先于边
    if (onTop && onLeft)        return HTTOPLEFT;
    if (onTop && onRight)       return HTTOPRIGHT;
    if (onBottom && onLeft)     return HTBOTTOMLEFT;
    if (onBottom && onRight)    return HTBOTTOMRIGHT;

    // 边
    if (onLeft)                 return HTLEFT;
    if (onRight)                return HTRIGHT;
    if (onTop)                  return HTTOP;
    if (onBottom)               return HTBOTTOM;

    // 标题栏区域（可拖动）
    if (y >= 0 && y < titleBarHeight) return HTCAPTION;

    // 其他区域
    return HTCLIENT;
}
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);

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

    m_parkingChart = new CircleProgress(ui->parkingChartWidget);

    // 手动设置固定位置和大小（x, y, width, height）
    m_parkingChart->setGeometry(30, 10, 120, 120);

    m_parkingChart->setProgress(70);
    m_parkingChart->setUsedText("已用空间");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTime()
{
    QDateTime currenTime = QDateTime::currentDateTime();
    QString timeStr = currenTime.toString("yyyy-MM-dd HH:mm:ss");
    ui->timeLabel->setText(timeStr);
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    // 只处理 Windows 原生消息
    if (eventType != "windows_generic_MSG")
        return QMainWindow::nativeEvent(eventType, message, result);

    MSG *msg = static_cast<MSG*>(message);
    if (msg->message != WM_NCHITTEST)
        return QMainWindow::nativeEvent(eventType, message, result);

    // 获取鼠标全局坐标并转换为窗口本地坐标
    QPoint globalPos(GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam));
    QPoint localPos = mapFromGlobal(globalPos);

    // 窗口矩形（本地坐标系，原点为 0,0）
    QRect rect(0, 0, width(), height());

    // 标题栏高度：topBarWidget 的高度（31px，与 UI 文件一致）
    int titleBarHeight = 31;

    // 获取命中区域
    int hitArea = getHitArea(localPos, rect, m_borderWidth, titleBarHeight);

    // 如果命中标题栏，需要排除按钮区域
    if (hitArea == HTCAPTION) {
        // 检查鼠标是否在窗口控制按钮上
        QWidget *buttons[] = {
            ui->setButton,
            ui->minimizeButton,
            ui->maximizeButton,
            ui->closeButton
        };
        for (QWidget *btn : buttons) {
            // 将按钮坐标转换为相对于 centralwidget 的坐标
            QPoint btnGlobal = btn->mapTo(this, QPoint(0, 0));
            QRect btnRectInWindow(btnGlobal, btn->size());
            if (btnRectInWindow.contains(localPos)) {
                hitArea = HTCLIENT;
                break;
            }
        }
    }

    // 最大化时禁用拉伸，只保留拖动（HTCAPTION）
    if (isMaximized() && hitArea != HTCAPTION && hitArea != HTCLIENT) {
        hitArea = HTCLIENT;
    }

    // 如果是客户区，交给 Qt 默认处理
    if (hitArea == HTCLIENT)
        return QMainWindow::nativeEvent(eventType, message, result);

    *result = hitArea;
    return true;
}
