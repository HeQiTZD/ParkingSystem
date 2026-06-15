#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QPoint>
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

    // 动画属性，用于QPropertyAnimation
    Q_PROPERTY(double focusProgress READ focusProgress WRITE setFocusProgress)

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_btnMinimize_clicked();
    void on_btnClose_clicked();

private:
    Ui::LoginDialog *ui;
    bool m_dragging;
    QPoint m_dragPosition;

    // UI控件（透明，用于交互）
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_togglePasswordBtn;
    QPushButton *m_loginButton;
    QPushButton *m_registerButton;
    QCheckBox *m_rememberMeCheckBox;
    QLabel *m_forgotPasswordLabel;

    // 背景图片
    QPixmap m_brandPixmap;

    // 图标资源
    QIcon m_personIcon;
    QIcon m_lockIcon;
    QIcon m_visibilityIcon;
    QIcon m_visibilityOffIcon;

    // 状态变量
    bool m_passwordVisible;
    bool m_usernameFocused;
    bool m_passwordFocused;
    bool m_usernameHovered;
    bool m_passwordHovered;
    bool m_loginButtonHovered;
    bool m_registerButtonHovered;
    bool m_isLoading;

    // 布局区域
    QRect m_brandRect;
    QRect m_loginRect;
    QRect m_usernameRect;
    QRect m_passwordRect;
    QRect m_toggleBtnRect;
    QRect m_loginBtnRect;
    QRect m_registerBtnRect;
    QRect m_rememberMeRect;
    QRect m_forgotPasswordRect;

    // 动画
    QPropertyAnimation *m_focusAnimation;
    double m_focusProgress;

    // 布局常量
    static constexpr double BRAND_PANEL_RATIO = 0.4;  // 左侧品牌区占比

    // 资源加载和UI设置
    void loadResources();
    void setupUI();
    void updateLayout();

    // 绘制方法
    void drawBrandPanel(QPainter &painter, const QRect &rect);
    void drawLoginPanel(QPainter &painter, const QRect &rect);
    void drawInputField(QPainter &painter, const QRect &rect, bool focused, bool hovered, bool isPassword, const QString &text, const QIcon &icon);
    void drawButton(QPainter &painter, const QRect &rect, const QString &text, bool primary, bool hovered);
    void drawRememberMe(QPainter &painter, const QRect &rect, bool checked);
    void drawForgotPassword(QPainter &painter, const QRect &rect);
    void drawLoadingIndicator(QPainter &painter, const QRect &rect);

    // 交互处理
    bool isPointInRect(const QPoint &pos, const QRect &rect);
    void handleLogin();
    void togglePasswordVisibility();

    // 动画属性
    double focusProgress() const;
    void setFocusProgress(double progress);
};

#endif // LOGINDIALOG_H
