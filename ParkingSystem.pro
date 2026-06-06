QT += widgets sql gui core uitools

#多媒体模块
QT += multimedia multimediawidgets

#图表模块
QT += charts

#作用：指定生成的目标文件名（不含扩展名）。
#结果：在 Windows 上生成 ParkingSystem.exe，在 Linux 上生成 ParkingSystem（可执行文件），或生成 libParkingSystem.so（如果是库）。
TARGET = ParkingSystem

#作用：启用 C++11 标准 支持。
#效果：qmake 会根据编译器自动添加对应的编译选项（如 GCC/Clang 的 -std=c++11，MSVC 的 /std:c++11 等效项）。这是推荐的标准写法。
CONFIG += c++11

# ==================== 警告抑制 ====================
# 去除"未使用参数"警告（第三方库代码会触发）
QMAKE_CXXFLAGS += -Wno-unused-parameter
# 去除"未使用变量"警告
QMAKE_CXXFLAGS += -Wno-unused-variable
# 去除"未使用函数"警告
QMAKE_CXXFLAGS_WARN_ON -= -Wunused-function
QMAKE_CXXFLAGS += -Wno-unused-function

# ==================== OpenMP 并行支持 ====================
# 用于图像处理加速（Sobel、形态学操作等）
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
LIBS += -fopenmp

# Windows系统库依赖
LIBS += -lmsvcrt -lole32 -luuid

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    # ==================== 业务模块（已实现） ====================
    initfile.cpp \
    main.cpp \
    mainwindow.cpp \
    mysqlinit.cpp \
    platerecognize.cpp \
    registerwindow.cpp \
    databasemanager.cpp \
    # ==================== EasyPR 核心识别库（第三方库） ====================
    src/core/chars_identify.cpp \
    src/core/chars_recognise.cpp \
    src/core/chars_segment.cpp \
    src/core/core_func.cpp \
    src/core/feature.cpp \
    src/core/params.cpp \
    src/core/plate_detect.cpp \
    src/core/plate_judge.cpp \
    src/core/plate_locate.cpp \
    src/core/plate_recognize.cpp \
    # ==================== EasyPR 工具库（第三方库） ====================
    src/util/kv.cpp \
    src/util/program_options.cpp \
    src/util/util.cpp \
    # ==================== 第三方依赖库 ====================
    thirdparty/LBP/helper.cpp \
    thirdparty/LBP/lbp.cpp \
    thirdparty/mser/mser2.cpp \
    thirdparty/svm/corrected_svm.cpp \
    thirdparty/textDetect/erfilter.cpp \
    thirdparty/xmlParser/xmlParser.cpp

HEADERS += \
    # ==================== 业务模块（已实现） ====================
    initfile.h \
    mainwindow.h \
    mysqlinit.h \
    platerecognize.h \
    registerwindow.h \
    databasemanager.h \
    # ==================== EasyPR 核心头文件（第三方库） ====================
    include/easypr/core/character.hpp \
    include/easypr/core/chars_identify.h \
    include/easypr/core/chars_recognise.h \
    include/easypr/core/chars_segment.h \
    include/easypr/core/core_func.h \
    include/easypr/core/feature.h \
    include/easypr/core/params.h \
    include/easypr/core/plate.hpp \
    include/easypr/core/plate_detect.h \
    include/easypr/core/plate_judge.h \
    include/easypr/core/plate_locate.h \
    include/easypr/core/plate_recognize.h \
    # ==================== EasyPR 工具头文件（第三方库） ====================
    include/easypr/util/kv.h \
    include/easypr/util/program_options.h \
    include/easypr/util/switch.hpp \
    include/easypr/util/util.h \
    include/easypr/api.hpp \
    include/easypr/config.h \
    include/easypr/version.h \
    include/easypr.h \
    # ==================== 第三方依赖库头文件 ====================
    thirdparty/LBP/helper.hpp \
    thirdparty/LBP/lbp.hpp \
    thirdparty/mser/mser2.hpp \
    thirdparty/svm/precomp.hpp \
    thirdparty/textDetect/erfilter.hpp \
    thirdparty/xmlParser/xmlParser.h \
    utils.h

FORMS += \
    mainwindow.ui \
    registerwindow.ui

# ==================== OpenCV 库配置 ====================
# 【重要】修改为本机的OpenCV安装路径
INCLUDEPATH += C:\OpenCV-MinGW-Build-OpenCV-3.4.8-x64\include \
               C:\OpenCV-MinGW-Build-OpenCV-3.4.8-x64\include\opencv2 \
               C:\OpenCV-MinGW-Build-OpenCV-3.4.8-x64\include\opencv
LIBS += -L C:\OpenCV-MinGW-Build-OpenCV-3.4.8-x64\x64\mingw\lib\libopencv_*.a

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
