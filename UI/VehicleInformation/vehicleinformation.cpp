#include "vehicleinformation.h"
#include "ui_vehicleinformation.h"
#include "src/utils/iconlineedit.h"
#include "src/utils/datelineedit.h"
#include "src/utils/customdatechooser.h"
#include "src/service/vehicleservice.h"
#include "src/utils/paginationwidget.h"
#include "src/utils/notification_global.h"
#include <QFile>
#include <QDialog>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

void VehicleInformation::setupTable()
{
    m_tableWidget = new QTableWidget(this);

    QStringList headers;
    headers << "序号" << "车牌号" << "进场时间"
            << "出场时间" << "停留时长" << "收费金额";

    m_tableWidget->setColumnCount(headers.size());
    m_tableWidget->setHorizontalHeaderLabels(headers);

    m_tableWidget->setColumnWidth(0, 60);
    m_tableWidget->setColumnWidth(1, 120);
    m_tableWidget->setColumnWidth(2, 160);
    m_tableWidget->setColumnWidth(3, 160);
    m_tableWidget->setColumnWidth(4, 100);

    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setFocusPolicy(Qt::NoFocus);
    m_tableWidget->verticalHeader()->setDefaultSectionSize(44);

    // ── 分页栏 + 每页条数选择 ──
    m_pagination = new PaginationWidget(this);
    m_pagination->setPageSize(10);

    m_pageSizeCombo = new QComboBox(this);
    m_pageSizeCombo->setObjectName("pageSizeCombo");
    m_pageSizeCombo->addItems({"10", "20", "50", "100"});
    m_pageSizeCombo->setCurrentIndex(0);
    m_pageSizeCombo->setFixedWidth(70);

    QLabel *pageSizeLabel = new QLabel("每页显示", this);
    pageSizeLabel->setStyleSheet("color: #737686; font-size: 13px;");

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setContentsMargins(16, 0, 0, 0);
    bottomLayout->addWidget(pageSizeLabel);
    bottomLayout->addWidget(m_pageSizeCombo);
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(m_pagination);

    // ── 替换 contentWidget 布局：表格在上，底部栏在下 ──
    QLayout *oldLayout = ui->contentWidget->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (item->widget())
                item->widget()->setParent(nullptr);
            delete item;
        }
        delete oldLayout;
    }

    QVBoxLayout *newLayout = new QVBoxLayout(ui->contentWidget);
    newLayout->setContentsMargins(0, 0, 0, 0);
    newLayout->setSpacing(0);
    newLayout->addWidget(m_tableWidget, 1);
    newLayout->addLayout(bottomLayout, 0);
}

