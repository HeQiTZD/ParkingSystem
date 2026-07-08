#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QQueue>
#include <QTimer>

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
    // 滑入动画
    void slideIn();
    // 滑出动画 + 完成回调
    void slideOut(const std::function<void()> &onFinished);

    /** Toast 实例化后调用，显示在 parent 右上角 */
    void showImmediatly(const QString &msg, ToastType type, int durationMs);

    QLabel      *m_iconLabel;// 图标
    QLabel      *m_msgLabel;// 提示文字
    QPushButton *m_closeButton;// 右侧 X 关闭按钮
    QPropertyAnimation *m_posAnim;// 位置动画
    QPropertyAnimation *m_opaAnim;// 透明度动画(淡入淡出)

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
    static constexpr int kSlideMs     = 200;
    static constexpr int kFadeMs      = 250;
};

#endif // TOASTWIDGET_H
