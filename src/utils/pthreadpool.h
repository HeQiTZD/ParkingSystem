#ifndef PTHREADPOOL_H
#define PTHREADPOOL_H
#include <QObject>
#include <QThreadPool>
#include <QRunnable>
#include <QMutex>
#include <opencv2/opencv.hpp>
class RecognitionTask : public QRunnable
{
public:
    explicit RecognitionTask(int taskId, const cv::Mat &frame);

    ~RecognitionTask() override;

    int getTaskId() const {return m_taskId;}
protected:
    void run() override;
private:
    int m_taskId;
    cv::Mat m_frame;
};
class ThreadPoolManager : public QObject{
    Q_OBJECT
public:
    static ThreadPoolManager& instance();
    void init(int maxThreadCount = QThread::idealThreadCount());
    int submitTask(const cv::Mat &frame);
    int activeThreadCount() const;
    void waitForDone(int timeout = -1);
    bool isInitialized() const {return m_initialized;}

    void shutdown();
signals:
    void taskFinished(int taskId, bool success, const QString &errorMsg = "");

    void statusChanged(int activeCount);
private:
    explicit ThreadPoolManager(QObject* parent = nullptr);

    ThreadPoolManager(const ThreadPoolManager&) = delete;
    ThreadPoolManager operator = (const ThreadPoolManager&) = delete;

    ~ThreadPoolManager();

    QThreadPool *m_threadPool;
    QMutex m_mutex;
    int m_taskCounter;
    bool m_initialized;
};
#endif