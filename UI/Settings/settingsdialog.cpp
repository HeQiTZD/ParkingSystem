#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "src/utils/initfile.h"

#include <QFile>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QSqlDatabase>
#include <QSqlError>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setupWindow();
    loadSettings();

    // 置信度滑块-标签联动
    connect(ui->sliderConfidence, &QSlider::valueChanged, this, [this](int val) {
        ui->lblConfidenceValue->setText(QString::number(val / 100.0, 'f', 2));
    });

    // 标题栏按钮
    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnSubmit, &QPushButton::clicked, this, &SettingsDialog::onSave);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnTestConnection, &QPushButton::clicked, this, &SettingsDialog::onTestConnection);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setupWindow()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QFile styleFile(":/styles/settings.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    // 投影阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    ui->mainContainer->setGraphicsEffect(shadow);

    // 标题栏拖拽
    ui->titleBar->installEventFilter(this);
}

bool SettingsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->titleBar) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragPosition = me->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
            break;
        }
        case QEvent::MouseMove: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (m_dragging && (me->buttons() & Qt::LeftButton)) {
                move(me->globalPosition().toPoint() - m_dragPosition);
                return true;
            }
            break;
        }
        case QEvent::MouseButtonRelease: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                m_dragging = false;
                return true;
            }
            break;
        }
        default:
            break;
        }
    }
    return QDialog::eventFilter(obj, event);
}

void SettingsDialog::loadSettings()
{
    InitFile &cfg = InitFile::instance();

    // 数据库
    m_oldHost     = cfg.getDbHost();
    m_oldPort     = cfg.getDbPort();
    m_oldDbName   = cfg.getDbName();
    m_oldUsername = cfg.getDbUsername();
    m_oldPassword = cfg.getDbPassword();

    ui->txtDbHost->setText(m_oldHost);
    ui->txtDbPort->setValue(m_oldPort);
    ui->txtDbName->setText(m_oldDbName);
    ui->txtDbUsername->setText(m_oldUsername);
    ui->txtDbPassword->setText(m_oldPassword);

    // 停车场
    ui->txtParkingName->setText(cfg.getParkingName());
    ui->txtPrice->setValue(cfg.getParkingPrice());
    ui->txtCapacity->setValue(cfg.getParkingCapacity());
    ui->txtFreeMinutes->setValue(cfg.getFreeMinutes());

    // 识别
    int confidenceVal = qBound(0, static_cast<int>(cfg.getConfidenceThreshold() * 100), 100);
    ui->sliderConfidence->setValue(confidenceVal);
    ui->lblConfidenceValue->setText(QString::number(confidenceVal / 100.0, 'f', 2));
}

void SettingsDialog::onSave()
{
    InitFile &cfg = InitFile::instance();

    cfg.setDbConfig(
        ui->txtDbHost->text(),
        ui->txtDbPort->value(),
        ui->txtDbName->text(),
        ui->txtDbUsername->text(),
        ui->txtDbPassword->text()
    );

    cfg.setParkingConfig(
        ui->txtParkingName->text(),
        ui->txtPrice->value(),
        ui->txtCapacity->value(),
        ui->txtFreeMinutes->value()
    );

    cfg.setRecognitionConfig(
        cfg.getModelPath(),
        ui->sliderConfidence->value() / 100.0
    );

    if (!cfg.saveConfig()) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("配置保存失败"));
        return;
    }

    bool dbChanged =
        ui->txtDbHost->text()     != m_oldHost     ||
        ui->txtDbPort->value()    != m_oldPort     ||
        ui->txtDbName->text()     != m_oldDbName   ||
        ui->txtDbUsername->text() != m_oldUsername ||
        ui->txtDbPassword->text() != m_oldPassword;

    if (dbChanged) {
        emit dbConfigChanged();
        QMessageBox::information(this, QStringLiteral("提示"),
            QStringLiteral("数据库配置已保存，重启应用后生效"));
    }

    emit settingsSaved();
    accept();
}

void SettingsDialog::onTestConnection()
{
    const QString testConn = "settings_test_connection";

    if (QSqlDatabase::contains(testConn))
        QSqlDatabase::removeDatabase(testConn);

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", testConn);
    db.setHostName(ui->txtDbHost->text());
    db.setPort(ui->txtDbPort->value());
    db.setDatabaseName(ui->txtDbName->text());
    db.setUserName(ui->txtDbUsername->text());
    db.setPassword(ui->txtDbPassword->text());

    if (db.open()) {
        QMessageBox::information(this, QStringLiteral("成功"), QStringLiteral("数据库连接成功"));
        db.close();
    } else {
        QMessageBox::warning(this, QStringLiteral("失败"),
            QStringLiteral("连接失败：") + db.lastError().text());
    }

    QSqlDatabase::removeDatabase(testConn);
}
