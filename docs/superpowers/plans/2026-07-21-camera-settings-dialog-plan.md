# Camera Settings Dialog Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 CameraManagement 页面的「⚙ 设置」按钮实现摄像头配置弹窗，支持全局采集参数和每路摄像头独立属性编辑。

**Architecture:** .ui 静态骨架（标题栏 + 全局区 + scrollArea + 按钮）+ C++ 代码动态创建摄像头行。复用 SettingsDialog 的无边框+投影+拖拽窗口模式，独立 `camerasettings.qss` 样式文件。设置保存后重启应用生效。

**Tech Stack:** Qt 6 (Widgets), C++17, qmake, InitFile (JSON config), CameraManager

**Spec:** `docs/superpowers/specs/2026-07-21-camera-settings-dialog-design.md`

---

## File Structure

| Role | Path | Responsibility |
|------|------|----------------|
| **Create** | `UI/CameraManagement/camerasettingsdialog.ui` | Qt Designer 表单骨架（标题栏+全局区+scrollArea+按钮） |
| **Create** | `UI/CameraManagement/camerasettingsdialog.h` | 类声明 |
| **Create** | `UI/CameraManagement/camerasettingsdialog.cpp` | 逻辑实现（load/save/buildCameraRows） |
| **Create** | `styles/camerasettings.qss` | 专属样式表（`#CameraSettingsDialog` 作用域） |
| **Modify** | `styles/styles.qrc` | 注册 `camerasettings.qss` |
| **Modify** | `ParkingSystem.pro` | 注册 .h/.cpp/.ui + RESOURCES |
| **Modify** | `UI/CameraManagement/cameramanagement.cpp:38-40` | 设置按钮连接 → CameraSettingsDialog |
| **Modify** | `src/camera/cameramanager.cpp:32-83` | scanCameras() 支持 per-camera 分辨率/FPS |

---

### Task 1: .ui 表单骨架

**Files:**
- Create: `UI/CameraManagement/camerasettingsdialog.ui`

**注意：** 尺寸 620×540。静态部分包含标题栏、全局采集参数区、scrollArea 占位、底部按钮栏。摄像头列表由 Task 4 的 `buildCameraRows()` 动态填充到 scrollArea 内的 `cameraListLayout` 中。

