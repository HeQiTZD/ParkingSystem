# 登录窗口无边框统一设计实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 统一登录窗口和配置初始化窗口的无边框窗口设计，修复登录窗口左侧品牌栏区域的布局问题

**Architecture:** 采用最小改动方案，修改登录窗口的标题栏按钮样式、修复品牌栏布局、将登录表单改为使用布局管理器，确保与配置初始化窗口一致

**Tech Stack:** Qt 5.15.2, QSS, XML UI文件

---

## 文件结构

### 修改文件
- `UI/Login/logindialog.ui` - 登录窗口UI定义
- `styles/login.qss` - 登录窗口样式定义

### 参考文件
- `UI/ConfigInit/configinitdialog.ui` - 配置初始化窗口UI定义（参考标题栏设计）
- `styles/config.qss` - 配置初始化窗口样式定义（参考标题栏样式）

---

## Task 1: 修改标题栏按钮样式

**Files:**
- Modify: `UI/Login/logindialog.ui`
- Modify: `styles/login.qss`

- [ ] **Step 1: 修改最小化按钮样式**

打开 `UI/Login/logindialog.ui`，找到 `btnMinimize` 按钮，将其从圆形彩色按钮改为矩形文字按钮：

```xml
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
  <property name="cursor">
    <cursorShape>PointingHandCursor</cursorShape>
  </property>
  <property name="text">
    <string>─</string>
  </property>
</widget>
```

- [ ] **Step 2: 修改关闭按钮样式**

在同一个文件中，找到 `btnClose` 按钮，将其从圆形彩色按钮改为矩形文字按钮：

```xml
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
  <property name="cursor">
    <cursorShape>PointingHandCursor</cursorShape>
  </property>
  <property name="text">
    <string>×</string>
  </property>
</widget>
```

- [ ] **Step 3: 更新标题栏容器样式**

打开 `styles/login.qss`，在文件末尾添加标题栏样式：

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
```

- [ ] **Step 4: 移除旧的标题栏样式**

在 `styles/login.qss` 中，如果存在以下样式定义，请删除：

```css
/* 如果存在以下样式，请删除 */
#titleBar {
    background: #F8F8F8;
    border-top-left-radius: 12px;
    border-top-right-radius: 12px;
    border-bottom: 1px solid #E5E5E5;
}

QPushButton#btnMinimize {
    background: #FFBD2E;
    border: none;
    border-radius: 6px;
}

QPushButton#btnMinimize:hover {
    background: #FFB800;
}

QPushButton#btnClose {
    background: #FF5F57;
    border: none;
    border-radius: 6px;
}

QPushButton#btnClose:hover {
    background: #FF3B30;
}
```

- [ ] **Step 5: 验证标题栏按钮显示**

运行应用程序，检查登录窗口的标题栏按钮是否正确显示为矩形文字按钮：

```bash
cd D:/QTproject/ParkingSystem
qmake ParkingSystem.pro
make
./ParkingSystem.exe
```

**预期结果：**
- 最小化按钮显示为 ─ 符号，36x28px
- 关闭按钮显示为 × 符号，36x28px
- 按钮样式与配置初始化窗口一致

- [ ] **Step 6: 测试标题栏按钮功能**

1. 点击最小化按钮，窗口应最小化
2. 点击关闭按钮，窗口应关闭
3. 按住标题栏拖拽，窗口应跟随移动

- [ ] **Step 7: 提交标题栏按钮样式修改**

```bash
git add UI/Login/logindialog.ui styles/login.qss
git commit -m "feat: 统一登录窗口标题栏按钮样式为矩形文字按钮"
```

---

## Task 2: 修复品牌栏布局

**Files:**
- Modify: `UI/Login/logindialog.ui`

- [ ] **Step 1: 为brandPanel添加布局管理器**

打开 `UI/Login/logindialog.ui`，找到 `brandPanel` 控件，将其从空容器改为带布局管理器的容器：

```xml
<widget class="QWidget" name="brandPanel" native="true">
  <property name="sizePolicy">
    <sizepolicy hsizetype="Preferred" vsizetype="Preferred">
      <horstretch>2</horstretch>
      <verstretch>0</verstretch>
    </sizepolicy>
  </property>
  <layout class="QVBoxLayout" name="brandPanelLayout">
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
      <spacer name="brandSpacer">
        <property name="orientation">
          <enum>Qt::Vertical</enum>
        </property>
        <property name="sizeHint" stdset="0">
          <size>
            <width>20</width>
            <height>40</height>
          </size>
        </property>
      </spacer>
    </item>
  </layout>
