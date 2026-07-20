#ifndef PLATERECOGNIZE_H
#define PLATERECOGNIZE_H

#include <QObject>
#include <QString>
#include <opencv2/core.hpp>
#include <vector>

// ── 替换 EasyPR 的类型：轻量结果结构体 ──
struct PlateResult {
    cv::Mat plateImg;     // 车牌裁剪图像
    QString  plateStr;    // 车牌号（如 "川A88888"，HyperLPR 直接返回）
    float    score = 0.0f;// 置信度 0~1
};

// ── 前向声明 HyperLPR（避免在头文件暴露 Pipeline.h 中的 using namespace） ──
namespace pr {
    class PipelinePR;
    class PlateInfo;
}

class PlateRecognize : public QObject
{
    Q_OBJECT
public:
    static PlateRecognize& instance();

    // 加载 HyperLPR 模型（modelPath = 包含 prototxt/caffemodel 的目录）
    bool loadModels(const QString &modelPath);

    // 识别图像 → 发射 recognizeFinished 信号，返回识别到的车牌数量
    int recognizePlate(const cv::Mat &image);

    bool isModelsLoaded() const { return m_modelsLoaded; }

    // 幂等释放模型资源(shutdown 序列调用)
    void shutdown();
    void setDebugMode(bool show);

    // 直接获取识别结果（供 RecognizeThread 使用）
    int plateRecognizeAccessor(const cv::Mat &image, std::vector<PlateResult> &results);

signals:
    void recognizeFinished(cv::Mat plateImg, QString plateStr);
    void recognizeError(QString errorMsg);
    void modelLoadStatus(bool loaded, QString message);

public slots:
    void recognizeSlot(cv::Mat rgbImg);

private:
    explicit PlateRecognize(QObject *parent = nullptr);
    PlateRecognize(const PlateRecognize&) = delete;
    PlateRecognize& operator=(const PlateRecognize&) = delete;

    // ── 成员变量 ──
    pr::PipelinePR *m_prc = nullptr;   // HyperLPR 流水线（替换 EasyPR CPlateRecognize）
    bool m_modelsLoaded = false;
    bool m_debugMode = false;
};

#endif // PLATERECOGNIZE_H
