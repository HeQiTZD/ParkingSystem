#include "logindialog.h"
#include "ui_logindialog.h"
#include <QResizeEvent>
#include <QPixmap>
#include <QPainter>
#include <QPalette>
#include <QFile>
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    // 设置密码输入框模式
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    ui->passwordEdit->setPlaceholderText("请输入密码");
    ui->usernameEdit->setPlaceholderText("请输入用户名");

    // 加载登录界面样式表
    QFile styleFile(":/styles/login.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        ui->loginPanel->setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qWarning() << "无法加载样式表:" << styleFile.fileName();
    }

    updateBrandPanelBackground();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    updateBrandPanelBackground();
}

void LoginDialog::updateBrandPanelBackground()
{
    QPixmap pixmap(":/new/prefix1/brandImage");
    if (pixmap.isNull()) {
        return;
    }

    // 获取 brandPanel 的尺寸
    QSize panelSize = ui->brandPanel->size();

    // 等比缩放图片，保持宽高比，平滑变换
    QPixmap scaledPixmap = pixmap.scaled(panelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建一个与 panel 同尺寸的透明画布
    QPixmap background(panelSize);
    background.fill(Qt::transparent);

    // 在画布中心绘制缩放后的图片
    QPainter painter(&background);
    int x = (panelSize.width() - scaledPixmap.width()) / 2;
    int y = (panelSize.height() - scaledPixmap.height()) / 2;
    painter.drawPixmap(x, y, scaledPixmap);
    painter.end();

    // 设置为背景
    ui->brandPanel->setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(ui->brandPanel->backgroundRole(), QBrush(background));
    ui->brandPanel->setPalette(palette);
}
