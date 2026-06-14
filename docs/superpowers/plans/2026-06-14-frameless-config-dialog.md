# ConfigInitDialog 无边框窗口实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 ConfigInitDialog 实现无边框窗口，包含自定义最小化和关闭按钮，支持顶部区域拖拽移动。

**Architecture:** 使用 Qt::FramelessWindowHint 移除系统标题栏，在 UI 中添加标题栏 QWidget 和按钮，通过事件过滤器实现拖拽，QGraphicsDropShadowEffect 实现阴影，QSS 样式化按钮。

**Tech Stack:** Qt 5/6, C++, QSS, Qt Designer

---

## 文件结构

| 文件 | 操作 | 职责 |
|------|------|------|
| `UI/ConfigInit/configinitdialog.ui` | 修改 | 添加标题栏、按钮、容器布局 |
| `UI/ConfigInit/configinitdialog.h` | 修改 | 添加事件过滤器和鼠标事件声明 |
| `UI/ConfigInit/configinitdialog.cpp` | 修改 | 实现窗口标志、阴影、拖拽、按钮功能 |
| `styles/config.qss` | 修改 | 添加标题栏和按钮样式 |

---

## Task 1: 修改 UI 文件 - 添加标题栏和容器

**Files:**
- Modify: `UI/ConfigInit/configinitdialog.ui`

- [ ] **Step 1: 重构 UI 布局结构**