- [ ] **Step 1: 创建 camerasettingsdialog.ui**

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>CameraSettingsDialog</class>
 <widget class="QDialog" name="CameraSettingsDialog">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>620</width>
    <height>540</height>
   </rect>
  </property>
  <property name="minimumSize">
   <size>
    <width>620</width>
    <height>540</height>
   </size>
  </property>
  <property name="maximumSize">
   <size>
    <width>620</width>
    <height>540</height>
   </size>
  </property>
  <property name="windowTitle">
   <string>摄像头设置</string>
  </property>
  <layout class="QVBoxLayout" name="outerLayout">
   <property name="spacing"><number>0</number></property>
   <property name="leftMargin"><number>10</number></property>
   <property name="topMargin"><number>10</number></property>
   <property name="rightMargin"><number>10</number></property>
   <property name="bottomMargin"><number>10</number></property>
   <item>
    <widget class="QWidget" name="mainContainer">
     <layout class="QVBoxLayout" name="containerLayout">
      <property name="spacing"><number>0</number></property>
      <property name="leftMargin"><number>0</number></property>
      <property name="topMargin"><number>0</number></property>
      <property name="rightMargin"><number>0</number></property>
      <property name="bottomMargin"><number>0</number></property>
      <item>
       <!-- ===== 标题栏 ===== -->
       <widget class="QWidget" name="titleBar">
        <property name="minimumSize"><size><width>0</width><height>40</height></size></property>
        <property name="maximumSize"><size><width>16777215</width><height>40</height></size></property>
        <layout class="QHBoxLayout" name="titleBarLayout">
         <property name="spacing"><number>8</number></property>
         <property name="leftMargin"><number>0</number></property>
         <property name="topMargin"><number>0</number></property>
         <property name="rightMargin"><number>16</number></property>
         <property name="bottomMargin"><number>0</number></property>
         <item>
          <spacer name="titleSpacer">
           <property name="orientation"><enum>Qt::Horizontal</enum></property>
           <property name="sizeHint" stdset="0"><size><width>40</width><height>20</height></size></property>
          </spacer>
         </item>
         <item>
          <widget class="QPushButton" name="btnClose">
           <property name="minimumSize"><size><width>36</width><height>28</height></size></property>
           <property name="maximumSize"><size><width>36</width><height>28</height></size></property>
           <property name="cursor"><cursorShape>PointingHandCursor</cursorShape></property>
           <property name="text"><string>×</string></property>
          </widget>
         </item>
        </layout>
       </widget>
      </item>
      <item>
       <!-- ===== 内容滚动区 ===== -->
       <widget class="QScrollArea" name="scrollArea">
        <property name="widgetResizable"><bool>true</bool></property>
        <property name="frameShape"><enum>QFrame::NoFrame</enum></property>
        <widget class="QWidget" name="scrollContent">
         <layout class="QVBoxLayout" name="scrollLayout">
          <property name="spacing"><number>15</number></property>
          <property name="leftMargin"><number>30</number></property>
          <property name="topMargin"><number>20</number></property>
          <property name="rightMargin"><number>30</number></property>
          <property name="bottomMargin"><number>20</number></property>
          <!-- ===== 全局采集参数区 ===== -->
          <item>
           <widget class="QLabel" name="globalSectionLabel">
            <property name="text">
             <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=" font-size:12pt; font-weight:600;"&gt;全局采集参数&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
            </property>
           </widget>
          </item>
          <item>
           <layout class="QHBoxLayout" name="globalWidthLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblGlobalWidth">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;分辨率&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item>
             <widget class="QComboBox" name="cmbGlobalWidth">
              <property name="editable"><bool>true</bool></property>
              <item><property name="text"><string>480</string></property></item>
              <item><property name="text"><string>640</string></property></item>
              <item><property name="text"><string>800</string></property></item>
              <item><property name="text"><string>1024</string></property></item>
              <item><property name="text"><string>1280</string></property></item>
              <item><property name="text"><string>1920</string></property></item>
              <item><property name="text"><string>2560</string></property></item>
             </widget>
            </item>
            <item>
             <widget class="QLabel" name="lblMultiply">
              <property name="text"><string>×</string></property>
              <property name="alignment"><set>Qt::AlignCenter</set></property>
             </widget>
            </item>
            <item>
             <widget class="QComboBox" name="cmbGlobalHeight">
              <property name="editable"><bool>true</bool></property>
              <item><property name="text"><string>480</string></property></item>
              <item><property name="text"><string>640</string></property></item>
              <item><property name="text"><string>800</string></property></item>
              <item><property name="text"><string>1024</string></property></item>
              <item><property name="text"><string>1280</string></property></item>
              <item><property name="text"><string>1920</string></property></item>
              <item><property name="text"><string>2560</string></property></item>
             </widget>
            </item>
           </layout>
          </item>
          <item>
           <layout class="QHBoxLayout" name="globalFpsLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblGlobalFps">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;帧率&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item>
             <widget class="QComboBox" name="cmbGlobalFps">
              <property name="editable"><bool>true</bool></property>
              <item><property name="text"><string>15</string></property></item>
              <item><property name="text"><string>20</string></property></item>
              <item><property name="text"><string>25</string></property></item>
              <item><property name="text"><string>30</string></property></item>
             </widget>
            </item>
           </layout>
          </item>
          <!-- ===== 摄像头列表区（动态填充） ===== -->
          <item>
           <widget class="QLabel" name="cameraListSectionLabel">
            <property name="text">
             <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=" font-size:12pt; font-weight:600;"&gt;摄像头列表&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
            </property>
           </widget>
          </item>
          <item>
           <widget class="QWidget" name="cameraListContainer">
            <layout class="QVBoxLayout" name="cameraListLayout">
             <property name="spacing"><number>10</number></property>
             <property name="leftMargin"><number>0</number></property>
             <property name="topMargin"><number>0</number></property>
             <property name="rightMargin"><number>0</number></property>
             <property name="bottomMargin"><number>0</number></property>
            </layout>
           </widget>
          </item>
          <!-- ===== 弹簧 + 按钮 ===== -->
          <item>
           <spacer name="buttonTopSpacer">
            <property name="orientation"><enum>Qt::Vertical</enum></property>
            <property name="sizeType"><enum>QSizePolicy::Expanding</enum></property>
           </spacer>
          </item>
          <item>
           <layout class="QHBoxLayout" name="buttonLayout">
            <property name="spacing"><number>20</number></property>
            <item>
             <spacer name="buttonLeftSpacer">
              <property name="orientation"><enum>Qt::Horizontal</enum></property>
              <property name="sizeHint" stdset="0"><size><width>40</width><height>20</height></size></property>
             </spacer>
            </item>
            <item>
             <widget class="QPushButton" name="btnCancel">
              <property name="minimumSize"><size><width>0</width><height>40</height></size></property>
              <property name="text"><string>取消</string></property>
             </widget>
            </item>
            <item>
             <widget class="QPushButton" name="btnSubmit">
              <property name="minimumSize"><size><width>0</width><height>40</height></size></property>
              <property name="text"><string>保存</string></property>
             </widget>
            </item>
           </layout>
          </item>
         </layout>
        </widget>
       </widget>
      </item>
     </layout>
    </widget>
   </item>
  </layout>
 </widget>
 <resources/>
 <connections/>
</ui>
```

- [ ] **Step 2: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add UI/CameraManagement/camerasettingsdialog.ui && git commit -m "feat: add CameraSettingsDialog .ui form skeleton"
```

