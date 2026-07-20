#include "camerasettingsdialog.h"
#include "ui_camerasettingsdialog.h"
#include "src/utils/initfile.h"
#include "src/camera/cameramanager.h"
#include "src/camera/camerainfo.h"

#include <QFile>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QFrame>
#include <QScrollArea>

CameraSettingsDialog::CameraSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CameraSettingsDialog)
{
    ui->setupUi(this);
    setupWindow();
    loadSettings();
    buildCameraRows();

    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnSubmit, &QPushButton::clicked, this, &CameraSettingsDialog::onSave);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

CameraSettingsDialog::~CameraSettingsDialog()
{
    delete ui;
}

void CameraSettingsDialog::setupWindow()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QFile styleFile(":/styles/camerasettings.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    ui->mainContainer->setGraphicsEffect(shadow);

    ui->titleBar->installEventFilter(this);
}

bool CameraSettingsDialog::eventFilter(QObject *obj, QEvent *event)
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

void CameraSettingsDialog::loadSettings()
{
    InitFile &cfg = InitFile::instance();

    QString wStr = QString::number(cfg.getCameraWidth());
    int wIdx = ui->cmbGlobalWidth->findText(wStr);
    if (wIdx >= 0) ui->cmbGlobalWidth->setCurrentIndex(wIdx);
    else ui->cmbGlobalWidth->setCurrentText(wStr);

    QString hStr = QString::number(cfg.getCameraHeight());
    int hIdx = ui->cmbGlobalHeight->findText(hStr);
    if (hIdx >= 0) ui->cmbGlobalHeight->setCurrentIndex(hIdx);
    else ui->cmbGlobalHeight->setCurrentText(hStr);

    QString fpsStr = QString::number(cfg.getCameraFps());
    int fpsIdx = ui->cmbGlobalFps->findText(fpsStr);
    if (fpsIdx >= 0) ui->cmbGlobalFps->setCurrentIndex(fpsIdx);
    else ui->cmbGlobalFps->setCurrentText(fpsStr);
}

void CameraSettingsDialog::buildCameraRows()
{
    CameraManager &mgr = CameraManager::instance();
    int n = mgr.count();

    if (n == 0) {
        QLabel *empty = new QLabel(QStringLiteral("未检测到摄像头"), ui->cameraListContainer);
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color:#94A3B8; font-size:13px; padding:20px;");
        ui->cameraListLayout->addWidget(empty);
        return;
    }

    for (int i = 0; i < n; ++i) {
        CameraInfo info = mgr.info(i);
        QWidget *row = createCameraRow(i, info);
        m_cameraRows.append(row);
        ui->cameraListLayout->addWidget(row);
    }
}

