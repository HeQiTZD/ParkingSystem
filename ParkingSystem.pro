QT += widgets sql gui core uitools

# 多媒体模块（摄像头相关）
QT += multimedia multimediawidgets

# 图表模块
QT += charts

# SVG模块（图标渲染）
QT += svg

# 作用：指定生成的目标文件名（不含扩展名）
# 结果：在 Windows 上生成 ParkingSystem.exe，在 Linux 上生成 ParkingSystem
TARGET = ParkingSystem

# 作用：启用 C++11 标准，支持 auto、lambda 等特性
CONFIG += c++11

# 源文件和翻译文件的编码格式（解决 qss 等资源文件中文乱码/解析失败）
CODECFORTR = UTF-8
CODECFORSRC = UTF-8

# ==================== 编译警告抑制 ====================
# 去除"未使用参数"警告（第三方库代码会触发）
QMAKE_CXXFLAGS += -Wno-unused-parameter
# 去除"未使用变量"警告
QMAKE_CXXFLAGS += -Wno-unused-variable
# 去除"未使用函数"警告
QMAKE_CXXFLAGS_WARN_ON -= -Wunused-function
QMAKE_CXXFLAGS += -Wno-unused-function

# ==================== OpenMP 并行加速 ====================
# 用于图像处理加速（Sobel、形态学操作等）
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
LIBS += -fopenmp

# Windows 系统库依赖
LIBS += -lmsvcrt -lole32 -luuid

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    # ==================== 核心业务模块 ====================
    UI/Login/logindialog.cpp \
    UI/Login/passwordedit.cpp \
    UI/Login/usernameedit.cpp \
    UI/MainWindow/circleprogress.cpp \
    UI/MainWindow/mainwindow.cpp \
    UI/Register/registerdialog.cpp \
    UI/ConfigInit/configinitdialog.cpp \
    # ==================== 应用程序核心代码 ====================
    src/app/main.cpp \
    src/app/car.cpp \
    src/app/platerecognize.cpp \
    src/app/ApplicationManager.cpp \
    # ==================== 数据库相关 ====================
    src/database/databasemanager.cpp \
    src/database/mysqlinit.cpp \
    # ==================== 摄像头相关 ====================
    src/camera/camerathread.cpp \
    # ==================== 工具和基础设施 ====================
    src/utils/initfile.cpp \
    src/utils/pthreadpool.cpp \
    src/utils/notificationdialog.cpp \
    src/utils/toastwidget.cpp \
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
    # ==================== 核心业务模块 ====================
    UI/Login/logindialog.h \
    UI/Login/passwordedit.h \
    UI/Login/usernameedit.h \
    UI/MainWindow/circleprogress.h \
    UI/MainWindow/mainwindow.h \
    UI/Register/registerdialog.h \
    UI/ConfigInit/configinitdialog.h \
    # ==================== 应用程序核心代码 ====================
    src/app/car.h \
    src/app/platerecognize.h \
    src/app/ApplicationManager.h \
    # ==================== 数据库相关 ====================
    src/database/databasemanager.h \
    src/database/mysqlinit.h \
    # ==================== 摄像头相关 ====================
    src/camera/camerathread.h \
    # ==================== 工具和基础设施 ====================
    src/utils/initfile.h \
    src/utils/pthreadpool.h \
    src/utils/utils.h \
    src/utils/notificationdialog.h \
    src/utils/toastwidget.h \
    src/utils/notification_global.h \
    src/utils/messageType.h \
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
    thirdparty/xmlParser/xmlParser.h

# ==================== OpenCV 库配置 ====================
# 【重要】修改为本机的 OpenCV 安装路径
INCLUDEPATH += C:\OpenCV-MinGW-Build-OpenCV-3.4.8-x64\include \
               C:\OpenCV-MinGW-Build-OpenCV-3.4.8-x64\include\opencv2 \
               C:\OpenCV-MinGW-Build-OpenCV-3.4.8-x64\include\opencv \
               UI\ \
               UI\Login \
               generated \
               .

LIBS += -L C:\OpenCV-MinGW-Build-OpenCV-3.4.8-x64\x64\mingw\lib\libopencv_*.a

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# ==================== UI 文件生成配置 ====================
# 指定 uic 工具生成的 ui_*.h 文件输出目录
UI_DIR = generated

FORMS += \
    UI/Login/logindialog.ui \
    UI/MainWindow/mainwindow.ui \
    UI/Register/registerdialog.ui \
    UI/ConfigInit/configinitdialog.ui

# ==================== DLL 复制配置 ====================
# 构建完成后将 OpenCV DLL 复制到可执行文件目录
OPENCV_DLL_DIR = C:/OpenCV-MinGW-Build-OpenCV-3.4.8-x64/x64/mingw/bin

# 根据构建模式选择目标目录（debug 或 release）
CONFIG(debug, debug|release) {
    DLL_TARGET_DIR = $$OUT_PWD/debug
} else {
    DLL_TARGET_DIR = $$OUT_PWD/release
}

# 复制 OpenCV DLL 文件到可执行文件所在目录（cmd /c 保证在 sh/cmd 终端均可执行）
QMAKE_POST_LINK += $$quote(cmd /c copy /Y \"$$OPENCV_DLL_DIR\\*.dll\" \"$$DLL_TARGET_DIR\" $$escape_expand(\n\t))

RESOURCES += \
    UI/imageQrc/image.qrc \
    styles/styles.qrc