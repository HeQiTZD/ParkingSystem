#ifndef PLATERECOGNIZE_H
#define PLATERECOGNIZE_H
#include <QObject>
#include <QString>
#include <opencv2/core.hpp>
#include <vector>
struct PlateResult {
    cv::Mat plateImg;
    QString  plateStr;
    float    score = 0.0f;
};
namespace pr {
    class PipelinePR;
    class PlateInfo;
}
class PlateRecognize : public QObject
{
    Q_OBJECT
public:
    static PlateRecognize& instance();

    bool loadModels(const QString &modelPath);

    int recognizePlate(const cv::Mat &image);
    bool isModelsLoaded() const { return m_modelsLoaded; }

    void shutdown();
    void setDebugMode(bool show);

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

    pr::PipelinePR *m_prc = nullptr;
    bool m_modelsLoaded = false;
    bool m_debugMode = false;
};
#endif