---

### Task 2: 专属样式文件 + 注册

**Files:**
- Create: `styles/camerasettings.qss`
- Modify: `styles/styles.qrc`

样式从 `config.qss` 复制骨架，将所有 `#ConfigInitDialog` 替换为 `#CameraSettingsDialog`，并新增摄像头行分组框、ComboBox、QRadioButton 样式。

- [ ] **Step 1: 创建 camerasettings.qss**

```css
/* ==================== CameraSettingsDialog 样式 ==================== */
/* 作用域限制：仅影响 CameraSettingsDialog (#CameraSettingsDialog) */

/* 对话框容器 */
#CameraSettingsDialog {
    background-color: #ffffff;
}

/* -------------------- 分组标题 -------------------- */
#CameraSettingsDialog QLabel#globalSectionLabel,
#CameraSettingsDialog QLabel#cameraListSectionLabel {
    color: #1a1a2e;
    padding: 8px 0;
    border-bottom: 2px solid #3b82f6;
    margin-top: 10px;
}

/* -------------------- 标签样式 -------------------- */
#CameraSettingsDialog QLabel#lblGlobalWidth,
#CameraSettingsDialog QLabel#lblGlobalFps {
    color: #475569;
    font-size: 14px;
    min-width: 80px;
    max-width: 80px;
}

/* -------------------- 静态输入控件 -------------------- */
#CameraSettingsDialog QComboBox {
    border: 1px solid #e2e8f0;
    border-radius: 6px;
    padding: 10px 14px;
    font-size: 14px;
    color: #1e293b;
    background-color: #f8fafc;
    min-height: 20px;
}

#CameraSettingsDialog QComboBox:hover {
    border-color: #94a3b8;
}

#CameraSettingsDialog QComboBox:focus {
    border-color: #3b82f6;
    background-color: #ffffff;
}

/* -------------------- 提交按钮 -------------------- */
#CameraSettingsDialog QPushButton#btnSubmit {
    background-color: #3b82f6;
    color: #ffffff;
    border: none;
    border-radius: 6px;
    padding: 10px 24px;
    font-size: 14px;
    font-weight: 600;
}

#CameraSettingsDialog QPushButton#btnSubmit:hover {
    background-color: #2563eb;
}

#CameraSettingsDialog QPushButton#btnSubmit:pressed {
    background-color: #1d4ed8;
}

/* -------------------- 取消按钮 -------------------- */
#CameraSettingsDialog QPushButton#btnCancel {
    background-color: #ffffff;
    color: #475569;
    border: 1px solid #e2e8f0;
    border-radius: 6px;
    padding: 10px 24px;
    font-size: 14px;
}

#CameraSettingsDialog QPushButton#btnCancel:hover {
    background-color: #f8fafc;
    border-color: #94a3b8;
}

#CameraSettingsDialog QPushButton#btnCancel:pressed {
    background-color: #f1f5f9;
}

/* ==================== 标题栏样式 ==================== */

#CameraSettingsDialog #titleBar {
    background-color: #ffffff;
    border-top-left-radius: 8px;
    border-top-right-radius: 8px;
    border-bottom: 1px solid #f1f5f9;
}

#CameraSettingsDialog QPushButton#btnClose {
    background: transparent;
    border: none;
    border-radius: 4px;
    color: #94a3b8;
    font-size: 14px;
    font-weight: bold;
    min-width: 36px;
    max-width: 36px;
    min-height: 28px;
    max-height: 28px;
    padding: 0px;
}

#CameraSettingsDialog QPushButton#btnClose:hover {
    background-color: #ef4444;
    color: #ffffff;
}

#CameraSettingsDialog QPushButton#btnClose:pressed {
    background-color: #dc2626;
}

/* ==================== 主容器样式 ==================== */
#CameraSettingsDialog #mainContainer {
    background-color: #ffffff;
    border-radius: 8px;
}

/* ==================== 动态摄像头行样式 ==================== */

/* 摄像头行分组框 */
#CameraSettingsDialog QFrame[objectName^="camRow_"] {
    background-color: #f8fafc;
    border: 1px solid #e2e8f0;
    border-radius: 8px;
    padding: 12px;
}

/* 摄像头行内的标题 */
#CameraSettingsDialog QLabel[objectName^="cam_title_"] {
    color: #1a1a2e;
    font-size: 12pt;
    font-weight: 600;
    padding-bottom: 4px;
}

/* 摄像头行内的标签 */
#CameraSettingsDialog QLabel[objectName^="cam_lbl_"] {
    color: #475569;
    font-size: 13px;
}

/* 摄像头行内的 QLineEdit */
#CameraSettingsDialog QLineEdit[objectName^="cam_name_"],
#CameraSettingsDialog QLineEdit[objectName^="cam_location_"] {
    border: 1px solid #e2e8f0;
    border-radius: 4px;
    padding: 6px 10px;
    font-size: 13px;
    color: #1e293b;
    background-color: #ffffff;
}

#CameraSettingsDialog QLineEdit[objectName^="cam_name_"]:focus,
#CameraSettingsDialog QLineEdit[objectName^="cam_location_"]:focus {
    border-color: #3b82f6;
}

/* 摄像头行内的 QComboBox */
#CameraSettingsDialog QComboBox[objectName^="cam_width_"],
#CameraSettingsDialog QComboBox[objectName^="cam_height_"],
#CameraSettingsDialog QComboBox[objectName^="cam_fps_"] {
    border: 1px solid #e2e8f0;
    border-radius: 4px;
    padding: 6px 10px;
    font-size: 13px;
    color: #1e293b;
    background-color: #ffffff;
}

#CameraSettingsDialog QComboBox[objectName^="cam_width_"]:focus,
#CameraSettingsDialog QComboBox[objectName^="cam_height_"]:focus,
#CameraSettingsDialog QComboBox[objectName^="cam_fps_"]:focus {
    border-color: #3b82f6;
}

/* 摄像头行内的 QRadioButton */
#CameraSettingsDialog QRadioButton[objectName^="cam_role_"] {
    color: #475569;
    font-size: 13px;
    spacing: 4px;
}

#CameraSettingsDialog QRadioButton[objectName^="cam_role_"]::indicator {
    width: 14px;
    height: 14px;
}
```

