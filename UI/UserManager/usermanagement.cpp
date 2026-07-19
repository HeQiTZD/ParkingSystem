#include "usermanagement.h"
#include "ui_usermanagement.h"
#include "src/database/databasemanager.h"
#include "src/utils/paginationwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QFile>
#include <QGraphicsDropShadowEffect>

void UserManagement::setupTable()
{
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setObjectName("dataTable");

    QStringList headers;
    headers << "序号" << "用户名" << "姓名" << "手机号" << "操作";
    m_tableWidget->setColumnCount(headers.size());
    m_tableWidget->setHorizontalHeaderLabels(headers);

    m_tableWidget->setColumnWidth(0, 96);
    m_tableWidget->setColumnWidth(4, 192);

    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);

    m_tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setFocusPolicy(Qt::NoFocus);
    m_tableWidget->verticalHeader()->setDefaultSectionSize(56);

    m_pagination = new PaginationWidget(this);
    m_pagination->setPageSize(10);

    m_emptyStateWidget = new QWidget(this);
    m_emptyStateWidget->setObjectName("emptyStateWidget");
    QVBoxLayout *emptyLayout = new QVBoxLayout(m_emptyStateWidget);
    emptyLayout->setAlignment(Qt::AlignCenter);
    QLabel *emptyIcon = new QLabel("\U0001F4CB", m_emptyStateWidget);
    emptyIcon->setStyleSheet("font-size: 48px;");
    emptyIcon->setAlignment(Qt::AlignCenter);
    QLabel *emptyText = new QLabel("暂无用户数据", m_emptyStateWidget);
    emptyText->setObjectName("emptyText");
    emptyText->setAlignment(Qt::AlignCenter);
    QLabel *emptyHint = new QLabel("点击上方「注册用户」添加第一个用户", m_emptyStateWidget);
    emptyHint->setObjectName("emptyHint");
    emptyHint->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyIcon);
    emptyLayout->addWidget(emptyText);
    emptyLayout->addWidget(emptyHint);

    QWidget *bottomBar = new QWidget(this);
    bottomBar->setObjectName("bottomBar");
    bottomBar->setFixedHeight(48);
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(20, 0, 12, 0);
    m_recordCountLabel = new QLabel("共 0 条记录", bottomBar);
    m_recordCountLabel->setObjectName("recordCountLabel");
    bottomLayout->addWidget(m_recordCountLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_pagination);

    QVBoxLayout *cardLayout = new QVBoxLayout;
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);
    cardLayout->addWidget(m_tableWidget, 1);
    cardLayout->addWidget(m_emptyStateWidget, 1);
    cardLayout->addWidget(bottomBar);

    QWidget *cardWidget = new QWidget(this);
    cardWidget->setObjectName("tableCard");
    cardWidget->setLayout(cardLayout);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(cardWidget);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(0, 0, 0, 40));
    cardWidget->setGraphicsEffect(shadow);

    ui->contentLayout->addWidget(cardWidget, 1);
}

