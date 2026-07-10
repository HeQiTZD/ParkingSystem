# QT 类学习文档

> **项目**：ParkingSystem（停车场管理系统）
> **创建日期**：2026-07-08
> **用途**：记录项目开发中用到的所有 Qt 类，方便每日写完后查看回忆，加强记忆

---

## 使用说明

### 文档结构
本文档按**功能分类**组织，每个类包含：
- **一句话作用**：快速回忆这个类干什么的
- **继承关系**：理清类之间的层级
- **本项目使用场景**：在哪个文件、做什么用
- **核心用法**：最简代码示例
- **核心方法速查**：高频方法一览
- **记忆口诀**：帮助快速记住使用模式

### 掌握状态标记
| 标记 | 含义 |
|---|---|
| ✅ 已掌握 | 能独立使用，无需查文档 |
| 🔶 学习中 | 了解用法，还需要练习 |
| ❌ 待学习 | 还没用过，先了解概念 |

### 维护规则
1. **每写完一天代码，把当天新用到的类补充进去**
2. 已掌握的类标记 `✅`，还在学习的标记 `🔶`
3. 每个类必须写"本项目使用场景"，知道它用在哪
4. 代码示例优先用**本项目的真实代码**

---

## 一、窗口与界面类

### 1.1 QMainWindow 🔶

**一句话作用**：带菜单栏、工具栏、状态栏的主窗口基类。

**继承关系**：`QObject → QWidget → QMainWindow`

**本项目使用场景**：
- 文件：`UI/MainWindow/mainwindow.h`
- 用途：系统主窗口，承载导航栏、视频画面、车位统计图表

**核心用法**：
```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);  // 无边框
    setMinimumSize(400, 300);
}
```

**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `setCentralWidget()` | 设置中央控件 |
| `setMenuBar()` | 设置菜单栏 |
| `setStatusBar()` | 设置状态栏 |
| `setWindowFlag()` | 设置窗口标志（如无边框） |
| `showMaximized() / showNormal()` | 最大化 / 还原 |
| `showMinimized()` | 最小化 |
| `installEventFilter()` | 安装事件过滤器 |

**记忆口诀**：`QMainWindow = 主窗口框架，setupUi + 信号槽 + 事件过滤器`

---

### 1.2 QWidget ✅

**一句话作用**：所有可视化组件的基类，Qt 界面编程的"万物之源"。

**继承关系**：`QObject → QWidget`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.h`、`UI/MainWindow/circleprogress.h`、`UI/HomePage/homepage.h` 等
- 用途：Toast 提示框、圆环进度条、所有页面（HomePage/CameraPage/ManagePage/VehicleInfoPage）的基类

**核心用法**：
cpp
// 作为独立窗口
ToastWidget *toast = new ToastWidget(parent);
toast->setAttribute(Qt::WA_DeleteOnClose);
toast->setFixedSize(320, 64);
toast->show();

// 作为子控件
QLabel *label = new QLabel(this);  // this 作为 parent，自动管理内存


**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `show() / hide()` | 显示 / 隐藏 |
| `setFixedSize()` | 固定大小 |
| `setWindowFlag()` | 设置窗口标志 |
| `setAttribute()` | 设置属性（如 `WA_TranslucentBackground` 透明背景） |
| `setStyleSheet()` | 设置 QSS 样式表 |
| `installEventFilter()` | 安装事件过滤器 |
| `move() / setGeometry()` | 设置位置 / 大小 |
| `update()` | 触发重绘（调用 paintEvent） |
| `setMouseTracking(true)` | 开启鼠标追踪（不按下也能收到移动事件） |

**记忆口诀**：`QWidget = 界面基石，show/hide + setStyleSheet + 事件重写`

---

### 1.3 QDialog 🔶

**一句话作用**：对话框基类，用于弹窗交互（登录、注册、配置等）。

**继承关系**：`QObject → QWidget → QDialog`

**本项目使用场景**：
- 文件：`UI/Login/logindialog.h`、`UI/Register/registerdialog.h`、`UI/ConfigInit/configinitdialog.h`、`src/utils/notificationdialog.h`
- 用途：登录弹窗、注册弹窗、配置初始化弹窗、统一通知弹窗

**核心用法**：
cpp
// 模态弹窗（阻塞父窗口）
LoginDialog dlg(nullptr, dbManager);
if (dlg.exec() == QDialog::Accepted) {
    // 用户点击了"确定"
}

// 非模态弹窗（不阻塞）
NotificationDialog *dlg = new NotificationDialog(parent, Info, title, msg);
dlg->setAttribute(Qt::WA_DeleteOnClose);
dlg->show();


**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `exec()` | 模态显示，阻塞等待用户操作 |
| `show()` | 非模态显示 |
| `accept()` | 关闭对话框，返回 `QDialog::Accepted` |
| `reject()` | 关闭对话框，返回 `QDialog::Rejected` |
| `setModal(true)` | 设置为模态 |
| `setFixedSize()` | 固定对话框大小 |

**记忆口诀**：`QDialog = 弹窗交互，exec() 阻塞 / show() 非阻塞，accept/reject 关闭`

---

## 二、布局管理类

### 2.1 QHBoxLayout ✅

**一句话作用**：水平布局，让控件从左到右排列。

**继承关系**：`QObject → QLayout → QHBoxLayout`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.cpp`、`src/utils/notificationdialog.cpp`
- 用途：Toast 内部（图标 + 消息 + 关闭按钮水平排列）、弹窗按钮排列

**核心用法**：
cpp
QHBoxLayout *rootLayout = new QHBoxLayout(this);
rootLayout->setContentsMargins(14, 12, 14, 12);  // 左、上、右、下边距
rootLayout->setSpacing(10);                        // 控件间距
rootLayout->addWidget(m_iconLabel);
rootLayout->addLayout(msgLayout, 1);               // 1 = 拉伸因子
rootLayout->addWidget(m_closeButton, 0, Qt::AlignTop);


**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `addWidget()` | 添加控件 |
| `addLayout()` | 嵌套子布局 |
| `addStretch()` | 添加弹性空间 |
| `setContentsMargins()` | 设置四周边距 |
| `setSpacing()` | 设置控件间距 |

**记忆口诀**：`QHBoxLayout = 水平排列，addWidget + setSpacing + addStretch`

---

### 2.2 QVBoxLayout ✅

**一句话作用**：垂直布局，让控件从上到下排列。

**继承关系**：`QObject → QLayout → QVBoxLayout`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.cpp`、`src/utils/notificationdialog.cpp`
- 用途：Toast 内部图标垂直居中、弹窗整体（图标 + 标题 + 消息 + 按钮）

**核心用法**：
cpp
QVBoxLayout *rootLayout = new QVBoxLayout(this);
rootLayout->setContentsMargins(28, 24, 28, 24);
rootLayout->setSpacing(14);
rootLayout->setAlignment(Qt::AlignHCenter);
rootLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
rootLayout->addWidget(m_titleLabel);
rootLayout->addWidget(m_msgLabel, 1);  // 1 = 拉伸因子，占满剩余空间


**记忆口诀**：`QVBoxLayout = 垂直排列，用法同 QHBoxLayout，方向换成上下`

---

## 三、控件类

### 3.1 QLabel ✅

**一句话作用**：显示文本或图片，最常用、最基础的可视化控件。

**继承关系**：`QObject → QWidget → QFrame → QLabel`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.h`、`src/utils/notificationdialog.h`、`UI/MainWindow/mainwindow.h`
- 用途：Toast 图标和消息文本、弹窗标题和消息、视频画面显示、时间显示

**核心用法**：
cpp
// 文本标签
QLabel *label = new QLabel("Hello", this);
label->setAlignment(Qt::AlignCenter);  // 居中对齐
label->setWordWrap(true);             // 自动换行

// 图片标签
label->setPixmap(QPixmap(":/icons/logo.png"));
label->setScaledContents(false);       // 不自动缩放

// 富文本（支持 HTML）
label->setText("<img src=':/icons/home.svg' width='16'/> <span>首页</span>");


