# Settings Dialog Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 MainWindow 的设置按钮实现可修改数据库连接、停车场参数和识别置信度的弹窗。

**Architecture:** Qt Designer .ui 表单 + 独立 QSS 样式文件，复用 ConfigInitDialog 的无边框+投影+拖拽窗口模式，直接读写 InitFile 单例，DatabaseManager 仅用于测试连接。

**Tech Stack:** Qt 5/C++11，qmake，QDialog（FramelessWindowHint），InitFile（JSON 配置），DatabaseManager

**Spec:** `docs/superpowers/specs/2026-07-21-settings-dialog-design.md`

## Global Constraints

- 只改 3 类配置：数据库、停车场、识别置信度（不含摄像头、系统信息、模型路径）
- 数据库参数保存后重启生效，非立即重连
- 测试连接用独立临时连接，不影响当前连接
- 样式使用独立 `settings.qss`，不耦合 ConfigInitDialog 的 `config.qss`

---

## File Structure

| Role | Path | Responsibility |
|------|------|----------------|
| **Create** | `UI/Settings/settingsdialog.ui` | Qt Designer 表单（布局+控件） |
| **Create** | `UI/Settings/settingsdialog.h` | 类声明 |
| **Create** | `UI/Settings/settingsdialog.cpp` | 逻辑实现（load/save/test 连接） |
| **Create** | `styles/settings.qss` | 专属样式表（`#SettingsDialog` 作用域） |
| **Modify** | `styles/styles.qrc` | 注册 `settings.qss` 资源 |
| **Modify** | `ParkingSystem.pro` | 添加 .ui 到 FORMS、.h 到 HEADERS、.cpp 到 SOURCES |
| **Modify** | `UI/MainWindow/mainwindow.cpp:328-331` | onSetButton() 弹出设置弹窗 |

---

### Task 1: .ui 表单文件

**Files:**
- Create: `UI/Settings/settingsdialog.ui`

**Interfaces:**
- Produces: `SettingsDialog` 表单 → UIC 编译器生成 `ui_settingsdialog.h` → Task 3 使用

`settingsdialog.ui` 结构完全参照 `configinitdialog.ui`，尺寸改为 **600×550**。

