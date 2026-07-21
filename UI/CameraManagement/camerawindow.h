#ifndef CAMERAWINDOW_H
#define CAMERAWINDOW_H
#include <QFrame>
#include <opencv2/core/mat.hpp>
#include "src/camera/camerainfo.h"
class QLabel;
class CameraThread;
class CameraWindow : public QFrame
{
    Q_OBJECT
public:
    explicit CameraWindow(QWidget *parent = nullptr);
    ~CameraWindow();
    void bind(CameraThread *thread, const CameraInfo &info);
signals:
    void doubleClicked();
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
private slots:
    void onFrame(cv::Mat frame);
    void onStatus(bool connected, const QString &msg);
    void onFps(int fps);
private:
    void showEmptyState(const QString &text);
    QLabel      *m_videoLabel = nullptr;
    QLabel      *m_infoLabel  = nullptr;
    QLabel      *m_statusDot  = nullptr;
    CameraThread *m_thread    = nullptr;
    CameraInfo   m_info;
    bool         m_connected = false;
};
#endif