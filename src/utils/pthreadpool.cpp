#include "pthreadpool.h"
#include "src/app/platerecognize.h"
#include <QDebug>
#include <QThread>
RecognitionTask::RecognitionTask(int taskId, const cv::Mat &frame)
    : m_taskId(taskId)
    , m_frame(frame)
{
    setAutoDelete(true);
}
RecognitionTask::~RecognitionTask()
{
    qDebug() << "任务" << m_taskId << "已销毁";
}
void RecognitionTask::run()
{
    qDebug() << "任务" << m_taskId << QStringLiteral("开始执行，线程ID") << QThread::currentThreadId();
    bool success = false;
    QString errorMsg;
    try{
        PlateRecognize& recognizer = PlateRecognize::instance();
        int result = recognizer.recognizePlate(m_frame);
        if(result > 0){
            qDebug() << "任务" << m_taskId << "识别成功，发现" << result << "个车牌";
            success = true;
        }else if(result == 0){
            qDebug() << "任务" << m_taskId << "未识别到车牌";
            success = true;
        }else{
            qDebug() << "任务" << m_taskId << "识别失败";
            errorMsg = "识别失败";
        }
    }catch(const std::exception &e){
        qDebug() << "任务" << m_taskId << "发生异常:" << e.what();
        errorMsg = QString::fromStdString(e.what());
    }catch(...){
        qDebug() << "任务" << m_taskId << "发生未知异常";
        errorMsg = "未知异常";
    }

    emit ThreadPoolManager::instance().taskFinished(m_taskId, success, errorMsg);
    emit ThreadPoolManager::instance().statusChanged(
        ThreadPoolManager::instance().activeThreadCount());
    qDebug() << "任务" << m_taskId << "执行完成";
}
ThreadPoolManager& ThreadPoolManager::instance()
{
    static ThreadPoolManager inst;
    return inst;
}
void ThreadPoolManager::init(int maxThreadCount)
{
    if(m_initialized){
        qDebug() << QStringLiteral("线程池已初始化，跳过");
        return;
    }
    m_threadPool = new QThreadPool();
    m_threadPool->setMaxThreadCount(maxThreadCount);
    m_initialized = true;
    qDebug() << QStringLiteral("线程池初始化完成，最大线程数:") << maxThreadCount;
}
int ThreadPoolManager::submitTask(const cv::Mat &frame)
{
    if(!m_initialized){
        qDebug() << QStringLiteral("线程池未初始化");
        return -1;
    }
    if(frame.empty()){
        qDebug() << QStringLiteral("提交空图像，任务被拒绝");
        return -1;
    }

    int taskId;
    {
        QMutexLocker locker(&m_mutex);
        taskId = ++m_taskCounter;
    }

    RecognitionTask* task = new RecognitionTask(taskId, frame);
    m_threadPool->start(task);
    qDebug() << "任务" << taskId << QStringLiteral("已提交");

    emit statusChanged(m_threadPool->activeThreadCount());
    return taskId;
}
int ThreadPoolManager::activeThreadCount() const
{
    if(!m_initialized || !m_threadPool){
        return 0;
    }
    return m_threadPool->activeThreadCount();
}
void ThreadPoolManager::waitForDone(int timeout)
{
    if(!m_initialized || !m_threadPool){
        return;
    }
    if(timeout < 0){
        m_threadPool->waitForDone();
    }else{
        m_threadPool->waitForDone(timeout);
    }
    qDebug() << QStringLiteral("所有任务已完成");
}
ThreadPoolManager::ThreadPoolManager(QObject *parent)
    : QObject(parent)
    , m_threadPool(nullptr)
    , m_taskCounter(0)
    , m_initialized(false)
{}
ThreadPoolManager::~ThreadPoolManager()
{
    shutdown();
}
void ThreadPoolManager::shutdown()
{
    if(!m_initialized) return;
    if(m_threadPool){
        m_threadPool->waitForDone();
        delete m_threadPool;
        m_threadPool = nullptr;
    }
    m_initialized = false;
    qDebug() << "ThreadPoolManager: shutdown 完成";
}