- [ ] **Step 2: 注册到 styles.qrc**

修改 `styles/styles.qrc`，在 `<qresource prefix="/styles">` 内末尾添加一行：

```xml
        <file alias="camerasettings.qss">camerasettings.qss</file>
```

完整文件变为：

```xml
<RCC>
    <qresource prefix="/styles">
        <file alias="config.qss">config.qss</file>
        <file alias="login.qss">login.qss</file>
        <file alias="main.qss">main.qss</file>
        <file alias="register.qss">register.qss</file>
        <file alias="vehicleInformation.qss">vehicleInformation.qss</file>
        <file alias="userManagement.qss">userManagement.qss</file>
        <file alias="cameramanagement.qss">cameramanagement.qss</file>
        <file alias="settings.qss">settings.qss</file>
        <file alias="camerasettings.qss">camerasettings.qss</file>
    </qresource>
</RCC>
```

- [ ] **Step 3: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add styles/camerasettings.qss styles/styles.qrc && git commit -m "feat: add CameraSettingsDialog stylesheet"
```

---

### Task 3: 头文件

**Files:**
- Create: `UI/CameraManagement/camerasettingsdialog.h`

- [ ] **Step 1: 创建 camerasettingsdialog.h**

```cpp
#ifndef CAMERASETTINGSDIALOG_H
#define CAMERASETTINGSDIALOG_H

#include <QDialog>
#include <QList>
#include <QPoint>

namespace Ui {
class CameraSettingsDialog;
}

class QButtonGroup;
class CameraInfo;

class CameraSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraSettingsDialog(QWidget *parent = nullptr);
    ~CameraSettingsDialog();

signals:
    void camerasUpdated();
    void cameraConfigChanged();

private slots:
    void onSave();

private:
    void setupWindow();
    void loadSettings();
    void buildCameraRows();
    QWidget* createCameraRow(int index, const CameraInfo &info);

    Ui::CameraSettingsDialog *ui;
    QList<QWidget*> m_cameraRows;
    QButtonGroup *m_roleGroup = nullptr;
    bool m_dragging = false;
    QPoint m_dragPosition;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // CAMERASETTINGSDIALOG_H
```

- [ ] **Step 2: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add UI/CameraManagement/camerasettingsdialog.h && git commit -m "feat: add CameraSettingsDialog header"
```

---

### Task 4: 实现文件

**Files:**
- Create: `UI/CameraManagement/camerasettingsdialog.cpp`

- [ ] **Step 1: 创建 camerasettingsdialog.cpp**