</widget>
```

- [ ] **Step 2: 验证品牌栏显示**

运行应用程序，检查品牌栏是否正确显示：

```bash
cd D:/QTproject/ParkingSystem
qmake ParkingSystem.pro
make
./ParkingSystem.exe
```

**预期结果：**
- 品牌图片正确显示在brandPanel中
- 品牌栏布局正确，与登录表单对齐
- 窗口大小变化时品牌栏自适应

- [ ] **Step 3: 测试品牌栏自适应**

1. 调整窗口大小，品牌栏应自适应变化
2. 品牌图片应保持宽高比，居中显示
3. 品牌栏与登录表单的对齐应保持正确

- [ ] **Step 4: 提交品牌栏布局修复**

```bash
git add UI/Login/logindialog.ui
git commit -m "fix: 修复登录窗口品牌栏布局问题"
```

---

## Task 3: 改造登录表单布局

**Files:**
- Modify: `UI/Login/logindialog.ui`

- [ ] **Step 1: 修改loginPanel布局**

打开 `UI/Login/logindialog.ui`，找到 `loginPanel` 控件，将其从绝对定位改为使用布局管理器：

```xml
<widget class="QWidget" name="loginPanel" native="true">
  <property name="sizePolicy">
    <sizepolicy hsizetype="Preferred" vsizetype="Preferred">
      <horstretch>3</horstretch>
      <verstretch>0</verstretch>
    </sizepolicy>
  </property>
  <layout class="QVBoxLayout" name="loginPanelLayout">
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
          <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:24pt;&quot;&gt;用户登录&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
        </property>
      </widget>
    </item>
    <item>
      <layout class="QFormLayout" name="formLayout">
        <property name="spacing">
          <number>10</number>
        </property>
        <item row="0" column="0">
          <widget class="QLabel" name="usernameLabel">
            <property name="text">
              <string>用户名</string>
            </property>
          </widget>
        </item>
        <item row="0" column="1">
          <widget class="QLineEdit" name="usernameEdit">
            <property name="minimumSize">
              <size>
                <width>500</width>
                <height>50</height>
              </size>
            </property>
            <property name="maximumSize">
              <size>
                <width>500</width>
                <height>50</height>
              </size>
            </property>
          </widget>
        </item>
        <item row="1" column="0">
          <widget class="QLabel" name="passwordLabel">
            <property name="text">
              <string>密码</string>
            </property>
          </widget>
        </item>
        <item row="1" column="1">
          <widget class="QLineEdit" name="passwordEdit">
            <property name="minimumSize">
              <size>
                <width>500</width>
                <height>50</height>
              </size>
            </property>
            <property name="maximumSize">
              <size>
                <width>500</width>
                <height>50</height>
              </size>
            </property>
          </widget>
        </item>
      </layout>
    </item>
    <item>
      <layout class="QHBoxLayout" name="rememberForgotLayout">
        <property name="spacing">
          <number>0</number>
        </property>
        <item>
          <widget class="QCheckBox" name="rememberMeCheckBox">
            <property name="text">
              <string>记住我</string>
            </property>
          </widget>
        </item>
        <item>
          <spacer name="rememberForgotSpacer">
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
          <widget class="QLabel" name="forgotPasswordLabel">
            <property name="text">
              <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align=&quot;right&quot;&gt;忘记密码&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
            </property>
          </widget>
        </item>
      </layout>
    </item>
    <item>
      <widget class="QPushButton" name="loginButton">
        <property name="minimumSize">
          <size>
            <width>500</width>
            <height>50</height>
          </size>
        </property>
        <property name="maximumSize">
          <size>
            <width>500</width>
            <height>50</height>
          </size>
        </property>
        <property name="text">
          <string>登 录</string>
        </property>
      </widget>
    </item>
    <item>
      <widget class="QWidget" name="separatorContainer" native="true">
        <layout class="QHBoxLayout" name="horizontalLayout_2">
          <item>
            <widget class="Line" name="leftSeparatorLine">
              <property name="sizePolicy">
                <sizepolicy hsizetype="Minimum" vsizetype="Fixed">
                  <horstretch>2</horstretch>
                  <verstretch>0</verstretch>
                </sizepolicy>
              </property>
              <property name="orientation">
                <enum>Qt::Horizontal</enum>
              </property>
            </widget>
          </item>
          <item>
            <widget class="QLabel" name="orLabel">
              <property name="text">
                <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align=&quot;center&quot;&gt;或&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
            </widget>
          </item>
          <item>
            <widget class="Line" name="rightSeparatorLine">
              <property name="sizePolicy">
                <sizepolicy hsizetype="Minimum" vsizetype="Fixed">
                  <horstretch>2</horstretch>
                  <verstretch>0</verstretch>
                </sizepolicy>
              </property>
              <property name="orientation">
                <enum>Qt::Horizontal</enum>
              </property>
            </widget>
          </item>
        </layout>
      </widget>
    </item>
    <item>
      <widget class="QPushButton" name="registerButton">
        <property name="minimumSize">
          <size>
            <width>500</width>
            <height>50</height>
          </size>
        </property>
        <property name="maximumSize">
          <size>
            <width>500</width>
            <height>50</height>
          </size>
        </property>
        <property name="text">
          <string>注 册</string>
        </property>
      </widget>
    </item>
    <item>
      <widget class="QWidget" name="footerPanel" native="true">
        <layout class="QHBoxLayout" name="footerLayout">
          <property name="spacing">
            <number>0</number>
          </property>
          <item>
            <widget class="QLabel" name="copyrightLabel">
              <property name="text">
                <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align=&quot;center&quot;&gt;© 2024 Sentinel Technology. All rights reserved.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
            </widget>
          </item>
          <item>
            <spacer name="footerSpacer">
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
            <widget class="QLabel" name="termsLabel">
              <property name="text">
                <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align=&quot;center&quot;&gt;服务条款&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
            </widget>
          </item>
          <item>
            <widget class="QLabel" name="privacyLabel">
              <property name="text">
                <string>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p align=&quot;center&quot;&gt;隐私政策&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</string>
              </property>
            </widget>
          </item>
        </layout>
      </widget>
    </item>
  </layout>