**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `setText()` | 设置文本 |
| `setPixmap()` | 设置图片 |
| `setAlignment()` | 设置对齐方式 |
| `setWordWrap()` | 设置自动换行 |
| `setScaledContents()` | 图片是否自适应大小 |
| `setCursor()` | 设置鼠标悬停光标 |
| `setStyleSheet()` | 设置样式 |

**记忆口诀**：`QLabel = 显示文本/图片，setText + setPixmap + setAlignment`

---

### 3.2 QPushButton ✅

**一句话作用**：按钮控件，用户点击触发操作。

**继承关系**：`QObject → QWidget → QAbstractButton → QPushButton`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.h`、`src/utils/notificationdialog.h`、`UI/MainWindow/mainwindow.cpp`
- 用途：Toast 关闭按钮、弹窗确认/取消按钮、导航栏按钮、窗口控制按钮

**核心用法**：
cpp
QPushButton *btn = new QPushButton("确定", this);
btn->setFixedSize(96, 36);
btn->setObjectName("notifPrimary");  // QSS 选择器用
btn->setIcon(QIcon(":/icons/close.svg"));
btn->setIconSize(QSize(16, 16));
btn->setCheckable(true);   // 可切换选中状态
btn->setChecked(true);     // 默认选中
btn->setCursor(Qt::PointingHandCursor);

// 信号连接
connect(btn, &QPushButton::clicked, this, &MyClass::onClicked);


**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `setText()` | 设置文本 |
| `setIcon() / setIconSize()` | 设置图标 / 图标大小 |
| `setCheckable()` | 是否可切换选中状态 |
| `setChecked()` | 设置选中状态 |
| `setCursor()` | 设置鼠标光标 |
| `setObjectName()` | 设置对象名（QSS 用） |
| `clicked` 信号 | 点击时发射 |

**记忆口诀**：`QPushButton = 按钮，setText + setIcon + connect(clicked)`

---

### 3.3 QLineEdit 🔶

**一句话作用**：单行文本输入框。

**继承关系**：`QObject → QWidget → QLineEdit`

**本项目使用场景**：
- 文件：`UI/Login/passwordedit.h`、`UI/Login/usernameedit.h`、`UI/Register/registerdialog.cpp`
- 用途：用户名输入、密码输入（带自定义绘制）、注册表单

**核心用法**：
cpp
QLineEdit *edit = new QLineEdit(this);
edit->setPlaceholderText("请输入用户名");  // 占位符文本
edit->setEchoMode(QLineEdit::Password);     // 密码模式（显示圆点）
edit->setClearButtonEnabled(true);          // 显示清除按钮

// 获取输入
QString text = edit->text().trimmed();


**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `text()` | 获取文本 |
| `setText()` | 设置文本 |
| `clear()` | 清空 |
| `setPlaceholderText()` | 设置占位符 |
| `setEchoMode()` | 设置显示模式（密码/正常） |
| `setFocus()` | 获取焦点 |

**记忆口诀**：`QLineEdit = 单行输入，text() 取值 + setPlaceholderText 提示`

---

### 3.4 QButtonGroup 🔶

**一句话作用**：管理一组按钮，实现互斥选中（类似单选）。

**继承关系**：`QObject → QButtonGroup`

**本项目使用场景**：
- 文件：`UI/MainWindow/mainwindow.cpp`
- 用途：导航栏按钮互斥（首页/车辆信息/用户管理/摄像头管理只能选一个）

**核心用法**：
cpp
QButtonGroup *navButtonGroup = new QButtonGroup(this);
navButtonGroup->addButton(ui->dashboardButton);
navButtonGroup->addButton(ui->vehicleInfoButton);
navButtonGroup->addButton(ui->userManagementButton);
navButtonGroup->addButton(ui->cameraManagementButton);
navButtonGroup->setExclusive(true);  // 互斥模式


**记忆口诀**：`QButtonGroup = 按钮分组，addButton + setExclusive(true) 互斥`

---

### 3.5 QTableWidget 🔶

**一句话作用**：表格控件，显示行列数据。

**继承关系**：`QObject → QWidget → QFrame → QAbstractScrollArea → QTableView → QTableWidget`

**本项目使用场景**：
- 文件：`UI/ManagePage/managepage.cpp`、`UI/VehicleInfo/vehicleinfopage.cpp`
- 用途：用户管理表格、车辆信息表格

**核心用法**：
cpp
ui->tableWidget->setColumnCount(5);
ui->tableWidget->setHorizontalHeaderLabels({"用户名", "姓名", "手机号", "角色", "状态"});
ui->tableWidget->setRowCount(6);


**记忆口诀**：`QTableWidget = 表格，setColumnCount + setHorizontalHeaderLabels`

---

## 四、动画与图形类

### 4.1 QPropertyAnimation ✅

**一句话作用**：动画化 Qt 属性，让属性值随时间平滑变化（如位置、透明度）。

**继承关系**：`QObject → QVariantAnimation → QPropertyAnimation`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.h`、`src/utils/notificationdialog.h`
- 用途：Toast 滑入/滑出动画（pos + windowOpacity）、弹窗缩放/淡入淡出

**核心用法**：
cpp
// 位置动画（Toast 滑入）
QPropertyAnimation *m_posAnim = new QPropertyAnimation(this, "pos", this);
m_posAnim->setDuration(200);
m_posAnim->setStartValue(QPoint(400, 60));
m_posAnim->setEndValue(QPoint(80, 60));
m_posAnim->setEasingCurve(QEasingCurve::OutCubic);
m_posAnim->start();

// 透明度动画（淡入淡出）
QPropertyAnimation *m_opaAnim = new QPropertyAnimation(this, "windowOpacity", this);
m_opaAnim->setDuration(250);
m_opaAnim->setStartValue(0.0);
m_opaAnim->setEndValue(1.0);
m_opaAnim->start();


**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `setTargetObject()` | 设置动画目标对象 |
| `setPropertyName("pos")` | 设置要动画的属性名 |
| `setDuration()` | 动画时长（ms） |
| `setStartValue() / setEndValue()` | 起始 / 结束值 |
| `setEasingCurve()` | 缓动曲线（OutCubic/InCubic/OutBack） |
| `start() / stop()` | 开始 / 停止 |
| `finished` 信号 | 动画完成时发射 |

**记忆口诀**：`QPropertyAnimation = 属性动画，目标对象 + 属性名 + 起止值 + 时长 → start()`

---

### 4.2 QPainter ✅

**一句话作用**：Qt 的绘图工具，所有自定义绘制的核心类。

**继承关系**：独立类（无继承）

**本项目使用场景**：
- 文件：`src/utils/toastwidget.cpp`、`UI/MainWindow/circleprogress.cpp`、`UI/Login/logindialog.cpp`、`UI/Login/passwordedit.cpp`、`UI/Login/usernameedit.cpp`、`src/utils/notificationdialog.cpp`
- 用途：Toast 圆角卡片 + 左侧竖条、圆环进度条、登录窗背景、输入框自定义绘制

