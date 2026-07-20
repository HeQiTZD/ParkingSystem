# Settings Dialog Design

**Date:** 2026-07-21
**Status:** Approved

## Overview

为 MainWindow 的 `onSetButton()` 实现设置弹窗，允许用户在运行时修改数据库连接、停车场参数和识别置信度阈值。设计目标：简洁、复用现有模式、最小改动。

### Scope

- **包含：** 数据库连接（5 字段）、停车场参数（4 字段）、识别置信度阈值（1 字段）
- **不包含：** 摄像头配置、系统信息（appName/version）、识别模型路径（硬编码打包）

## File Structure

### New Files

| File | Purpose |
|------|---------|
| `UI/Settings/settingsdialog.h` | 类声明 |
| `UI/Settings/settingsdialog.cpp` | 实现 |
| `UI/Settings/settingsdialog.ui` | Qt Designer 表单（可选，代码构建也可） |

### Modified Files

| File | Change |
|------|--------|
| `UI/MainWindow/mainwindow.cpp` | `onSetButton()` 中实例化并弹出 SettingsDialog |
| `UI/MainWindow/mainwindow.h` | 引入 SettingsDialog 头文件 |

## Class Interface

```cpp
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
    void onCancel();

private:
    void loadSettings();   // InitFile → 控件
    void setupUi();        // 构建布局和样式
    void setupTitleBar();  // 无边框 + 拖拽

    DatabaseManager *m_db; // 仅用于测试连接，不持有所有权
    bool m_dragging;
    QPoint m_dragPosition;
};
```

**依赖：**
- `InitFile::instance()` — 读写所有配置
- `DatabaseManager` — 外部传入，仅用于 `onTestConnection()` 中的临时连接测试

## UI Layout

弹窗尺寸：**600 × 550**，无边框 + 半透明背景 + 投影阴影（复用 `config.qss` 和 ConfigInitDialog 模式）。

```
┌──────────────────────────────────┐
│  设置                    ✕ ─ □  │  ← 标题栏（拖拽区域）
├──────────────────────────────────┤
│                                  │
│  ┌─ 数据库连接 ────────────────┐ │
│  │  主机地址  [______________] │ │  QLineEdit
│  │  端口号    [______________] │ │  QSpinBox (0-65535)
│  │  数据库名  [______________] │ │  QLineEdit
│  │  用户名    [______________] │ │  QLineEdit
│  │  密码      [______________] │ │  QLineEdit (Password echo)
│  │              [测试连接]     │ │  QPushButton
│  └──────────────────────────────┘ │
│                                  │
│  ┌─ 停车场参数 ────────────────┐ │
│  │  停车场名称 [______________]│ │  QLineEdit
│  │  每小时价格 [___] 元/小时   │ │  QDoubleSpinBox
│  │  总车位数   [___] 个        │ │  QSpinBox
│  │  免费分钟   [___] 分钟      │ │  QSpinBox
│  └──────────────────────────────┘ │
│                                  │
│  ┌─ 识别参数 ──────────────────┐ │
│  │  置信度阈值 [====○====] 0.70│ │  QSlider (0-100) + QLabel
│  └──────────────────────────────┘ │
│                                  │
│              [保存]  [取消]       │  QPushButton × 2
└──────────────────────────────────┘
```

- 密码字段：`QLineEdit::EchoMode::Password`
- 置信度滑块：0-100 映射到 0.0-1.0，标签实时显示当前值
- 底部按钮：居中排列

## Data Flow

### Load (打开弹窗时)

```
InitFile::instance()
  → getDbHost / getDbPort / getDbName / getDbUsername / getDbPassword
  → getParkingName / getParkingPrice / getParkingCapacity / getFreeMinutes
  → getConfidenceThreshold
  → 填充控件
```

### Save (点击保存)

```
控件值
  → InitFile::instance().setDbConfig(...)
  → InitFile::instance().setParkingConfig(...)
  → InitFile::instance().setRecognitionConfig(modelPath="", threshold)
  → InitFile::instance().saveConfig()
  → 比对数据库参数是否变更
    → 变更: emit dbConfigChanged(), QMessageBox("数据库配置已保存，重启后生效")
  → emit settingsSaved()
  → accept()
```

### Test Connection (点击测试连接)

```
控件值 → 构造临时 QSqlDatabase 连接（独立于当前连接）
  → 成功 → QMessageBox::information("连接成功")
  → 失败 → QMessageBox::warning(错误信息)
```

测试连接使用独立的临时数据库连接名，不干扰当前正在使用的线程连接。

### Cancel

```
reject() — 不保存，直接关闭
```

## Error Handling

| 场景 | 处理 |
|------|------|
| 测试连接失败 | 弹窗提示具体错误，不阻断保存流程 |
| 保存时 JSON 写盘失败 | InitFile 内部已有信号 `configError`，弹窗提示"保存失败" |
| 空字段提交 | 空检查，非空字段才写入 |
| 端口/价格/容量 为负数 | QSpinBox/QDoubleSpinBox 天然不允许（min=0 或 1） |

## Signal Wiring (MainWindow)

```cpp
// mainwindow.cpp onSetButton()
auto *dialog = new SettingsDialog(m_db, this);
dialog->show(); // 非模态，用户可同时操作主窗口
```

`dbConfigChanged` 信号由 MainWindow 或 SettingsDialog 内部直接弹窗处理，不额外引入中间层。

## Testing

- 打开弹窗 → 验证各字段与 InitFile 中的值一致
- 修改字段 → 保存 → 重新打开弹窗 → 验证值已持久化
- 输入错误的数据库参数 → 测试连接 → 验证错误提示
- 修改数据库参数 → 保存 → 验证"重启后生效"提示
- 取消按钮 → 验证旧值不变
