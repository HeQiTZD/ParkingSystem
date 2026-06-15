# LoginDialog 无边框窗口改造实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 LoginDialog 改造为与 ConfigInitDialog 相同样式的无边框窗口，保留原有的左右分栏布局

**Architecture:** 参考 ConfigInitDialog 的实现，添加 mainContainer、titleBar、systemBtnsPanel 组件，实现窗口拖动、最小化、关闭功能

**Tech Stack:** Qt 5/6, C++, QSS 样式表

---

## 文件结构

**修改的文件：**
- `UI/Login/logindialog.ui` - 添加无边框窗口 UI 结构
- `UI/Login/logindialog.h` - 添加成员变量和方法声明
- `UI/Login/logindialog.cpp` - 实现无边框窗口功能

**参考的文件：**
- `UI/ConfigInit/configinitdialog.ui` - UI 结构参考
- `UI/ConfigInit/configinitdialog.h` - 头文件参考
- `UI/ConfigInit/configinitdialog.cpp` - 实现参考

---

### Task 1: 修改 logindialog.ui 文件

**Files:**
- Modify: `UI/Login/logindialog.ui`

- [ ] **Step 1: 备份当前 UI 文件**

```bash
cp UI/Login/logindialog.ui UI/Login/logindialog.ui.backup
```

- [ ] **Step 2: 修改根 QWidget 为无边框窗口**

将根 QWidget 的 objectName 改为 `nativeParentWidget`，并添加窗口标志：

```xml
<widget class="QWidget" name="nativeParentWidget">
 <property name="windowTitle">
  <string>登录</string>
 </property>
 <property name="styleSheet">
  <string notr="true">QWidget#nativeParentWidget {
    background: transparent;
}</string>
 </property>
```

- [ ] **Step 3: 添加 mainContainer 作为主容器**

在 nativeParentWidget 下添加 mainContainer：

```xml
<widget class="QWidget" name="mainContainer">
 <property name="styleSheet">
  <string notr="true">QWidget#mainContainer {
    background: #FFFFFF;
    border: 1px solid #E5E5E5;
    border-radius: 12px;
}</string>
 </property>
 <layout class="QVBoxLayout" name="verticalLayout_4">
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
   <widget class="QWidget" name="titleBar" native="true">
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
    <property name="styleSheet">
     <string notr="true">background: #F8F8F8;
border-top-left-radius: 12px;
border-top-right-radius: 12px;
border-bottom: 1px solid #E5E5E5;</string>
    </property>
    <layout class="QHBoxLayout" name="horizontalLayout_2">
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
      <number>16</number>
     </property>
     <property name="bottomMargin">
      <number>0</number>
     </property>
     <item>
      <spacer name="horizontalSpacer">
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
      <widget class="QWidget" name="systemBtnsPanel" native="true">
       <layout class="QHBoxLayout" name="horizontalLayout_3">
        <property name="spacing">
         <number>8</number>
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
         <widget class="QPushButton" name="btnMinimize">
          <property name="minimumSize">
           <size>
            <width>12</width>
            <height>12</height>
           </size>
          </property>
          <property name="maximumSize">
           <size>
            <width>12</width>
            <height>12</height>
           </size>
          </property>
          <property name="styleSheet">
           <string notr="true">QPushButton {
    background: #FFBD2E;
    border: none;
    border-radius: 6px;
}
QPushButton:hover {
    background: #FFB800;
}</string>
          </property>
          <property name="text">
           <string/>
          </property>
         </widget>
        </item>
        <item>
         <widget class="QPushButton" name="btnClose">
          <property name="minimumSize">
           <size>
            <width>12</width>
            <height>12</height>
           </size>
          </property>
          <property name="maximumSize">
           <size>
            <width>12</width>
            <height>12</height>
           </size>
          </property>
          <property name="styleSheet">
           <string notr="true">QPushButton {
    background: #FF5F57;
    border: none;
    border-radius: 6px;
}
QPushButton:hover {
    background: #FF3B30;
}</string>
          </property>
          <property name="text">
           <string/>
          </property>
         </widget>
        </item>
       </layout>
      </widget>
     </item>
    </layout>
   </widget>
  </item>
  <item>
   <widget class="QWidget" name="contentArea" native="true">
    <layout class="QHBoxLayout" name="horizontalLayout">
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
      <widget class="QWidget" name="brandPanel" native="true">
       <!-- 保留原有的品牌面板内容 -->
      </widget>
     </item>
     <item>
      <widget class="QWidget" name="loginPanel" native="true">
       <!-- 保留原有的登录表单内容 -->
      </widget>
     </item>
    </layout>
   </widget>
  </item>
 </layout>
</widget>
```

- [ ] **Step 4: 保留原有的 brandPanel 和 loginPanel 内容**

将原有的 brandPanel 和 loginPanel 的完整内容移动到 contentArea 的水平布局中。

- [ ] **Step 5: 验证 UI 文件格式正确**

```bash
cd d:/QTproject/ParkingSystem
qmake -v  # 验证 qmake 可用
```

---

### Task 2: 修改 logindialog.h 文件

**Files:**
- Modify: `UI/Login/logindialog.h`

- [ ] **Step 1: 添加必要的头文件包含**

在文件顶部添加：

```cpp
#include <QDialog>
#include <QPoint>
```

- [ ] **Step 2: 添加成员变量声明**

在 private 区域添加：

