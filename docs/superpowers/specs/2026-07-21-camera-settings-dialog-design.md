# Camera Management Settings Dialog Design

**Date:** 2026-07-21
**Status:** Approved

## 1. Overview

为 `CameraManagement` 页面的「⚙ 设置」按钮实现摄像头配置弹窗。与 MainWindow 的全局设置（数据库/停车场/识别）区分，专注摄像头参数配置。

### Scope

- **包含：** 全局采集参数（分辨率/帧率）+ 每路摄像头独立属性（名称/位置/角色/分辨率/帧率）
- **不包含：** 数据库配置、停车场参数、识别置信度、摄像头增删

### Key Decisions

| 维度 | 决策 |
|------|------|
| 设置范围 | 全局参数 + 每路摄像头全部字段独立配置 |
| 交互形式 | QDialog 独立弹窗（无边框+投影+拖拽） |
| 列表组织 | 全局区在上方，摄像头列表垂直排列 |
| 每路可编辑 | name, location, role, width, height, fps |
| 角色约束 | 严格互斥，同时只能有一路为 "entry" |
| UI 构建 | .ui 静态骨架 + 代码动态填充摄像头行 |
| 样式 | 专属 `camerasettings.qss` |
| 生效时机 | 配置保存后，重启应用后生效 |

## 2. File Structure

### New Files

| File | Purpose |
|------|---------|
| `UI/CameraManagement/camerasettingsdialog.h` | 类声明 |
| `UI/CameraManagement/camerasettingsdialog.cpp` | 实现（load/save/buildCameraRows） |
| `UI/CameraManagement/camerasettingsdialog.ui` | Qt Designer 表单（全局参数区 + 骨架） |
| `styles/camerasettings.qss` | 专属样式表 |

### Modified Files

| File | Change |
|------|--------|
| `UI/CameraManagement/cameramanagement.cpp:38-40` | 设置按钮连接 → `CameraSettingsDialog` |
| `styles/styles.qrc` | 注册 `camerasettings.qss` |
| `ParkingSystem.pro` | 注册 .h/.cpp/.ui + RESOURCES |

## 3. Class Interface

```cpp
class CameraSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CameraSettingsDialog(QWidget *parent = nullptr);
    ~CameraSettingsDialog();

signals:
    void camerasUpdated();       // 配置变更（名称/角色/位置）
    void cameraConfigChanged();  // 分辨率/FPS 变更（需重启摄像头线程）

private slots:
    void onSave();
    void onCancel();

private:
    void setupWindow();          // 无边框 + QSS + 投影 + 拖拽
    void loadSettings();         // InitFile + CameraManager → 控件
    void buildCameraRows();      // 动态创建每路摄像头编辑行
    QWidget* createCameraRow(int index, const CameraInfo &info);

    Ui::CameraSettingsDialog *ui;
    QList<QWidget*> m_cameraRows; // 动态行引用，用于保存时读取值
    bool m_dragging = false;
    QPoint m_dragPosition;
};
```

**依赖：**
- `InitFile::instance()` — 读写摄像头配置（getCameraWidth/Height/Fps, getCameras/setCameras）
- `CameraManager::instance()` — 获取当前摄像头数量和信息
- 无 `DatabaseManager` 依赖（摄像头专用设置）

**事件处理：** 标题栏 `eventFilter` 实现拖拽，复用 ConfigInitDialog 模式。

## 4. UI Layout

弹窗尺寸：**620×520**，无边框 + 半透明背景 + `QGraphicsDropShadowEffect` 投影。

```
┌──────────────────────────────────────┐
│  摄像头设置                     ×   │  ← 标题栏（可拖拽区域）
├──────────────────────────────────────┤
│  ┌─ 全局采集参数 ──────────────────┐ │
│  │  分辨率  [1920 ▼] × [1080 ▼]   │ │  ← QComboBox 预设+可编辑
│  │  帧率    [30 ▼] FPS            │ │  ← QComboBox (15/20/25/30)
│  └──────────────────────────────────┘ │
│                                      │
│  ┌─ 摄像头列表（QScrollArea）──────┐ │
│  │ ┌─ 📷 摄像头 1 ──────────────┐ │ │  ← createCameraRow(0, info)
│  │ │ 名称 [主入口        ]      │ │ │
│  │ │ 位置 [大门          ]      │ │ │
│  │ │ 角色 [● entry ○ monitor] │ │ │  ← 同一 group，全局互斥
│  │ │ 分辨率[1920▼]×[1080▼]    │ │ │
│  │ │ 帧率  [30▼] FPS           │ │ │
│  │ └─────────────────────────────┘ │ │
│  │ ┌─ 📷 摄像头 2 ──────────────┐ │ │
│  │ │ 名称 [地下车库A      ]    │ │ │
│  │ │ ...                        │ │ │
│  │ └─────────────────────────────┘ │ │
│  └──────────────────────────────────┘ │
│                                      │
│              [取消]    [保存]         │
└──────────────────────────────────────┘
```

### Widget Details

- **分辨率选择：** QComboBox，预设 [480, 640, 800, 1024, 1280, 1920, 2560]，`setEditable(true)` 允许手动输入
- **帧率选择：** QComboBox，预设 [15, 20, 25, 30]，`setEditable(true)`
- **角色选择：** QRadioButton ("entry" / "monitor")，所有摄像头行属于同一个 `QButtonGroup`
- **摄像头列表：** QScrollArea 内 QVBoxLayout，`buildCameraRows()` 中 for 循环动态追加
- **控件命名：** 动态控件使用 `cam_name_N`、`cam_location_N` 等前缀，避免与 .ui 静态控件冲突

