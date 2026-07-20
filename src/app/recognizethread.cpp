#include "recognizethread.h"
#include "src/camera/framequeue.h"
#include "src/app/platerecognize.h"
#include "src/database/dbconnectionpool.h"
#include <QDebug>

RecognizeThread::RecognizeThread(QObject *parent)
{
    // 注册 cv::Mat 类型，保证信号槽跨线程传递时能正确序列化
    qRegisterMetaType<cv::Mat>("cv::Mat");
}

RecognizeThread::~RecognizeThread()
{
    stop();
    wait(1500);
}

void RecognizeThread::setFrameQueue(FrameQueue *queue)
{
    m_queue = queue;
}

void RecognizeThread::setSamplingInterval(int ms)
{
    m_samplingInterval = ms;
}

void RecognizeThread::stop()
{
    m_running = false;
}

void RecognizeThread::run()
{
    qDebug() << QStringLiteral("识别线程启动，线程ID:") << QThread::currentThreadId();

    if(!m_queue){
        qWarning() << QStringLiteral("识别线程启动失败：FrameQueue 未设置");
        return;
    }

    PlateRecognize& recognizer = PlateRecognize::instance();
    if(!recognizer.isModelsLoaded()){
        emit recognizeError(QStringLiteral("HyperLPR 模型未加载"));
        return;
    }

    m_running = true;

    while(m_running){
        // 步骤 1：从队列取帧（带超时）
        cv::Mat frame;
        if(!m_queue->take(frame, m_samplingInterval)){
            continue;
        }

        if(frame.empty()){
            continue;
        }

        // 步骤 2：调用 HyperLPR 识别
        try{
            std::vector<PlateResult> results;
            int ret = recognizer.plateRecognizeAccessor(frame, results);

            if(ret > 0){
                for(size_t i = 0; i < results.size(); ++i){
                    PlateResult &r = results[i];

                    // HyperLPR 直接返回号牌字符串（如 "川A88888"），无需解析 "颜色:号码" 格式
                    QString plateNumber = r.plateStr;

                    if(!plateNumber.isEmpty()){
                        emit plateRecognized(plateNumber, r.plateImg.clone());
                        qDebug() << QStringLiteral("识别线程发射结果:") << plateNumber;
                    }
                }
            }
        }catch (const std::exception &e){
            qDebug() << QStringLiteral("识别异常:") << e.what();
        }
    }

    DbConnectionPool::instance().closeThreadConnection();
    qDebug() << QStringLiteral("识别线程已结束");
}