void UserManagement::populateTable()
{
    m_tableWidget->setRowCount(0);

    if (m_allData.isEmpty()) {
        m_tableWidget->hide();
        m_emptyStateWidget->show();
        m_pagination->setTotalRecords(0);
        m_recordCountLabel->setText("共 0 条记录");
        return;
    }

    m_emptyStateWidget->hide();
    m_tableWidget->show();
    m_recordCountLabel->setText(QString("共 %1 条记录").arg(m_allData.size()));

    int page = m_pagination->currentPage();
    int pageSize = m_pagination->pageSize();
    int start = (page - 1) * pageSize;
    int end = qMin(start + pageSize, m_allData.size());

    m_tableWidget->setRowCount(end - start);

    for (int row = start; row < end; ++row) {
        const QVariantList &rec = m_allData[row];
        int displayRow = row - start;
        int userId = rec[0].toInt();
        QString username = rec[1].toString();
        QString truename = rec[2].toString();
        QString phone = rec[3].toString();

        QTableWidgetItem *indexItem = new QTableWidgetItem(QString::number(row + 1));
        indexItem->setTextAlignment(Qt::AlignCenter);
        indexItem->setData(Qt::UserRole, userId);
        m_tableWidget->setItem(displayRow, 0, indexItem);

        QTableWidgetItem *usernameItem = new QTableWidgetItem(username);
        QFont bf = usernameItem->font();
        bf.setBold(true);
        usernameItem->setFont(bf);
        m_tableWidget->setItem(displayRow, 1, usernameItem);

        QTableWidgetItem *nameItem = new QTableWidgetItem(truename.isEmpty() ? "--" : truename);
        m_tableWidget->setItem(displayRow, 2, nameItem);

        QTableWidgetItem *phoneItem = new QTableWidgetItem(formatPhone(phone));
        phoneItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_tableWidget->setItem(displayRow, 3, phoneItem);

        QWidget *actionWidget = new QWidget(m_tableWidget);
        actionWidget->setObjectName("actionWidget");
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 0, 4, 0);
        actionLayout->setSpacing(8);

        QPushButton *editBtn = new QPushButton(actionWidget);
        editBtn->setObjectName("editButton");
        editBtn->setText("修改");
        editBtn->setCursor(Qt::PointingHandCursor);
        editBtn->setStyleSheet(
            "QPushButton { color: #2563EB; background: transparent; border: none; font-size: 13px; font-weight: 500; padding: 4px 8px; border-radius: 4px; }"
            "QPushButton:hover { background: #DBEAFE; color: #1D4ED8; }"
            "QPushButton:pressed { background: #BFDBFE; }"
        );

        QPushButton *deleteBtn = new QPushButton(actionWidget);
        deleteBtn->setObjectName("deleteButton");
        deleteBtn->setText("删除");
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setStyleSheet(
            "QPushButton { color: #DC2626; background: transparent; border: none; font-size: 13px; font-weight: 500; padding: 4px 8px; border-radius: 4px; }"
            "QPushButton:hover { background: #FEE2E2; color: #B91C1C; }"
            "QPushButton:pressed { background: #FECACA; }"
        );

        actionLayout->addStretch();
        actionLayout->addWidget(editBtn);
        actionLayout->addWidget(deleteBtn);

        m_tableWidget->setCellWidget(displayRow, 4, actionWidget);
    }
}

void UserManagement::showEmptyState()
{
    m_tableWidget->hide();
    m_emptyStateWidget->show();
}

void UserManagement::hideEmptyState()
{
    m_emptyStateWidget->hide();
    m_tableWidget->show();
}

QString UserManagement::formatPhone(const QString &phone)
{
    if (phone.length() == 11) {
        return phone.left(3) + "-" + phone.mid(3, 4) + "-" + phone.right(4);
    }
    return phone;
}

void UserManagement::onSearchClicked()
{
    QString keyword = ui->searchEdit->text().trimmed();
    m_allData = m_db ? m_db->searchUsers(keyword) : QList<QVariantList>();
    m_pagination->setTotalRecords(m_allData.size());
}

void UserManagement::onPageChanged(int page)
{
    Q_UNUSED(page);
    populateTable();
}

UserManagement::UserManagement(QWidget *parent, DatabaseManager *db)
    : QWidget(parent)
    , ui(new Ui::UserManagement)
    , m_db(db)
{
    ui->setupUi(this);

    setupTable();

    connect(ui->searchButton, &QPushButton::clicked,
            this, &UserManagement::onSearchClicked);
    connect(ui->searchEdit, &QLineEdit::returnPressed,
            this, &UserManagement::onSearchClicked);
    connect(m_pagination, &PaginationWidget::pageChanged,
            this, &UserManagement::onPageChanged);

    QFile styleFile(":/styles/userManagement.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }

    if (m_db) {
        m_allData = m_db->searchUsers();
        m_pagination->setTotalRecords(m_allData.size());
        populateTable();
    }
}

UserManagement::~UserManagement()
{
    delete ui;
}