**核心用法**：
cpp
void CircleProgress::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿

    // 画背景圆环
    QPen bgPen(QColor(232, 237, 243), 10, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(bgPen);
    painter.drawArc(rect, 0, 360 * 16);  // 单位：1/16 度

    // 画进度圆环
    QPen progressPen(QColor(59, 130, 246), 10, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(progressPen);
    painter.drawArc(rect, 90 * 16, -spanAngle);  // 从顶部开始

    // 画文字
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(rect, Qt::AlignCenter, "70%");
}


**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `setRenderHint(Antialiasing)` | 开启抗锯齿 |
| `setPen() / setBrush()` | 设置画笔 / 画刷 |
| `drawRoundedRect()` | 画圆角矩形 |
| `drawArc()` | 画弧线（圆环用） |
| `drawText()` | 画文字 |
| `drawPixmap()` | 画图片 |
| `fillRect()` | 填充矩形 |
| `setClipPath()` | 设置裁剪区域 |

**记忆口诀**：`QPainter = 画笔，setPen + setBrush + drawRoundedRect/drawArc/drawText`

---

### 4.3 QPainterPath 🔶

**一句话作用**：路径类，用于定义复杂图形形状（圆角矩形、曲线等）。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`UI/Login/logindialog.cpp`、`UI/Login/passwordedit.cpp`、`UI/Login/usernameedit.cpp`、`src/utils/notificationdialog.cpp`
- 用途：定义圆角裁剪区域，让绘制只在路径内生效

**核心用法**：
cpp
QPainterPath path;
path.addRoundedRect(rect(), 10, 10);  // 圆角矩形路径
painter.setClipPath(path);            // 设置裁剪区域
painter.fillPath(path, QColor(0xE3F2FD));  // 填充路径


**记忆口诀**：`QPainterPath = 路径，addRoundedRect + setClipPath 裁剪`

---

### 4.4 QPixmap ✅

**一句话作用**：图片类，专门用于在屏幕上显示图片（与设备相关）。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`UI/Login/logindialog.cpp`、`UI/Login/passwordedit.cpp`、`UI/Login/usernameedit.cpp`、`UI/MainWindow/mainwindow.cpp`
- 用途：登录页品牌图、输入框图标、导航栏图标、视频帧显示

**核心用法**：
cpp
// 从资源文件加载
QPixmap pix(":/icons/login-brand-image");
painter.drawPixmap(0, 0, width(), height(), pix);  // 拉伸绘制

// 从 QIcon 获取
QPixmap pixmap = QIcon(":/icons/home.svg").pixmap(24, 24);
label->setPixmap(pixmap);

// 缩放
pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);


**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `QPixmap("路径")` | 从文件/资源加载 |
| `isNull()` | 判断是否加载成功 |
| `scaled()` | 缩放 |
| `width() / height()` | 获取宽高 |

**记忆口诀**：`QPixmap = 显示图片，load + drawPixmap + scaled`

---

### 4.5 QIcon ✅

**一句话作用**：图标类，可存储多种分辨率的图标，自动选择最合适的大小。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`UI/MainWindow/mainwindow.cpp`
- 用途：导航栏按钮图标、窗口控制按钮图标

**核心用法**：
cpp
QIcon icon(":/icons/home.svg");
button->setIcon(icon);
button->setIconSize(QSize(20, 20));

// 获取 QPixmap
QPixmap pixmap = icon.pixmap(24, 24);


**记忆口诀**：`QIcon = 图标，setIcon + setIconSize`

---

### 4.6 QImage 🔶

**一句话作用**：图片类，与设备无关，适合图像处理和与 OpenCV 互转。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`UI/MainWindow/mainwindow.cpp`、`src/camera/camerathread.h`
- 用途：摄像头帧（cv::Mat）→ QImage → QPixmap 显示到 QLabel

**核心用法**：
cpp
// cv::Mat → QImage
QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);

// QImage → QPixmap
QPixmap pixmap = QPixmap::fromImage(img);

// QImage → cv::Mat
cv::Mat mat(image.height(), image.width(), CV_8UC3,
            const_cast<uchar*>(image.bits()), image.bytesPerLine());


**记忆口诀**：`QImage = 图像处理，Mat↔Image↔Pixmap 转换枢纽`

---

### 4.7 QGraphicsDropShadowEffect 🔶

**一句话作用**：图形阴影效果，给控件添加投影。

**继承关系**：`QObject → QGraphicsEffect → QGraphicsDropShadowEffect`

**本项目使用场景**：
- 文件：`UI/ConfigInit/configinitdialog.cpp`
- 用途：配置窗口的阴影效果

**核心用法**：
cpp
QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
shadow->setBlurRadius(20);                    // 模糊半径
shadow->setColor(QColor(0, 0, 0, 80));       // 阴影颜色 + 透明度
shadow->setOffset(0, 0);                      // 偏移量
ui->mainContainer->setGraphicsEffect(shadow);


**记忆口诀**：`QGraphicsDropShadowEffect = 阴影，setBlurRadius + setColor + setOffset`

---

## 五、事件与定时器类

### 5.1 QTimer ✅

**一句话作用**：定时器，周期性触发或单次延迟执行。

**继承关系**：`QObject → QTimer`

**本项目使用场景**：
- 文件：`UI/MainWindow/mainwindow.h`、`src/utils/toastwidget.cpp`、`src/utils/notificationdialog.cpp`
- 用途：时间显示每秒更新、Toast 自动消失、复制详情后恢复文本

**核心用法**：
cpp
// 周期性定时器
QTimer *timer = new QTimer(this);
connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
timer->start(1000);  // 每 1000ms 触发一次

// 单次定时器（最常用！）
QTimer::singleShot(3000, this, [this]() {
    slideOut([this]() { close(); });  // 3秒后执行
});


**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `start(ms)` | 启动定时器（周期 ms） |
| `stop()` | 停止 |
| `setSingleShot(true)` | 设置为单次 |
| `QTimer::singleShot()` | 静态方法，单次延迟 |
| `timeout` 信号 | 超时时发射 |

**记忆口诀**：`QTimer = 定时器，start(周期) 循环 / singleShot 单次`

---

### 5.2 QEvent 🔶

**一句话作用**：所有事件的基类，事件系统的核心。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/toastwidget.h`、`UI/MainWindow/mainwindow.cpp`、`UI/ConfigInit/configinitdialog.cpp`
- 用途：事件过滤器中判断事件类型（`QEvent::Resize`、`QEvent::Destroy`）

**核心用法**：
cpp
bool eventFilter(QObject *obj, QEvent *event) override
{
    if (event->type() == QEvent::Resize) {
        // 处理窗口大小变化
    }
    if (event->type() == QEvent::Destroy) {
        // 父窗口销毁时清理
    }
    return QWidget::eventFilter(obj, event);
}


**记忆口诀**：`QEvent = 事件基类，event->type() 判断类型`

---

### 5.3 QMouseEvent 🔶

**一句话作用**：鼠标事件，携带鼠标位置、按键信息。

**继承关系**：`QEvent → QInputEvent → QMouseEvent`

**本项目使用场景**：
- 文件：`UI/MainWindow/mainwindow.h`、`UI/Login/logindialog.h`、`UI/ConfigInit/configinitdialog.cpp`
- 用途：窗口拖动、窗口拉伸、登录窗拖动

**核心用法**：
```cpp
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;
    QPoint pos = event->position().toPoint();
    m_dragPos = event->globalPosition().toPoint();
    m_resizeGeo = geometry();
    event->accept();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint current = event->globalPosition().toPoint();
    QPoint delta = current - m_dragPos;
    move(m_resizeGeo.topLeft() + delta);
}
```

**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `event->button()` | 获取按下的按钮 |
| `event->pos()` | 获取鼠标位置（控件坐标） |
| `event->globalPos()` | 获取鼠标位置（屏幕坐标） |
| `event->position().toPoint()` | 浮点坐标转 QPoint |

**记忆口诀**：`QMouseEvent = 鼠标事件，button/pos/globalPos + accept()`

---

### 5.4 QPaintEvent 🔶

**一句话作用**：绘制事件，在 `paintEvent()` 中触发，用于自定义控件外观。

**继承关系**：`QEvent → QPaintEvent`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.h`、`UI/MainWindow/circleprogress.h`、`UI/Login/logindialog.h`、`UI/Login/passwordedit.h`、`UI/Login/usernameedit.h`、`src/utils/notificationdialog.h`
- 用途：所有自定义绘制的入口

**核心用法**：
```cpp
void CircleProgress::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    // ... 绘制代码 ...
}
```

**记忆口诀**：`QPaintEvent = 绘制事件，重写 paintEvent + QPainter 绘制`

---

### 5.5 QKeyEvent 🔶

**一句话作用**：键盘事件，携带按键信息。

**继承关系**：`QEvent → QInputEvent → QKeyEvent`

