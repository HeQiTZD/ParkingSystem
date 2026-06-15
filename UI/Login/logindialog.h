#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QPoint>
#include <QPixmap>
#include <QIcon>
#include <QPainter>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void on_btnMinimize_clicked();
    void on_btnClose_clicked();
    void on_togglePasswordVisibility_clicked();

private:
    Ui::LoginDialog *ui;
    bool m_dragging;
    QPoint m_dragPosition;

    // 自定义绘制相关成员变量
    QPixmap m_brandPixmap;          // 品牌背景图片
    QIcon m_personIcon;             // 用户图标
    QIcon m_lockIcon;               // 锁图标
    QIcon m_visibilityIcon;         // 可见图标
    QIcon m_visibilityOffIcon;      // 不可见图标
    bool m_passwordVisible;         // 密码是否可见

    // 布局常量
    static constexpr double BRAND_PANEL_RATIO = 0.4;  // 左侧品牌区占比

    // 私有方法
    void setupWindowFlags();
    void setupShadow();
    void setupTitleBar();
    void updateBrandPanelBackground();
    void loadResources();
    void drawBrandPanel(QPainter &painter, const QRect &rect);
    void drawLoginPanel(QPainter &painter, const QRect &rect);
    void updatePasswordVisibilityIcon();
};

#endif // LOGINDIALOG_H
