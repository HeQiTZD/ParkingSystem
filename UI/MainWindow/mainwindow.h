#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <opencv2/core/mat.hpp>

namespace Ui { class MainWindow; }

class CircleProgress;
class CameraThread;
class DatabaseManager;
class ParkingService;
class UserService;
class VehicleService;
class RecognizeThread;
class PlateConfirmTracker;
class FrameQueue;
class VehicleInformation;
class UserManagement;
class CameraManagement;
class QTimer;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr,
                     DatabaseManager *db = nullptr,
                     ParkingService *parkingSvc = nullptr,
                     UserService *userSvc = nullptr,
                     VehicleService *vehicleSvc = nullptr);
    ~MainWindow();

    enum MouseArea {
        NoArea,
        DragArea,
        ResizeTop,
        ResizeBottom,
        ResizeLeft,
        ResizeRight,
        ResizeTopLeft,
        ResizeTopRight,
        ResizeBottomLeft,
        ResizeBottomRight
    };

    void startRecognition();  // 进入主页时自动开启识别
    void stopRecognition();   // 离开主页时自动停止识别

signals:
    void logoutRequested();// 退出登录
    void appExitRequested();// 关闭整个应用

private slots:
    void updateTime();
    void updateFrame(cv::Mat frame);

    void onCloseButton();
    void onMinButton();
    void onMaxButton();
    void onSetButton();
    void onlogoutButton();

    void onEntrySearchButton();// 入库
    void onExitSearchButton();// 出库

    void onUpdateParkingCount();// 更新车位使用情况

    /**
     * @brief 接收自动识别结果
     *
     * 由 RecognizeThread::plateRecognized 信号触发（跨线程 QueuedConnection）。
     * 在主线程中执行：防重复校验 → 自动出入库 → 更新 UI。
     *
     * @param plate 识别到的车牌号
     * @param plateImg 车牌区域图像（预留，暂未使用）
     */
    void onPlateRecognized(const QString &plate, const cv::Mat &plateImg);

    /** @brief 自动识别开关切换 */
    void onAutoRecognizeToggled(bool checked);

private:
    MouseArea getMouseArea(const QPoint &pos) const;
    void setCursorShape(MouseArea area);
    QString getParkingNameFromConfig() const;  // 供 checkIn/getParkingStats 等使用

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Ui::MainWindow *ui;
    QTimer *m_timer;
    CircleProgress *m_parkingChart;
    CameraThread *m_cameraThread;
    QLabel *m_videoLabel;

    QPoint m_dragPos;
    QRect m_resizeGeo;
    MouseArea m_mouseArea = NoArea;
    int m_dragBarHeight = 50;
    static const int kResizeBorder = 5;

    DatabaseManager *m_db = nullptr;          // 过渡: 业务方法暂用
    ParkingService *m_parkingSvc = nullptr;
    UserService    *m_userSvc    = nullptr;
    VehicleService *m_vehicleSvc = nullptr;

    // ===== 自动识别相关 =====
    FrameQueue *m_frameQueue = nullptr;
    RecognizeThread *m_recognizeThread = nullptr;
    PlateConfirmTracker *m_confirmTracker = nullptr;
    bool m_autoRecognizeEnabled = false;

    VehicleInformation *m_vehicleInfoPage  = nullptr;  // 车辆信息页面
    UserManagement *m_userManagementPage  = nullptr;    // 用户管理页面
    CameraManagement *m_cameraManagementPage = nullptr;
};

#endif // MAINWINDOW_H
