#ifndef PLATERECOGNIZE_H
#define PLATERECOGNIZE_H

#include <QObject>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <QImage>

//引入EasyPR库
#include "include/easypr.h"

using namespace easypr;

/**
 * @brief 车牌识别封装类
 */
class PlateRecognize : public QObject
{
    Q_OBJECT
public:
    // 获取单例实例
    static PlateRecognize* instance();

    //加载识别模型
    bool loadModels(const QString &modelPath);

    //执行车牌识别
    int recognizePlate(const cv::Mat &image);//cv::Mat是 OpenCV 里的"图像对象"

    //检查模型是否已加载
    bool isModelsLoaded() const {return m_modelsLoaded;}

    //设置是否显示调试信息
    void setDebugMode(bool show);

    /**
     * @brief 直接访问 EasyPR 识别结果（供 RecognizeThread 使用）
     *
     * 与 recognizePlate() 不同，本方法不发射信号，
     * 而是直接返回识别到的 CPlate 向量，由调用方自行处理。
     *
     * @param image 输入图像
     * @param[out] plates 输出的车牌向量
     * @return 0=成功, -1=失败
     */
    int plateRecognizeAccessor(const cv::Mat &image, std::vector<easypr::CPlate> &plates);

signals:
    //识别完成信号
    void recognizeFinished(cv::Mat plateImg, QString plateStr);

    //识别错误信号
    void recognizeError(QString errorMsg);

    //模型加载状态信号
    void modelLoadStatus(bool loaded, QString message);

public slots:
    //识别槽函数
    void recognizeSlot(cv::Mat rgbImg);

private:
    //构造函数（私有，单例模式）
    explicit PlateRecognize(QObject *parent = nullptr);

    //禁止拷贝构造和赋值操作符
    PlateRecognize(const PlateRecognize&) = delete;
    PlateRecognize& operator=(const PlateRecognize&) = delete;

    //解析车牌字符串
    void parsePlateString(const std::string &plateStr, QString &color, QString &number);

    //EasyPR识别器实例
    easypr::CPlateRecognize m_plateRecognize;

    //模型加载状态
    bool m_modelsLoaded;

    //调试模式
    bool m_debugMode;

    //单例实例
    static PlateRecognize* s_instance;
};

#endif // PLATERECOGNIZE_H
