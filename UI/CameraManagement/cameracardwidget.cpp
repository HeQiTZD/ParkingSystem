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
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);

    auto *header = new QHBoxLayout;
    header->setSpacing(6);
    auto *iconLabel = new QLabel;
    QPixmap iconPix(":/icons/videocam");
    iconLabel->setPixmap(iconPix.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setFixedSize(18, 18);
    auto *title = new QLabel(QString("摄像头 %1").arg(index + 1));
    title->setStyleSheet("font-weight: 600; font-size: 15px;");
    header->addWidget(iconLabel);
    header->addWidget(title);
    header->addStretch();
    mainLayout->addLayout(header);

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(24);
    grid->setVerticalSpacing(12);

    auto makeField = [](const QString &labelText, QWidget *input) -> QWidget* {
        auto *container = new QWidget;
        auto *layout = new QVBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(4);
        auto *label = new QLabel(labelText);
        label->setStyleSheet("font-size: 11px; color: #94A3B8; letter-spacing: 0.05em;");
        layout->addWidget(label);
        layout->addWidget(input);
        return container;
    };

    m_nameEdit = new QLineEdit;
    m_nameEdit->setFixedHeight(32);
    m_nameEdit->setStyleSheet(
        "QLineEdit { background: #0F172A; border: 1px solid #434655;"
        "  border-radius: 4px; padding: 0 8px; font-size: 13px; color: #d8e3fb; }"
        "QLineEdit:focus { border-color: #b4c5ff; }"
    );
    grid->addWidget(makeField(QStringLiteral("名称"), m_nameEdit), 0, 0);

    m_locationEdit = new QLineEdit;
    m_locationEdit->setFixedHeight(32);
    m_locationEdit->setStyleSheet(m_nameEdit->styleSheet());
    grid->addWidget(makeField(QStringLiteral("位置"), m_locationEdit), 0, 1);

    auto *roleContainer = new QWidget;
    auto *roleLayout = new QHBoxLayout(roleContainer);
    roleLayout->setContentsMargins(0, 0, 0, 0);
    roleLayout->setSpacing(12);

    auto *group = new QButtonGroup(this);
    m_entryRadio = new QRadioButton(QStringLiteral("入口"));
    m_monitorRadio = new QRadioButton(QStringLiteral("监控"));
    group->addButton(m_entryRadio);
    group->addButton(m_monitorRadio);
    m_entryRadio->setChecked(true);
    roleLayout->addWidget(m_entryRadio);
    roleLayout->addWidget(m_monitorRadio);
    roleLayout->addStretch();

    auto *roleField = new QWidget;
    auto *roleFieldLayout = new QVBoxLayout(roleField);
    roleFieldLayout->setContentsMargins(0, 0, 0, 0);
    roleFieldLayout->setSpacing(4);
    auto *roleLabel = new QLabel(QStringLiteral("角色"));
    roleLabel->setStyleSheet("font-size: 11px; color: #94A3B8; letter-spacing: 0.05em;");
    roleFieldLayout->addWidget(roleLabel);
    roleFieldLayout->addWidget(roleContainer);
    grid->addWidget(roleField, 1, 0);

    m_resolutionCombo = new QComboBox;
    m_resolutionCombo->addItems({"1080P", "720P"});
    m_resolutionCombo->setFixedHeight(32);
    m_resolutionCombo->setStyleSheet(
        "QComboBox { background: #0F172A; border: 1px solid #434655;"
        "  border-radius: 4px; padding: 0 4px; font-size: 11px; color: #d8e3fb; }"
    );

    m_fpsCombo = new QComboBox;
    m_fpsCombo->addItems({"15", "20", "25", "30"});
    m_fpsCombo->setFixedHeight(32);
    m_fpsCombo->setFixedWidth(64);
    m_fpsCombo->setStyleSheet(m_resolutionCombo->styleSheet());

    auto *resContainer = new QWidget;
    auto *resLayout = new QHBoxLayout(resContainer);
    resLayout->setContentsMargins(0, 0, 0, 0);
    resLayout->setSpacing(6);
    resLayout->addWidget(m_resolutionCombo);
    resLayout->addWidget(m_fpsCombo);

    auto *resField = new QWidget;
    auto *resFieldLayout = new QVBoxLayout(resField);
    resFieldLayout->setContentsMargins(0, 0, 0, 0);
    resFieldLayout->setSpacing(4);
    auto *resLabel = new QLabel(QStringLiteral("分辨率/帧率"));
    resLabel->setStyleSheet("font-size: 11px; color: #94A3B8; letter-spacing: 0.05em;");
    resFieldLayout->addWidget(resLabel);
    resFieldLayout->addWidget(resContainer);
    grid->addWidget(resField, 1, 1);

    mainLayout->addLayout(grid);
}

void CameraCardWidget::setCameraInfo(const CameraInfo &info)
{
    m_nameEdit->setText(info.name);
    m_locationEdit->setText(info.location);
    if (info.role == "monitor")
        m_monitorRadio->setChecked(true);
    else
        m_entryRadio->setChecked(true);

    QString resStr = QString("%1P").arg(info.height);
    int resIdx = m_resolutionCombo->findText(resStr);
    if (resIdx >= 0) m_resolutionCombo->setCurrentIndex(resIdx);

    int fpsIdx = m_fpsCombo->findText(QString::number(info.fps));
    if (fpsIdx >= 0) m_fpsCombo->setCurrentIndex(fpsIdx);
}

CameraInfo CameraCardWidget::cameraInfo() const
{
    CameraInfo info;
    info.index = m_cameraIndex;
    info.name = m_nameEdit->text();
    info.location = m_locationEdit->text();
    info.role = m_entryRadio->isChecked() ? "entry" : "monitor";

    QString resText = m_resolutionCombo->currentText();
    if (resText == "1080P") { info.width = 1920; info.height = 1080; }
    else if (resText == "720P") { info.width = 1280; info.height = 720; }

    info.fps = m_fpsCombo->currentText().toInt();
    return info;
}
