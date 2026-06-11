#include "pthreadpool.h"
#include "platerecognize.h"
#include <QDebug>
#include <QThread>

// ========== RecognitionTask 实现 ==========
RecognitionTask::RecognitionTask(int taskId, const cv::Mat &frame)
    : m_taskId(taskId)
    , m_frame(frame)
{
    // 设置为自动删除，任务完成后自动释放内存
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
        // 获取车牌识别器实例
        PlateRecognize *recognizer = PlateRecognize::instance();

        // 执行识别
        int result = recognizer->recognizePlate(m_frame);

        if(result > 0){
            qDebug() << "任务" << m_taskId << "识别成功，发现" << result << "个车牌";
            success = true;
        }else if(result == 0){
            qDebug() << "任务" << m_taskId << "未识别到车牌";
            success = true;  // 未识别到车牌也是正常结果
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

    // 发射任务完成信号
    emit ThreadPoolManager::instance()->taskFinished(m_taskId, success, errorMsg);

    // 发射状态变化信号
    emit ThreadPoolManager::instance()->statusChanged(
        ThreadPoolManager::instance()->activeThreadCount());

    qDebug() << "任务" << m_taskId << "执行完成";
}

// ========== ThreadPoolManager 实现 ==========

// 初始化静态成员变量
ThreadPoolManager* ThreadPoolManager::s_instance = nullptr;


ThreadPoolManager *ThreadPoolManager::instance()
{
    if(s_instance == nullptr){
        s_instance = new ThreadPoolManager();
    }
    return s_instance;
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

    // 生成任务ID
    int taskId;
    {
        QMutexLocker locker(&m_mutex);
        taskId = ++m_taskCounter;
    }

    // 创建任务并提交到线程池（Qt自动管理队列和调度）
    RecognitionTask* task = new RecognitionTask(taskId, frame);
    m_threadPool->start(task);

    qDebug() << "任务" << taskId << QStringLiteral("已提交");

    // 发射状态变化信号
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
    if(m_threadPool){
        m_threadPool->waitForDone();
        delete m_threadPool;
        m_threadPool = nullptr;
    }
}