- [ ] **Step 1: 创建 .ui 文件**

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>SettingsDialog</class>
 <widget class="QDialog" name="SettingsDialog">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>600</width>
    <height>550</height>
   </rect>
  </property>
  <property name="minimumSize">
   <size>
    <width>600</width>
    <height>550</height>
   </size>
  </property>
  <property name="maximumSize">
   <size>
    <width>600</width>
    <height>550</height>
   </size>
  </property>
  <property name="windowTitle">
   <string>设置</string>
  </property>
  <layout class="QVBoxLayout" name="outerLayout">
   <property name="spacing">
    <number>0</number>
   </property>
   <property name="leftMargin">
    <number>10</number>
   </property>
   <property name="topMargin">
    <number>10</number>
   </property>
   <property name="rightMargin">
    <number>10</number>
   </property>
   <property name="bottomMargin">
    <number>10</number>
   </property>
   <item>
    <widget class="QWidget" name="mainContainer">
     <layout class="QVBoxLayout" name="containerLayout">
      <property name="spacing">
       <number>0</number>
      </property>
      <property name="leftMargin">
       <number>0</number>
      </property>
      <property name="topMargin">
       <number>0</number>
      </property>
      <property name="rightMargin">
       <number>0</number>
      </property>
      <property name="bottomMargin">
       <number>0</number>
      </property>
      <item>
       <!-- ===== 标题栏 ===== -->
       <widget class="QWidget" name="titleBar">
        <property name="minimumSize">
         <size><width>0</width><height>40</height></size>
        </property>
        <property name="maximumSize">
         <size><width>16777215</width><height>40</height></size>
        </property>
        <layout class="QHBoxLayout" name="titleBarLayout">
         <property name="spacing"><number>8</number></property>
         <property name="leftMargin"><number>0</number></property>
         <property name="topMargin"><number>0</number></property>
         <property name="rightMargin"><number>16</number></property>
         <property name="bottomMargin"><number>0</number></property>
         <item>
          <spacer name="titleSpacer">
           <property name="orientation"><enum>Qt::Horizontal</enum></property>
           <property name="sizeHint" stdset="0">
            <size><width>40</width><height>20</height></size>
           </property>
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
          <!-- ===== 数据库配置区 ===== -->
          <item>
           <widget class="QLabel" name="dbSectionLabel">
            <property name="text">
             <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=" font-size:12pt; font-weight:600;"&gt;数据库连接&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
            </property>
           </widget>
          </item>
          <item>
           <layout class="QHBoxLayout" name="dbHostLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblDbHost">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;主机地址&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item><widget class="QLineEdit" name="txtDbHost"><property name="placeholderText"><string>localhost</string></property></widget></item>
           </layout>
          </item>
          <item>
           <layout class="QHBoxLayout" name="dbPortLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblDbPort">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;端口号&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item><widget class="QSpinBox" name="txtDbPort"><property name="minimum"><number>1</number></property><property name="maximum"><number>65535</number></property></widget></item>
           </layout>
          </item>
          <item>
           <layout class="QHBoxLayout" name="dbNameLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblDbName">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;数据库名&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item><widget class="QLineEdit" name="txtDbName"><property name="placeholderText"><string>请输入数据库名</string></property></widget></item>
           </layout>
          </item>
          <item>
           <layout class="QHBoxLayout" name="dbUsernameLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblDbUsername">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;用户名&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item><widget class="QLineEdit" name="txtDbUsername"><property name="placeholderText"><string>请输入用户名</string></property></widget></item>
           </layout>
          </item>
          <item>
           <layout class="QHBoxLayout" name="dbPasswordLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblDbPassword">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;密码&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item>
             <widget class="QLineEdit" name="txtDbPassword">
              <property name="echoMode"><enum>QLineEdit::Password</enum></property>
              <property name="placeholderText"><string>请输入数据库密码</string></property>
             </widget>
            </item>
           </layout>
          </item>
          <item>
           <layout class="QHBoxLayout" name="testBtnLayout">
            <item><spacer name="testBtnSpacer"><property name="orientation"><enum>Qt::Horizontal</enum></property><property name="sizeHint" stdset="0"><size><width>40</width><height>20</height></size></property></spacer></item>
            <item><widget class="QPushButton" name="btnTestConnection"><property name="text"><string>测试连接</string></property></widget></item>
           </layout>
          </item>
          <!-- ===== 停车场参数区 ===== -->
          <item>
           <widget class="QLabel" name="parkingSectionLabel">
            <property name="text">
             <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=" font-size:12pt; font-weight:600;"&gt;停车场参数&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
            </property>
           </widget>
          </item>
          <item>
           <layout class="QHBoxLayout" name="parkingNameLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblParkingName">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;停车场名称&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item><widget class="QLineEdit" name="txtParkingName"><property name="placeholderText"><string>请输入停车场名称</string></property></widget></item>
           </layout>
          </item>
          <item>
           <layout class="QHBoxLayout" name="priceLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblPrice">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;每小时价格&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item><widget class="QDoubleSpinBox" name="txtPrice"><property name="minimum"><double>0.00</double></property><property name="maximum"><double>9999.00</double></property><property name="suffix"><string> 元/小时</string></property></widget></item>
           </layout>
          </item>
          <item>
           <layout class="QHBoxLayout" name="capacityLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblCapacity">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;总车位数&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item><widget class="QSpinBox" name="txtCapacity"><property name="minimum"><number>1</number></property><property name="maximum"><number>99999</number></property></widget></item>
           </layout>
          </item>
          <item>
           <layout class="QHBoxLayout" name="freeMinLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblFreeMinutes">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;免费分钟&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item><widget class="QSpinBox" name="txtFreeMinutes"><property name="minimum"><number>0</number></property><property name="maximum"><number>120</number></property><property name="suffix"><string> 分钟</string></property></widget></item>
           </layout>
          </item>
          <!-- ===== 识别参数区 ===== -->
          <item>
           <widget class="QLabel" name="recognitionSectionLabel">
            <property name="text">
             <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=" font-size:12pt; font-weight:600;"&gt;识别参数&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
            </property>
           </widget>
          </item>
          <item>
           <layout class="QHBoxLayout" name="confidenceLayout">
            <property name="spacing"><number>10</number></property>
            <item>
             <widget class="QLabel" name="lblConfidence">
              <property name="minimumSize"><size><width>80</width><height>0</height></size></property>
              <property name="maximumSize"><size><width>80</width><height>16777215</height></size></property>
              <property name="text">
               <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align="right"&gt;&lt;span style=" font-size:10pt;"&gt;置信度阈值&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
              <property name="alignment"><set>Qt::AlignRight|Qt::AlignVCenter</set></property>
             </widget>
            </item>
            <item>
             <widget class="QSlider" name="sliderConfidence">
              <property name="orientation"><enum>Qt::Horizontal</enum></property>
              <property name="minimum"><number>0</number></property>
              <property name="maximum"><number>100</number></property>
             </widget>
            </item>
            <item>
             <widget class="QLabel" name="lblConfidenceValue">
              <property name="minimumSize"><size><width>40</width><height>0</height></size></property>
              <property name="text"><string>0.70</string></property>
             </widget>
            </item>
           </layout>
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
cd D:/QTproject/ParkingSystem && git add UI/Settings/settingsdialog.ui && git commit -m "feat: add SettingsDialog .ui form"
```

---

### Task 2: 专属样式文件

**Files:**
- Create: `styles/settings.qss`
- Modify: `styles/styles.qrc`

**Interfaces:**
- Produces: `:/styles/settings.qss` 资源 → Task 4 加载

样式从 `config.qss` 复制，将所有 `#ConfigInitDialog` 替换为 `#SettingsDialog`。

