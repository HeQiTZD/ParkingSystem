# Settings Dialog Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 MainWindow 的设置按钮实现可修改数据库连接、停车场参数和识别置信度的弹窗。

**Architecture:** 纯代码构建 UI 的 QDialog，复用 ConfigInitDialog 的无边框+投影+拖拽模式，直接读写 InitFile 单例，DatabaseManager 仅用于测试连接。

**Tech Stack:** Qt 5/C++11，qmake，QDialog（FramelessWindowHint），InitFile（JSON 配置），DatabaseManager

**Spec:** `docs/superpowers/specs/2026-07-21-settings-dialog-design.md`

## Global Constraints

- 只改 3 类配置：数据库、停车场、识别置信度（不含摄像头、系统信息、模型路径）
- 数据库参数保存后重启生效，非立即重连
- 测试连接用独立临时连接，不影响当前连接
- 复用 `:/styles/config.qss` 样式

---

## File Structure

| Role | Path | Responsibility |
|------|------|----------------|
| **Create** | `UI/Settings/settingsdialog.h` | 类声明 |
| **Create** | `UI/Settings/settingsdialog.cpp` | UI 构建 + 配置读写 + 测试连接 |
| **Modify** | `UI/MainWindow/mainwindow.cpp:328-331` | onSetButton() 弹出设置弹窗 |

纯代码构建 UI，无 .ui 文件，无需修改 ParkingSystem.pro。

---

### Task 1: SettingsDialog 头文件

**Files:**
- Create: `UI/Settings/settingsdialog.h`

**Interfaces:**
- Produces: `SettingsDialog` 类 — 供 `MainWindow::onSetButton()` 示例化

- [ ] **Step 1: 创建头文件**

```cpp
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QPoint>

class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QSlider;
class QLabel;
class DatabaseManager;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(DatabaseManager *db, QWidget *parent = nullptr);
    ~SettingsDialog();

signals:
    void settingsSaved();
    void dbConfigChanged();

private slots:
    void onTestConnection();
    void onSave();

private:
    void setupUi();
    void setupTitleBar();
    void loadSettings();

    // 数据库控件
    QLineEdit  *m_hostEdit;
    QSpinBox   *m_portSpin;
    QLineEdit  *m_dbNameEdit;
    QLineEdit  *m_usernameEdit;
    QLineEdit  *m_passwordEdit;

    // 停车场控件
    QLineEdit      *m_parkingNameEdit;
    QDoubleSpinBox *m_priceSpin;
    QSpinBox       *m_capacitySpin;
    QSpinBox       *m_freeMinSpin;

    // 识别控件
    QSlider *m_confidenceSlider;
    QLabel  *m_confidenceLabel;

    DatabaseManager *m_db;
    bool m_dragging = false;
    QPoint m_dragPosition;

    // 保存前快照，用于比对数据库参数是否变更
    QString m_oldHost;
    int     m_oldPort = 0;
    QString m_oldDbName;
    QString m_oldUsername;
    QString m_oldPassword;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // SETTINGSDIALOG_H
```

- [ ] **Step 2: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add UI/Settings/settingsdialog.h && git commit -m "feat: add SettingsDialog header"
```

---

### Task 2: SettingsDialog 实现 — 窗口框架 + UI 构建

**Files:**
- Create: `UI/Settings/settingsdialog.cpp`

**Interfaces:**
- Consumes: `SettingsDialog` 声明 (Task 1)
- Produces: `setupUi()`, `setupTitleBar()`, `eventFilter()`, 构造/析构函数

- [ ] **Step 1: 实现构造函数、setupTitleBar、eventFilter**

```cpp
#include "settingsdialog.h"
#include "src/utils/initfile.h"
#include "src/database/databasemanager.h"
#include "src/utils/messageType.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QFile>
#include <QMouseEvent>
#include <QSqlDatabase>