```cpp
#include "camerasettingsdialog.h"
#include "ui_camerasettingsdialog.h"
#include "src/utils/initfile.h"
#include "src/camera/cameramanager.h"
#include "src/camera/camerainfo.h"

#include <QFile>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFrame>
#include <QScrollArea>

CameraSettingsDialog::CameraSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CameraSettingsDialog)
{
    ui->setupUi(this);
    setupWindow();
    loadSettings();
    buildCameraRows();

    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnSubmit, &QPushButton::clicked, this, &CameraSettingsDialog::onSave);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

CameraSettingsDialog::~CameraSettingsDialog()
{
    delete ui;
}

void CameraSettingsDialog::setupWindow()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QFile styleFile(":/styles/camerasettings.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    ui->mainContainer->setGraphicsEffect(shadow);

    ui->titleBar->installEventFilter(this);
}

bool CameraSettingsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->titleBar) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragPosition = me->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
            break;
        }
        case QEvent::MouseMove: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (m_dragging && (me->buttons() & Qt::LeftButton)) {
                move(me->globalPosition().toPoint() - m_dragPosition);
                return true;
            }
            break;
        }
        case QEvent::MouseButtonRelease: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                m_dragging = false;
                return true;
            }
            break;
        }
        default:
            break;
        }
    }
    return QDialog::eventFilter(obj, event);
}

void CameraSettingsDialog::loadSettings()
{
    InitFile &cfg = InitFile::instance();

    QString wStr = QString::number(cfg.getCameraWidth());
    int wIdx = ui->cmbGlobalWidth->findText(wStr);
    if (wIdx >= 0) ui->cmbGlobalWidth->setCurrentIndex(wIdx);
    else ui->cmbGlobalWidth->setCurrentText(wStr);

    QString hStr = QString::number(cfg.getCameraHeight());
    int hIdx = ui->cmbGlobalHeight->findText(hStr);
    if (hIdx >= 0) ui->cmbGlobalHeight->setCurrentIndex(hIdx);
    else ui->cmbGlobalHeight->setCurrentText(hStr);

    QString fpsStr = QString::number(cfg.getCameraFps());
    int fpsIdx = ui->cmbGlobalFps->findText(fpsStr);
    if (fpsIdx >= 0) ui->cmbGlobalFps->setCurrentIndex(fpsIdx);
    else ui->cmbGlobalFps->setCurrentText(fpsStr);
}

void CameraSettingsDialog::buildCameraRows()
{
    CameraManager &mgr = CameraManager::instance();
    int n = mgr.count();

    m_roleGroup = new QButtonGroup(this);

    if (n == 0) {
        QLabel *empty = new QLabel(QStringLiteral("未检测到摄像头"), ui->cameraListContainer);
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color:#94A3B8; font-size:13px; padding:20px;");
        ui->cameraListLayout->addWidget(empty);
        return;
    }

    for (int i = 0; i < n; ++i) {
        CameraInfo info = mgr.info(i);
        QWidget *row = createCameraRow(i, info);
        m_cameraRows.append(row);
        ui->cameraListLayout->addWidget(row);
    }
}

QWidget* CameraSettingsDialog::createCameraRow(int index, const CameraInfo &info)
{
    // 外层分组框
    QFrame *frame = new QFrame(ui->cameraListContainer);
    frame->setObjectName(QStringLiteral("camRow_%1").arg(index));

    QVBoxLayout *outer = new QVBoxLayout(frame);
    outer->setContentsMargins(12, 10, 12, 10);
    outer->setSpacing(8);

    // 标题行
    QLabel *title = new QLabel(QStringLiteral("📷 摄像头 %1").arg(index + 1), frame);
    title->setObjectName(QStringLiteral("cam_title_%1").arg(index));
    outer->addWidget(title);

    // 名称
    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *lblName = new QLabel(QStringLiteral("名称"), frame);
    lblName->setObjectName(QStringLiteral("cam_lbl_name_%1").arg(index));
    lblName->setFixedWidth(60);
    QLineEdit *txtName = new QLineEdit(frame);
    txtName->setObjectName(QStringLiteral("cam_name_%1").arg(index));
    txtName->setText(info.name);
    txtName->setPlaceholderText(QStringLiteral("摄像头 %1").arg(index + 1));
    nameLayout->addWidget(lblName);
    nameLayout->addWidget(txtName);
    outer->addLayout(nameLayout);

    // 位置
    QHBoxLayout *locLayout = new QHBoxLayout();
    QLabel *lblLoc = new QLabel(QStringLiteral("位置"), frame);
    lblLoc->setObjectName(QStringLiteral("cam_lbl_location_%1").arg(index));
    lblLoc->setFixedWidth(60);
    QLineEdit *txtLocation = new QLineEdit(frame);
    txtLocation->setObjectName(QStringLiteral("cam_location_%1").arg(index));
    txtLocation->setText(info.location);
    txtLocation->setPlaceholderText(QStringLiteral("位置 %1").arg(index + 1));
    locLayout->addWidget(lblLoc);
    locLayout->addWidget(txtLocation);
    outer->addLayout(locLayout);

    // 角色
    QHBoxLayout *roleLayout = new QHBoxLayout();
    QLabel *lblRole = new QLabel(QStringLiteral("角色"), frame);
    lblRole->setObjectName(QStringLiteral("cam_lbl_role_%1").arg(index));
    lblRole->setFixedWidth(60);
    QRadioButton *rbEntry = new QRadioButton("entry", frame);
    rbEntry->setObjectName(QStringLiteral("cam_role_entry_%1").arg(index));
    QRadioButton *rbMonitor = new QRadioButton("monitor", frame);
    rbMonitor->setObjectName(QStringLiteral("cam_role_monitor_%1").arg(index));
    if (info.role == "entry")
        rbEntry->setChecked(true);
    else
        rbMonitor->setChecked(true);
    m_roleGroup->addButton(rbEntry);
    m_roleGroup->addButton(rbMonitor);
    roleLayout->addWidget(lblRole);
    roleLayout->addWidget(rbEntry);
    roleLayout->addWidget(rbMonitor);
    roleLayout->addStretch();
    outer->addLayout(roleLayout);

    // 分辨率
    QHBoxLayout *resLayout = new QHBoxLayout();
    QLabel *lblRes = new QLabel(QStringLiteral("分辨率"), frame);
    lblRes->setObjectName(QStringLiteral("cam_lbl_res_%1").arg(index));
    lblRes->setFixedWidth(60);

    QComboBox *cmbWidth = new QComboBox(frame);
    cmbWidth->setObjectName(QStringLiteral("cam_width_%1").arg(index));
    cmbWidth->setEditable(true);
    cmbWidth->addItems({QStringLiteral("480"), QStringLiteral("640"), QStringLiteral("800"),
                        QStringLiteral("1024"), QStringLiteral("1280"), QStringLiteral("1920"),
                        QStringLiteral("2560")});
    QString wStr = QString::number(info.width);
    int wIdx = cmbWidth->findText(wStr);
    if (wIdx >= 0) cmbWidth->setCurrentIndex(wIdx);
    else cmbWidth->setCurrentText(wStr);

    QLabel *lblX = new QLabel(QStringLiteral("×"), frame);
    lblX->setFixedWidth(16);
    lblX->setAlignment(Qt::AlignCenter);

    QComboBox *cmbHeight = new QComboBox(frame);
    cmbHeight->setObjectName(QStringLiteral("cam_height_%1").arg(index));
    cmbHeight->setEditable(true);
    cmbHeight->addItems({QStringLiteral("480"), QStringLiteral("640"), QStringLiteral("800"),
                         QStringLiteral("1024"), QStringLiteral("1280"), QStringLiteral("1920"),
                         QStringLiteral("2560")});
    QString hStr = QString::number(info.height);
    int hIdx = cmbHeight->findText(hStr);
    if (hIdx >= 0) cmbHeight->setCurrentIndex(hIdx);
    else cmbHeight->setCurrentText(hStr);

    resLayout->addWidget(lblRes);
    resLayout->addWidget(cmbWidth);
    resLayout->addWidget(lblX);
    resLayout->addWidget(cmbHeight);
    resLayout->addStretch();
    outer->addLayout(resLayout);

    // 帧率
    QHBoxLayout *fpsLayout = new QHBoxLayout();
    QLabel *lblFps = new QLabel(QStringLiteral("帧率"), frame);
    lblFps->setObjectName(QStringLiteral("cam_lbl_fps_%1").arg(index));
    lblFps->setFixedWidth(60);

    QComboBox *cmbFps = new QComboBox(frame);
    cmbFps->setObjectName(QStringLiteral("cam_fps_%1").arg(index));
    cmbFps->setEditable(true);
    cmbFps->addItems({QStringLiteral("15"), QStringLiteral("20"), QStringLiteral("25"), QStringLiteral("30")});
    QString fpsStr = QString::number(info.fps);
    int fpsIdx = cmbFps->findText(fpsStr);
    if (fpsIdx >= 0) cmbFps->setCurrentIndex(fpsIdx);
    else cmbFps->setCurrentText(fpsStr);

    QLabel *lblFpsUnit = new QLabel("FPS", frame);
    lblFpsUnit->setObjectName(QStringLiteral("cam_lbl_fpsunit_%1").arg(index));
    lblFpsUnit->setStyleSheet("color:#94A3B8; font-size:12px;");

    fpsLayout->addWidget(lblFps);
    fpsLayout->addWidget(cmbFps);
    fpsLayout->addWidget(lblFpsUnit);
    fpsLayout->addStretch();
    outer->addLayout(fpsLayout);

    return frame;
}

void CameraSettingsDialog::onSave()
{
    InitFile &cfg = InitFile::instance();
    CameraManager &mgr = CameraManager::instance();
    int n = mgr.count();

    // 全局值
    int globalWidth = ui->cmbGlobalWidth->currentText().toInt();
    int globalHeight = ui->cmbGlobalHeight->currentText().toInt();
    int globalFps = ui->cmbGlobalFps->currentText().toInt();

    // 校验全局值
    if (globalWidth <= 0 || globalHeight <= 0 || globalFps <= 0) {
        QMessageBox::warning(this, QStringLiteral("错误"),
            QStringLiteral("全局分辨率或帧率无效，请输入正整数"));
        return;
    }

    // 角色互斥校验
    int entryCount = 0;
    for (int i = 0; i < m_cameraRows.size(); ++i) {
        QRadioButton *rbEntry = m_cameraRows[i]->findChild<QRadioButton*>(
            QStringLiteral("cam_role_entry_%1").arg(i));
        if (rbEntry && rbEntry->isChecked())
            entryCount++;
    }

    if (entryCount > 1) {
        QMessageBox::warning(this, QStringLiteral("错误"),
            QStringLiteral("只能设置一个入口摄像头（entry）"));
        return;
    }
    if (entryCount == 0) {
        auto reply = QMessageBox::question(this, QStringLiteral("警告"),
            QStringLiteral("当前无入口摄像头，车牌识别功能将暂停。确定继续？"));
        if (reply != QMessageBox::No)
            return;
    }

    // 写全局配置
    cfg.setCameraConfig(0, globalWidth, globalHeight, globalFps);

    // 写每路配置
    QJsonArray camerasArray;
    for (int i = 0; i < m_cameraRows.size(); ++i) {
        QWidget *row = m_cameraRows[i];

        QJsonObject obj;
        obj["index"] = i;

        QLineEdit *txtName = row->findChild<QLineEdit*>(QStringLiteral("cam_name_%1").arg(i));
        QString name = txtName ? txtName->text().trimmed() : QString();
        obj["name"] = name.isEmpty() ? QStringLiteral("摄像头 %1").arg(i + 1) : name;

        QLineEdit *txtLocation = row->findChild<QLineEdit*>(QStringLiteral("cam_location_%1").arg(i));
        obj["location"] = txtLocation ? txtLocation->text().trimmed() : QString();

        QRadioButton *rbEntry = row->findChild<QRadioButton*>(QStringLiteral("cam_role_entry_%1").arg(i));
        obj["role"] = (rbEntry && rbEntry->isChecked()) ? "entry" : "monitor";

        QComboBox *cmbWidth = row->findChild<QComboBox*>(QStringLiteral("cam_width_%1").arg(i));
        int perWidth = cmbWidth ? cmbWidth->currentText().toInt() : 0;
        // 校验每路分辨率
        if (perWidth <= 0) {
            QMessageBox::warning(this, QStringLiteral("错误"),
                QStringLiteral("摄像头 %1 的分辨率宽度无效，请输入正整数").arg(i + 1));
            return;
        }
        obj["width"] = perWidth;

        QComboBox *cmbHeight = row->findChild<QComboBox*>(QStringLiteral("cam_height_%1").arg(i));
        int perHeight = cmbHeight ? cmbHeight->currentText().toInt() : 0;
        if (perHeight <= 0) {
            QMessageBox::warning(this, QStringLiteral("错误"),
                QStringLiteral("摄像头 %1 的分辨率高度无效，请输入正整数").arg(i + 1));
            return;
        }
        obj["height"] = perHeight;

        QComboBox *cmbFps = row->findChild<QComboBox*>(QStringLiteral("cam_fps_%1").arg(i));
        int perFps = cmbFps ? cmbFps->currentText().toInt() : 0;
        if (perFps <= 0) {
            QMessageBox::warning(this, QStringLiteral("错误"),
                QStringLiteral("摄像头 %1 的帧率无效，请输入正整数").arg(i + 1));
            return;
        }
        obj["fps"] = perFps;

        camerasArray.append(obj);
    }

    cfg.setCameras(camerasArray);

    if (!cfg.saveConfig()) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("配置保存失败"));
        return;
    }

    emit camerasUpdated();
    emit cameraConfigChanged();
    QMessageBox::information(this, QStringLiteral("提示"),
        QStringLiteral("摄像头配置已保存，重启应用后生效"));
    accept();
}
```

