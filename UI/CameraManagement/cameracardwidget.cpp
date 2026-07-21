#include "cameracardwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QIcon>
#include <QPixmap>

CameraCardWidget::CameraCardWidget(int index, QWidget *parent)
    : QWidget(parent)
    , m_cameraIndex(index)
{
    setObjectName("cameraCard");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 12, 16, 14);
    root->setSpacing(10);

    // ── Header ──
    auto *header = new QHBoxLayout;
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(6);
    auto *iconLabel = new QLabel;
    QPixmap iconPix(":/icons/videocam");
    iconLabel->setPixmap(iconPix.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setFixedSize(18, 18);
    auto *title = new QLabel(QString("摄像头 %1").arg(index + 1));
    title->setObjectName("cameraCardLabel");
    header->addWidget(iconLabel);
    header->addWidget(title);
    header->addStretch();
    root->addLayout(header);

    // ── Form grid: 2 columns, 2 rows ──
    auto *grid = new QGridLayout;
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(10);

    // Row 0 ─ 名称 / 位置
    m_nameEdit = new QLineEdit;
    m_nameEdit->setObjectName("cameraInput");
    m_nameEdit->setFixedHeight(32);

    m_locationEdit = new QLineEdit;
    m_locationEdit->setObjectName("cameraInput");
    m_locationEdit->setFixedHeight(32);

    auto makeLabel = [](const QString &text) -> QLabel* {
        auto *l = new QLabel(text);
        l->setObjectName("cameraInputLabel");
        return l;
    };

    auto wrapV = [&](QLabel *label, QWidget *ctrl) -> QWidget* {
        auto *w = new QWidget;
        auto *lay = new QVBoxLayout(w);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(4);
        lay->addWidget(label);
        lay->addWidget(ctrl);
        return w;
    };

    grid->addWidget(wrapV(makeLabel(QStringLiteral("名称")), m_nameEdit), 0, 0);
    grid->addWidget(wrapV(makeLabel(QStringLiteral("位置")), m_locationEdit), 0, 1);

    // Row 1 ─ 角色 / 分辨率·帧率
    auto *group = new QButtonGroup(this);
    m_entryRadio = new QRadioButton(QStringLiteral("入口"));
    m_monitorRadio = new QRadioButton(QStringLiteral("监控"));
    m_entryRadio->setObjectName("cameraRadio");
    m_monitorRadio->setObjectName("cameraRadio");
    group->addButton(m_entryRadio);
    group->addButton(m_monitorRadio);
    m_entryRadio->setChecked(true);

    auto *roleContainer = new QWidget;
    auto *roleRow = new QHBoxLayout(roleContainer);
    roleRow->setContentsMargins(0, 0, 0, 0);
    roleRow->setSpacing(16);
    roleRow->addWidget(m_entryRadio);
    roleRow->addWidget(m_monitorRadio);
    roleRow->addStretch();

    grid->addWidget(wrapV(makeLabel(QStringLiteral("角色")), roleContainer), 1, 0);

    m_widthCombo = new QComboBox;
    m_widthCombo->setObjectName("cameraCombo");
    m_widthCombo->setEditable(true);
    m_widthCombo->addItems({"480", "640", "800", "1024", "1280", "1920", "2560"});
    m_widthCombo->setFixedHeight(32);

    m_heightCombo = new QComboBox;
    m_heightCombo->setObjectName("cameraCombo");
    m_heightCombo->setEditable(true);
    m_heightCombo->addItems({"480", "640", "800", "1024", "1280", "1920", "2560"});
    m_heightCombo->setFixedHeight(32);

    m_fpsCombo = new QComboBox;
    m_fpsCombo->setObjectName("cameraCombo");
    m_fpsCombo->setEditable(true);
    m_fpsCombo->addItems({"15", "20", "25", "30"});
    m_fpsCombo->setFixedHeight(32);
    m_fpsCombo->setMinimumWidth(72);

    auto *resContainer = new QWidget;
    auto *resRow = new QHBoxLayout(resContainer);
    resRow->setContentsMargins(0, 0, 0, 0);
    resRow->setSpacing(8);
    resRow->addWidget(m_widthCombo, 1);
    resRow->addWidget(m_heightCombo, 1);
    resRow->addWidget(m_fpsCombo, 0);

    grid->addWidget(wrapV(makeLabel(QStringLiteral("分辨率/帧率")), resContainer), 1, 1);

    // 两列等宽
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    root->addLayout(grid);

    // 转发所有控件的变更信号，供父级追踪脏状态
    connect(m_nameEdit, &QLineEdit::textChanged, this, &CameraCardWidget::changed);
    connect(m_locationEdit, &QLineEdit::textChanged, this, &CameraCardWidget::changed);
    connect(m_entryRadio, &QRadioButton::toggled, this, &CameraCardWidget::changed);
    connect(m_widthCombo, &QComboBox::currentTextChanged, this, &CameraCardWidget::changed);
    connect(m_heightCombo, &QComboBox::currentTextChanged, this, &CameraCardWidget::changed);
    connect(m_fpsCombo, &QComboBox::currentTextChanged, this, &CameraCardWidget::changed);
}

void CameraCardWidget::setCameraInfo(const CameraInfo &info)
{
    m_nameEdit->setText(info.name);
    m_locationEdit->setText(info.location);
    if (info.role == "monitor")
        m_monitorRadio->setChecked(true);
    else
        m_entryRadio->setChecked(true);

    m_widthCombo->setCurrentText(QString::number(info.width));
    m_heightCombo->setCurrentText(QString::number(info.height));
    m_fpsCombo->setCurrentText(QString::number(info.fps));
}

CameraInfo CameraCardWidget::cameraInfo() const
{
    CameraInfo info;
    info.index = m_cameraIndex;
    info.name = m_nameEdit->text();
    info.location = m_locationEdit->text();
    info.role = m_entryRadio->isChecked() ? "entry" : "monitor";

    info.width = m_widthCombo->currentText().toInt();
    info.height = m_heightCombo->currentText().toInt();
    info.fps = m_fpsCombo->currentText().toInt();
    return info;
}