**本项目使用场景**：
- 文件：`src/utils/notificationdialog.h`
- 用途：ESC 键关闭弹窗

**核心用法**：
```cpp
void NotificationDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        playCloseAnimation([this]() { reject(); });
    } else {
        QDialog::keyPressEvent(event);
    }
}
```

**记忆口诀**：`QKeyEvent = 键盘事件，event->key() 判断按键`

---

### 5.6 QFocusEvent 🔶

**一句话作用**：焦点事件，控件获得/失去焦点时触发。

**继承关系**：`QEvent → QFocusEvent`

**本项目使用场景**：
- 文件：`UI/Login/passwordedit.h`、`UI/Login/usernameedit.h`
- 用途：输入框获得焦点时变蓝色边框，失去焦点时变灰色

**核心用法**：
```cpp
void PasswordEdit::focusInEvent(QFocusEvent *event)
{
    m_hasFocus = true;
    QLineEdit::focusInEvent(event);  // 必须调用父类！
    update();
}

void PasswordEdit::focusOutEvent(QFocusEvent *event)
{
    m_hasFocus = false;
    QLineEdit::focusOutEvent(event);
    update();
}
```

**记忆口诀**：`QFocusEvent = 焦点事件，focusInEvent/focusOutEvent + 调用父类`

---

## 六、文件与数据类

### 6.1 QFile ✅

**一句话作用**：文件读写类，读写本地文件（文本或二进制）。

**继承关系**：`QObject → QFileDevice → QFile`

**本项目使用场景**：
- 文件：`src/utils/initfile.cpp`、`UI/MainWindow/mainwindow.cpp`、`UI/Login/logindialog.cpp`、`UI/ConfigInit/configinitdialog.cpp`、`src/database/databasemanager.cpp`
- 用途：读取 QSS 样式文件、读写 JSON 配置文件

**核心用法**：
```cpp
// 读取文件
QFile file(":/styles/main.qss");
if (file.open(QFile::ReadOnly)) {
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);
    file.close();
}

// 写入文件
QFile file(configFilePath);
if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

// 检查是否存在
QFile::exists(configFilePath);
```

**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `open(QIODevice::ReadOnly)` | 打开文件 |
| `readAll()` | 读取全部内容 |
| `write()` | 写入内容 |
| `close()` | 关闭文件 |
| `exists()` | 静态方法，检查文件是否存在 |
| `errorString()` | 获取错误信息 |

**记忆口诀**：`QFile = 文件读写，open + readAll/write + close`

---

### 6.2 QDir 🔶

**一句话作用**：目录操作类，创建/删除目录、遍历文件。

**继承关系**：`QObject → QDir`

**本项目使用场景**：
- 文件：`src/utils/initfile.cpp`
- 用途：确保配置文件所在目录存在

**核心用法**：
```cpp
QDir dir = fileInfo.absoluteDir();
if (!dir.exists()) {
    if (!dir.mkpath(".")) {
        qDebug() << "创建配置目录失败";
    }
}
```

**记忆口诀**：`QDir = 目录操作，exists + mkpath 创建目录`

---

### 6.3 QFileInfo 🔶

**一句话作用**：文件信息类，获取文件的路径、大小、修改时间等元信息。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/initfile.cpp`
- 用途：获取配置文件所在目录

**核心用法**：
```cpp
QFileInfo fileInfo(configFilePath);
QDir dir = fileInfo.absoluteDir();
QString fileName = fileInfo.fileName();
```

**记忆口诀**：`QFileInfo = 文件元信息，absoluteDir + fileName`

---

### 6.4 QJsonDocument 🔶

**一句话作用**：JSON 文档类，在 QJsonObject/QJsonArray 与 文本/二进制 格式之间转换。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/initfile.h`、`src/database/databasemanager.cpp`
- 用途：读写 JSON 配置文件

**核心用法**：
```cpp
// 解析 JSON
QJsonParseError parseError;
QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
if (parseError.error != QJsonParseError::NoError) {
    qDebug() << "JSON 解析错误";
}
configData = doc.object();

// 生成 JSON
QJsonDocument doc(configData);
QString jsonStr = doc.toJson(QJsonDocument::Indented);
```

**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `fromJson()` | 从 JSON 文本解析 |
| `toJson()` | 序列化为 JSON 文本 |
| `object()` | 获取 QJsonObject |
| `array()` | 获取 QJsonArray |

**记忆口诀**：`QJsonDocument = JSON 文档，fromJson 解析 + toJson 序列化`

---

### 6.5 QJsonObject 🔶

**一句话作用**：JSON 对象（键值对集合）。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/initfile.h`
- 用途：存储配置数据（数据库、摄像头、停车场配置等）

**核心用法**：
```cpp
// 创建
QJsonObject dbConfig;
dbConfig["host"] = "localhost";
dbConfig["port"] = 3306;
configData["database"] = dbConfig;

// 读取
QString host = configData["database"].toObject()["host"].toString();
int port = configData["database"].toObject()["port"].toInt();

// 检查键是否存在
if (config.contains("database")) { ... }
```

**记忆口诀**：`QJsonObject = JSON 对象，[key] 读写 + toObject/toInt 转换`

---

### 6.6 QJsonArray 🔶

**一句话作用**：JSON 数组。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/initfile.h`
- 用途：配置文件中可能用到的数组结构

**记忆口诀**：`QJsonArray = JSON 数组，append + 遍历`

---

### 6.7 QStandardPaths 🔶

**一句话作用**：获取不同操作系统上的标准路径（桌面、文档、AppData 等）。

**继承关系**：独立类（工具类）

**本项目使用场景**：
- 文件：`src/utils/initfile.h`（已包含，暂未使用）
- 用途：跨平台获取标准目录

**核心用法**：
```cpp
QString appData = QStandardPaths::writableLocation(
    QStandardPaths::AppDataLocation);
```

**记忆口诀**：`QStandardPaths = 标准路径，writableLocation(AppDataLocation)`

---

### 6.8 QCoreApplication 🔶

**一句话作用**：非 GUI 应用的核心类，管理事件循环。

**继承关系**：`QObject → QCoreApplication`

**本项目使用场景**：
- 文件：`src/utils/initfile.h`、`src/database/databasemanager.cpp`
- 用途：获取应用程序目录路径

**核心用法**：
```cpp
QString appPath = QCoreApplication::applicationDirPath();
```

**记忆口诀**：`QCoreApplication = 应用核心，applicationDirPath 获取路径`

---

### 6.9 QApplication ✅

**一句话作用**：GUI 应用的核心类，管理整个应用程序的事件循环和设置。

**继承关系**：`QObject → QCoreApplication → QGuiApplication → QApplication`

**本项目使用场景**：
- 文件：`src/app/main.cpp`、`src/utils/toastwidget.cpp`、`src/utils/notificationdialog.cpp`
- 用途：程序入口、获取活动窗口、剪贴板操作

**核心用法**：
```cpp
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // ...
    return app.exec();  // 进入事件循环
}

QWidget *activeWin = QApplication::activeWindow();
QApplication::clipboard()->setText("复制的内容");
```

**记忆口诀**：`QApplication = 应用入口，main 中创建 + exec() 进入事件循环`

---

### 6.10 QGuiApplication 🔶

**一句话作用**：GUI 应用基类，提供窗口系统相关功能。

**继承关系**：`QObject → QCoreApplication → QGuiApplication`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.cpp`、`src/utils/notificationdialog.cpp`
- 用途：获取主屏幕信息

**核心用法**：
```cpp
QScreen *screen = QGuiApplication::primaryScreen();
QRect geo = screen->availableGeometry();
```

**记忆口诀**：`QGuiApplication = GUI 基类，primaryScreen 获取屏幕`

---

### 6.11 QScreen 🔶

**一句话作用**：屏幕信息类，获取屏幕分辨率、可用区域等。

**继承关系**：`QObject → QScreen`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.cpp`、`src/utils/notificationdialog.cpp`
- 用途：Toast 和弹窗在屏幕居中显示