- [ ] **Step 2: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add UI/CameraManagement/camerasettingsdialog.cpp && git commit -m "feat: add CameraSettingsDialog implementation"
```

---

### Task 5: 更新 scanCameras() 支持 per-camera 分辨率/FPS

**Files:**
- Modify: `src/camera/cameramanager.cpp:32-83`

当前 `scanCameras()` 对所有线程统一设置全局 width/height/fps。保存对话框的 per-camera 值后，重启时 `scanCameras()` 需从 JSON 中读取各摄像头独立的参数，找不到时 fallback 到全局值。同时 `CameraInfo` 中也记录 per-camera 的值。

- [ ] **Step 1: 修改 scanCameras() 循环中的逻辑**

在 `src/camera/cameramanager.cpp` 中，将 `scanCameras()` 的第 50-76 行的 for 循环体替换为以下内容（外层的 `int n`, `int w`, `int h`, `int fps`, `QJsonArray camerasCfg` 变量声明保持不变）：

```cpp
    for(int i = 0; i < n; ++i){
        CameraInfo info;
        info.index = i;
        info.width  = w;
        info.height = h;
        info.fps    = fps;

        info.name     = QStringLiteral("摄像头 %1").arg(i + 1);
        info.location = QStringLiteral("位置 %1").arg(i + 1);
        info.role     = (i == 0) ? "entry" : "monitor";

        for(const QJsonValue& v : camerasCfg){
            QJsonObject o = v.toObject();
            if(o["index"].toInt() == i){
                if(!o["name"].toString().isEmpty())
                    info.name = o["name"].toString();
                if(!o["location"].toString().isEmpty())
                    info.location = o["location"].toString();
                if(!o["role"].toString().isEmpty())
                    info.role = o["role"].toString();
                // per-camera 分辨率/FPS 覆盖全局值
                if(o.contains("width") && o["width"].toInt() > 0)
                    info.width = o["width"].toInt();
                if(o.contains("height") && o["height"].toInt() > 0)
                    info.height = o["height"].toInt();
                if(o.contains("fps") && o["fps"].toInt() > 0)
                    info.fps = o["fps"].toInt();
                break;
            }
        }

        CameraThread* t = new CameraThread(i, this);
        t->setResolution(info.width, info.height);
        t->setTargetFps(info.fps);

        m_threads.append(t);
        m_infos.append(info);
    }