```cpp
private:
    Ui::LoginDialog *ui;
    bool m_dragging;
    QPoint m_dragPosition;
```

- [ ] **Step 3: 添加方法声明**

在 private 区域添加：

```cpp
private:
    void setupWindowFlags();
    void setupShadow();
    void setupTitleBar();
    void updateBrandPanelBackground();
```

- [ ] **Step 4: 添加槽函数声明**

在 private slots 区域添加：

```cpp
private slots:
    void on_btnMinimize_clicked();
    void on_btnClose_clicked();
```

- [ ] **Step 5: 添加事件过滤器声明**

在 protected 区域添加：

```cpp
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
```

- [ ] **Step 6: 验证头文件语法正确**

```bash
cd d:/QTproject/ParkingSystem
g++ -fsyntax-only -std=c++11 UI/Login/logindialog.h  # 检查语法
```

---

### Task 3: 修改 logindialog.cpp 文件

**Files:**
- Modify: `UI/Login/logindialog.cpp`

- [ ] **Step 1: 添加必要的头文件包含**

在文件顶部添加：

```cpp
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
```

- [ ] **Step 2: 修改构造函数**

修改构造函数以调用新的设置方法：

```cpp
LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_dragging(false)
{
    ui->setupUi(this);

    // 设置密码输入框模式
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    ui->passwordEdit->setPlaceholderText("请输入密码");
    ui->usernameEdit->setPlaceholderText("请输入用户名");

    // 设置无边框窗口
    setupWindowFlags();
    setupShadow();
    setupTitleBar();

    // 加载登录界面样式表
    QFile styleFile(":/styles/login.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        ui->loginPanel->setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qWarning() << "无法加载样式表:" << styleFile.fileName();
    }

    updateBrandPanelBackground();
}
```

- [ ] **Step 3: 实现 setupWindowFlags() 方法**

```cpp
void LoginDialog::setupWindowFlags()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
}
```

- [ ] **Step 4: 实现 setupShadow() 方法**

```cpp
void LoginDialog::setupShadow()
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    ui->mainContainer->setGraphicsEffect(shadow);
}
```

- [ ] **Step 5: 实现 setupTitleBar() 方法**

```cpp
void LoginDialog::setupTitleBar()
{
    ui->titleBar->installEventFilter(this);
}
```

- [ ] **Step 6: 实现 eventFilter() 方法**

```cpp
bool LoginDialog::eventFilter(QObject *obj, QEvent *event)
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

- [ ] **Step 7: 实现槽函数**

```cpp
void LoginDialog::on_btnMinimize_clicked()
{
    showMinimized();
}

void LoginDialog::on_btnClose_clicked()
{
    close();
}
```

- [ ] **Step 8: 验证编译通过**

```bash
cd d:/QTproject/ParkingSystem
qmake ParkingSystem.pro
make debug
```

---

### Task 4: 测试和验证

**Files:**
- Test: `UI/Login/logindialog.ui`
- Test: `UI/Login/logindialog.h`
- Test: `UI/Login/logindialog.cpp`

- [ ] **Step 1: 编译项目**

```bash
cd d:/QTproject/ParkingSystem
qmake ParkingSystem.pro
make clean
make debug
```

- [ ] **Step 2: 运行程序测试登录窗口**

```bash
cd d:/QTproject/ParkingSystem
./debug/ParkingSystem.exe
```

- [ ] **Step 3: 验证无边框样式**

检查项：
- [ ] 窗口无边框
- [ ] 窗口有圆角效果
- [ ] 窗口有阴影效果
- [ ] 标题栏显示在顶部
- [ ] 系统按钮（最小化、关闭）显示正确

- [ ] **Step 4: 验证窗口功能**

检查项：
- [ ] 可以拖动标题栏移动窗口
- [ ] 点击最小化按钮可以最小化窗口
- [ ] 点击关闭按钮可以关闭窗口
- [ ] 登录功能正常工作

- [ ] **Step 5: 验证布局保持不变**

检查项：
- [ ] 左侧品牌面板显示正常
- [ ] 右侧登录表单显示正常
- [ ] 输入框、按钮等元素位置正确

- [ ] **Step 6: 提交代码**

```bash
cd d:/QTproject/ParkingSystem
git add UI/Login/logindialog.ui UI/Login/logindialog.h UI/Login/logindialog.cpp
git commit -m "feat: 为 LoginDialog 实现无边框窗口

- 参考 ConfigInitDialog 的无边框窗口设计
- 添加 mainContainer、titleBar、systemBtnsPanel 组件
- 实现窗口拖动、最小化、关闭功能
- 保留原有的左右分栏布局
- 添加阴影效果和圆角样式"
```

---

## 完成检查

所有任务完成后，验证以下内容：

- [ ] LoginDialog 显示为无边框窗口
- [ ] 窗口样式与 ConfigInitDialog 一致
- [ ] 窗口拖动功能正常
- [ ] 最小化和关闭功能正常
- [ ] 登录功能正常工作
- [ ] 左右分栏布局保持不变
- [ ] 代码已提交到 git

---

## 参考资料

- ConfigInitDialog 实现：`UI/ConfigInit/configinitdialog.ui`、`configinitdialog.h`、`configinitdialog.cpp`
- Qt 无边框窗口文档：Qt::FramelessWindowHint
- Qt 阴影效果文档：QGraphicsDropShadowEffect
