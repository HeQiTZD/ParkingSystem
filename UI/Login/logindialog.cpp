#include "logindialog.h"
#include "ui_logindialog.h"
#include <QResizeEvent>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QFile>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QLinearGradient>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_dragging(false)
{
    ui->setupUi(this);

    // 设置密码输入框模式
    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    // 设置无边框窗口
    setupWindowFlags();
    setupShadow();
    setupTitleBar();

    // 加载登录界面样式表
    QFile styleFile(":/styles/login.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        this->setStyleSheet(styleSheet);
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
    ui->brandPanel->setAutoFillBackground(false);
    ui->brandPanel->setStyleSheet("background-image: url(:/icons/login-brand-image-old); background-position: center; background-repeat: no-repeat; background-color: #1e3a5f;");
}

void LoginDialog::setupWindowFlags()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void LoginDialog::setupShadow()
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    ui->mainContainer->setGraphicsEffect(shadow);
}

void LoginDialog::setupTitleBar()
{
    ui->titleBar->installEventFilter(this);
}

bool LoginDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->titleBar) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        switch (event->type()) {
        case QEvent::MouseButtonPress:
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragPosition = mouseEvent->globalPos() - frameGeometry().topLeft();
                return true;
            }
            break;

        case QEvent::MouseMove:
            if (m_dragging && (mouseEvent->buttons() & Qt::LeftButton)) {
                move(mouseEvent->globalPos() - m_dragPosition);
                return true;
            }
            break;

        case QEvent::MouseButtonRelease:
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = false;
                return true;
            }
            break;

        default:
            break;
        }
    }

    return QDialog::eventFilter(obj, event);
}

void LoginDialog::on_btnMinimize_clicked()
{
    showMinimized();
}

void LoginDialog::on_btnClose_clicked()
{
    close();
}