- [ ] **Step 1: 创建 settings.qss**

```css
/* ==================== SettingsDialog 样式 ==================== */
/* 作用域限制：仅影响 SettingsDialog (#SettingsDialog) */

/* 对话框容器 */
#SettingsDialog {
    background-color: #ffffff;
}

/* -------------------- 分组标题 -------------------- */
#SettingsDialog QLabel#dbSectionLabel,
#SettingsDialog QLabel#parkingSectionLabel,
#SettingsDialog QLabel#recognitionSectionLabel {
    color: #1a1a2e;
    padding: 8px 0;
    border-bottom: 2px solid #3b82f6;
    margin-top: 10px;
}

/* -------------------- 标签样式 -------------------- */
#SettingsDialog QLabel#lblDbHost,
#SettingsDialog QLabel#lblDbPort,
#SettingsDialog QLabel#lblDbName,
#SettingsDialog QLabel#lblDbUsername,
#SettingsDialog QLabel#lblDbPassword,
#SettingsDialog QLabel#lblParkingName,
#SettingsDialog QLabel#lblPrice,
#SettingsDialog QLabel#lblCapacity,
#SettingsDialog QLabel#lblFreeMinutes,
#SettingsDialog QLabel#lblConfidence {
    color: #475569;
    font-size: 14px;
    min-width: 80px;
    max-width: 80px;
}

/* -------------------- 输入框样式 -------------------- */
#SettingsDialog QLineEdit,
#SettingsDialog QSpinBox,
#SettingsDialog QDoubleSpinBox {
    border: 1px solid #e2e8f0;
    border-radius: 6px;
    padding: 10px 14px;
    font-size: 14px;
    color: #1e293b;
    background-color: #f8fafc;
    min-height: 20px;
}

#SettingsDialog QLineEdit:hover,
#SettingsDialog QSpinBox:hover,
#SettingsDialog QDoubleSpinBox:hover {
    border-color: #94a3b8;
}

#SettingsDialog QLineEdit:focus,
#SettingsDialog QSpinBox:focus,
#SettingsDialog QDoubleSpinBox:focus {
    border-color: #3b82f6;
    background-color: #ffffff;
    border-width: 2px;
    padding: 9px 13px;
}

/* -------------------- 滑块样式 -------------------- */
#SettingsDialog QSlider::groove:horizontal {
    border: 1px solid #e2e8f0;
    border-radius: 3px;
    height: 6px;
    background-color: #f1f5f9;
}

#SettingsDialog QSlider::handle:horizontal {
    background-color: #3b82f6;
    border: none;
    width: 16px;
    height: 16px;
    margin: -5px 0;
    border-radius: 8px;
}

#SettingsDialog QSlider::handle:horizontal:hover {
    background-color: #2563eb;
}

#SettingsDialog QSlider::sub-page:horizontal {
    background-color: #3b82f6;
    border-radius: 3px;
}

/* -------------------- 测试连接按钮 -------------------- */
#SettingsDialog QPushButton#btnTestConnection {
    background-color: #ffffff;
    color: #3b82f6;
    border: 1px solid #3b82f6;
    border-radius: 6px;
    padding: 6px 16px;
    font-size: 13px;
}

#SettingsDialog QPushButton#btnTestConnection:hover {
    background-color: #eff6ff;
}

/* -------------------- 提交按钮 -------------------- */
#SettingsDialog QPushButton#btnSubmit {
    background-color: #3b82f6;
    color: #ffffff;
    border: none;
    border-radius: 6px;
    padding: 10px 24px;
    font-size: 14px;
    font-weight: 600;
}

#SettingsDialog QPushButton#btnSubmit:hover {
    background-color: #2563eb;
}

#SettingsDialog QPushButton#btnSubmit:pressed {
    background-color: #1d4ed8;
}

/* -------------------- 取消按钮 -------------------- */
#SettingsDialog QPushButton#btnCancel {
    background-color: #ffffff;
    color: #475569;
    border: 1px solid #e2e8f0;
    border-radius: 6px;
    padding: 10px 24px;
    font-size: 14px;
}

#SettingsDialog QPushButton#btnCancel:hover {
    background-color: #f8fafc;
    border-color: #94a3b8;
}

#SettingsDialog QPushButton#btnCancel:pressed {
    background-color: #f1f5f9;
}

/* ==================== 标题栏样式 ==================== */

#SettingsDialog #titleBar {
    background-color: #ffffff;
    border-top-left-radius: 8px;
    border-top-right-radius: 8px;
    border-bottom: 1px solid #f1f5f9;
}

#SettingsDialog QPushButton#btnClose {
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

#SettingsDialog QPushButton#btnClose:hover {
    background-color: #ef4444;
    color: #ffffff;
}

#SettingsDialog QPushButton#btnClose:pressed {
    background-color: #dc2626;
}

/* ==================== 主容器样式 ==================== */
#SettingsDialog #mainContainer {
    background-color: #ffffff;
    border-radius: 8px;
}
```

