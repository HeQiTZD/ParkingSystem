#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QQueue>
#include <QTimer>
#include <QGraphicsOpacityEffect>

/**
 * @brief Toast 自动消失提示框
 *
 * 用法（静态方法，无需实例化）：
 *   ToastWidget::show(parent, "操作成功",          ToastWidget::Success);
 *   ToastWidget::show(parent, "操作失败",          ToastWidget::Failure);
 *   ToastWidget::show(parent, "磁盘空间不足",       ToastWidget::Warning, 5000);
 *
 * 最多同时显示 5 条，超出丢弃最旧的；
 * 成功/信息 3 秒自动消失，失败/警告 3.5 秒；
 * 点击右侧 X 可手动关闭。
 */
class ToastWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    // 枚举四种提示类型
    enum ToastType { Success, Failure, Info, Warning };

    explicit ToastWidget(QWidget *parent = nullptr);

    /** 静态便捷调用入口 */
    static void show(QWidget *parent, const QString &msg,
                     ToastType type = Info, int durationMs = 0);
    /** 0 = 使用默认值（成功/信息 3000ms，失败/警告 3500ms） */

protected:
    // 事件过滤
    bool eventFilter(QObject *obj, QEvent *event) override;
    // 自定义绘制
    void paintEvent(QPaintEvent *event) override;

private:
    void buildUi(const QString &msg, ToastType type);
    void slideIn();// 从目标位置上方滑入 + 淡入
    void slideOut(const std::function<void()> &onFinished);// 淡出 + 完成回调
    void restackAll(); // 把全部活跃 Toast 重排到队列中的对应行(顶部 row=0 保持最新)

    /** Toast 实例化后调用，显示在 parent 右上角 */
    void showImmediatly(const QString &msg, ToastType type, int durationMs);

    /** 根据队列索引计算 Toast 应处的绝对屏幕坐标 */
    QPoint targetPosition(int row) const;

    // opacity 属性读写
    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal o);

    QLabel      *m_iconLabel;// 图标
    QLabel      *m_msgLabel;// 提示文字
    QPushButton *m_closeButton;// 右侧 X 关闭按钮
    QPropertyAnimation *m_posAnim;// 位置动画（入场滑入 + 挤开旧 Toast）
    QPropertyAnimation *m_opaAnim;// 透明度动画
    QGraphicsOpacityEffect *m_opacityEffect; // 透明度效果器
    qreal m_opacity = 0.0;   // 当前透明度（动画驱动）
    QPoint  m_targetPos;     // 入场滑入的目标位置(绝对屏幕坐标)

    // ── 全局活跃队列 ─────────────────────────────────────
    /*
    多 Toast 排队机制的核心数据结构——用一个队列管理当前正在显示的所有 Toast，
    方便后续 Toast 计算堆叠位置（后出现的在下方堆叠）。
    */
    static QQueue<ToastWidget *> s_active;
    static constexpr int kToastWidth  = 320;
    static constexpr int kToastHeight = 64;
    static constexpr int kMaxActive   = 5;
    static constexpr int kSpacing     = 10;
    static constexpr int kMarginRight = 20;
    static constexpr int kMarginTop   = 60;   // 避开自定义标题栏
    static constexpr int kFadeMs      = 250;   // 淡出时长
    static constexpr int kPushMs      = 200;   // 挤开旧 Toast 的位移时长
    static constexpr int kSlideInMs   = 280;   // 入场滑入时长
    static constexpr int kSlideOffset = 60;    // 入场起点相对目标位置的向下偏移
};

#endif // TOASTWIDGET_H