**核心用法**：
```cpp
QScreen *screen = QGuiApplication::primaryScreen();
QRect geo = screen->availableGeometry();
QPoint center = geo.center();
```

**记忆口诀**：`QScreen = 屏幕信息，availableGeometry + center`

---

### 6.12 QClipboard 🔶

**一句话作用**：剪贴板类，实现复制/粘贴功能。

**继承关系**：`QObject → QClipboard`

**本项目使用场景**：
- 文件：`src/utils/notificationdialog.cpp`
- 用途：错误弹窗"复制详情"按钮

**核心用法**：
```cpp
QApplication::clipboard()->setText(m_titleLabel->text() + "\n" + m_detail);
```

**记忆口诀**：`QClipboard = 剪贴板，setText 复制 + text() 粘贴`

---

### 6.13 QStyleFactory 🔶

**一句话作用**：样式工厂类，创建 Qt 内置样式（Fusion、Windows 等）。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/app/main.cpp`（已包含，暂未使用）
- 用途：设置应用程序全局样式

**核心用法**：
```cpp
QApplication::setStyle(QStyleFactory::create("Fusion"));
```

**记忆口诀**：`QStyleFactory = 样式工厂，create("Fusion")`

---

### 6.14 QLatin1String 🔶

**一句话作用**：轻量级字符串视图，用于与 Qt API 交互时避免构造 QString。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/initfile.cpp`、`UI/MainWindow/mainwindow.cpp`、`UI/Login/logindialog.cpp`
- 用途：读取文件后转换为 QString

**核心用法**：
```cpp
QString styleSheet = QLatin1String(styleFile.readAll());
```

**记忆口诀**：`QLatin1String = 轻量字符串，readAll() 后转 QString`

---

## 七、数据库类

### 7.1 QSqlDatabase 🔶

**一句话作用**：数据库连接类，管理一个数据库连接。

**继承关系**：`QObject → QSqlDatabase`

**本项目使用场景**：
- 文件：`src/database/databasemanager.h`
- 用途：MySQL 数据库连接管理

**核心用法**：
```cpp
db = QSqlDatabase::addDatabase("QMYSQL");
db.setHostName(host);
db.setPort(port);
db.setDatabaseName(dbName);
db.setUserName(username);
db.setPassword(password);
db.open();

db.isOpen();
db.lastError();
db.transaction();
db.commit();
db.rollback();
```

**记忆口诀**：`QSqlDatabase = 数据库连接，addDatabase + setXxx + open`

---

### 7.2 QSqlQuery 🔶

**一句话作用**：SQL 执行类，执行 SQL 语句并获取结果。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/database/databasemanager.h`、`src/database/mysqlinit.h`
- 用途：执行所有 SQL 操作

**核心用法**：
```cpp
// 预处理（防 SQL 注入，推荐！）
QSqlQuery query(db);
query.prepare("SELECT role FROM User WHERE username = :u AND password = :p");
query.bindValue(":u", username);
query.bindValue(":p", password);
query.exec();

// 遍历结果
while (query.next()) {
    QString role = query.value("role").toString();
    int id = query.value(0).toInt();
}
```

**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `exec()` | 执行 SQL |
| `prepare()` | 预处理 SQL |
| `bindValue()` | 绑定参数 |
| `next()` | 移动到下一行 |
| `value()` | 获取当前行的值 |
| `lastError()` | 获取错误信息 |

**记忆口诀**：`QSqlQuery = SQL 执行，prepare + bindValue + exec + next 遍历`

---

### 7.3 QSqlError 🔶

**一句话作用**：数据库错误类，携带错误信息。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/database/databasemanager.h`、`src/database/mysqlinit.h`
- 用途：数据库操作失败时输出错误信息

**核心用法**：
```cpp
qDebug() << "数据库连接失败" << db.lastError().text();
```

**记忆口诀**：`QSqlError = 数据库错误，lastError().text() 输出`

---

## 八、多线程类

### 8.1 QThread 🔶

**一句话作用**：线程类，在独立线程中执行耗时任务，避免阻塞 UI。

**继承关系**：`QObject → QThread`

**本项目使用场景**：
- 文件：`src/camera/camerathread.h`、`src/utils/pthreadpool.cpp`
- 用途：摄像头视频捕获线程、线程池任务执行

**核心用法**：
```cpp
class CameraThread : public QThread {
    void run() override {
        while (m_running) {
            emit newFrameCaptured(frame);
        }
    }
};

CameraThread *thread = new CameraThread(0, this);
thread->start();
thread->quit();
thread->wait();
```

**核心方法速查**：
| 方法 | 作用 |
|---|---|
| `start()` | 启动线程 |
| `quit()` | 退出事件循环 |
| `wait()` | 等待线程结束 |
| `run()` | 线程主函数（重写） |
| `msleep()` | 线程休眠（毫秒） |
| `currentThreadId()` | 获取当前线程 ID |

**记忆口诀**：`QThread = 线程，继承重写 run + start 启动 + wait 等待`

---

### 8.2 QMutex 🔶

**一句话作用**：互斥锁，保护共享资源，防止多线程数据竞争。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/camera/camerathread.h`、`src/utils/pthreadpool.h`
- 用途：保护摄像头帧数据、任务计数器

**核心用法**：
```cpp
// 推荐：QMutexLocker 自动管理
{
    QMutexLocker locker(&m_mutex);
    m_latestFrame = frame.clone();
}  // 析构时自动解锁
```

**记忆口诀**：`QMutex = 互斥锁，lock/unlock 或 QMutexLocker 自动管理`

---

### 8.3 QMutexLocker 🔶

**一句话作用**：RAII 风格的互斥锁管理器，构造时加锁，析构时自动解锁。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/camera/camerathread.cpp`、`src/utils/pthreadpool.cpp`
- 用途：所有需要加锁的地方

**核心用法**：
```cpp
QMutexLocker locker(&m_mutex);  // 构造时加锁
// ... 临界区代码 ...
// 析构时自动解锁，即使发生异常也能正确释放
```

**记忆口诀**：`QMutexLocker = 自动锁，构造加锁 + 析构解锁`

---

### 8.4 QWaitCondition 🔶

**一句话作用**：条件变量，实现线程间的等待/唤醒机制。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/camera/camerathread.h`
- 用途：摄像头线程暂停/恢复

**核心用法**：
```cpp
void resume() {
    QMutexLocker locker(&m_mutex);
    m_paused = false;
    m_pauseCond.wakeAll();
}

while (m_paused && m_running) {
    m_pauseCond.wait(&m_mutex);
}
```

**记忆口诀**：`QWaitCondition = 条件变量，wait 等待 + wakeAll 唤醒`

---

### 8.5 QThreadPool 🔶

**一句话作用**：线程池类，管理并复用一组线程，自动调度任务。

**继承关系**：`QObject → QThreadPool`

**本项目使用场景**：
- 文件：`src/utils/pthreadpool.h`
- 用途：车牌识别任务线程池

**核心用法**：
```cpp
QThreadPool *pool = new QThreadPool();
pool->setMaxThreadCount(QThread::idealThreadCount());
RecognitionTask *task = new RecognitionTask(taskId, frame);
task->setAutoDelete(true);
pool->start(task);
pool->waitForDone();
```

**记忆口诀**：`QThreadPool = 线程池，setMaxThreadCount + start + waitForDone`

---

### 8.6 QRunnable 🔶

**一句话作用**：可运行任务类，表示一个可在线程池中执行的任务。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/pthreadpool.h`
- 用途：封装车牌识别任务

**核心用法**：
```cpp
class RecognitionTask : public QRunnable {
    void run() override {
        PlateRecognize::instance()->recognizePlate(m_frame);
    }
};
task->setAutoDelete(true);
```

**记忆口诀**：`QRunnable = 可运行任务，重写 run + setAutoDelete`

---

### 8.7 QSemaphore 🔶

