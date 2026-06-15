# 登录表单布局改进实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 改进登录窗口右侧的表单布局，优化尺寸和视觉层次，创造更宽松的布局效果

**Architecture:** 采用优化固定宽度方案，调整输入框和按钮宽度，增加间距和边距，优化标题字号和按钮样式

**Tech Stack:** Qt 5.15.2, QSS, XML UI文件

---

## 文件结构

### 修改文件
- `UI/Login/logindialog.ui` - 登录窗口UI定义
- `styles/login.qss` - 登录窗口样式定义

---

## Task 1: 修改UI文件布局属性

**Files:**
- Modify: `UI/Login/logindialog.ui`

- [ ] **Step 1: 修改loginPanel布局属性**

打开 `UI/Login/logindialog.ui`，找到 `loginPanelLayout`，修改布局属性：

```xml
<layout class="QVBoxLayout" name="loginPanelLayout">
  <property name="spacing">
    <number>24</number>  <!-- 从15改为24 -->
  </property>
  <property name="leftMargin">
    <number>40</number>  <!-- 从30改为40 -->
  </property>
  <property name="topMargin">
    <number>30</number>  <!-- 从20改为30 -->
  </property>
  <property name="rightMargin">
    <number>40</number>  <!-- 从30改为40 -->
  </property>
  <property name="bottomMargin">
    <number>30</number>  <!-- 从20改为30 -->
  </property>
</layout>
```

- [ ] **Step 2: 修改输入框尺寸**

找到 `usernameEdit` 和 `passwordEdit`，修改尺寸属性：

```xml
<widget class="QLineEdit" name="usernameEdit">
  <property name="minimumSize">
    <size>
      <width>420</width>  <!-- 从500改为420 -->
      <height>48</height>  <!-- 从50改为48 -->
    </size>
  </property>
  <property name="maximumSize">
    <size>
      <width>420</width>  <!-- 从500改为420 -->
      <height>48</height>  <!-- 从50改为48 -->
    </size>
  </property>
</widget>

<widget class="QLineEdit" name="passwordEdit">
  <property name="minimumSize">
    <size>
      <width>420</width>  <!-- 从500改为420 -->
      <height>48</height>  <!-- 从50改为48 -->
    </size>
  </property>
  <property name="maximumSize">
    <size>
      <width>420</width>  <!-- 从500改为420 -->
      <height>48</height>  <!-- 从50改为48 -->
    </size>
  </property>
</widget>
```

- [ ] **Step 3: 修改按钮尺寸**

找到 `loginButton` 和 `registerButton`，修改尺寸属性：

```xml
<widget class="QPushButton" name="loginButton">
  <property name="minimumSize">
    <size>
      <width>420</width>  <!-- 从500改为420 -->
      <height>48</height>  <!-- 从50改为48 -->
    </size>
  </property>
  <property name="maximumSize">
    <size>
      <width>420</width>  <!-- 从500改为420 -->
      <height>48</height>  <!-- 从50改为48 -->
    </size>
  </property>
</widget>

<widget class="QPushButton" name="registerButton">
  <property name="minimumSize">
    <size>
      <width>420</width>  <!-- 从500改为420 -->
      <height>48</height>  <!-- 从50改为48 -->
    </size>
  </property>
  <property name="maximumSize">
    <size>
      <width>420</width>  <!-- 从500改为420 -->
      <height>48</height>  <!-- 从50改为48 -->
    </size>
  </property>
</widget>
```

- [ ] **Step 4: 验证UI文件修改**

运行应用程序，检查登录表单布局是否正确显示：

```bash
cd D:/QTproject/ParkingSystem
qmake ParkingSystem.pro
make
./ParkingSystem.exe
```

**预期结果：**
- 输入框宽度为420px
- 按钮宽度为420px
- 间距为24px
- 边距为40px左右，30px上下

- [ ] **Step 5: 提交UI文件修改**

```bash
git add UI/Login/logindialog.ui
git commit -m "feat: 优化登录表单布局尺寸"
```

---

## Task 2: 修改样式文件

**Files:**
- Modify: `styles/login.qss`