SettingsDialog::SettingsDialog(DatabaseManager *db, QWidget *parent)
    : QDialog(parent), m_db(db)
{
    setupUi();
    setupTitleBar();
    loadSettings();

    // 置信度滑块-标签联动
    connect(m_confidenceSlider, &QSlider::valueChanged, this, [this](int val) {
        m_confidenceLabel->setText(QString::number(val / 100.0, 'f', 2));
    });
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::setupTitleBar()
{
    // 匹配 config.qss 的顶层选择器 #ConfigInitDialog
    setObjectName("ConfigInitDialog");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(600, 550);

    QFile styleFile(":/styles/config.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    // 投影阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    // 找顶层容器并设置 shadow
    QWidget *root = findChild<QWidget *>("mainContainer");
    if (root)
        root->setGraphicsEffect(shadow);
}

bool SettingsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->objectName() == "titleBar") {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            if (me->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragPosition = me->globalPos() - frameGeometry().topLeft();
                return true;
            }
            break;
        case QEvent::MouseMove:
            if (m_dragging && (me->buttons() & Qt::LeftButton)) {
                move(me->globalPos() - m_dragPosition);
                return true;
            }
            break;
        case QEvent::MouseButtonRelease:
            if (me->button() == Qt::LeftButton) {
                m_dragging = false;
                return true;
            }
            break;
        default:
            break;
        }
    }
    return QDialog::eventFilter(obj, event);
}
```

- [ ] **Step 2: 实现 setupUi — 构建完整布局**

```cpp
void SettingsDialog::setupUi()
{
    // ========== 根布局 ==========
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);

    // ========== 主容器（配合 config.qss 的 #mainContainer） ==========
    auto *mainContainer = new QWidget(this);
    mainContainer->setObjectName("mainContainer");
    auto *mainLayout = new QVBoxLayout(mainContainer);

    // ---- 标题栏 ----
    auto *titleBar = new QWidget(mainContainer);
    titleBar->setObjectName("titleBar");
    titleBar->setFixedHeight(40);
    titleBar->installEventFilter(this);

    auto *titleLayout = new QHBoxLayout(titleBar);
    auto *titleLabel = new QLabel(QStringLiteral("设置"), titleBar);
    titleLabel->setObjectName("titleLabel");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    auto *btnClose = new QPushButton(titleBar);
    btnClose->setObjectName("btnClose");
    btnClose->setFixedSize(30, 30);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::reject);
    titleLayout->addWidget(btnClose);

    mainLayout->addWidget(titleBar);

    // ---- 滚动区域 ----
    auto *scrollArea = new QScrollArea(mainContainer);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    auto *scrollContent = new QWidget(scrollArea);
    auto *scrollLayout = new QVBoxLayout(scrollContent);

    // ===== 数据库连接分组 =====
    auto *dbGroup = new QGroupBox(QStringLiteral("数据库连接"), scrollContent);
    auto *dbForm = new QFormLayout(dbGroup);

    m_hostEdit    = new QLineEdit(dbGroup);
    m_portSpin    = new QSpinBox(dbGroup);
    m_dbNameEdit  = new QLineEdit(dbGroup);
    m_usernameEdit = new QLineEdit(dbGroup);
    m_passwordEdit = new QLineEdit(dbGroup);

    m_hostEdit->setPlaceholderText("localhost");
    m_portSpin->setRange(1, 65535);
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    dbForm->addRow(QStringLiteral("主机地址"), m_hostEdit);
    dbForm->addRow(QStringLiteral("端口号"),   m_portSpin);
    dbForm->addRow(QStringLiteral("数据库名"), m_dbNameEdit);
    dbForm->addRow(QStringLiteral("用户名"),   m_usernameEdit);
    dbForm->addRow(QStringLiteral("密码"),     m_passwordEdit);

    auto *testBtn = new QPushButton(QStringLiteral("测试连接"), dbGroup);
    connect(testBtn, &QPushButton::clicked, this, &SettingsDialog::onTestConnection);
    dbForm->addRow(nullptr, testBtn);

    scrollLayout->addWidget(dbGroup);

    // ===== 停车场参数分组 =====
    auto *parkGroup = new QGroupBox(QStringLiteral("停车场参数"), scrollContent);
    auto *parkForm = new QFormLayout(parkGroup);

    m_parkingNameEdit = new QLineEdit(parkGroup);
    m_priceSpin       = new QDoubleSpinBox(parkGroup);
    m_capacitySpin    = new QSpinBox(parkGroup);
    m_freeMinSpin     = new QSpinBox(parkGroup);

    m_priceSpin->setRange(0.0, 9999.0);
    m_priceSpin->setSuffix(QStringLiteral(" 元/小时"));
    m_capacitySpin->setRange(1, 99999);
    m_freeMinSpin->setRange(0, 120);
    m_freeMinSpin->setSuffix(QStringLiteral(" 分钟"));

    parkForm->addRow(QStringLiteral("停车场名称"), m_parkingNameEdit);
    parkForm->addRow(QStringLiteral("每小时价格"), m_priceSpin);
    parkForm->addRow(QStringLiteral("总车位数"),   m_capacitySpin);
    parkForm->addRow(QStringLiteral("免费分钟"),   m_freeMinSpin);

    scrollLayout->addWidget(parkGroup);

    // ===== 识别参数分组 =====
    auto *recGroup = new QGroupBox(QStringLiteral("识别参数"), scrollContent);
    auto *recLayout = new QVBoxLayout(recGroup);

    auto *sliderRow = new QHBoxLayout();
    auto *sliderLabel = new QLabel(QStringLiteral("置信度阈值"), recGroup);
    m_confidenceSlider = new QSlider(Qt::Horizontal, recGroup);
    m_confidenceSlider->setRange(0, 100);  // 0.00 ~ 1.00
    m_confidenceLabel = new QLabel("0.70", recGroup);
    m_confidenceLabel->setFixedWidth(40);

    sliderRow->addWidget(sliderLabel);
    sliderRow->addWidget(m_confidenceSlider);
    sliderRow->addWidget(m_confidenceLabel);
    recLayout->addLayout(sliderRow);

    scrollLayout->addWidget(recGroup);
    scrollLayout->addStretch();

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    // ---- 底部按钮 ----
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    auto *btnSave = new QPushButton(QStringLiteral("保存"), mainContainer);
    auto *btnCancel = new QPushButton(QStringLiteral("取消"), mainContainer);
    btnSave->setObjectName("btnSubmit");
    btnCancel->setObjectName("btnCancel");
    connect(btnSave, &QPushButton::clicked, this, &SettingsDialog::onSave);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(btnSave);
    btnRow->addWidget(btnCancel);
    mainLayout->addLayout(btnRow);

    rootLayout->addWidget(mainContainer);
}
```

- [ ] **Step 3: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add UI/Settings/settingsdialog.cpp && git commit -m "feat: add SettingsDialog UI construction"
```