**一句话作用**：信号量，控制同时访问某资源的线程数。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/pthreadpool.h`（已包含，暂未使用）
- 用途：限制并发线程数、实现生产者-消费者队列

**核心用法**：
```cpp
QSemaphore sem(3);
sem.acquire();      // 获取一个资源（没有则等待）
// ... 访问资源 ...
sem.release();      // 释放资源
```

**记忆口诀**：`QSemaphore = 信号量，acquire 获取 + release 释放`

---

## 九、工具类

### 9.1 QObject ✅

**一句话作用**：Qt 对象模型的基类，提供对象树、信号槽、事件机制。

**继承关系**：独立类（所有 Qt 类的根）

**本项目使用场景**：
- 文件：几乎所有 `.h` 文件
- 用途：所有需要信号槽、对象树、事件机制的类的基类

**核心用法**：
```cpp
class MyClass : public QObject {
    Q_OBJECT  // 必须添加这个宏！
signals:
    void mySignal(int value);
public slots:
    void mySlot(int value);
};

QPushButton *btn = new QPushButton(this);  // parent 析构时自动析构 child
connect(sender, &Sender::signal, receiver, &Receiver::slot);
```

**记忆口诀**：`QObject = Qt 根基，Q_OBJECT 宏 + 信号槽 + 对象树`

---

### 9.2 QString ✅

**一句话作用**：Qt 字符串类，Unicode 支持，功能极其丰富。

**继承关系**：独立类

**本项目使用场景**：
- 文件：几乎所有 `.cpp` 文件
- 用途：所有文本操作

**核心用法**：
```cpp
QString s = "Hello";
QString s2 = QStringLiteral("中文");  // 编译期创建，更高效
QString msg = QString("姓名：%1，年龄：%2").arg("张三").arg(25);
int num = s.toInt();
QByteArray bytes = s.toUtf8();
s.isEmpty();
s.trimmed();
s.contains("abc");
```

**记忆口诀**：`QString = Qt 字符串，arg 格式 + toXxx 转换 + trimmed/split`

---

### 9.3 QDebug ✅

**一句话作用**：调试输出类，替代 `printf`/`cout`，支持 Qt 所有类型。

**继承关系**：独立类

**本项目使用场景**：
- 文件：几乎所有 `.cpp` 文件
- 用途：调试日志输出

**核心用法**：
```cpp
qDebug() << "Hello" << 123 << QString("中文");
qDebug() << QStringLiteral("数据库连接成功");
```

**记忆口诀**：`qDebug() << 输出，替代 cout，支持 Qt 类型`

---

### 9.4 QMessageBox 🔶

**一句话作用**：标准消息对话框，快速弹出提示/警告/错误。

**继承关系**：`QObject → QWidget → QDialog → QMessageBox`

**本项目使用场景**：
- 文件：`src/app/main.cpp`、`UI/Login/logindialog.cpp`、`UI/Register/registerdialog.cpp`、`UI/ConfigInit/configinitdialog.cpp`
- 用途：登录失败提示、注册成功提示、配置验证错误

**核心用法**：
```cpp
// 错误弹窗
QMessageBox::critical(this, "错误", "数据库未初始化！");

// 警告弹窗
QMessageBox::warning(this, "提示", "请输入用户名和密码！");

// 信息弹窗
QMessageBox::information(this, "成功", "注册成功");

