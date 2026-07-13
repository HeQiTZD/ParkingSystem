#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "circleprogress.h"
#include "vehicleentryexitwidget.h"
#include "src/camera/camerathread.h"
#include "src/database/databasemanager.h"
#include "src/app/recognizethread.h"
#include "src/app/plateconfirmtracker.h"
#include "src/camera/framequeue.h"
#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QCursor>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, DatabaseManager *db = nullptr);
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

    DatabaseManager *m_db;

    // ===== 自动识别相关 =====
    FrameQueue *m_frameQueue = nullptr;
    RecognizeThread *m_recognizeThread = nullptr;
    PlateConfirmTracker *m_confirmTracker = nullptr;
    bool m_autoRecognizeEnabled = false;
};

#endif // MAINWINDOW_H
