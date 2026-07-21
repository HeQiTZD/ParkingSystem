#include "camerasettingsdialog.h"
#include "ui_camerasettingsdialog.h"
#include "cameracardwidget.h"
#include "src/camera/cameramanager.h"
#include "src/utils/initfile.h"

#include <QButtonGroup>
#include <QCloseEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScrollBar>

CameraSettingsDialog::CameraSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CameraSettingsDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);

    QFile styleFile(":/styles/camerasettings.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }

    ui->pushButton->setIconSize(QSize(16, 16));
    ui->pushButton->setIcon(QIcon(":/icons/close"));

    ui->widget->installEventFilter(this);

    // 取消和关闭按钮：跳过脏检查直接关闭
    auto onCancel = [this]() {
        m_dirty = false;
        reject();
    };
    connect(ui->pushButton, &QPushButton::clicked, this, onCancel);
    connect(ui->pushButton_2, &QPushButton::clicked, this, onCancel);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &CameraSettingsDialog::onSave);

    loadGlobalParams();
    loadCameraCards();
}

CameraSettingsDialog::~CameraSettingsDialog()
{
    delete ui;
}

void CameraSettingsDialog::loadGlobalParams()
{
    auto &ini = InitFile::instance();
    int w = ini.getCameraWidth();
    int h = ini.getCameraHeight();
    int fps = ini.getCameraFps();
    if (w <= 0) w = 1920;
    if (h <= 0) h = 1080;
    if (fps <= 0) fps = 30;

    ui->cmbGlobalWidth->setCurrentText(QString::number(w));
    ui->cmbGlobalHeight->setCurrentText(QString::number(h));
    ui->cmbGlobalFps->setCurrentText(QString::number(fps));
}

void CameraSettingsDialog::loadCameraCards()
{
    qDeleteAll(m_cards);
    m_cards.clear();

    if (m_roleGroup) {
        delete m_roleGroup;
        m_roleGroup = nullptr;
    }

    auto &mgr = CameraManager::instance();
    int count = mgr.count();

    if (count == 0) {
        auto *emptyLabel = new QLabel(QStringLiteral("未检测到摄像头"));
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color: #94a3b8; font-size: 14px; padding: 40px;");
        ui->cameraListLayout->addWidget(emptyLabel);
        return;
    }

    m_roleGroup = new QButtonGroup(this);

    for (int i = 0; i < count; ++i) {
        auto *card = new CameraCardWidget(i, this);
        card->setCameraInfo(mgr.info(i));
        ui->cameraListLayout->addWidget(card);
        m_cards.append(card);

        m_roleGroup->addButton(card->entryRadio());
        m_roleGroup->addButton(card->monitorRadio());

        connect(card, &CameraCardWidget::changed, this, [this]() {
            m_dirty = true;
        });
    }
}

void CameraSettingsDialog::onSave()
{
    // ── 全局参数校验 ──
    bool ok;
    int globalW = ui->cmbGlobalWidth->currentText().toInt(&ok);
    if (!ok || globalW <= 0) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("全局分辨率宽度必须为正整数"));
        return;
    }
    int globalH = ui->cmbGlobalHeight->currentText().toInt(&ok);
    if (!ok || globalH <= 0) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("全局分辨率高度必须为正整数"));
        return;
    }
    int globalFps = ui->cmbGlobalFps->currentText().toInt(&ok);
    if (!ok || globalFps <= 0) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("全局帧率必须为正整数"));
        return;
    }

    // ── 角色互斥校验 ──
    int entryCount = 0;
    for (auto *card : m_cards) {
        if (card->cameraInfo().role == "entry")
            ++entryCount;
    }
    if (entryCount > 1) {
        QMessageBox::warning(this, QStringLiteral("角色冲突"),
                             QStringLiteral("只能设置一个入口摄像头（entry）"));
        return;
    }
    if (entryCount == 0) {
        auto ret = QMessageBox::question(this, QStringLiteral("无入口摄像头"),
                                         QStringLiteral("当前无入口摄像头，车牌识别功能将暂停。确定继续？"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes)
            return;
    }

    // ── 每路摄像头校验 ──
    for (auto *card : m_cards) {
        CameraInfo info = card->cameraInfo();
        if (info.width <= 0 || info.height <= 0 || info.fps <= 0) {
            QMessageBox::warning(this, QStringLiteral("输入错误"),
                                 QStringLiteral("摄像头 %1 的分辨率或帧率必须为正整数").arg(info.index + 1));
            return;
        }
    }

    // ── 写入 InitFile ──
    auto &ini = InitFile::instance();
    ini.setCameraConfig(0, globalW, globalH, globalFps);

    QJsonArray camerasJson;
    for (auto *card : m_cards) {
        CameraInfo info = card->cameraInfo();

        // 名称为空自动回退
        if (info.name.trimmed().isEmpty())
            info.name = QStringLiteral("摄像头 %1").arg(info.index + 1);

        auto &mgr = CameraManager::instance();
        mgr.updateInfo(info.index, info);

        QJsonObject obj;
        obj["index"]    = info.index;
        obj["name"]     = info.name;
        obj["location"] = info.location;
        obj["role"]     = info.role;
        obj["width"]    = info.width;
        obj["height"]   = info.height;
        obj["fps"]      = info.fps;
        camerasJson.append(obj);
    }

    ini.setCameras(camerasJson);

    if (!ini.saveConfig()) {
        QMessageBox::warning(this, QStringLiteral("保存失败"),
                             QStringLiteral("配置保存失败"));
        return;
    }

    m_dirty = false;

    emit camerasUpdated();
    emit cameraConfigChanged();

    QMessageBox::information(this, QStringLiteral("保存成功"),
                             QStringLiteral("摄像头配置已保存，重启应用后生效"));
    accept();
}

void CameraSettingsDialog::closeEvent(QCloseEvent *event)
{
    if (m_dirty) {
        auto ret = QMessageBox::question(
            this,
            QStringLiteral("未保存的修改"),
            QStringLiteral("有未保存的修改，是否放弃？"),
            QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Cancel);
        if (ret != QMessageBox::Discard) {
            event->ignore();
            return;
        }
    }
    QDialog::closeEvent(event);
}

bool CameraSettingsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->widget) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                m_dragPos = me->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->buttons() & Qt::LeftButton) {
                move(me->globalPosition().toPoint() - m_dragPos);
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}