- [ ] **Step 1: 修改标题样式**

打开 `styles/login.qss`，找到 `#titleLabel`，修改字号：

```css
/* 标题样式 */
#titleLabel {
    color: #1a1a2e;
    font-size: 28pt;  /* 从24pt改为28pt */
    font-weight: bold;
    padding: 5px 0;
}
```

- [ ] **Step 2: 修改登录按钮样式**

找到 `#loginButton`，修改内边距和最小高度：

```css
/* 登录按钮样式 */
#loginButton {
    background-color: #4a90d9;
    color: #ffffff;
    border: none;
    border-radius: 8px;
    padding: 12px 24px;  /* 从14px改为12px */
    font-size: 12pt;
    font-weight: bold;
    min-height: 48px;  /* 从50px改为48px */
}
```

- [ ] **Step 3: 修改注册按钮样式**

找到 `#registerButton`，修改内边距和最小高度：

```css
/* 注册按钮样式 */
#registerButton {
    background-color: transparent;
    color: #4a90d9;
    border: 2px solid #4a90d9;
    border-radius: 8px;
    padding: 12px 24px;  /* 从14px改为12px */
    font-size: 12pt;
    font-weight: 600;
    min-height: 48px;  /* 从50px改为48px */
}
```

- [ ] **Step 4: 验证样式修改**

运行应用程序，检查样式是否正确应用：

```bash
cd D:/QTproject/ParkingSystem
qmake ParkingSystem.pro
make
./ParkingSystem.exe
```

**预期结果：**
- 标题字号为28pt
- 按钮内边距为12px 24px
- 按钮最小高度为48px

- [ ] **Step 5: 提交样式文件修改**

```bash
git add styles/login.qss
git commit -m "feat: 优化登录表单样式"
```

---

## Task 3: 最终验证和清理

**Files:**
- None (仅验证)

- [ ] **Step 1: 完整功能测试**

运行应用程序，测试所有功能：

1. 登录表单布局正确显示
2. 输入框尺寸符合预期（420px宽，48px高）
3. 按钮尺寸符合预期（420px宽，48px高）
4. 间距符合预期（24px）
5. 边距符合预期（40px左右，30px上下）
6. 标题字号符合预期（28pt）

- [ ] **Step 2: 视觉一致性检查**

1. 检查布局是否宽松舒适
2. 检查视觉层次是否分明
3. 检查元素对齐是否正确

- [ ] **Step 3: 代码审查**

1. 检查UI文件中的布局定义是否正确
2. 检查样式文件中的样式定义是否完整
3. 确保所有尺寸参数都已更新

- [ ] **Step 4: 提交最终版本**

```bash
git add .
git commit -m "feat: 完成登录表单布局改进"
```

---

## 自审查清单

### 规范覆盖检查
- [x] 输入框宽度优化（500px → 420px）
- [x] 按钮宽度优化（500px → 420px）
- [x] 间距优化（15px → 24px）
- [x] 边距优化（30px → 40px左右，20px → 30px上下）
- [x] 标题字号优化（24pt → 28pt）
- [x] 按钮样式优化（padding、min-height）

### 占位符扫描
- [x] 无TBD或TODO标记
- [x] 所有步骤都有具体实现
- [x] 所有代码都是完整的

### 类型一致性检查
- [x] 控件名称一致（usernameEdit、passwordEdit、loginButton、registerButton等）
- [x] 布局属性一致（spacing、leftMargin、topMargin、rightMargin、bottomMargin）
- [x] 样式定义一致（font-size、padding、min-height等）

### 范围检查
- [x] 聚焦于登录表单布局改进
- [x] 不涉及其他窗口的修改
- [x] 不涉及功能逻辑的修改

---

## 执行选项

**计划完成并保存到 `docs/superpowers/plans/2026-06-14-login-form-layout-design.md`。两种执行选项：**

**1. 子代理驱动（推荐）** - 我为每个任务分发一个新的子代理，任务之间进行审查，快速迭代

**2. 内联执行** - 在此会话中使用 executing-plans 执行任务，批量执行并设置检查点

**选择哪种方式？**