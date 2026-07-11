#ifndef NOTIFICATIONDIALOG_H
#define NOTIFICATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>

/**
 * @brief 统一模态弹窗（对标 QMessageBox）
 *
 * 静态调用：
 *   NotificationDialog::info(parent, "标题", "消息");
 *   NotificationDialog::warning(parent, "确认？", "是否继续？");  → 返回 bool
 *   NotificationDialog::error(parent, "错误", "数据库连接失败");
 *   NotificationDialog::question(parent, "询问", "是否删除？");  → 返回 bool
 *   NotificationDialog::errorWithDetail(parent, "错误", "简述", "技术细节");
 */
class NotificationDialog : public QDialog
{
    Q_OBJECT
public:
    enum DialogType { Info, Warning, Error, Question };

    static void info(QWidget *parent, const QString &title, const QString &msg);
    static bool warning(QWidget *parent, const QString &title, const QString &msg);
    static void error(QWidget *parent, const QString &title, const QString &msg);
    static bool question(QWidget *parent, const QString &title, const QString &msg);
    static void errorWithDetail(QWidget *parent, const QString &title,
                                const QString &brief, const QString &detail);

private:
    NotificationDialog(QWidget *parent, DialogType type,
                       const QString &title, const QString &msg,
                       const QString &detail = QString());
    void buildUi();
    void playShowAnimation();
    void playCloseAnimation(std::function<void()> onFinished);
    void EscClose();

    // 重写事件
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    // 构建按钮
    void buildButtons();

    DialogType m_type;
    QString m_detail;       // 错误详情（可选）

    QLabel      *m_iconLabel;
    QLabel      *m_titleLabel;
    QLabel      *m_msgLabel;
    QPushButton *m_primaryButton;
    QPushButton *m_secondaryButton;

    QWidget     *m_mask;    // 半透明遮罩，覆盖父窗口
    QPropertyAnimation *m_scaleAnim;
    QPropertyAnimation *m_opaAnim;

    static constexpr int kDialogWidth     = 440;    // 弹窗默认宽度（消息区约 28 个汉字/行）
    static constexpr int kDialogHeight    = 280;    // 弹窗默认高度（消息区约 5.5 行）
    static constexpr int kMaxDialogWidth  = 520;    // 弹窗最大宽度兜底
    static constexpr int kMaxDialogHeight = 400;    // 弹窗最大高度兜底
};

#endif // NOTIFICATIONDIALOG_H