将现有布局包裹在 mainContainer 中，并在顶部添加 titleBar。

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>ConfigInitDialog</class>
 <widget class="QDialog" name="ConfigInitDialog">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>700</width>
    <height>900</height>
   </rect>
  </property>
  <property name="minimumSize">
   <size>
    <width>700</width>
    <height>900</height>
   </size>
  </property>
  <property name="maximumSize">
   <size>
    <width>700</width>
    <height>900</height>
   </size>
  </property>
  <property name="windowTitle">
   <string>停车场数据初始化</string>
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
     <property name="styleSheet">
      <string notr="true">background-color: #ffffff; border-radius: 8px;</string>
     </property>
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
       <widget class="QWidget" name="titleBar">
        <property name="minimumSize">
         <size>
          <width>0</width>
          <height>40</height>
         </size>
        </property>
        <property name="maximumSize">
         <size>
          <width>16777215</width>
          <height>40</height>
         </size>
        </property>
        <layout class="QHBoxLayout" name="titleBarLayout">
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
          <number>8</number>
         </property>
         <property name="bottomMargin">
          <number>0</number>
         </property>
         <item>
          <spacer name="titleSpacer">
           <property name="orientation">
            <enum>Qt::Horizontal</enum>
           </property>
           <property name="sizeHint" stdset="0">
            <size>
             <width>40</width>
             <height>20</height>
            </size>
           </property>
          </spacer>
         </item>
         <item>
          <widget class="QPushButton" name="btnMinimize">
           <property name="minimumSize">
            <size>
             <width>36</width>
             <height>28</height>
            </size>
           </property>
           <property name="maximumSize">
            <size>
             <width>36</width>
             <height>28</height>
            </size>
           </property>
           <property name="text">
            <string>─</string>
           </property>
           <property name="cursor">
            <cursorShape>Qt::PointingHandCursor</cursorShape>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="btnClose">
           <property name="minimumSize">
            <size>
             <width>36</width>
             <height>28</height>
            </size>
           </property>
           <property name="maximumSize">
            <size>
             <width>36</width>
             <height>28</height>
            </size>
           </property>
           <property name="text">
            <string>×</string>
           </property>
           <property name="cursor">
            <cursorShape>Qt::PointingHandCursor</cursorShape>
           </property>
          </widget>
         </item>
        </layout>
       </widget>
      </item>
      <item>
       <widget class="QWidget" name="contentArea">
        <layout class="QVBoxLayout" name="mainLayout">
         <property name="spacing">
          <number>15</number>
         </property>
         <property name="leftMargin">
          <number>30</number>
         </property>
         <property name="topMargin">
          <number>20</number>
         </property>
         <property name="rightMargin">
          <number>30</number>
         </property>
         <property name="bottomMargin">
          <number>20</number>
         </property>
         <item>
          <widget class="QLabel" name="titleLabel">
           <property name="text">
            <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align=&quot;center&quot;&gt;&lt;span style=&quot; font-size:18pt; font-weight:600;&quot;&gt;停车场数据初始化&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
           </property>
           <property name="alignment">
            <set>Qt::AlignCenter</set>
           </property>
          </widget>
         </item>
         <item>
          <spacer name="topSpacer">
           <property name="orientation">
            <enum>Qt::Vertical</enum>
           </property>
           <property name="sizeType">
            <enum>QSizePolicy::Fixed</enum>
           </property>
           <property name="sizeHint" stdset="0">
            <size>
             <width>20</width>
             <height>10</height>
            </size>
           </property>
          </spacer>
         </item>
         <item>
          <widget class="QLabel" name="dbSectionLabel">
           <property name="text">
            <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;数据库配置&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
           </property>
          </widget>
         </item>
         <item>
          <layout class="QHBoxLayout" name="ipLayout">
           <property name="spacing">
            <number>10</number>
           </property>
           <item>
            <widget class="QLabel" name="lblIP">
             <property name="minimumSize">
              <size>
               <width>80</width>
               <height>0</height>
              </size>
             </property>
             <property name="maximumSize">
              <size>
               <width>80</width>
               <height>16777215</height>
              </size>
             </property>
             <property name="text">
              <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;IP:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
             </property>
             <property name="alignment">
              <set>Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter</set>
             </property>
            </widget>
           </item>
           <item>
            <widget class="QLineEdit" name="txtIP">
             <property name="placeholderText">
              <string>请输入数据库IP地址</string>
             </property>
            </widget>
           </item>
          </layout>
         </item>
         <item>
          <layout class="QHBoxLayout" name="portLayout">
           <property name="spacing">
            <number>10</number>
           </property>
           <item>
            <widget class="QLabel" name="lblPort">
             <property name="minimumSize">
              <size>
               <width>80</width>
               <height>0</height>
              </size>
             </property>
             <property name="maximumSize">
              <size>
               <width>80</width>
               <height>16777215</height>
              </size>
             </property>
             <property name="text">
              <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;端口号:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
             </property>
             <property name="alignment">
              <set>Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter</set>
             </property>
            </widget>
           </item>
           <item>
            <widget class="QLineEdit" name="txtPort">
             <property name="placeholderText">
              <string>请输入端口号（如3306）</string>
             </property>
            </widget>
           </item>
          </layout>
         </item>
         <item>
          <layout class="QHBoxLayout" name="dbNameLayout">
           <property name="spacing">
            <number>10</number>
           </property>
           <item>
            <widget class="QLabel" name="lblDBName">
             <property name="minimumSize">
              <size>
               <width>80</width>
               <height>0</height>
              </size>
             </property>
             <property name="maximumSize">
              <size>
               <width>80</width>
               <height>16777215</height>
              </size>
             </property>
             <property name="text">
              <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;数据库名:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
             </property>
             <property name="alignment">
              <set>Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter</set>
             </property>
            </widget>
           </item>
           <item>
            <widget class="QLineEdit" name="txtDBName">
             <property name="placeholderText">
              <string>请输入数据库名</string>
             </property>
            </widget>
           </item>
          </layout>
         </item>
         <item>
          <layout class="QHBoxLayout" name="usernameLayout">
           <property name="spacing">
            <number>10</number>
           </property>
           <item>
            <widget class="QLabel" name="lblUsername">
             <property name="minimumSize">
              <size>
               <width>80</width>
               <height>0</height>
              </size>
             </property>
             <property name="maximumSize">
              <size>
               <width>80</width>
               <height>16777215</height>
              </size>
             </property>
             <property name="text">
              <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;账户:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
             </property>
             <property name="alignment">
              <set>Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter</set>
             </property>
            </widget>
           </item>
           <item>
            <widget class="QLineEdit" name="txtUsername">
             <property name="placeholderText">
              <string>请输入数据库账户</string>
             </property>
            </widget>
           </item>
          </layout>
         </item>
         <item>
          <layout class="QHBoxLayout" name="passwordLayout">
           <property name="spacing">
            <number>10</number>
           </property>
           <item>
            <widget class="QLabel" name="lblPassword">
             <property name="minimumSize">
              <size>
               <width>80</width>
               <height>0</height>
              </size>
             </property>
             <property name="maximumSize">
              <size>
               <width>80</width>
               <height>16777215</height>
              </size>
             </property>
             <property name="text">
              <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;密码:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
             </property>
             <property name="alignment">
              <set>Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter</set>
             </property>
            </widget>
           </item>
           <item>
            <widget class="QLineEdit" name="txtPassword">
             <property name="echoMode">
              <enum>QLineEdit::Password</enum>
             </property>
             <property name="placeholderText">
              <string>请输入数据库密码</string>
             </property>
            </widget>
           </item>
          </layout>
         </item>
         <item>
          <spacer name="middleSpacer">
           <property name="orientation">
            <enum>Qt::Vertical</enum>
           </property>
           <property name="sizeType">
            <enum>QSizePolicy::Fixed</enum>
           </property>
           <property name="sizeHint" stdset="0">
            <size>
             <width>20</width>
             <height>10</height>
            </size>
           </property>
          </spacer>
         </item>
         <item>
          <widget class="QLabel" name="parkingSectionLabel">
           <property name="text">
            <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;停车场配置&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
           </property>
          </widget>
         </item>
         <item>
          <layout class="QHBoxLayout" name="parkingNameLayout">
           <property name="spacing">
            <number>10</number>
           </property>
           <item>
            <widget class="QLabel" name="lblParkingName">
             <property name="minimumSize">
              <size>
               <width>80</width>
               <height>0</height>
              </size>
             </property>
             <property name="maximumSize">
              <size>
               <width>80</width>
               <height>16777215</height>
              </size>
             </property>
             <property name="text">
              <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;停车场名:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
             </property>
             <property name="alignment">
              <set>Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter</set>
             </property>
            </widget>
           </item>
           <item>
            <widget class="QLineEdit" name="txtParkingName">
             <property name="placeholderText">
              <string>请输入停车场名称</string>
             </property>
            </widget>
           </item>
          </layout>
         </item>
         <item>
          <layout class="QHBoxLayout" name="priceLayout">
           <property name="spacing">
            <number>10</number>
           </property>
           <item>
            <widget class="QLabel" name="lblPrice">
             <property name="minimumSize">
              <size>
               <width>80</width>
               <height>0</height>
              </size>
             </property>
             <property name="maximumSize">
              <size>
               <width>80</width>
               <height>16777215</height>
              </size>
             </property>
             <property name="text">
              <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;价格:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
             </property>
             <property name="alignment">
              <set>Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter</set>
             </property>
            </widget>
           </item>
           <item>
            <widget class="QLineEdit" name="txtPrice">
             <property name="placeholderText">
              <string>请输入停车价格（元/小时）</string>
             </property>
            </widget>
           </item>
          </layout>
         </item>
         <item>
          <layout class="QHBoxLayout" name="capacityLayout">
           <property name="spacing">
            <number>10</number>
           </property>
           <item>
            <widget class="QLabel" name="lblCapacity">
             <property name="minimumSize">
              <size>
               <width>80</width>
               <height>0</height>
              </size>
             </property>
             <property name="maximumSize">
              <size>
               <width>80</width>
               <height>16777215</height>
              </size>
             </property>
             <property name="text">
              <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;容量:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
             </property>
             <property name="alignment">
              <set>Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter</set>
             </property>
            </widget>
           </item>
           <item>
            <widget class="QLineEdit" name="txtCapacity">
             <property name="placeholderText">
              <string>请输入停车场车位容量</string>
             </property>
            </widget>
           </item>
          </layout>
         </item>
         <item>
          <spacer name="buttonTopSpacer">
           <property name="orientation">
            <enum>Qt::Vertical</enum>
           </property>
           <property name="sizeType">
            <enum>QSizePolicy::Expanding</enum>
           </property>
           <property name="sizeHint" stdset="0">
            <size>
             <width>20</width>
             <height>10</height>
            </size>
           </property>
          </spacer>
         </item>
         <item>
          <layout class="QHBoxLayout" name="buttonLayout">
           <property name="spacing">
            <number>20</number>
           </property>
           <item>
            <widget class="QPushButton" name="btnCancel">
             <property name="minimumSize">
              <size>
               <width>0</width>
               <height>40</height>
              </size>
             </property>
             <property name="text">
              <string>取消</string>
             </property>
            </widget>
           </item>
           <item>
            <widget class="QPushButton" name="btnSubmit">
             <property name="minimumSize">
              <size>
               <width>0</width>
               <height>40</height>
              </size>
             </property>
             <property name="text">
              <string>提交</string>
             </property>
            </widget>
           </item>
          </layout>
         </item>
        </layout>
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

