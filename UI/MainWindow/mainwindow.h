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
    void startRecognition();
    void stopRecognition();
signals:
    void logoutRequested();
    void appExitRequested();
private slots:
    void updateTime();
    void updateFrame(cv::Mat frame);
    void onCloseButton();
    void onMinButton();
    void onMaxButton();
    void onSetButton();
    void onlogoutButton();
    void onEntrySearchButton();
    void onExitSearchButton();
    void onUpdateParkingCount();

    void onPlateRecognized(const QString &plate, const cv::Mat &plateImg);

    void onAutoRecognizeToggled(bool checked);
private:
    MouseArea getMouseArea(const QPoint &pos) const;
    void setCursorShape(MouseArea area);
    QString getParkingNameFromConfig() const;
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
    DatabaseManager *m_db = nullptr;
    ParkingService *m_parkingSvc = nullptr;
    UserService    *m_userSvc    = nullptr;
    VehicleService *m_vehicleSvc = nullptr;

    FrameQueue *m_frameQueue = nullptr;
    RecognizeThread *m_recognizeThread = nullptr;
    PlateConfirmTracker *m_confirmTracker = nullptr;
    bool m_autoRecognizeEnabled = false;
    VehicleInformation *m_vehicleInfoPage  = nullptr;
    UserManagement *m_userManagementPage  = nullptr;
    CameraManagement *m_cameraManagementPage = nullptr;
};
#endif