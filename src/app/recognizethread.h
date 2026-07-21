#ifndef RECOGNIZETHREAD_H
#define RECOGNIZETHREAD_H
#include <QThread>
#include <opencv2/opencv.hpp>
#include "src/camera/framequeue.h"
class RecognizeThread : public QThread
{
    Q_OBJECT
public:
    explicit RecognizeThread(QObject *parent = nullptr);
    ~RecognizeThread();

    void setFrameQueue(FrameQueue *queue);

    void setSamplingInterval(int ms);

    void stop();
signals:
    void plateRecognized(const QString &plate, const cv::Mat &plateImg);

    void recognizeError(const QString &errorMsg);
protected:
    void run() override;
private:
    FrameQueue *m_queue = nullptr;
    int m_samplingInterval = 1500;
    bool m_running = false;
};
#endif
RECOGNIZETHREAD_H