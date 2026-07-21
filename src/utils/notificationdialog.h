#ifndef NOTIFICATIONDIALOG_H
#define NOTIFICATIONDIALOG_H
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
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

    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    void buildButtons();
    DialogType m_type;
    QString m_detail;
    QLabel      *m_iconLabel;
    QLabel      *m_titleLabel;
    QLabel      *m_msgLabel;
    QPushButton *m_primaryButton;
    QPushButton *m_secondaryButton;
    QWidget     *m_mask;
    QPropertyAnimation *m_scaleAnim;
    QPropertyAnimation *m_opaAnim;
    static constexpr int kDialogWidth     = 440;
    static constexpr int kDialogHeight    = 280;
    static constexpr int kMaxDialogWidth  = 520;
    static constexpr int kMaxDialogHeight = 400;
};
#endif