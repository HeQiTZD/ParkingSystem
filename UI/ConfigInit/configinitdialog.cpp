#include "configinitdialog.h"
#include "ui_configinitdialog.h"
#include "initfile.h"
#include <QMessageBox>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

ConfigInitDialog::ConfigInitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigInitDialog),
    m_dragging(false)
{
    ui->setupUi(this);
    setFixedSize(700, 900);

    setupWindowFlags();
    setupShadow();
    setupTitleBar();

    // 加载配置窗口样式
    QFile styleFile(":/styles/config.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
}

ConfigInitDialog::~ConfigInitDialog()
{
    delete ui;
}

void ConfigInitDialog::setupWindowFlags()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void ConfigInitDialog::setupShadow()
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    ui->mainContainer->setGraphicsEffect(shadow);
}

void ConfigInitDialog::setupTitleBar()
{
    ui->titleBar->installEventFilter(this);
}

bool ConfigInitDialog::eventFilter(QObject *obj, QEvent *event)
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

void ConfigInitDialog::on_btnMinimize_clicked()
{
    showMinimized();
}

void ConfigInitDialog::on_btnClose_clicked()
{
    close();
}

bool ConfigInitDialog::validateInputs()
{
    if (ui->txtIP->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入IP地址");
        return false;
    }
    if (ui->txtPort->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入端口号");
        return false;
    }
    if (ui->txtDBName->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入数据库名");
        return false;
    }
    if (ui->txtUsername->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入账户");
        return false;
    }
    if (ui->txtPassword->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入密码");
        return false;
    }
    if (ui->txtParkingName->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入停车场名");
        return false;
    }
    if (ui->txtPrice->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入价格");
        return false;
    }
    if (ui->txtCapacity->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入容量");
        return false;
    }
    return true;
}

void ConfigInitDialog::saveConfig()
{
    InitFile initFile;
    initFile.loadConfig();

    // 保存数据库配置
    initFile.setDbConfig(ui->txtIP->text(),
                         ui->txtPort->text().toInt(),
                         ui->txtDBName->text(),
                         ui->txtUsername->text(),
                         ui->txtPassword->text());

    // 保存停车场配置
    initFile.setParkingConfig(ui->txtParkingName->text(),
                              ui->txtPrice->text().toDouble(),
                              ui->txtCapacity->text().toInt());

    initFile.saveConfig();
}

void ConfigInitDialog::on_btnSubmit_clicked()
{
    if (validateInputs()) {
        saveConfig();
        QMessageBox::information(this, "成功", "配置初始化完成");
        accept();
    }
}

void ConfigInitDialog::on_btnCancel_clicked()
{
    reject();
}