```

即变更为：先填充 info 默认值 → 从 JSON 覆盖 → 再创建 CameraThread 并用 info 的值设置分辨率/FPS。

- [ ] **Step 2: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add src/camera/cameramanager.cpp && git commit -m "feat: scanCameras supports per-camera resolution and FPS from JSON config"
```

---

### Task 6: 注册到 ParkingSystem.pro

**Files:**
- Modify: `ParkingSystem.pro`

在 `SOURCES`、`HEADERS`、`FORMS` 三个块中分别插入新文件。

- [ ] **Step 1: 添加 SOURCES**

在 `SOURCES +=` 块中，`UI/CameraManagement/camerawindow.cpp \` 之后插入一行：

```
    UI/CameraManagement/camerasettingsdialog.cpp \
```

- [ ] **Step 2: 添加 HEADERS**

在 `HEADERS +=` 块中，`UI/CameraManagement/camerawindow.h \` 之后插入一行：

```
    UI/CameraManagement/camerasettingsdialog.h \
```

- [ ] **Step 3: 添加 FORMS**

在 `FORMS +=` 块末尾（`UI/Settings/settingsdialog.ui` 之后）插入一行：

```
    UI/CameraManagement/camerasettingsdialog.ui
```

- [ ] **Step 4: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add ParkingSystem.pro && git commit -m "build: register CameraSettingsDialog in qmake project"
```