// ─── 从 m_allData 按当前页切片填入表格 ────────────────────────
void VehicleInformation::populateTable()
{
    m_tableWidget->setRowCount(0);

    if (m_allData.isEmpty()) return;

    // 计算当前页的数据范围
    int page     = m_pagination->currentPage();
    int pageSize = m_pagination->pageSize();
    int start    = (page - 1) * pageSize;
    int end      = qMin(start + pageSize, m_allData.size());

    m_tableWidget->setRowCount(end - start);

    for (int row = start; row < end; ++row) {
        const QVariantList &rec = m_allData[row];
        int displayRow = row - start;  // 表格内行号从 0 开始

        // rec 的字段顺序由 searchCars() 决定（databasemanager.cpp:441-449）：
        //   [0] id              → int
        //   [1] license_plate   → QString
        //   [2] check_in_time   → QDateTime
        //   [3] check_out_time  → QDateTime (NULL 时无效)
        //   [4] fee             → double (NULL 时无效)
        //   [5] parking_minutes → int
        //   [6] status_text     → QString ("在场" / "已离场")

        // 列 0：序号（从 1 开始）
        QTableWidgetItem *indexItem = new QTableWidgetItem(QString::number(row + 1));
        indexItem->setTextAlignment(Qt::AlignCenter);
        indexItem->setForeground(QColor("#737686"));
        indexItem->setData(Qt::UserRole, rec[0].toInt());
        m_tableWidget->setItem(displayRow, 0, indexItem);

        QTableWidgetItem *plateItem = new QTableWidgetItem(rec[1].toString());
        plateItem->setForeground(QColor("#003FB1"));
        QFont boldFont = plateItem->font();
        boldFont.setBold(true);
        plateItem->setFont(boldFont);
        m_tableWidget->setItem(displayRow, 1, plateItem);

        QDateTime checkIn = rec[2].toDateTime();
        QTableWidgetItem *entryItem = new QTableWidgetItem(
            checkIn.toString("yyyy-MM-dd HH:mm:ss"));
        entryItem->setForeground(QColor("#191b23"));
        m_tableWidget->setItem(displayRow, 2, entryItem);

        QDateTime checkOut = rec[3].toDateTime();
        QString exitText = checkOut.isValid()
            ? checkOut.toString("yyyy-MM-dd HH:mm:ss")
            : QString("--");
        QTableWidgetItem *exitItem = new QTableWidgetItem(exitText);
        exitItem->setForeground(QColor("#191b23"));
        m_tableWidget->setItem(displayRow, 3, exitItem);

        int minutes = rec[5].toInt();
        QString durationText = (rec[6].toString() == "在场")
            ? QString("在场")
            : QString("%1h %2m").arg(minutes / 60).arg(minutes % 60);
        QTableWidgetItem *durationItem = new QTableWidgetItem(durationText);
        durationItem->setForeground(
            rec[6].toString() == "在场" ? QColor("#059669") : QColor("#191b23"));
        m_tableWidget->setItem(displayRow, 4, durationItem);

        QString feeText;
        if (!checkOut.isValid()) {
            feeText = "--";
        } else {
            feeText = QString("¥ %1").arg(rec[4].toDouble(), 0, 'f', 2);
        }
        QTableWidgetItem *feeItem = new QTableWidgetItem(feeText);
        feeItem->setForeground(QColor("#191b23"));
        feeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableWidget->setItem(displayRow, 5, feeItem);
    }
}

// ─── 查询按钮的槽函数 ────────────────────────────────────────
void VehicleInformation::onQueryClicked()
{
    QString plate = ui->plateLineEdit->text().trimmed();
    plate.remove(QChar(0xB7));
    plate.remove(' ');
    plate.remove('-');

    QDateTime startTime, endTime;
    QString startStr = ui->startTimeLineEdit->text().trimmed();
    QString endStr   = ui->endTimeLineEdit->text().trimmed();
    if (!startStr.isEmpty())
        startTime = QDateTime::fromString(startStr, "yyyy-MM-dd HH:mm:ss");
    if (!endStr.isEmpty())
        endTime = QDateTime::fromString(endStr, "yyyy-MM-dd HH:mm:ss");

    // 执行查询，缓存全量结果
    m_allData = m_vehicleSvc->search(plate, startTime, endTime, 0);

    // 更新分页控件 → 自动回到第1页 → 触发 pageChanged 信号
    m_pagination->setTotalRecords(m_allData.size());
}

void VehicleInformation::onDeleteClicked()
{
    QList<QTableWidgetItem *> selected = m_tableWidget->selectedItems();
    if (selected.isEmpty()) {
        notifyInfo(this, "请先选择要删除的记录");
        return;
    }

    // 从选中行中提取不重复的 id
    QSet<int> idSet;
    for (QTableWidgetItem *item : selected) {
        int row = item->row();
        idSet.insert(m_tableWidget->item(row, 0)->data(Qt::UserRole).toInt());
    }
    QList<int> ids = QList<int>(idSet.constBegin(), idSet.constEnd());

    if (!notifyConfirm(this, "确认删除",
                       QString("确定要删除选中的 %1 条记录吗？此操作不可撤销。").arg(ids.size())))
        return;

    QString errMsg;
    if (m_vehicleSvc->deleteRecords(ids, errMsg)) {
        notifySuccess(this, QString("成功删除 %1 条记录").arg(ids.size()));
        onQueryClicked();
    } else {
        notifyError(this, "删除失败", "数据库删除操作失败，请重试");
    }
}

