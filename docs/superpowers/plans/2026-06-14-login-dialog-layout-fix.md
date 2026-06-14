# 登录界面布局修复实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 修复 LoginDialog 中"用户登录"标题被遮挡和"忘记密码"标签显示不全的问题。

**Architecture:** 仅调整 UI 文件中控件的尺寸和位置参数，保持绝对定位方式不变。

**Tech Stack:** Qt Designer UI 文件

---

## 文件结构

| 文件 | 操作 | 职责 |
|------|------|------|
| `UI/Login/logindialog.ui` | 修改 | 调整 titleLabel 和 forgotPasswordLabel 的尺寸/位置 |

---

## Task 1: 修改 UI 文件尺寸参数

**Files:**
- Modify: `UI/Login/logindialog.ui`

- [ ] **Step 1: 调整 titleLabel 宽度**

找到 titleLabel 的 geometry 定义，将 width 从 131 改为 200：

```xml
<widget class="QLabel" name="titleLabel">
 <property name="geometry">
  <rect>
   <x>30</x>
   <y>20</y>
   <width>200</width>
   <height>41</height>
  </rect>
 </property>
</widget>
```

- [ ] **Step 2: 调整 forgotPasswordLabel 位置和宽度**

找到 forgotPasswordLabel 的 geometry 定义，将 x 从 470 改为 430，width 从 51 改为 100：

```xml
<widget class="QLabel" name="forgotPasswordLabel">
 <property name="geometry">
  <rect>
   <x>430</x>
   <y>310</y>
   <width>100</width>
   <height>21</height>
  </rect>
 </property>
</widget>
```

- [ ] **Step 3: 验证 XML 格式**

确认修改后的 UI 文件 XML 格式正确。

- [ ] **Step 4: 提交代码**

```bash
git add UI/Login/logindialog.ui
git commit -m "fix: 调整登录界面标题和忘记密码标签尺寸

- titleLabel 宽度从 131px 增加到 200px，解决标题被遮挡问题
- forgotPasswordLabel x 从 470 调整到 430，宽度从 51px 增加到 100px，解决显示不全问题"
```

---

## 自检清单

- [x] 所有需求已覆盖
- [x] 无 TBD/TODO 占位符
- [x] 文件路径准确
- [x] 代码完整可执行
- [x] 命令和预期输出明确
