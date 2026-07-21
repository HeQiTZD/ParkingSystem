#include "camerasettingsdialog.h"
#include "ui_camerasettingsdialog.h"
#include "cameracardwidget.h"
#include "src/camera/cameramanager.h"

#include <QFile>
#include <QScrollBar>

CameraSettingsDialog::CameraSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CameraSettingsDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    ui->pushButton->setIconSize(QSize(16, 16));
    ui->pushButton->setIcon(QIcon(":/icons/close"));

    QFile styleFile(":/styles/camerasettings.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        setStyleSheet(styleSheet);
        styleFile.close();
    }

    connect(ui->pushButton, &QPushButton::clicked, this, &QDialog::close);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &CameraSettingsDialog::onSave);

    loadCameraCards();
}

CameraSettingsDialog::~CameraSettingsDialog()
{
    delete ui;
}

void CameraSettingsDialog::loadCameraCards()
{
    auto &mgr = CameraManager::instance();
    int count = mgr.count();

    for (int i = 0; i < count; ++i) {
        auto *card = new CameraCardWidget(i, this);
        card->setCameraInfo(mgr.info(i));
        ui->cameraListLayout->addWidget(card);
        m_cards.append(card);
    }
}

void CameraSettingsDialog::onSave()
{
    auto &mgr = CameraManager::instance();
    for (auto *card : m_cards) {
        CameraInfo info = card->cameraInfo();
        // write back to manager – placeholder for real API
    }
    accept();
}