void VehicleInformation::onPageChanged(int page)
{
    Q_UNUSED(page);
    populateTable();
}

VehicleInformation::VehicleInformation(QWidget *parent, VehicleService *vehicleSvc)
    : QWidget(parent), ui(new Ui::VehicleInformation), m_vehicleSvc(vehicleSvc)
{
    ui->setupUi(this);

    // 日期输入框：左右双图标（左 calendar_today + 右 date_range）
    ui->startTimeLineEdit->setLeftIcon(QIcon(":/icons/calendar_today.svg"));
    ui->startTimeLineEdit->setRightIcon(QIcon(":/icons/date_range.svg"));
    ui->startTimeLineEdit->setRightIconSize(QSize(14, 14));
    ui->endTimeLineEdit->setLeftIcon(QIcon(":/icons/calendar_today.svg"));
    ui->endTimeLineEdit->setRightIcon(QIcon(":/icons/date_range.svg"));
    ui->endTimeLineEdit->setRightIconSize(QSize(14, 14));

    // 日期输入框：右图标点击弹出 CustomDateChooser 日期时间选择器
    auto setupDatePicker = [this](DateLineEdit *edit) {
        connect(edit, &DateLineEdit::rightIconClicked, this, [this, edit]() {
            QDialog popup(this, Qt::FramelessWindowHint | Qt::Popup);

            auto *chooser = new CustomDateChooser(&popup);
            auto *layout = new QVBoxLayout(&popup);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(chooser);

            QString text = edit->text().trimmed();
            if (!text.isEmpty()) {
                QDateTime dt = QDateTime::fromString(text, "yyyy-MM-dd HH:mm:ss");
                if (dt.isValid())
                    chooser->setSelectedDateTime(dt);
            }

            connect(chooser, &CustomDateChooser::dateTimeConfirmed, this, [edit, &popup](const QDateTime &dt) {
                edit->setText(dt.toString("yyyy-MM-dd HH:mm:ss"));
                popup.accept();
            });

            popup.move(edit->mapToGlobal(QPoint(0, edit->height() + 2)));
            popup.exec();
        });
    };
    setupDatePicker(ui->startTimeLineEdit);
    setupDatePicker(ui->endTimeLineEdit);

    // 车牌输入框：运行时替换为 IconLineEdit（左侧图标）
    auto replaceWithIconEdit = [this](QGridLayout *layout, QLineEdit *oldEdit, const QString &iconPath)->IconLineEdit *
    {
        int idx = layout->indexOf(oldEdit);
        int row, col, rowSpan, colSpan;
        layout->getItemPosition(idx, &row, &col, &rowSpan, &colSpan);

        layout->removeWidget(oldEdit);
        oldEdit->hide();

        IconLineEdit *newEdit = new IconLineEdit(this);
        newEdit->setObjectName(oldEdit->objectName());
        newEdit->setIconPath(iconPath);
        layout->addWidget(newEdit, row, col, rowSpan, colSpan);

        return newEdit;
    };
    replaceWithIconEdit(ui->plateGridLayout, ui->plateLineEdit, ":/icons/directions.svg");


    QFile styleFile(":/styles/vehicleInformation.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }

    setupTable();

    connect(ui->queryPushButton, &QPushButton::clicked,
            this, &VehicleInformation::onQueryClicked);

    connect(m_pagination, &PaginationWidget::pageChanged,
            this, &VehicleInformation::onPageChanged);

    connect(ui->deletePushButton, &QPushButton::clicked,
            this, &VehicleInformation::onDeleteClicked);

    connect(m_pageSizeCombo, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
        m_pagination->setPageSize(text.toInt());
    });

    // 首次加载最近 50 条记录
    m_allData = m_vehicleSvc->recentRecords(50);
    m_pagination->setTotalRecords(m_allData.size());
}

VehicleInformation::~VehicleInformation()
{
    delete ui;
}
