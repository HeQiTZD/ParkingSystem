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

# 生成文件归类到子目录，避免 Makefile 依赖追踪失准导致 moc 过时
MOC_DIR     = generated/moc
OBJECTS_DIR = generated/obj
RCC_DIR     = generated/rcc

# ── 强制指定 MinGW 13.1.0 编译器（避免 PATH 中的 mingw810_64 / GCC 8.1 误链接） ──
QMAKE_CXX = C:/Qt/Tools/mingw1310_64/bin/g++.exe
QMAKE_LINK = C:/Qt/Tools/mingw1310_64/bin/g++.exe

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
    UI/MainWindow/vehicleentryexitwidget.cpp \
    UI/UserManager/usermanagement.cpp \
    UI/VehicleInformation/vehicleinformation.cpp \
    UI/Register/registerdialog.cpp \
    UI/ConfigInit/configinitdialog.cpp \
    UI/Settings/settingsdialog.cpp \
    UI/CameraManagement/cameramanagement.cpp \
    UI/CameraManagement/camerawindow.cpp \
    UI/CameraManagement/camerasettingsdialog.cpp \
    # ==================== 业务服务层 ====================
    src/service/parkingservice.cpp \
    src/service/userservice.cpp \
    src/service/vehicleservice.cpp \
    # ==================== 应用程序核心代码 ====================
    src/app/main.cpp \
    src/app/car.cpp \
    src/app/platerecognize.cpp \
    src/app/ApplicationManager.cpp \
    src/app/recognizethread.cpp \
    src/app/plateconfirmtracker.cpp \
    # ==================== 数据库相关 ====================
    src/database/databasemanager.cpp \
    src/database/dbconnectionpool.cpp \
    src/database/mysqlinit.cpp \
    # ==================== 摄像头相关 ====================
    src/camera/camerathread.cpp \
    src/camera/framequeue.cpp \
    src/camera/cameramanager.cpp \
    # ==================== 工具和基础设施 ====================
    src/utils/initfile.cpp \
    src/utils/pthreadpool.cpp \
    src/utils/notificationdialog.cpp \
    src/utils/toastwidget.cpp \
    src/utils/iconlineedit.cpp \
    src/utils/datelineedit.cpp \
    src/utils/customdatechooser.cpp \
    src/utils/calendardaybtn.cpp \
    src/utils/yearmonthpopup.cpp \
    src/utils/timeselector.cpp \
    src/utils/paginationwidget.cpp \
    # ==================== HyperLPR-2 车牌识别库（替换 EasyPR） ====================
    thirdparty/hyperlpr/src/Pipeline.cpp \
    thirdparty/hyperlpr/src/PlateDetection.cpp \
    thirdparty/hyperlpr/src/PlateRecognation.cpp \
    thirdparty/hyperlpr/src/FineTune.cpp

HEADERS += \
    # ==================== 核心业务模块 ====================
    UI/Login/logindialog.h \
    UI/Login/passwordedit.h \
    UI/Login/usernameedit.h \
    UI/MainWindow/circleprogress.h \
    UI/MainWindow/mainwindow.h \
    UI/MainWindow/vehicleentry.h \
    UI/MainWindow/vehicleentryexitwidget.h \
    UI/UserManager/usermanagement.h \
    UI/VehicleInformation/vehicleinformation.h \
    UI/Register/registerdialog.h \
    UI/ConfigInit/configinitdialog.h \
    UI/Settings/settingsdialog.h \
    UI/CameraManagement/cameramanagement.h \
    UI/CameraManagement/camerawindow.h \
    UI/CameraManagement/camerasettingsdialog.h \
    # ==================== 业务服务层 ====================
    src/service/parkingservice.h \
    src/service/userservice.h \
    src/service/vehicleservice.h \
    # ==================== 应用程序核心代码 ====================
    src/app/car.h \
    src/app/platerecognize.h \
    src/app/ApplicationManager.h \
    src/app/recognizethread.h \
    src/app/plateconfirmtracker.h \
    # ==================== 数据库相关 ====================
    src/database/dbconfig.h \
    src/database/dbconnectionpool.h \
    src/database/databasemanager.h \
    src/database/mysqlinit.h \
    # ==================== 摄像头相关 ====================
    src/camera/camerathread.h \
    src/camera/camerainfo.h \
    src/camera/framequeue.h \
    src/camera/cameramanager.h \
    # ==================== 工具和基础设施 ====================
    src/utils/initfile.h \
    src/utils/pthreadpool.h \
    src/utils/utils.h \
    src/utils/notificationdialog.h \
    src/utils/toastwidget.h \
    src/utils/notification_global.h \
    src/utils/messageType.h \
    src/utils/iconlineedit.h \
    src/utils/datelineedit.h \
    src/utils/customdatechooser.h \
    src/utils/calendardaybtn.h \
    src/utils/yearmonthpopup.h \
    src/utils/timeselector.h \
    src/utils/paginationwidget.h \
    # ==================== HyperLPR-2 头文件（替换 EasyPR） ====================
    thirdparty/hyperlpr/include/Pipeline.h \
    thirdparty/hyperlpr/include/Platedetect.h \
    thirdparty/hyperlpr/include/PlateRecognation.h \
    thirdparty/hyperlpr/include/Finetune.h \
    thirdparty/hyperlpr/include/PlateInfo.h

# ==================== OpenCV 4.11 + HyperLPR 库配置（替换 3.4.8 + EasyPR） ====================
INCLUDEPATH += thirdparty/opencv4/include \
               thirdparty/hyperlpr/include \
               UI/ \
               UI/Login \
               UI/MainWindow \
               generated \
               .

# OpenCV 4.11：链接 world 导入库（Hyperlpr 改为源码直接编译，无需 -lhyperlpr）
LIBS += -L$$PWD/thirdparty/opencv4/lib -lopencv_world4110

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# ==================== UI 文件生成配置 ====================
# 指定 uic 工具生成的 ui_*.h 文件输出目录
UI_DIR = generated

FORMS += \
    UI/CameraManagement/cameramanagement.ui \
    UI/Login/logindialog.ui \
    UI/MainWindow/mainwindow.ui \
    UI/UserManager/usermanagement.ui \
    UI/VehicleInformation/vehicleinformation.ui \
    UI/Register/registerdialog.ui \
    UI/ConfigInit/configinitdialog.ui \
    UI/Settings/settingsdialog.ui \
    UI/CameraManagement/camerasettingsdialog.ui

# ==================== DLL 复制配置 ====================
# 构建完成后将 DLL 复制到可执行文件目录

CONFIG(debug, debug|release) {
    DLL_TARGET_DIR = $$OUT_PWD/debug
} else {
    DLL_TARGET_DIR = $$OUT_PWD/release
}

# 复制 OpenCV 4.11 world DLL（从导入库目录获取）
QMAKE_POST_LINK += $$quote(cmd /c copy /Y \"$$shell_path($$PWD/thirdparty/opencv4/lib/libopencv_world4110.dll)\" \"$$shell_path($$DLL_TARGET_DIR)\" $$escape_expand(\n\t))

RESOURCES += \
    UI/imageQrc/image.qrc \
    styles/styles.qrc

# ==================== 模型文件部署 ====================
# HyperLPR 模型已直接部署到 debug/model/
# 此部署规则已无实际文件可复制（model/ 目录已删除）
model.files = model/*
model.path = $$OUT_PWD/model
INSTALLS += model