</widget>
```

- [ ] **Step 2: 验证登录表单布局**

运行应用程序，检查登录表单是否正确显示：

```bash
cd D:/QTproject/ParkingSystem
qmake ParkingSystem.pro
make
./ParkingSystem.exe
```

**预期结果：**
- 登录表单使用布局管理器，控件自动对齐
- 用户名和密码输入框正确显示
- 登录按钮、注册按钮正确显示
- 底部版权信息正确显示

- [ ] **Step 3: 测试登录表单功能**

1. 输入用户名和密码，应能正常输入
2. 点击登录按钮，应触发登录逻辑
3. 点击注册按钮，应触发注册逻辑
4. 勾选记住我复选框，应能正常勾选

- [ ] **Step 4: 测试窗口自适应**

1. 调整窗口大小，登录表单应自适应变化
2. 控件应保持正确的对齐和间距
3. 窗口大小变化时不应出现布局错乱

- [ ] **Step 5: 提交登录表单布局改造**

```bash
git add UI/Login/logindialog.ui
git commit -m "feat: 将登录表单从绝对定位改为布局管理器"
```

---

## Task 4: 统一样式定义

**Files:**
- Modify: `styles/login.qss`

- [ ] **Step 1: 确保标题栏样式一致**

检查 `styles/login.qss` 中的标题栏样式是否与 `styles/config.qss` 中的标题栏样式一致。

**login.qss 中的标题栏样式应为：**
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
```

- [ ] **Step 2: 验证两个窗口的视觉一致性**

运行应用程序，同时打开登录窗口和配置初始化窗口，检查：

1. 标题栏按钮样式是否完全一致
2. 按钮悬浮和按下状态是否一致
3. 标题栏背景和边框是否一致

- [ ] **Step 3: 测试交互行为一致性**

1. 在两个窗口中测试最小化按钮，行为应一致
2. 在两个窗口中测试关闭按钮，行为应一致
3. 在两个窗口中测试标题栏拖拽，行为应一致

- [ ] **Step 4: 提交样式统一**

```bash
git add styles/login.qss
git commit -m "feat: 统一登录窗口和配置初始化窗口的标题栏样式"
```

---

## Task 5: 最终验证和清理

**Files:**
- None (仅验证)

- [ ] **Step 1: 完整功能测试**

运行应用程序，测试所有功能：

1. 登录窗口显示正确
2. 标题栏按钮功能正常
3. 品牌栏显示正确
4. 登录表单功能正常
5. 窗口自适应正常

- [ ] **Step 2: 视觉一致性检查**

1. 对比登录窗口和配置初始化窗口的标题栏样式
2. 检查所有按钮的悬浮和按下状态
3. 检查整体视觉效果的协调性

- [ ] **Step 3: 代码审查**

1. 检查UI文件中的布局定义是否正确
2. 检查样式文件中的样式定义是否完整
3. 确保没有遗留的绝对定位代码

- [ ] **Step 4: 提交最终版本**

```bash
git add .
git commit -m "feat: 完成登录窗口无边框统一设计"
```

---

## 自审查清单

### 规范覆盖检查
- [x] 标题栏按钮样式统一
- [x] 品牌栏布局修复
- [x] 登录表单布局改造
- [x] 样式定义统一

### 占位符扫描
- [x] 无TBD或TODO标记
- [x] 所有步骤都有具体实现
- [x] 所有代码都是完整的

### 类型一致性检查
- [x] 控件名称一致（btnMinimize, btnClose, titleBar等）
- [x] 布局属性一致（minimumSize, maximumSize等）
- [x] 样式定义一致（background-color, border-radius等）

### 范围检查
- [x] 聚焦于登录窗口的无边框统一设计
- [x] 不涉及其他窗口的修改
- [x] 不涉及功能逻辑的修改

---

## 执行选项

**计划完成并保存到 `docs/superpowers/plans/2026-06-14-login-dialog-unified-design.md`。两种执行选项：**

**1. 子代理驱动（推荐）** - 我为每个任务分发一个新的子代理，任务之间进行审查，快速迭代

**2. 内联执行** - 在此会话中使用 executing-plans 执行任务，批量执行并设置检查点

**选择哪种方式？**