- [ ] **Step 2: 验证 UI 文件格式**

在 Qt Designer 中打开 `configinitdialog.ui`，确认布局结构正确，无报错。

---

## Task 2: 修改头文件 - 添加事件处理声明

**Files:**
- Modify: `UI/ConfigInit/configinitdialog.h`

- [ ] **Step 1: 添加必要的头文件和声明**

```cpp
#ifndef CONFIGINITDIALOG_H
#define CONFIGINITDIALOG_H

#include <QDialog>
#include <QPoint>

namespace Ui {
class ConfigInitDialog;
}

class ConfigInitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigInitDialog(QWidget *parent = nullptr);
    ~ConfigInitDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_btnSubmit_clicked();
    void on_btnCancel_clicked();
    void on_btnMinimize_clicked();
    void on_btnClose_clicked();

private:
    Ui::ConfigInitDialog *ui;
    bool m_dragging;
    QPoint m_dragPosition;

    bool validateInputs();
    void saveConfig();
    void setupWindowFlags();
    void setupShadow();
    void setupTitleBar();
};

#endif // CONFIGINITDIALOG_H
```

---

## Task 3: 修改实现文件 - 实现无边框窗口功能

**Files:**
- Modify: `UI/ConfigInit/configinitdialog.cpp`

- [ ] **Step 1: 更新头文件引用和构造函数**