// 询问弹窗（带返回值）
QMessageBox::StandardButton btn = QMessageBox::critical(
    this, "错误", "配置文件加载失败! 是否重新配置",
    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
if (btn == QMessageBox::Yes) { ... }
```

**记忆口诀**：`QMessageBox = 标准弹窗，critical/warning/information + 按钮组合`

---

### 9.5 QDateTime 🔶

**一句话作用**：日期时间类，处理日期、时间、时间戳。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`UI/MainWindow/mainwindow.cpp`、`src/camera/camerathread.cpp`、`src/database/databasemanager.h`
- 用途：状态栏时间显示、帧率计算、数据库时间筛选

**核心用法**：
```cpp
// 获取当前时间
QDateTime currentTime = QDateTime::currentDateTime();
QString timeStr = currentTime.toString("yyyy-MM-dd HH:mm:ss");

// 获取毫秒时间戳
qint64 ms = QDateTime::currentMSecsSinceEpoch();

// 从 QVariant 转换
QDateTime checkOutTime = query.value(0).toDateTime();
```

**记忆口诀**：`QDateTime = 日期时间，currentDateTime + toString 格式化`

---

### 9.6 QElapsedTimer 🔶

**一句话作用**：高精度计时器，测量代码执行时间。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/camera/camerathread.cpp`
- 用途：帧率计算辅助

**核心用法**：
```cpp
QElapsedTimer timer;
timer.start();
// ... 执行代码 ...
qint64 elapsed = timer.elapsed();  // 毫秒
```

**记忆口诀**：`QElapsedTimer = 计时器，start + elapsed() 获取毫秒`

---

### 9.7 QCursor 🔶

**一句话作用**：光标类，控制鼠标指针样式。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`UI/MainWindow/mainwindow.h`、`UI/Login/logindialog.cpp`
- 用途：窗口拉伸时改变光标形状、按钮悬停手型光标

**核心用法**：
```cpp
// 设置光标
setCursor(Qt::PointingHandCursor);  // 手型
setCursor(Qt::SizeVerCursor);       // 垂直调整
setCursor(Qt::SizeHorCursor);       // 水平调整
setCursor(Qt::SizeFDiagCursor);     // 对角线调整
setCursor(Qt::SizeBDiagCursor);     // 反对角线调整

// 恢复默认
unsetCursor();
```

**记忆口诀**：`QCursor = 光标，setCursor(Qt::XxxCursor) + unsetCursor`

---

### 9.8 QSize / QRect / QPoint ✅

**一句话作用**：几何类，表示大小、矩形区域、坐标点。

**继承关系**：独立类

**本项目使用场景**：
- 文件：几乎所有 `.cpp` 文件
- 用途：设置窗口大小、计算位置、绘制区域

**核心用法**：
```cpp
QSize size(16, 16);                    // 大小
QSize iconSize = QSize(20, 20);
QPoint pos(100, 200);                  // 坐标点
QPoint global = event->globalPosition().toPoint();
QRect rect(0, 0, 400, 300);            // 矩形区域
QRect geo = screen->availableGeometry();
QRectF rectF(0.0, 0.0, 100.5, 100.5);  // 浮点矩形

// 常用操作
rect.adjusted(1, 1, -1, -1);  // 向内收缩
rect.center();                // 获取中心点
rect.contains(pos);           // 是否包含某点
```

**记忆口诀**：`QSize = 大小，QPoint = 坐标，QRect = 矩形，adjusted 收缩`

---

### 9.9 QColor ✅

**一句话作用**：颜色类，表示 RGBA 颜色。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/toastwidget.cpp`、`UI/MainWindow/circleprogress.cpp`、`UI/Login/passwordedit.cpp`、`UI/Login/usernameedit.cpp`、`src/utils/notificationdialog.cpp`
- 用途：绘制颜色、背景色、边框色

**核心用法**：
```cpp
QColor color(0x3498db);           // 十六进制构造
QColor color(59, 130, 246);       // RGB
QColor color(59, 130, 246, 128);  // RGBA（带透明度）
color.setNamedColor("#3498db");   // 命名颜色
color.setNamedColor("red");

// 常用静态颜色
Qt::white;
Qt::black;
Qt::NoPen;  // 无画笔
```

**记忆口诀**：`QColor = 颜色，setNamedColor + RGB 构造 + 透明度`

---

### 9.10 QPen 🔶

**一句话作用**：画笔类，定义如何绘制线条和边框。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`UI/MainWindow/circleprogress.cpp`、`UI/Login/passwordedit.cpp`
- 用途：圆环进度条、输入框边框

**核心用法**：
```cpp
QPen pen(QColor(59, 130, 246), 10, Qt::SolidLine, Qt::RoundCap);
painter.setPen(pen);

// 常用样式
Qt::SolidLine;    // 实线
Qt::RoundCap;     // 圆角线帽
Qt::NoPen;        // 无画笔（不画边框）
```

**记忆口诀**：`QPen = 画笔，颜色 + 宽度 + 线型 + 线帽`

---

### 9.11 QFont 🔶

**一句话作用**：字体类，定义文字的字体、大小、粗细。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`UI/MainWindow/circleprogress.cpp`、`UI/Login/passwordedit.cpp`、`UI/Login/usernameedit.cpp`
- 用途：圆环进度条百分比文字、输入框文字

**核心用法**：
```cpp
QFont font("Arial", 10, QFont::Bold);
painter.setFont(font);

// 常用字体
QFont::Bold;      // 粗体
QFont::Normal;    // 正常
"Microsoft YaHei"; // 微软雅黑
```

**记忆口诀**：`QFont = 字体，字体名 + 大小 + 粗细`

---

### 9.12 QBrush 🔶

**一句话作用**：画刷类，定义如何填充区域。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/toastwidget.cpp`
- 用途：Toast 背景填充、左侧竖条填充

**核心用法**：
```cpp
painter.setBrush(Qt::white);           // 白色填充
painter.setBrush(QColor(0, 0, 0, 40)); // 半透明黑色
painter.setBrush(Qt::NoBrush);         // 不填充（只画边框）
painter.drawRoundedRect(rect(), 10, 10);
```

**记忆口诀**：`QBrush = 画刷，setBrush + drawRoundedRect 填充`

---

### 9.13 QEasingCurve 🔶

**一句话作用**：缓动曲线类，控制动画的速度变化（加速、减速、弹跳等）。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/toastwidget.cpp`、`src/utils/notificationdialog.cpp`
- 用途：Toast 滑入/滑出动画、弹窗缩放动画

**核心用法**：
```cpp
anim->setEasingCurve(QEasingCurve::OutCubic);  // 减速（滑入）
anim->setEasingCurve(QEasingCurve::InCubic);   // 加速（滑出）
anim->setEasingCurve(QEasingCurve::OutBack);   // 回弹（弹窗）
anim->setEasingCurve(QEasingCurve::InQuad);    // 缓出（关闭）
```

**记忆口诀**：`QEasingCurve = 缓动曲线，OutCubic 减速 / InCubic 加速 / OutBack 回弹`

---

### 9.14 QQueue 🔶

**一句话作用**：队列容器（FIFO），继承自 QList。

**继承关系**：`QList → QQueue`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.h`、`src/camera/camerathread.h`、`src/utils/pthreadpool.h`
- 用途：Toast 活跃队列管理、帧队列

**核心用法**：
```cpp
QQueue<ToastWidget *> s_active;
s_active.enqueue(this);           // 入队
ToastWidget *old = s_active.dequeue();  // 出队
s_active.removeOne(this);         // 移除指定元素
s_active.size();                  // 队列大小
```

**记忆口诀**：`QQueue = 队列，enqueue 入队 + dequeue 出队`

---

### 9.15 QList / QVector 🔶

**一句话作用**：动态数组容器，QList 更常用，QVector 连续内存。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/database/databasemanager.h`
- 用途：查询结果集 `QList<QVariantList>`

**核心用法**：
```cpp
QList<QVariantList> resultList;
resultList << row;              // 追加
resultList.append(row);         // 追加
resultList.isEmpty();           // 是否为空

QVector<int> vec;
vec.reserve(100);               // 预分配
```

**记忆口诀**：`QList = 动态数组，append/<< 追加 + isEmpty 判断`

---

### 9.16 QVariant / QVariantMap / QVariantList 🔶

**一句话作用**：通用变量类型，可以存储任意 Qt 数据类型。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/database/databasemanager.h`
- 用途：数据库查询结果的通用容器

**核心用法**：
```cpp
QVariant value = query.value("name");  // 从查询结果获取
QString s = value.toString();
int n = value.toInt();
QDateTime dt = value.toDateTime();

QVariantList row;
row << query.value("id") << query.value("name");

QVariantMap bindValues;
bindValues[":plate"] = "%" + plate + "%";
```

**记忆口诀**：`QVariant = 通用变量，toXxx 转换 + QVariantMap 键值对`

---

### 9.17 QCryptographicHash 🔶

**一句话作用**：加密哈希算法类，支持 MD5、SHA-256 等。

**继承关系**：独立类

**本项目使用场景**：
- 文件：`src/utils/utils.h`
- 用途：密码 SHA-256 加密存储

**核心用法**：
```cpp
QByteArray passwordBytes = password.toUtf8();
QByteArray hashBytes = QCryptographicHash::hash(
    passwordBytes, QCryptographicHash::Sha256);
QString hashStr = QString::fromLatin1(hashBytes.toHex());
```

**记忆口诀**：`QCryptographicHash = 哈希加密，hash(data, Sha256) + toHex`

---

### 9.18 QStyle 🔶

**一句话作用**：样式类，控制控件的外观绘制。

**继承关系**：`QObject → QStyle`

**本项目使用场景**：
- 文件：`src/utils/toastwidget.cpp`
- 用途：动态修改控件属性后刷新样式（QSS 伪状态）

**核心用法**：
```cpp
setProperty("severity", severity);  // 设置动态属性
style()->unpolish(this);            // 取消旧样式
style()->polish(this);              // 应用新样式
```

**记忆口诀**：`QStyle = 样式刷新，setProperty + unpolish + polish`

---

### 9.19 QAction 🔶

**一句话作用**：动作类，表示一个可触发的操作（菜单项、工具栏按钮）。

**继承关系**：`QObject → QAction`

**本项目使用场景**：
- 文件：暂未使用（但常用于菜单和工具栏）
- 用途：菜单项、快捷键绑定

**核心用法**：
```cpp
QAction *action = new QAction("打开", this);
action->setShortcut(QKeySequence("Ctrl+O"));
action->setIcon(QIcon(":/icons/open.svg"));
connect(action, &QAction::triggered, this, &MyClass::onOpen);
menu->addAction(action);
```

**记忆口诀**：`QAction = 动作，setShortcut + triggered 信号`

---

## 十、类外重要概念

### 10.1 Qt 信号与槽 ✅

**一句话作用**：Qt 的核心通信机制，对象之间松耦合的通信方式。

**本项目使用场景**：
- 文件：几乎所有 `.cpp` 文件
- 用途：按钮点击、线程信号、数据更新通知

**核心用法**：
```cpp
// 连接（新式语法，推荐）
connect(sender, &Sender::signal, receiver, &Receiver::slot);

// 连接（Lambda 表达式）
connect(btn, &QPushButton::clicked, this, [this]() {
    // 处理点击
});

// 断开
disconnect(sender, &Sender::signal, receiver, &Receiver::slot);

// 发射信号
emit mySignal(value);
```

**记忆口诀**：`信号槽 = 通信机制，connect(发送者, 信号, 接收者, 槽)`

---

### 10.2 Qt 事件过滤器 🔶

**一句话作用**：在事件到达目标对象之前拦截并处理。

**本项目使用场景**：
- 文件：`UI/MainWindow/mainwindow.cpp`、`UI/ConfigInit/configinitdialog.cpp`、`src/utils/toastwidget.cpp`
- 用途：窗口拉伸、标题栏拖动、父窗口销毁监听

**核心用法**：
```cpp
// 安装过滤器
widget->installEventFilter(this);

// 重写 eventFilter
bool eventFilter(QObject *obj, QEvent *event) override
{
    if (obj == targetWidget && event->type() == QEvent::Resize) {
        // 处理
        return true;  // true = 事件已处理，不再传递
    }
    return QWidget::eventFilter(obj, event);  // 交给父类处理
}
```

**记忆口诀**：`事件过滤器 = 拦截事件，installEventFilter + eventFilter 重写`

---

### 10.3 Qt 对象树 ✅

**一句话作用**：parent-child 关系管理内存，parent 析构时自动析构所有 child。

**本项目使用场景**：
- 文件：所有 `new QWidget(this)` 的地方
- 用途：自动内存管理，避免内存泄漏

**核心用法**：
```cpp
// parent 析构时自动析构 child
QPushButton *btn = new QPushButton(this);
QLabel *label = new QLabel(this);
QHBoxLayout *layout = new QHBoxLayout(this);

// 不需要手动 delete，parent 会处理
```

**记忆口诀**：`对象树 = 自动内存管理，new 时传 this 作为 parent`

---

### 10.4 Qt 样式表（QSS）🔶

**一句话作用**：类似 CSS 的样式系统，控制控件外观。

**本项目使用场景**：
- 文件：`src/utils/toastwidget.cpp`、`src/utils/notificationdialog.cpp`、`UI/MainWindow/mainwindow.cpp`、`UI/Login/logindialog.cpp`
- 用途：所有自定义外观

**核心用法**：
```cpp
// 从文件加载
QFile file(":/styles/main.qss");
if (file.open(QFile::ReadOnly)) {
    setStyleSheet(QLatin1String(file.readAll()));
}

// 内联 QSS
setStyleSheet(R"(
    ToastWidget { background:transparent; }
    QLabel#toastMsg { font-size:13px; color:#1a1a2e; }
    QPushButton#toastClose { color:#bbb; border:none; }
    QPushButton#toastClose:hover { color:#555; }
)");

// 选择器类型
QWidget { }           // 类型选择器
#objectName { }       // ID 选择器
.className { }        // 类选择器
[property="value"] { } // 属性选择器
:hover { }            // 伪状态
```

**记忆口诀**：`QSS = Qt 样式，类似 CSS，#ID :hover [attr] 选择器`

---

## 附录：本项目 Qt 类速查表

| 分类 | 类名 | 掌握度 | 使用文件 |
|---|---|---|---|
| **窗口** | QMainWindow | 🔶 | mainwindow.h |
| | QWidget | ✅ | 所有自定义控件 |
| | QDialog | 🔶 | logindialog/registerdialog/configinitdialog/notificationdialog |
| **布局** | QHBoxLayout | ✅ | toastwidget/notificationdialog |
| | QVBoxLayout | ✅ | toastwidget/notificationdialog |
| **控件** | QLabel | ✅ | toastwidget/notificationdialog/mainwindow |
| | QPushButton | ✅ | toastwidget/notificationdialog/mainwindow |
| | QLineEdit | 🔶 | passwordedit/usernameedit/registerdialog |
| | QButtonGroup | 🔶 | mainwindow.cpp |
| | QTableWidget | 🔶 | managepage/vehicleinfopage |
| **动画** | QPropertyAnimation | ✅ | toastwidget/notificationdialog |
| | QEasingCurve | 🔶 | toastwidget/notificationdialog |
| **绘图** | QPainter | ✅ | toastwidget/circleprogress/logindialog/passwordedit/usernameedit/notificationdialog |
| | QPainterPath | 🔶 | logindialog/passwordedit/usernameedit/notificationdialog |
| | QPixmap | ✅ | logindialog/passwordedit/usernameedit/mainwindow |
| | QIcon | ✅ | mainwindow.cpp |
| | QImage | 🔶 | mainwindow/camerathread |
| | QColor | ✅ | 多个绘制文件 |
| | QPen | 🔶 | circleprogress/passwordedit |
| | QFont | 🔶 | circleprogress/passwordedit/usernameedit |
| | QBrush | 🔶 | toastwidget.cpp |
| | QGraphicsDropShadowEffect | 🔶 | configinitdialog.cpp |
| **事件** | QTimer | ✅ | mainwindow/toastwidget/notificationdialog |
| | QEvent | 🔶 | mainwindow/toastwidget/configinitdialog |
| | QMouseEvent | 🔶 | mainwindow/logindialog/configinitdialog |
| | QPaintEvent | 🔶 | 所有自定义绘制 |
| | QKeyEvent | 🔶 | notificationdialog |
| | QFocusEvent | 🔶 | passwordedit/usernameedit |
| **文件** | QFile | ✅ | initfile/mainwindow/logindialog/configinitdialog/databasemanager |
| | QDir | 🔶 | initfile.cpp |
| | QFileInfo | 🔶 | initfile.cpp |
| | QJsonDocument | 🔶 | initfile/databasemanager |
| | QJsonObject | 🔶 | initfile.h |
| | QJsonArray | 🔶 | initfile.h |
| | QStandardPaths | 🔶 | initfile.h（未使用） |
| | QCoreApplication | 🔶 | initfile/databasemanager |
| | QApplication | ✅ | main.cpp/toastwidget/notificationdialog |
| | QGuiApplication | 🔶 | toastwidget/notificationdialog |
| | QScreen | 🔶 | toastwidget/notificationdialog |
| | QClipboard | 🔶 | notificationdialog.cpp |
| | QStyleFactory | 🔶 | main.cpp（未使用） |
| | QLatin1String | 🔶 | initfile/mainwindow/logindialog |
| **数据库** | QSqlDatabase | 🔶 | databasemanager.h |
| | QSqlQuery | 🔶 | databasemanager/mysqlinit |
| | QSqlError | 🔶 | databasemanager/mysqlinit |
| **线程** | QThread | 🔶 | camerathread/pthreadpool |
| | QMutex | 🔶 | camerathread/pthreadpool |
| | QMutexLocker | 🔶 | camerathread/pthreadpool |
| | QWaitCondition | 🔶 | camerathread.h |
| | QThreadPool | 🔶 | pthreadpool.h |
| | QRunnable | 🔶 | pthreadpool.h |
| | QSemaphore | 🔶 | pthreadpool.h（未使用） |
| **工具** | QObject | ✅ | 所有类 |
| | QString | ✅ | 所有文件 |
| | QDebug | ✅ | 所有文件 |
| | QMessageBox | 🔶 | main/logindialog/registerdialog/configinitdialog |
| | QDateTime | 🔶 | mainwindow/camerathread/databasemanager |
| | QElapsedTimer | 🔶 | camerathread.cpp |
| | QCursor | 🔶 | mainwindow/logindialog |
| | QSize/QRect/QPoint | ✅ | 几乎所有文件 |
| | QQueue | 🔶 | toastwidget/camerathread/pthreadpool |
| | QList/QVector | 🔶 | databasemanager.h |
| | QVariant/QVariantMap | 🔶 | databasemanager.h |
| | QCryptographicHash | 🔶 | utils.h |
| | QStyle | 🔶 | toastwidget.cpp |

---

## 附录：常用 Qt 全局函数/枚举速查

| 函数/枚举 | 作用 |
|---|---|
| `qDebug()` | 调试输出 |
| `QStringLiteral()` | 编译期创建 QString（中文推荐） |
| `Q_UNUSED()` | 消除未使用参数警告 |
| `QThread::idealThreadCount()` | 获取 CPU 核心数 |
| `Qt::KeepAspectRatio` | 保持宽高比缩放 |
| `Qt::SmoothTransformation` | 平滑缩放算法 |
| `Qt::AlignCenter` | 居中对齐 |
| `Qt::FramelessWindowHint` | 无边框窗口 |
| `Qt::WA_TranslucentBackground` | 透明背景 |
| `Qt::WA_DeleteOnClose` | 关闭时自动删除 |
| `Qt::WA_ShowWithoutActivating` | 显示但不抢焦点 |
| `Qt::Tool` | 工具窗口（无任务栏图标） |
| `Qt::WindowStaysOnTopHint` | 窗口置顶 |
| `Qt::LeftButton` | 鼠标左键 |
| `Qt::Key_Escape` | ESC 键 |
| `Qt::NoPen` | 无画笔 |
| `Qt::NoBrush` | 无画刷 |
| `Qt::RoundCap` | 圆角线帽 |
| `Qt::SolidLine` | 实线 |
| `Qt::Antialiasing` | 抗锯齿 |
| `Qt::PointingHandCursor` | 手型光标 |

---

> **文档维护记录**
> - 2026-07-08：首次创建，扫描项目所有 Qt 类，共记录 **80+ 个类**
> - 下次更新：每写完一天代码后，补充新用到的类