QWidget* CameraSettingsDialog::createCameraRow(int index, const CameraInfo &info)
{
    QFrame *frame = new QFrame(ui->cameraListContainer);
    frame->setObjectName(QStringLiteral("camRow_%1").arg(index));

    QVBoxLayout *outer = new QVBoxLayout(frame);
    outer->setContentsMargins(12, 10, 12, 10);
    outer->setSpacing(8);

    // Title
    QLabel *title = new QLabel(QStringLiteral("📷 摄像头 %1").arg(index + 1), frame);
    title->setObjectName(QStringLiteral("cam_title_%1").arg(index));
    outer->addWidget(title);

    // Name
    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *lblName = new QLabel(QStringLiteral("名称"), frame);
    lblName->setObjectName(QStringLiteral("cam_lbl_name_%1").arg(index));
    lblName->setFixedWidth(60);
    QLineEdit *txtName = new QLineEdit(frame);
    txtName->setObjectName(QStringLiteral("cam_name_%1").arg(index));
    txtName->setText(info.name);
    txtName->setPlaceholderText(QStringLiteral("摄像头 %1").arg(index + 1));
    nameLayout->addWidget(lblName);
    nameLayout->addWidget(txtName);
    outer->addLayout(nameLayout);

    // Location
    QHBoxLayout *locLayout = new QHBoxLayout();
    QLabel *lblLoc = new QLabel(QStringLiteral("位置"), frame);
    lblLoc->setObjectName(QStringLiteral("cam_lbl_location_%1").arg(index));
    lblLoc->setFixedWidth(60);
    QLineEdit *txtLocation = new QLineEdit(frame);
    txtLocation->setObjectName(QStringLiteral("cam_location_%1").arg(index));
    txtLocation->setText(info.location);
    txtLocation->setPlaceholderText(QStringLiteral("位置 %1").arg(index + 1));
    locLayout->addWidget(lblLoc);
    locLayout->addWidget(txtLocation);
    outer->addLayout(locLayout);

    // Role
    QHBoxLayout *roleLayout = new QHBoxLayout();
    QLabel *lblRole = new QLabel(QStringLiteral("角色"), frame);
    lblRole->setObjectName(QStringLiteral("cam_lbl_role_%1").arg(index));
    lblRole->setFixedWidth(60);
    QRadioButton *rbEntry = new QRadioButton("entry", frame);
    rbEntry->setObjectName(QStringLiteral("cam_role_entry_%1").arg(index));
    QRadioButton *rbMonitor = new QRadioButton("monitor", frame);
    rbMonitor->setObjectName(QStringLiteral("cam_role_monitor_%1").arg(index));
    if (info.role == "entry")
        rbEntry->setChecked(true);
    else
        rbMonitor->setChecked(true);
    roleLayout->addWidget(lblRole);
    roleLayout->addWidget(rbEntry);
    roleLayout->addWidget(rbMonitor);
    roleLayout->addStretch();
    outer->addLayout(roleLayout);

    // Resolution
    QHBoxLayout *resLayout = new QHBoxLayout();
    QLabel *lblRes = new QLabel(QStringLiteral("分辨率"), frame);
    lblRes->setObjectName(QStringLiteral("cam_lbl_res_%1").arg(index));
    lblRes->setFixedWidth(60);

    QComboBox *cmbWidth = new QComboBox(frame);
    cmbWidth->setObjectName(QStringLiteral("cam_width_%1").arg(index));
    cmbWidth->setEditable(true);
    cmbWidth->addItems({QStringLiteral("480"), QStringLiteral("640"), QStringLiteral("800"),
                        QStringLiteral("1024"), QStringLiteral("1280"), QStringLiteral("1920"),
                        QStringLiteral("2560")});
    QString wStr = QString::number(info.width);
    int wIdx = cmbWidth->findText(wStr);
    if (wIdx >= 0) cmbWidth->setCurrentIndex(wIdx);
    else cmbWidth->setCurrentText(wStr);

    QLabel *lblX = new QLabel(QStringLiteral("×"), frame);
    lblX->setFixedWidth(16);
    lblX->setAlignment(Qt::AlignCenter);

    QComboBox *cmbHeight = new QComboBox(frame);
    cmbHeight->setObjectName(QStringLiteral("cam_height_%1").arg(index));
    cmbHeight->setEditable(true);
    cmbHeight->addItems({QStringLiteral("480"), QStringLiteral("640"), QStringLiteral("800"),
                         QStringLiteral("1024"), QStringLiteral("1280"), QStringLiteral("1920"),
                         QStringLiteral("2560")});
    QString hStr = QString::number(info.height);
    int hIdx = cmbHeight->findText(hStr);
    if (hIdx >= 0) cmbHeight->setCurrentIndex(hIdx);
    else cmbHeight->setCurrentText(hStr);

    resLayout->addWidget(lblRes);
    resLayout->addWidget(cmbWidth);
    resLayout->addWidget(lblX);
    resLayout->addWidget(cmbHeight);
    resLayout->addStretch();
    outer->addLayout(resLayout);

    // FPS
    QHBoxLayout *fpsLayout = new QHBoxLayout();
    QLabel *lblFps = new QLabel(QStringLiteral("帧率"), frame);
    lblFps->setObjectName(QStringLiteral("cam_lbl_fps_%1").arg(index));
    lblFps->setFixedWidth(60);

    QComboBox *cmbFps = new QComboBox(frame);
    cmbFps->setObjectName(QStringLiteral("cam_fps_%1").arg(index));
    cmbFps->setEditable(true);
    cmbFps->addItems({QStringLiteral("15"), QStringLiteral("20"), QStringLiteral("25"), QStringLiteral("30")});
    QString fpsStr = QString::number(info.fps);
    int fpsIdx = cmbFps->findText(fpsStr);
    if (fpsIdx >= 0) cmbFps->setCurrentIndex(fpsIdx);
    else cmbFps->setCurrentText(fpsStr);

    QLabel *lblFpsUnit = new QLabel("FPS", frame);
    lblFpsUnit->setObjectName(QStringLiteral("cam_lbl_fpsunit_%1").arg(index));
    lblFpsUnit->setStyleSheet("color:#94A3B8; font-size:12px;");

    fpsLayout->addWidget(lblFps);
    fpsLayout->addWidget(cmbFps);
    fpsLayout->addWidget(lblFpsUnit);
    fpsLayout->addStretch();
    outer->addLayout(fpsLayout);

    return frame;
}