---

### Task 3: SettingsDialog 实现 — loadSettings + onSave + onTestConnection

**Files:**
- Modify: `UI/Settings/settingsdialog.cpp`

**Interfaces:**
- Consumes: `setupUi()` (Task 2), `InitFile::instance()`, `DatabaseManager::connectDatabase()`

- [ ] **Step 1: 实现 loadSettings**

```cpp
void SettingsDialog::loadSettings()
{
    InitFile &cfg = InitFile::instance();

    // 数据库
    m_oldHost     = cfg.getDbHost();
    m_oldPort     = cfg.getDbPort();
    m_oldDbName   = cfg.getDbName();
    m_oldUsername = cfg.getDbUsername();
    m_oldPassword = cfg.getDbPassword();

    m_hostEdit->setText(m_oldHost);
    m_portSpin->setValue(m_oldPort);
    m_dbNameEdit->setText(m_oldDbName);
    m_usernameEdit->setText(m_oldUsername);
    m_passwordEdit->setText(m_oldPassword);

    // 停车场
    m_parkingNameEdit->setText(cfg.getParkingName());
    m_priceSpin->setValue(cfg.getParkingPrice());
    m_capacitySpin->setValue(cfg.getParkingCapacity());
    m_freeMinSpin->setValue(cfg.getFreeMinutes());

    // 识别
    int confidenceVal = qBound(0, static_cast<int>(cfg.getConfidenceThreshold() * 100), 100);
    m_confidenceSlider->setValue(confidenceVal);
    m_confidenceLabel->setText(QString::number(confidenceVal / 100.0, 'f', 2));
}
```

- [ ] **Step 2: 实现 onSave**