- [ ] **Step 2: 注册到 styles.qrc**

修改 `styles/styles.qrc`，在 `<qresource prefix="/styles">` 内末尾添加一行：

```xml
        <file alias="settings.qss">settings.qss</file>
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
    </qresource>
</RCC>
```

- [ ] **Step 3: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add styles/settings.qss styles/styles.qrc && git commit -m "feat: add SettingsDialog stylesheet"
```

---

### Task 3: SettingsDialog 头文件

**Files:**
- Create: `UI/Settings/settingsdialog.h`

**Interfaces:**
- Consumes: `ui_settingsdialog.h`（UIC 从 Task 1 的 .ui 生成）
- Produces: `SettingsDialog` 类

- [ ] **Step 1: 创建头文件**

```cpp
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QPoint>

namespace Ui {
class SettingsDialog;
}

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
    void setupWindow();
    void loadSettings();

    Ui::SettingsDialog *ui;
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

### Task 4: SettingsDialog 实现

**Files:**
- Create: `UI/Settings/settingsdialog.cpp`

**Interfaces:**
- Consumes: `ui_settingsdialog.h`（UIC 生成），`InitFile::instance()`

- [ ] **Step 1: 完整实现**

```cpp
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "src/utils/initfile.h"
#include "src/database/databasemanager.h"

#include <QFile>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QSqlDatabase>

SettingsDialog::SettingsDialog(DatabaseManager *db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_db(db)
{
    ui->setupUi(this);
    setupWindow();
    loadSettings();

    // 置信度滑块-标签联动
    connect(ui->sliderConfidence, &QSlider::valueChanged, this, [this](int val) {
        ui->lblConfidenceValue->setText(QString::number(val / 100.0, 'f', 2));
    });

    // 标题栏按钮
    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnSubmit, &QPushButton::clicked, this, &SettingsDialog::onSave);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnTestConnection, &QPushButton::clicked, this, &SettingsDialog::onTestConnection);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setupWindow()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QFile styleFile(":/styles/settings.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    // 投影阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    ui->mainContainer->setGraphicsEffect(shadow);

    // 标题栏拖拽
    ui->titleBar->installEventFilter(this);
}

bool SettingsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->titleBar) {
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

void SettingsDialog::loadSettings()
{
    InitFile &cfg = InitFile::instance();

    // 数据库
    m_oldHost     = cfg.getDbHost();
    m_oldPort     = cfg.getDbPort();
    m_oldDbName   = cfg.getDbName();
    m_oldUsername = cfg.getDbUsername();
    m_oldPassword = cfg.getDbPassword();

    ui->txtDbHost->setText(m_oldHost);
    ui->txtDbPort->setValue(m_oldPort);
    ui->txtDbName->setText(m_oldDbName);
    ui->txtDbUsername->setText(m_oldUsername);
    ui->txtDbPassword->setText(m_oldPassword);

    // 停车场
    ui->txtParkingName->setText(cfg.getParkingName());
    ui->txtPrice->setValue(cfg.getParkingPrice());
    ui->txtCapacity->setValue(cfg.getParkingCapacity());
    ui->txtFreeMinutes->setValue(cfg.getFreeMinutes());

    // 识别
    int confidenceVal = qBound(0, static_cast<int>(cfg.getConfidenceThreshold() * 100), 100);
    ui->sliderConfidence->setValue(confidenceVal);
    ui->lblConfidenceValue->setText(QString::number(confidenceVal / 100.0, 'f', 2));
}

void SettingsDialog::onSave()
{
    InitFile &cfg = InitFile::instance();

    cfg.setDbConfig(
        ui->txtDbHost->text(),
        ui->txtDbPort->value(),
        ui->txtDbName->text(),
        ui->txtDbUsername->text(),
        ui->txtDbPassword->text()
    );

    cfg.setParkingConfig(
        ui->txtParkingName->text(),
        ui->txtPrice->value(),
        ui->txtCapacity->value(),
        ui->txtFreeMinutes->value()
    );

    cfg.setRecognitionConfig(
        cfg.getModelPath(),
        ui->sliderConfidence->value() / 100.0
    );

    if (!cfg.saveConfig()) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("配置保存失败"));
        return;
    }

    bool dbChanged =
        ui->txtDbHost->text()     != m_oldHost     ||
        ui->txtDbPort->value()    != m_oldPort     ||
        ui->txtDbName->text()     != m_oldDbName   ||
        ui->txtDbUsername->text() != m_oldUsername ||
        ui->txtDbPassword->text() != m_oldPassword;

    if (dbChanged) {
        emit dbConfigChanged();
        QMessageBox::information(this, QStringLiteral("提示"),
            QStringLiteral("数据库配置已保存，重启应用后生效"));
    }

    emit settingsSaved();
    accept();
}

void SettingsDialog::onTestConnection()
{
    const QString testConn = "settings_test_connection";

    if (QSqlDatabase::contains(testConn))
        QSqlDatabase::removeDatabase(testConn);

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", testConn);
    db.setHostName(ui->txtDbHost->text());
    db.setPort(ui->txtDbPort->value());
    db.setDatabaseName(ui->txtDbName->text());
    db.setUserName(ui->txtDbUsername->text());
    db.setPassword(ui->txtDbPassword->text());

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

- [ ] **Step 2: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add UI/Settings/settingsdialog.cpp && git commit -m "feat: add SettingsDialog implementation"
```