void CameraSettingsDialog::onSave()
{
    InitFile &cfg = InitFile::instance();
    CameraManager &mgr = CameraManager::instance();
    int n = mgr.count();

    int globalWidth = ui->cmbGlobalWidth->currentText().toInt();
    int globalHeight = ui->cmbGlobalHeight->currentText().toInt();
    int globalFps = ui->cmbGlobalFps->currentText().toInt();

    if (globalWidth <= 0 || globalHeight <= 0 || globalFps <= 0) {
        QMessageBox::warning(this, QStringLiteral("错误"),
            QStringLiteral("全局分辨率或帧率无效，请输入正整数"));
        return;
    }

    // Role exclusivity check
    int entryCount = 0;
    for (int i = 0; i < m_cameraRows.size(); ++i) {
        QRadioButton *rbEntry = m_cameraRows[i]->findChild<QRadioButton*>(
            QStringLiteral("cam_role_entry_%1").arg(i));
        if (rbEntry && rbEntry->isChecked())
            entryCount++;
    }

    if (entryCount > 1) {
        QMessageBox::warning(this, QStringLiteral("错误"),
            QStringLiteral("只能设置一个入口摄像头（entry）"));
        return;
    }
    if (entryCount == 0) {
        auto reply = QMessageBox::question(this, QStringLiteral("警告"),
            QStringLiteral("当前无入口摄像头，车牌识别功能将暂停。确定继续？"));
        if (reply == QMessageBox::No)
            return;
    }

    // Write global config
    cfg.setCameraConfig(0, globalWidth, globalHeight, globalFps);

    // Write per-camera config
    QJsonArray camerasArray;
    for (int i = 0; i < m_cameraRows.size(); ++i) {
        QWidget *row = m_cameraRows[i];

        QJsonObject obj;
        obj["index"] = i;

        QLineEdit *txtName = row->findChild<QLineEdit*>(QStringLiteral("cam_name_%1").arg(i));
        QString name = txtName ? txtName->text().trimmed() : QString();
        obj["name"] = name.isEmpty() ? QStringLiteral("摄像头 %1").arg(i + 1) : name;

        QLineEdit *txtLocation = row->findChild<QLineEdit*>(QStringLiteral("cam_location_%1").arg(i));
        obj["location"] = txtLocation ? txtLocation->text().trimmed() : QString();

        QRadioButton *rbEntry = row->findChild<QRadioButton*>(QStringLiteral("cam_role_entry_%1").arg(i));
        obj["role"] = (rbEntry && rbEntry->isChecked()) ? "entry" : "monitor";

        QComboBox *cmbWidth = row->findChild<QComboBox*>(QStringLiteral("cam_width_%1").arg(i));
        int perWidth = cmbWidth ? cmbWidth->currentText().toInt() : 0;
        if (perWidth <= 0) {
            QMessageBox::warning(this, QStringLiteral("错误"),
                QStringLiteral("摄像头 %1 的分辨率宽度无效，请输入正整数").arg(i + 1));
            return;
        }
        obj["width"] = perWidth;

        QComboBox *cmbHeight = row->findChild<QComboBox*>(QStringLiteral("cam_height_%1").arg(i));
        int perHeight = cmbHeight ? cmbHeight->currentText().toInt() : 0;
        if (perHeight <= 0) {
            QMessageBox::warning(this, QStringLiteral("错误"),
                QStringLiteral("摄像头 %1 的分辨率高度无效，请输入正整数").arg(i + 1));
            return;
        }
        obj["height"] = perHeight;

        QComboBox *cmbFps = row->findChild<QComboBox*>(QStringLiteral("cam_fps_%1").arg(i));
        int perFps = cmbFps ? cmbFps->currentText().toInt() : 0;
        if (perFps <= 0) {
            QMessageBox::warning(this, QStringLiteral("错误"),
                QStringLiteral("摄像头 %1 的帧率无效，请输入正整数").arg(i + 1));
            return;
        }
        obj["fps"] = perFps;

        camerasArray.append(obj);
    }

    cfg.setCameras(camerasArray);

    if (!cfg.saveConfig()) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("配置保存失败"));
        return;
    }

    emit camerasUpdated();
    emit cameraConfigChanged();
    QMessageBox::information(this, QStringLiteral("提示"),
        QStringLiteral("摄像头配置已保存，重启应用后生效"));
    accept();
}
