#include "usermanagement.h"
#include "ui_usermanagement.h"
#include "UI/Register/registerdialog.h"
#include "src/service/userservice.h"
#include "src/utils/paginationwidget.h"
#include "src/utils/notification_global.h"
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
UserManagement::UserManagement(QWidget *parent, UserService *userSvc)
    : QWidget(parent)
    , ui(new Ui::UserManagement)
    , m_userSvc(userSvc)
{
    ui->setupUi(this);

    QFile styleFile(":/styles/userManagement.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }

    ui->searchEdit->setPlaceholderText("搜索用户名/姓名/手机号");

    setupTable();

    connect(ui->searchButton, &QPushButton::clicked,
            this, &UserManagement::onSearchClicked);
    connect(ui->registerButton, &QPushButton::clicked,
            this, &UserManagement::onRegisterClicked);
    connect(m_pagination, &PaginationWidget::pageChanged,
            this, &UserManagement::onPageChanged);
    connect(m_pageSizeCombo, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
        m_pagination->setPageSize(text.toInt());
    });

    connect(ui->searchEdit, &QLineEdit::returnPressed,
            this, &UserManagement::onSearchClicked);

    if (m_userSvc) {
        m_allUsers = m_userSvc->listUsers("");
        m_pagination->setTotalRecords(m_allUsers.size());
    }
}
UserManagement::~UserManagement()
{
    delete ui;
}
void UserManagement::setupTable()
{
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setObjectName("dataTable");
    QStringList headers;
    headers << "序号" << "用户名" << "姓名" << "手机号" << "操作";
    m_tableWidget->setColumnCount(headers.size());
    m_tableWidget->setHorizontalHeaderLabels(headers);

    m_tableWidget->setColumnWidth(0, 80);
    m_tableWidget->setColumnWidth(4, 140);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setFocusPolicy(Qt::NoFocus);
    m_tableWidget->verticalHeader()->setDefaultSectionSize(48);

    m_pagination = new PaginationWidget(this);
    m_pagination->setPageSize(10);

    m_pageSizeCombo = new QComboBox(this);
    m_pageSizeCombo->setObjectName("pageSizeCombo");
    m_pageSizeCombo->addItems({"10", "20", "50", "100"});
    m_pageSizeCombo->setCurrentIndex(0);
    m_pageSizeCombo->setFixedWidth(70);
    QLabel *pageSizeLabel = new QLabel("每页显示", this);
    pageSizeLabel->setStyleSheet("color: #737686; font-size: 13px;");

    QWidget *bottomBar = new QWidget(this);
    bottomBar->setObjectName("bottomBar");
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(16, 0, 0, 0);
    bottomLayout->addWidget(pageSizeLabel);
    bottomLayout->addWidget(m_pageSizeCombo);
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(m_pagination);

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
    newLayout->addWidget(bottomBar, 0);
}
void UserManagement::populateTable()
{
    m_tableWidget->setRowCount(0);
    if (m_allUsers.isEmpty()) return;
    int page     = m_pagination->currentPage();
    int pageSize = m_pagination->pageSize();
    int start    = (page - 1) * pageSize;
    int end      = qMin(start + pageSize, m_allUsers.size());
    m_tableWidget->setRowCount(end - start);
    for (int i = start; i < end; ++i) {
        int row = i - start;
        const QVariantList &u = m_allUsers[i];

        int    userId   = u[0].toInt();
        QString username = u[1].toString();
        QString truename = u[2].toString();
        QString phone    = u[3].toString();

        QTableWidgetItem *idxItem = new QTableWidgetItem(QString::number(i + 1));
        idxItem->setTextAlignment(Qt::AlignCenter);
        idxItem->setForeground(QColor("#737686"));
        idxItem->setData(Qt::UserRole, userId);
        m_tableWidget->setItem(row, 0, idxItem);

        QTableWidgetItem *unameItem = new QTableWidgetItem(username);
        QFont boldFont = unameItem->font();
        boldFont.setBold(true);
        unameItem->setFont(boldFont);
        unameItem->setTextAlignment(Qt::AlignCenter);
        m_tableWidget->setItem(row, 1, unameItem);

        QTableWidgetItem *nameItem = new QTableWidgetItem(truename);
        nameItem->setTextAlignment(Qt::AlignCenter);
        m_tableWidget->setItem(row, 2, nameItem);

        QTableWidgetItem *phoneItem = new QTableWidgetItem(phone);
        phoneItem->setTextAlignment(Qt::AlignCenter);
        phoneItem->setForeground(QColor("#737686"));
        m_tableWidget->setItem(row, 3, phoneItem);

        QWidget *btnWidget = new QWidget();
        btnWidget->setObjectName("actionBtnWidget");
        QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        btnLayout->setSpacing(12);
        btnLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QPushButton *editBtn = new QPushButton("修改", btnWidget);
        editBtn->setObjectName("editBtn");
        editBtn->setCursor(Qt::PointingHandCursor);
        QPushButton *delBtn = new QPushButton("删除", btnWidget);
        delBtn->setObjectName("delBtn");
        delBtn->setCursor(Qt::PointingHandCursor);

        QVariantList userData = u;
        connect(editBtn, &QPushButton::clicked, this, [this, userData]() {
            onEditUser(userData);
        });
        connect(delBtn, &QPushButton::clicked, this, [this, userData]() {
            onDeleteUser(userData);
        });
        btnLayout->addWidget(editBtn);
        btnLayout->addWidget(delBtn);
        m_tableWidget->setCellWidget(row, 4, btnWidget);
    }
}
void UserManagement::onSearchClicked()
{
    if (!m_userSvc) return;
    QString keyword = ui->searchEdit->text().trimmed();
    m_allUsers = m_userSvc->listUsers(keyword);
    m_pagination->setTotalRecords(m_allUsers.size());
}
void UserManagement::onPageChanged(int page)
{
    Q_UNUSED(page);
    populateTable();
}
void UserManagement::onRegisterClicked()
{
    RegisterDialog dlg(this, m_userSvc);
    if (dlg.exec() == QDialog::Accepted) {
        onSearchClicked();
    }
}
void UserManagement::onEditUser(const QVariantList &userData)
{
    if (!m_userSvc) return;
    int    userId   = userData[0].toInt();
    QString username = userData[1].toString();
    QString truename = userData[2].toString();
    QString phone    = userData[3].toString();

    notifyInfo(this, QString("编辑用户: %1 (%2)").arg(username, truename));
    Q_UNUSED(phone);
}
void UserManagement::onDeleteUser(const QVariantList &userData)
{
    if (!m_userSvc) return;
    int    userId   = userData[0].toInt();
    QString username = userData[1].toString();
    QString truename = userData[2].toString();
    if (!notifyConfirm(this, "确认删除",
                       QString("确定要删除用户 \"%1 (%2)\" 吗？此操作不可撤销。")
                           .arg(username, truename)))
        return;
    QString errMsg;
    if (m_userSvc->deleteUser(userId, errMsg)) {
        notifySuccess(this, QString("用户 \"%1\" 已删除").arg(username));
        onSearchClicked();
    } else {
        notifyError(this, "删除失败", "数据库删除操作失败，请重试");
    }
}