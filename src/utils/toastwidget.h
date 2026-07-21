#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QQueue>
#include <QGraphicsOpacityEffect>
class ToastWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    enum ToastType { Success, Failure, Info, Warning };
    explicit ToastWidget(QWidget *parent = nullptr);

    static void show(QWidget *parent, const QString &msg,
                     ToastType type = Info, int durationMs = 0);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    void buildUi(const QString &msg, ToastType type);
    void slideIn();
    void slideOut(const std::function<void()> &onFinished);
    void restackAll();

    void showImmediatly(const QString &msg, ToastType type, int durationMs);

    QPoint targetPosition(int row) const;

    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal o);
    QLabel      *m_iconLabel;
    QLabel      *m_msgLabel;
    QPushButton *m_closeButton;
    QPropertyAnimation *m_posAnim;
    QPropertyAnimation *m_opaAnim;
    QGraphicsOpacityEffect *m_opacityEffect;
    qreal m_opacity = 0.0;
    QPoint  m_targetPos;

    static QQueue<ToastWidget *> s_active;
    static constexpr int kToastWidth  = 320;
    static constexpr int kToastHeight = 64;
    static constexpr int kMaxActive   = 5;
    static constexpr int kSpacing     = 10;
    static constexpr int kMarginRight = 20;
    static constexpr int kMarginTop   = 60;
    static constexpr int kFadeMs      = 250;
    static constexpr int kPushMs      = 200;
    static constexpr int kSlideInMs   = 280;
    static constexpr int kSlideOffset = 60;
};
#endif