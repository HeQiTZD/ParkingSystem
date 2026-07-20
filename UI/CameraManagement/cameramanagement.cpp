#include "cameramanagement.h"
#include "ui_cameramanagement.h"
#include "camerawindow.h"
#include "src/camera/cameramanager.h"
#include "src/database/databasemanager.h"
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>

CameraManagement::CameraManagement(QWidget *parent, DatabaseManager *db)
    : QWidget(parent)
    , ui(new Ui::CameraManagement)
    , m_db(db)
{
    ui->setupUi(this);

    QFile styleFile(":/styles/cameramanagement.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }

    setupUI();
    buildCameraWindows();
    updateGridLayout();
    updateCountLabel();

    connect(m_startAllBtn, &QPushButton::clicked, this, [this](){
        CameraManager::instance().startAll();
    });
    connect(m_stopAllBtn, &QPushButton::clicked, this, [this](){
        CameraManager::instance().stopAll();
    });
    connect(m_settingsBtn, &QPushButton::clicked, this, [this](){
        // Placeholder: settings not implemented in this version
    });
}

CameraManagement::~CameraManagement()
{
    delete ui;
}

void CameraManagement::setupUI()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    m_videoContainer = new QFrame(this);
    m_videoContainer->setObjectName("videoContainer");
    m_gridLayout = new QGridLayout(m_videoContainer);
    m_gridLayout->setContentsMargins(8, 8, 8, 8);
    m_gridLayout->setSpacing(12);

    m_emptyLabel = new QLabel(m_videoContainer);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color:#94A3B8; font-size:14px; letter-spacing:2px;");
    m_emptyLabel->setText(QStringLiteral(
        "<div style='text-align:center;'>"
        "<img src=':/icons/videocam_off.svg' width='64' height='64'/><br><br>"
        "<span>No Cameras Detected</span>"
        "</div>"));
    m_emptyLabel->hide();

    root->addWidget(m_videoContainer, 1);

    // Bottom control bar
    QFrame *controlBar = new QFrame(this);
    controlBar->setObjectName("controlBar");
    controlBar->setFixedHeight(64);
    QHBoxLayout *ctrlLayout = new QHBoxLayout(controlBar);
    ctrlLayout->setContentsMargins(20, 12, 20, 12);
    ctrlLayout->setSpacing(12);

    m_countLabel = new QLabel(controlBar);
    m_countLabel->setObjectName("countLabel");
    ctrlLayout->addWidget(m_countLabel);
    ctrlLayout->addStretch();

    m_startAllBtn = new QPushButton(QStringLiteral("▶ 开启所有"), controlBar);
    m_startAllBtn->setObjectName("startAllBtn");
    ctrlLayout->addWidget(m_startAllBtn);

    m_stopAllBtn = new QPushButton(QStringLiteral("⏻ 停止所有"), controlBar);
    m_stopAllBtn->setObjectName("stopAllBtn");
    ctrlLayout->addWidget(m_stopAllBtn);

    m_settingsBtn = new QPushButton(QStringLiteral("⚙ 设置"), controlBar);
    m_settingsBtn->setObjectName("settingsBtn");
    ctrlLayout->addWidget(m_settingsBtn);

    root->addWidget(controlBar);
}

void CameraManagement::buildCameraWindows()
{
    CameraManager &mgr = CameraManager::instance();
    for(int i = 0; i < mgr.count(); ++i){
        auto *win = new CameraWindow(m_videoContainer);
        win->bind(mgr.getThread(i), mgr.info(i));
        connect(win, &CameraWindow::doubleClicked, this, [this, i](){
            if(m_soloMode && m_soloIndex == i){
                m_soloMode = false;
                m_soloIndex = -1;
            }else{
                m_soloMode = true;
                m_soloIndex = i;
            }
            updateGridLayout();
        });
        m_windows.append(win);
    }
}

int CameraManagement::gridCols(int count) const
{
    switch(count){
        case 1: return 1;
        case 2: return 2;
        case 3: case 4: return 2;
        case 5: case 6: return 3;
        case 7: case 8: return 3;
        default: return 1;
    }
}

void CameraManagement::updateGridLayout()
{
    // Clear grid
    while(QLayoutItem *item = m_gridLayout->takeAt(0)){
        delete item; // QLayoutItem takes ownership, widget stays with parent
    }

    int count = m_windows.size();
    if(count == 0){
        m_emptyLabel->setGeometry(m_videoContainer->rect());
        m_emptyLabel->show();
        m_emptyLabel->raise();
        return;
    }
    m_emptyLabel->hide();

    // Solo mode: show only one
    if(m_soloMode && m_soloIndex >= 0 && m_soloIndex < count){
        for(int i = 0; i < count; ++i) m_windows[i]->hide();
        m_gridLayout->addWidget(m_windows[m_soloIndex], 0, 0);
        m_windows[m_soloIndex]->show();
        return;
    }

    // Multi-grid
    int cols = gridCols(count);
    for(int i = 0; i < count; ++i){
        int row = i / cols;
        int col = i % cols;
        m_gridLayout->addWidget(m_windows[i], row, col);
        m_windows[i]->show();
    }
}

void CameraManagement::updateCountLabel()
{
    CameraManager &mgr = CameraManager::instance();
    if(mgr.count() == 0){
        m_countLabel->setText(QStringLiteral("0 cameras detected"));
        return;
    }
    CameraInfo first = mgr.info(0);
    m_countLabel->setText(QStringLiteral("%1 cameras · %2x%3 · %4 FPS")
                               .arg(mgr.count())
                               .arg(first.width).arg(first.height)
                               .arg(first.fps));
}

void CameraManagement::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if(m_windows.isEmpty()){
        m_emptyLabel->setGeometry(m_videoContainer->rect());
    }
}