### .ui 骨架

```xml
<!-- 标题栏 + 全局参数区 + scrollArea + 按钮区，所有静态部分 -->
<!-- 摄像头列表区域为空，由 buildCameraRows() 填充 -->
```

## 5. Data Flow

### Load (打开弹窗 / loadSettings)

```
InitFile::instance().getCameraWidth()  → 全局分辨率宽度 ComboBox
InitFile::instance().getCameraHeight() → 全局分辨率高度 ComboBox
InitFile::instance().getCameraFps()    → 全局帧率 ComboBox

CameraManager::instance().count()      → 确定行数
  for i in 0..count-1:
    CameraManager::instance().info(i)  → createCameraRow(i, info)
```

### Save (点击保存 / onSave)

```
1. 角色互斥校验
   → 多于 1 个 entry → QMessageBox::warning("只能设置一个入口摄像头") → return
   → 0 个 entry → QMessageBox::question("无入口摄像头，识别将暂停，继续？") → No → return

2. 写全局配置
   InitFile::instance().setCameraConfig(globalWidth, globalHeight, globalFps)

3. 写每路配置
   构建 QJsonArray，遍历 m_cameraRows 读取值
   InitFile::instance().setCameras(camerasArray)

4. 持久化
   InitFile::instance().saveConfig()
   → 失败 → QMessageBox::warning("配置保存失败") → return

5. emit camerasUpdated()
   emit cameraConfigChanged()
   QMessageBox::information("摄像头配置已保存，重启应用后生效")
   accept()
```

### Cancel

```
reject() — 不保存，直接关闭
```

## 6. Role Exclusivity Logic

所有摄像头行的 role QRadioButton 属于同一个 `QButtonGroup`，天然互斥。保存时额外校验：

```cpp
int entryCount = 0;
for (auto *row : m_cameraRows) {
    if (row->findChild<QRadioButton*>("cam_role_entry")->isChecked())
        entryCount++;
}
if (entryCount > 1) {
    // 防御性检查（QButtonGroup 已保证互斥，此处为冗余安全）
    QMessageBox::warning(this, "错误", "只能设置一个入口摄像头");
    return;
}
if (entryCount == 0) {
    auto reply = QMessageBox::question(this, "警告",
        "当前无入口摄像头，车牌识别功能将暂停。确定继续？");
    if (reply != QMessageBox::Yes) return;
}
```

## 7. Error Handling

| Scenario | Handling |
|----------|----------|
| 角色互斥冲突 | 弹窗阻止保存 |
| 0 个 entry（全部 monitor） | 警告确认后允许保存 |
| 分辨率/帧率手动输入非数字或 ≤0 | QComboBox validator 或保存时校验提示 |
| 名称为空 | 保存时回退为 "摄像头 N" |
| 保存 JSON 写盘失败 | 弹窗提示具体错误 |
| 无摄像头时打开设置 | 全局区正常显示，摄像头列表显示 "未检测到摄像头" 提示 |
| 端口/价格为负数 | QSpinBox/QDoubleSpinBox 天然阻止（min >= 0） |

## 8. Implementation Approach：.ui 骨架 + 代码动态填充

**选择：方案 B**

- 静态部分（标题栏、全局区、保存/取消按钮）写在 .ui 文件中
- 摄像头列表区域在 .cpp 的 `buildCameraRows()` 中动态创建
- 样式写在独立 `camerasettings.qss` 中，作用域为 `#CameraSettingsDialog`
- .ui 文件复用 ConfigInitDialog 的窗口标志模式（FramelessWindowHint）

## 9. Configuration Persistence

### InitFile 存储格式

现有 JSON 结构基础上扩展：

```json
{
  "cameraWidth": 1920,
  "cameraHeight": 1080,
  "cameraFps": 30,
  "cameras": [
    {"index": 0, "name": "主入口", "location": "大门", "role": "entry", "width": 1920, "height": 1080, "fps": 30},
    {"index": 1, "name": "地下车库A", "location": "地下A区", "role": "monitor", "width": 1280, "height": 720, "fps": 25}
  ]
}
```

### 重启后生效

保存后通过 `cameraConfigChanged` 信号通知，但实际生效在下一次 `scanCameras()` 调用时（应用启动）。`cameramanagement.cpp` 连接 `camerasUpdated` 信号刷新名称/位置/角色 UI 标签，连接 `cameraConfigChanged` 提示用户重启。

## 10. Testing

| # | Operation | Expected |
|---|-----------|----------|
| 1 | 点击设置按钮 | 弹窗打开，各字段与当前配置一致 |
| 2 | 修改摄像头名称→保存→关闭→重新打开 | 新名称已持久化 |
| 3 | 修改全局分辨率→保存 | 弹出"重启后生效"提示 |
| 4 | 所有摄像头设为 monitor→保存→确认警告 | JSON 中无 entry 行 |
| 5 | 点击取消→重新打开 | 所有值恢复旧值 |
| 6 | 拖拽标题栏 | 弹窗可拖拽移动 |
| 7 | 点击 × 关闭 | 不保存关闭 |
| 8 | 分辨率手动输入 "abc" → 保存 | 校验拦截 |