---

### Task 7: 接入设置按钮

**Files:**
- Modify: `UI/CameraManagement/cameramanagement.cpp:5` (追加 include)
- Modify: `UI/CameraManagement/cameramanagement.cpp:38-40` (替换占位符)

- [ ] **Step 1: 添加 include**

在 `cameramanagement.cpp` 现有 include 区域末尾（第 13 行 `#include <QResizeEvent>` 之后）添加：

```cpp
#include "camerasettingsdialog.h"
```

- [ ] **Step 2: 替换设置按钮连接**

将第 38-40 行的：

```cpp
    connect(m_settingsBtn, &QPushButton::clicked, this, [this](){
        // Placeholder: settings not implemented in this version
    });
```

替换为：

```cpp
    connect(m_settingsBtn, &QPushButton::clicked, this, [this](){
        auto *dialog = new CameraSettingsDialog(this);
        dialog->setModal(true);
        dialog->exec();
        dialog->deleteLater();
    });
```

- [ ] **Step 3: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add UI/CameraManagement/cameramanagement.cpp && git commit -m "feat: wire CameraSettingsDialog to settings button"
```

---

### Task 8: 构建验证 + 冒烟测试

**Files:** 无需修改代码

- [ ] **Step 1: 构建项目**

```bash
cd D:/QTproject/ParkingSystem && qmake && make clean && make -j4
```

预期：编译通过，无警告。UIC 自动从 `camerasettingsdialog.ui` 生成 `generated/ui_camerasettingsdialog.h`。

- [ ] **Step 2: 冒烟测试清单**

| # | 操作 | 预期结果 |
|---|------|----------|
| 1 | 点击摄像头管理页面"⚙ 设置"按钮 | 弹出设置弹窗，全局参数和摄像头列表字段值与 config.json 一致 |
| 2 | 修改全局帧率→保存→重新打开 | 新帧率已持久化 |
| 3 | 修改摄像头 1 名称为"大门入口"→保存→重新打开 | 新名称已持久化 |
| 4 | 点击取消→重新打开 | 所有值恢复到保存前的旧值 |
| 5 | 拖拽标题栏 | 弹窗可拖拽移动 |
| 6 | 点击 × 关闭 | 弹窗不保存直接关闭 |
| 7 | 手动输入分辨率"abc"→保存 | 校验拦截，弹出错误提示 |

- [ ] **Step 3: 提交（如有微调）**

```bash
cd D:/QTproject/ParkingSystem && git add -A && git commit -m "chore: build verification after CameraSettingsDialog"
```