---

### Task 5: 更新 ParkingSystem.pro

**Files:**
- Modify: `ParkingSystem.pro`

**Interfaces:**
- 无代码接口 — 构建系统注册新文件

- [ ] **Step 1: 添加 SOURCES、HEADERS、FORMS**

在 `SOURCES` 块中（ConfigInit 之后）插入：

```
    UI/Settings/settingsdialog.cpp \
```

在 `HEADERS` 块中（ConfigInit 之后）插入：

```
    UI/Settings/settingsdialog.h \
```

在 `FORMS` 块中（ConfigInit 之后）插入：

```
    UI/Settings/settingsdialog.ui \
```

完整修改后的块：

```
SOURCES += \
    ...
    UI/ConfigInit/configinitdialog.cpp \
    UI/Settings/settingsdialog.cpp \
    ...

HEADERS += \
    ...
    UI/ConfigInit/configinitdialog.h \
    UI/Settings/settingsdialog.h \
    ...

FORMS += \
    ...
    UI/ConfigInit/configinitdialog.ui \
    UI/Settings/settingsdialog.ui
```

- [ ] **Step 2: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add ParkingSystem.pro && git commit -m "build: register SettingsDialog in qmake project"
```

---

### Task 6: 接入 MainWindow

**Files:**
- Modify: `UI/MainWindow/mainwindow.cpp:328-331`

**Interfaces:**
- Consumes: `SettingsDialog`（Task 3-4）

- [ ] **Step 1: 添加 include + 替换 onSetButton**

在 `mainwindow.cpp` 现有 include 区域末尾添加：

```cpp
#include "UI/Settings/settingsdialog.h"
```

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

- [ ] **Step 2: 提交**

```bash
cd D:/QTproject/ParkingSystem && git add UI/MainWindow/mainwindow.cpp && git commit -m "feat: wire SettingsDialog to setButton"
```

---

### Task 7: 构建验证 + 冒烟测试

**Files:** 无需修改代码

- [ ] **Step 1: 构建项目**

```bash
cd D:/QTproject/ParkingSystem && qmake && make clean && make -j$(nproc)
```

预期：编译通过，无警告。UIC 自动从 `settingsdialog.ui` 生成 `generated/ui_settingsdialog.h`。

- [ ] **Step 2: 冒烟测试清单**

| # | 操作 | 预期结果 |
|---|------|----------|
| 1 | 点击主窗口设置按钮 | 弹出设置弹窗，样式与 ConfigInitDialog 一致，字段值与 `config.json` 一致 |
| 2 | 修改停车场价格→保存→重新打开 | 新价格已持久化 |
| 3 | 修改数据库密码→保存 | 弹出"重启后生效"提示 |
| 4 | 输入错误数据库参数→测试连接 | 弹出"连接失败"及具体错误 |
| 5 | 正确数据库参数→测试连接 | 弹出"连接成功" |
| 6 | 拖动置信度滑块→标签联动 | 实时显示 0.00~1.00 |
| 7 | 点击取消→重新打开 | 所有值恢复到保存前旧值 |

- [ ] **Step 3: 提交（如有微调）**

```bash
cd D:/QTproject/ParkingSystem && git add -A && git commit -m "chore: build verification"
```
