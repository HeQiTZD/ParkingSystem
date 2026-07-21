#include "platerecognize.h"
#include "thirdparty/hyperlpr/include/Pipeline.h"
#include <QDebug>
#include <QCoreApplication>
PlateRecognize::PlateRecognize(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
    m_modelsLoaded = false;
    m_debugMode = false;
}
PlateRecognize& PlateRecognize::instance()
{
    static PlateRecognize inst;
    return inst;
}
void PlateRecognize::shutdown()
{
    if(m_prc){
        delete m_prc;
        m_prc = nullptr;
    }
    m_modelsLoaded = false;
    qDebug() << "PlateRecognize: shutdown 完成";
}
bool PlateRecognize::loadModels(const QString &modelPath)
{
    try {
        const QString detectProto  = modelPath + "/mininet_ssd_v1.prototxt";
        const QString detectModel  = modelPath + "/mininet_ssd_v1.caffemodel";
        const QString finetuneProto = modelPath + "/refinenet.prototxt";
        const QString finetuneModel = modelPath + "/refinenet.caffemodel";
        const QString recProto     = modelPath + "/SegmenationFree-Inception.prototxt";
        const QString recModel     = modelPath + "/SegmenationFree-Inception.caffemodel";
        const QString cascadeXml   = modelPath + "/cascade_double.xml";

        if (m_prc) {
            delete m_prc;
            m_prc = nullptr;
        }

        m_prc = new pr::PipelinePR(
            detectProto.toStdString(),
            detectModel.toStdString(),
            finetuneProto.toStdString(),
            finetuneModel.toStdString(),
            recProto.toStdString(),
            recModel.toStdString(),
            cascadeXml.toStdString()
        );
        m_modelsLoaded = true;
        qDebug() << QStringLiteral("HyperLPR 模型加载成功:") << modelPath;
        emit modelLoadStatus(true, QStringLiteral("HyperLPR 模型加载成功"));
        return true;
    } catch (const std::exception &e) {
        m_modelsLoaded = false;
        QString errorMsg = QString("模型加载失败：%1").arg(e.what());
        qDebug() << errorMsg;
        emit modelLoadStatus(false, errorMsg);
        return false;
    }
}
int PlateRecognize::recognizePlate(const cv::Mat &image)
{
    if (!m_modelsLoaded || !m_prc) {
        emit recognizeError(QStringLiteral("模型未加载，请先调用 loadModels()"));
        return -1;
    }
    if (image.empty()) {
        emit recognizeError(QStringLiteral("输入图像为空"));
        return -1;
    }
    try {
        cv::Mat localImg = image.clone();

        std::vector<pr::PlateInfo> results = m_prc->RunPiplineAsImage(localImg, 0);
        if (results.empty()) {
            qDebug() << QStringLiteral("未识别到车牌");
            return 0;
        }
        for (auto &plate : results) {
            if (plate.confidence > 0.5f) {
                cv::Mat plateImg = plate.getPlateImage();
                QString plateStr = QString::fromStdString(plate.getPlateName());
                qDebug() << QStringLiteral("HyperLPR 识别:") << plateStr
                         << QStringLiteral("置信度:") << plate.confidence;
                emit recognizeFinished(plateImg, plateStr);
            }
        }
        return static_cast<int>(results.size());
    } catch (const std::exception &e) {
        QString errorMsg = QString("识别出错：%1").arg(e.what());
        qDebug() << errorMsg;
        emit recognizeError(errorMsg);
        return -1;
    }
}
void PlateRecognize::setDebugMode(bool show)
{
    m_debugMode = show;
}
void PlateRecognize::recognizeSlot(cv::Mat rgbImg)
{
    recognizePlate(rgbImg);
}
int PlateRecognize::plateRecognizeAccessor(const cv::Mat &image,
                                           std::vector<PlateResult> &results)
{
    if (!m_modelsLoaded || !m_prc || image.empty()) {
        return -1;
    }
    try {
        cv::Mat localImg = image.clone();
        std::vector<pr::PlateInfo> plates = m_prc->RunPiplineAsImage(localImg, 0);
        for (auto &p : plates) {
            if (p.confidence > 0.5f) {
                PlateResult r;
                r.plateImg = p.getPlateImage();
                r.plateStr = QString::fromStdString(p.getPlateName());
                r.score = p.confidence;
                results.push_back(r);
            }
        }
        return static_cast<int>(results.size());
    } catch (const std::exception &e) {
        qDebug() << QStringLiteral("plateRecognizeAccessor 异常:") << e.what();
        return -1;
    }
}