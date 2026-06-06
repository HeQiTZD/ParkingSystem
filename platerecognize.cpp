#include "platerecognize.h"

#include <QDebug>

//初始化静态成员变量
PlateRecognize* PlateRecognize::s_instance = nullptr;

PlateRecognize::PlateRecognize(QObject *parent) : QObject(parent)
{
    //注册Mat类型，用于信号槽传递
    qRegisterMetaType<cv::Mat>("cv::Mat");

    m_modelsLoaded = false;
    m_debugMode = false;
}

//解析车牌字符串
void PlateRecognize::parsePlateString(const std::string &plateStr, QString &color, QString &number)
{
    QString qPlateStr = QString::fromLocal8Bit(plateStr.c_str());
    QStringList list = qPlateStr.split(":");

    if(list.size() >= 2){
        color = list[0].trimmed();
        number = list[1].trimmed();
    }else{
        color = "未知";
        number = qPlateStr;
    }
}

//单例模式实现
PlateRecognize* PlateRecognize::instance()
{
    if(s_instance == nullptr){
        s_instance = new PlateRecognize();
    }
    return s_instance;
}

// 加载模型文件
bool PlateRecognize::loadModels(const QString &modelPath)
{
    try{
        // 1. 加载SVM模型（用于车牌/非车牌判断）
        QString svmPath = modelPath + "/svm_hist.xml";
        m_plateRecognize.LoadSVM(svmPath.toStdString());
        qDebug() << QStringLiteral("SVM模型加载成功:") <<svmPath;

        // 2. 加载ANN模型（用于字符识别）
        QString annPath = modelPath + "/ann.xml";
        m_plateRecognize.LoadANN(annPath.toStdString());
        qDebug() << QStringLiteral("ANN模型加载成功:") << annPath;

        // 3. 加载中文ANN模型（用于省份汉字识别）
        QString chinesePath = modelPath + "/ann_chinese.xml";
        m_plateRecognize.LoadChineseANN(chinesePath.toStdString());
        qDebug() << QStringLiteral("中文ANN模型加载成功:") << chinesePath;

        // 4. 加载灰度字符ANN模型
        QString grayChAnnPath = modelPath + "/annCh.xml";
        m_plateRecognize.LoadGrayChANN(grayChAnnPath.toStdString());
        qDebug() << QStringLiteral("灰度字符ANN模型加载成功:") << grayChAnnPath;

        // 5. 加载省份映射文件
        QString mappingPath = modelPath + "/province_mapping";
        m_plateRecognize.LoadChineseMapping(mappingPath.toStdString());
        qDebug() << QStringLiteral("省份映射文件加载成功:") << mappingPath;

        m_modelsLoaded = true;
        emit modelLoadStatus(true,QStringLiteral("使用模型加载成功"));
        return true;
    }catch (const std::exception &e){
        m_modelsLoaded = false;
        QString errorMsg = QString("模型加载失败：%1").arg(e.what());
        qDebug() << errorMsg;
        emit modelLoadStatus(false,errorMsg);
        return false;
    }
}

// 执行车牌识别
int PlateRecognize::recognizePlate(const Mat &image)
{
    if(!m_modelsLoaded){
        emit recognizeError(QStringLiteral("模型未加载，请先调用loadModels()"));
        return -1;
    }

    if(image.empty()){
        emit recognizeError(QStringLiteral("输入图像为空"));
        return -1;
    }

    try{
        // 克隆图像，避免修改原图
        Mat localImag = image.clone();

        // 存储识别结果
        std::vector<CPlate> plates;

        // 调用EasyPR进行识别
        // 参数：输入图像，输出车牌向量，图像索引
        int result = m_plateRecognize.plateRecognize(localImag, plates, 0);

        if(result == 0 && plates.size() >0){
            //遍历所有识别到的车牌
            for(size_t i = 0; i < plates.size(); i++){
                CPlate &plate = plates[i];

                //获取车牌图像
                cv::Mat plateImg = plate.getPlateMat();

                //获取车牌字符串（格式：“颜色:号码”）
                std::string plateStr = plate.getPlateStr();

                //解析车牌信息
                QString color, number;
                parsePlateString(plateStr, color, number);

                qDebug() << QStringLiteral("识别结果 - 颜色:") << color << QStringLiteral("号码:") << number;

                //发射识别完成信号
                emit recognizeFinished(plateImg, color + ":" + number);
            }
            return static_cast<int>(plates.size());
        }else{
            qDebug() << QStringLiteral("未识别到车牌");
            return 0;
        }
    }catch (const std::exception &e){
        QString errorMsg = QString("识别出错：%1").arg(e.what());
        qDebug() << errorMsg;
        emit recognizeError(errorMsg);
        return -1;
    }
}

