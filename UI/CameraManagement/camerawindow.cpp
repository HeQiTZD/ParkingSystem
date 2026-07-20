#include "camerawindow.h"
#include "src/camera/camerathread.h"
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QImage>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

CameraWindow::CameraWindow(QWidget *parent) : QFrame(parent)
{
    setObjectName("cameraWindow");
    setAttribute(Qt::WA_StyledBackground, true);

    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setScaledContents(false);
    m_videoLabel->setStyleSheet("background: transparent;");

    m_infoLabel = new QLabel(this);
    m_infoLabel->setObjectName("cameraInfoPanel");
    m_infoLabel->raise();

    m_statusDot = new QLabel(this);
    m_statusDot->setObjectName("cameraStatusDot");
    m_statusDot->setFixedSize(12, 12);
    m_statusDot->raise();

    showEmptyState(QStringLiteral("Signal Lost / Awaiting Connection"));
}

CameraWindow::~CameraWindow()
{
    if(m_thread){
        disconnect(m_thread, nullptr, this, nullptr);
    }
}

void CameraWindow::bind(CameraThread *thread, const CameraInfo &info)
{
    if(m_thread){
        disconnect(m_thread, nullptr, this, nullptr);
    }
    m_thread = thread;
    m_info = info;

    if(m_thread){
        connect(m_thread, &CameraThread::newFrameCaptured,
                this, &CameraWindow::onFrame);
        connect(m_thread, &CameraThread::cameraStatusChanged,
                this, &CameraWindow::onStatus);
        connect(m_thread, &CameraThread::fpsUpdated,
                this, &CameraWindow::onFps);

        m_infoLabel->setText(QStringLiteral("%1 · %2x%3 · %4 FPS")
                                 .arg(m_info.name)
                                 .arg(m_info.width).arg(m_info.height)
                                 .arg(m_info.fps));
        m_connected = false;
        m_statusDot->setProperty("status", "offline");
        m_statusDot->style()->unpolish(m_statusDot);
        m_statusDot->style()->polish(m_statusDot);
    }
}

void CameraWindow::onFrame(cv::Mat frame)
{
    if(frame.empty()) return;
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    QImage img(rgb.data, rgb.cols, rgb.rows,
               static_cast<int>(rgb.step), QImage::Format_RGB888);
    QPixmap pm = QPixmap::fromImage(img.copy());
    m_videoLabel->setPixmap(pm.scaled(m_videoLabel->size(),
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation));
}

void CameraWindow::onStatus(bool connected, const QString &msg)
{
    Q_UNUSED(msg);
    m_connected = connected;
    m_statusDot->setProperty("status", connected ? "online" : "offline");
    m_statusDot->style()->unpolish(m_statusDot);
    m_statusDot->style()->polish(m_statusDot);

    if(!connected){
        showEmptyState(QStringLiteral("Signal Lost / Awaiting Connection"));
    }
}

void CameraWindow::onFps(int fps)
{
    m_info.fps = fps;
    m_infoLabel->setText(QStringLiteral("%1 · %2x%3 · %4 FPS")
                             .arg(m_info.name)
                             .arg(m_info.width).arg(m_info.height)
                             .arg(fps));
}

void CameraWindow::showEmptyState(const QString &text)
{
    m_videoLabel->setText(
        QStringLiteral("<div style='text-align:center;'>"
                       "<img src=':/icons/videocam_off.svg' width='48' height='48'/><br>"
                       "<span style='color:#94A3B8; font-size:13px; letter-spacing:2px;'>%1</span>"
                       "</div>").arg(text));
}

void CameraWindow::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    QSize s = size();
    m_videoLabel->setGeometry(rect());
    m_infoLabel->adjustSize();
    int panelW = m_infoLabel->width() + 24;
    int panelH = m_infoLabel->height() + 16;
    m_infoLabel->setGeometry(8, 8, panelW, panelH);
    m_statusDot->setGeometry(s.width() - 24, 12, 12, 12);
}

void CameraWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit doubleClicked();
}