```cpp
void SettingsDialog::onSave()
{
    InitFile &cfg = InitFile::instance();

    // 数据库（不设置 modelPath 参数，保存旧值）
    cfg.setDbConfig(
        m_hostEdit->text(),
        m_portSpin->value(),
        m_dbNameEdit->text(),
        m_usernameEdit->text(),
        m_passwordEdit->text()
    );

    // 停车场
    cfg.setParkingConfig(
        m_parkingNameEdit->text(),
        m_priceSpin->value(),
        m_capacitySpin->value(),
        m_freeMinSpin->value()
    );

    // 识别（保留旧 modelPath，只更新阈值）
    cfg.setRecognitionConfig(
        cfg.getModelPath(),
        m_confidenceSlider->value() / 100.0
    );

    if (!cfg.saveConfig()) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("配置保存失败"));
        return;
    }

    // 比对数据库参数是否变更
    bool dbChanged =
        m_hostEdit->text()    != m_oldHost     ||
        m_portSpin->value()   != m_oldPort     ||
        m_dbNameEdit->text()  != m_oldDbName   ||
        m_usernameEdit->text()!= m_oldUsername ||
        m_passwordEdit->text()!= m_oldPassword;

    if (dbChanged) {
        emit dbConfigChanged();
        QMessageBox::information(this, QStringLiteral("提示"),
            QStringLiteral("数据库配置已保存，重启应用后生效"));
    }

    emit settingsSaved();
    accept();
}
```

- [ ] **Step 3: 实现 onTestConnection**

```cpp
void SettingsDialog::onTestConnection()
{
    // 临时连接名，避免与主连接冲突
    const QString testConn = "settings_test_connection";

    // 移除可能残留的同名连接
    if (QSqlDatabase::contains(testConn))
        QSqlDatabase::removeDatabase(testConn);

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", testConn);
    db.setHostName(m_hostEdit->text());
    db.setPort(m_portSpin->value());
    db.setDatabaseName(m_dbNameEdit->text());
    db.setUserName(m_usernameEdit->text());
    db.setPassword(m_passwordEdit->text());

    if (db.open()) {
        QMessageBox::information(this, QStringLiteral("成功"), QStringLiteral("数据库连接成功"));
        db.close();
    } else {
        QMessageBox::warning(this, QStringLiteral("失败"),
            QStringLiteral("连接失败：") + db.lastError().text());
    }

    QSqlDatabase::removeDatabase(testConn);
}
```

- [ ] **Step 4: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add UI/Settings/settingsdialog.cpp && git commit -m "feat: add SettingsDialog load/save/test-connection logic"
```

---

### Task 4: 接入 MainWindow

**Files:**
- Modify: `UI/MainWindow/mainwindow.cpp:328-331`

**Interfaces:**
- Consumes: `SettingsDialog` (Task 1-3)

- [ ] **Step 1: 添加 include**

在 `mainwindow.cpp` 顶端已有 include 区域末尾添加：

```cpp
#include "UI/Settings/settingsdialog.h"
```

- [ ] **Step 2: 替换 onSetButton 空实现**

将第 328-331 行的：

```cpp
void MainWindow::onSetButton()
{
    return;
}
```

替换为：

```cpp
void MainWindow::onSetButton()
{
    auto *dialog = new SettingsDialog(m_db, this);
    dialog->setModal(true);
    dialog->exec();
    dialog->deleteLater();
}
```

- [ ] **Step 3: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add UI/MainWindow/mainwindow.cpp && git commit -m "feat: wire SettingsDialog to setButton"
```

---

### Task 5: 构建验证 + 冒烟测试

**Files:** 无需修改代码

- [ ] **Step 1: 构建项目**

```bash
cd D:/QTproject/ParkingSystem && qmake && make clean && make -j$(nproc)
```

预期：编译通过，无警告。

- [ ] **Step 2: 冒烟测试清单**

| # | 操作 | 预期结果 |
|---|------|----------|
| 1 | 点击主窗口设置按钮 | 弹出设置弹窗，各字段值与 `config.json` 一致 |
| 2 | 修改停车场价格→保存→重新打开 | 新价格已持久化 |
| 3 | 修改数据库密码→保存 | 弹出"重启后生效"提示 |
| 4 | 输入错误数据库参数→测试连接 | 弹出"连接失败"及具体错误 |
| 5 | 正确数据库参数→测试连接 | 弹出"连接成功" |
| 6 | 拖动滑块→置信度标签 | 实时显示 0.00~1.00 |
| 7 | 点击取消→重新打开 | 所有值恢复到保存前的值 |

- [ ] **Step 3: 提交（如有微调）**

```bash
cd D:/QTproject/ParkingSystem && git add -A && git commit -m "chore: build verification, no code changes"
```
