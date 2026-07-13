#include "recognizethread.h" 
#include "src/camera/framequeue.h"
#include "src/app/platerecognize.h"
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

/*
 * run() 核心循环的执行流程：
 *
 *   ┌───────────────────────────────────────────────┐
 *   │                while (m_running)               │
 *   │                                               │
 *   │  1. m_queue->take(frame, 1500ms)              │
 *   │     ├── 超时(无新帧) → continue 重新等         │
 *   │     └── 取到帧 → 进入步骤 2                    │
 *   │                                               │
 *   │  2. PlateRecognize::instance()->recognizePlate │
 *   │     ├── 返回 0（未识别到车牌）→ continue       │
 *   │     ├── 返回 >0（识别成功）                    │
 *   │     │   → recognizeFinished 信号已被            │
 *   │     │     PlateRecognize 内部发射               │
 *   │     │   → 这里不重复发射，由连接关系处理        │
 *   │     └── 返回 -1（出错）→ continue              │
 *   └───────────────────────────────────────────────┘
 *
 * 关于 PlateRecognize 的信号：
 *   PlateRecognize::recognizePlate() 内部识别成功时会 emit recognizeFinished。
 *   但 PlateRecognize 是单例，在主线程中创建。
 *   这里在工作线程中调用它的方法，信号会在调用线程（即本线程）中发射。
 *   所以我们不依赖 PlateRecognize 的信号，而是自己发射 plateRecognized。
 *
 * 关于 EasyPR 识别结果格式：
 *   PlateRecognize::recognizeFinished 信号携带 plateStr 格式为 "颜色:号码"，
 *   如 "蓝:苏A12345"。我们需要从中解析出纯车牌号用于数据库查询。
 */
void RecognizeThread::run()
{
    qDebug() << QStringLiteral("识别线程启动，线程ID:") << QThread::currentThreadId();

    if(!m_queue){
        qWarning() << QStringLiteral("识别线程启动失败：FrameQueue 未设置");
        return;
    }

    PlateRecognize *recognizer = PlateRecognize::instance();
    if(!recognizer->isModelsLoaded()){
        emit recognizeError(QStringLiteral("EasyPR 模型未加载"));
        return;
    }

    m_running = true;

    while(m_running){
        // 步骤 1：从队列取帧（带超时）
        cv::Mat frame;
        if(!m_queue->take(frame, m_samplingInterval)){
            // 超时，没有新帧可识别，继续等待
            continue;
        }

        if(frame.empty()){
            continue;
        }

        // 步骤 2：调用 EasyPR 识别
        // recognizePlate() 是 CPU 密集型操作（~200-500ms），
        // 在本独立线程中执行，不影响摄像头采集和 UI 响应。
        try{
            // 存储结果
            std::vector<easypr::CPlate> plates;
            int result = recognizer->plateRecognizeAccessor(frame, plates);

            if(result == 0 && !plates.empty()){
                for(size_t i = 0; i < plates.size(); ++i){
                    easypr::CPlate &plate = plates[i];
                    cv::Mat plateImg = plate.getPlateMat();
                    std::string plateStr = plate.getPlateStr();

                    // 解析车牌字符串："蓝:苏A12345" → 取 "苏A12345"
                    QString qPlateStr = QString::fromLocal8Bit(plateStr.c_str());
                    QStringList parts = qPlateStr.split(":");
                    QString plateNumber = (parts.size() >= 2) ? parts[1].trimmed() : qPlateStr;

                    if(!plateNumber.isEmpty()){
                        emit plateRecognized(plateNumber, plateImg.clone());
                        qDebug() << QStringLiteral("识别线程发射结果:") << plateNumber;
                    }
                }
            }
        }catch (const std::exception &e){
            qDebug() << QStringLiteral("识别异常:") << e.what();
        }
    }

    qDebug() << QStringLiteral("识别线程已结束");
}