```cpp
#include "configinitdialog.h"
#include "ui_configinitdialog.h"
#include "initfile.h"
#include <QMessageBox>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

ConfigInitDialog::ConfigInitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigInitDialog),
    m_dragging(false)
{
    ui->setupUi(this);
    setFixedSize(700, 900);

    setupWindowFlags();
    setupShadow();
    setupTitleBar();

    // 加载配置窗口样式
    QFile styleFile(":/styles/config.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
}
```

- [ ] **Step 2: 实现窗口标志设置**

```cpp
void ConfigInitDialog::setupWindowFlags()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
}
```

- [ ] **Step 3: 实现阴影效果**

```cpp
void ConfigInitDialog::setupShadow()
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    ui->mainContainer->setGraphicsEffect(shadow);
}
```

- [ ] **Step 4: 实现标题栏设置**

```cpp
void ConfigInitDialog::setupTitleBar()
{
    ui->titleBar->installEventFilter(this);
}
```

- [ ] **Step 5: 实现事件过滤器（拖拽逻辑）**

```cpp
bool ConfigInitDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->titleBar) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        switch (event->type()) {
        case QEvent::MouseButtonPress:
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragPosition = mouseEvent->globalPos() - frameGeometry().topLeft();
                return true;
            }
            break;

        case QEvent::MouseMove:
            if (m_dragging && (mouseEvent->buttons() & Qt::LeftButton)) {
                move(mouseEvent->globalPos() - m_dragPosition);
                return true;
            }
            break;

        case QEvent::MouseButtonRelease:
            if (mouseEvent->button() == Qt::LeftButton) {
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

- [ ] **Step 6: 实现按钮槽函数**

```cpp
void ConfigInitDialog::on_btnMinimize_clicked()
{
    showMinimized();
}