void PlateRecognize::setDebugMode(bool show)
{
    m_debugMode = show;
    m_plateRecognize.setResultShow(show);
}

void PlateRecognize::recognizeSlot(Mat rgbImg)
{
    recognizePlate(rgbImg);
}

/*
1. 单例模式 (Singleton Pattern)

// 单例模式的三个要素：
// 1. 静态成员变量存储唯一实例
static PlateRecognize* s_instance;

// 2. 私有构造函数防止外部创建
explicit PlateRecognize(QObject *parent = nullptr);

// 3. 静态方法获取实例
static PlateRecognize* instance();
作用：确保整个应用程序中只有一个识别实例，避免重复加载模型浪费内存。

2. Qt信号槽机制 (Signals & Slots)

signals:
    void recognizeFinished(cv::Mat plateImg, QString plateStr);

public slots:
    void recognizeSlot(cv::Mat rgbImg);
作用：

信号 (Signal)：当事件发生时发射，类似"广播"
槽 (Slot)：接收信号的函数，类似"接收器"
实现对象间的松耦合通信
3. 元类型注册 (Meta Type Registration)

qRegisterMetaType<cv::Mat>("cv::Mat");
作用：Qt的信号槽系统需要知道如何传递自定义类型。cv::Mat是OpenCV的矩阵类，需要注册后才能在信号槽中使用。

4. 异常处理 (Exception Handling)

try {
    // 可能出错的代码
    m_plateRecognize.LoadSVM(svmPath.toStdString());
}
catch (const std::exception &e) {
    // 捕获并处理异常
    QString errorMsg = QString("模型加载失败: %1").arg(e.what());
}
作用：防止程序因错误而崩溃，提供优雅的错误处理机制。

5. EasyPR识别流程

输入图像
    ↓
┌─────────────────────────────────────┐
│  1. 车牌定位 (Plate Locate)         │
│     - Sobel边缘检测                 │
│     - 颜色筛选                      │
│     - MSER区域检测                  │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│  2. 车牌判断 (Plate Judge)          │
│     - SVM分类器                     │
│     - 置信度评估                    │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│  3. 字符分割 (Chars Segment)        │
│     - 倾斜校正                      │
│     - 垂直投影分割                  │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│  4. 字符识别 (Chars Identify)       │
│     - ANN神经网络                   │
│     - 特征提取与分类                │
└─────────────────────────────────────┘
    ↓
识别结果
6. 模型文件说明
模型文件	用途	算法
svm_hist.xml	车牌/非车牌分类	支持向量机 (SVM)
ann.xml	字符识别	人工神经网络 (ANN)
ann_chinese.xml	省份汉字识别	人工神经网络 (ANN)
annCh.xml	灰度字符识别	人工神经网络 (ANN)
province_mapping	省份代码映射	文本文件
7. 关键数据结构

// CPlate类 - 存储车牌信息
class CPlate {
    Mat m_plateMat;      // 车牌图像
    RotatedRect m_platePos;  // 车牌位置（带旋转角度的矩形）
    String m_plateStr;   // 车牌字符串
    Color m_plateColor;  // 车牌颜色（蓝/黄/白）
    double m_score;      // 置信度分数
};

// Color枚举 - 车牌颜色
enum Color { BLUE, YELLOW, WHITE, UNKNOWN };
*/