#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "circleprogress.h"
#include "src/camera/camerathread.h"
#include "src/database/databasemanager.h"
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

private:
    MouseArea getMouseArea(const QPoint &pos) const;
    void setCursorShape(MouseArea area);

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
};

#endif // MAINWINDOW_H