void ConfigInitDialog::on_btnClose_clicked()
{
    close();
}
```

- [ ] **Step 7: 保留现有函数**

确保 `validateInputs()`、`saveConfig()`、`on_btnSubmit_clicked()`、`on_btnCancel_clicked()` 和析构函数保持不变。

---

## Task 4: 更新 QSS 样式

**Files:**
- Modify: `styles/config.qss`

- [ ] **Step 1: 添加标题栏和按钮样式**

在 `config.qss` 文件末尾添加：

```css
/* ==================== 标题栏样式 ==================== */

/* 标题栏容器 */
#titleBar {
    background-color: #ffffff;
    border-top-left-radius: 8px;
    border-top-right-radius: 8px;
    border-bottom: 1px solid #f1f5f9;
}

/* 标题栏按钮基础样式 */
QPushButton#btnMinimize,
QPushButton#btnClose {
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

/* 最小化按钮悬浮 */
QPushButton#btnMinimize:hover {
    background-color: #f1f5f9;
    color: #64748b;
}

/* 最小化按钮按下 */
QPushButton#btnMinimize:pressed {
    background-color: #e2e8f0;
}

/* 关闭按钮悬浮 */
QPushButton#btnClose:hover {
    background-color: #ef4444;
    color: #ffffff;
}

/* 关闭按钮按下 */
QPushButton#btnClose:pressed {
    background-color: #dc2626;
}

/* ==================== 主容器样式 ==================== */
#mainContainer {
    background-color: #ffffff;
    border-radius: 8px;
}
```

- [ ] **Step 2: 验证样式**

编译并运行程序，检查按钮悬浮效果是否正确显示。

---

## Task 5: 编译和测试

**Files:**
- None (testing only)

- [ ] **Step 1: 编译项目**

```bash
cd d:/QTproject/ParkingSystem
qmake ParkingSystem.pro
make
```

- [ ] **Step 2: 运行程序验证**

运行程序，打开 ConfigInitDialog，验证：
- 窗口无系统标题栏
- 最小化按钮悬浮时显示浅灰色背景
- 关闭按钮悬浮时显示红色背景
- 顶部区域可拖拽移动窗口
- 窗口有柔和阴影效果
- 点击最小化按钮窗口最小化
- 点击关闭按钮窗口关闭
- 现有表单功能不受影响

- [ ] **Step 3: 提交代码**

```bash
git add UI/ConfigInit/configinitdialog.ui UI/ConfigInit/configinitdialog.h UI/ConfigInit/configinitdialog.cpp styles/config.qss
git commit -m "feat: 为 ConfigInitDialog 实现无边框窗口

- 移除系统标题栏，使用 Qt::FramelessWindowHint
- 添加自定义标题栏，支持顶部区域拖拽移动
- 添加最小化按钮（悬浮浅灰色）和关闭按钮（悬浮红色）
- 使用 QGraphicsDropShadowEffect 实现窗口阴影
- 更新 QSS 样式"
```

---

## 自检清单

- [ ] 所有需求已覆盖
- [ ] 无 TBD/TODO 占位符
- [ ] 文件路径准确
- [ ] 代码完整可执行
- [ ] 命令和预期输出明确
