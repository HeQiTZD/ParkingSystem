#include "paginationwidget.h"
#include <QHBoxLayout>
#include <QSpacerItem>

// ════════════════════════════════════════════════════════════════
// 构造函数：搭建页面骨架
// ════════════════════════════════════════════════════════════════
PaginationWidget::PaginationWidget(QWidget *parent)
    : QWidget(parent)
{
    // ── 外层：水平布局 ──
    // 左 → 中 → 右：信息标签 → 弹性空间 → 按钮组
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 12, 16, 12);

    // ── 左侧：记录信息 ──
    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet("color: #737686; font-size: 13px;");
    mainLayout->addWidget(m_infoLabel);

    // ── 中间：弹性伸缩器 ──
    // 把左侧的文字推向最左，按钮组推向最右
    mainLayout->addStretch(1);

    // ── 右侧：按钮组 ──
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(4);    // 按钮之间的间距 4px

    // 上一页按钮 "←"
    m_prevButton = new QPushButton("←", this);
    m_prevButton->setFixedSize(32, 32);
    m_prevButton->setCursor(Qt::PointingHandCursor);
    m_buttonLayout->addWidget(m_prevButton);

    // 页码按钮先空着，等 setTotalRecords 时生成

    // 下一页按钮 "→"
    m_nextButton = new QPushButton("→", this);
    m_nextButton->setFixedSize(32, 32);
    m_nextButton->setCursor(Qt::PointingHandCursor);
    m_buttonLayout->addWidget(m_nextButton);

    mainLayout->addLayout(m_buttonLayout);

    // ── 连接信号 ──
    connect(m_prevButton, &QPushButton::clicked,
            this, &PaginationWidget::onPrevButtonClicked);
    connect(m_nextButton, &QPushButton::clicked,
            this, &PaginationWidget::onNextButtonClicked);

    // 初始状态：没数据时显示空
    m_infoLabel->setText("共 0 条记录");
    updateButtons();
}

// ════════════════════════════════════════════════════════════════
// 公共接口
// ════════════════════════════════════════════════════════════════
void PaginationWidget::setTotalRecords(int total)
{
    m_totalRecords = total;
    m_currentPage = 1;
    updateButtons();
    emit pageChanged(m_currentPage);
}

void PaginationWidget::setPageSize(int size)
{
    m_pageSize = size;
    m_currentPage = 1;
    updateButtons();
    emit pageChanged(m_currentPage);
}

void PaginationWidget::setCurrentPage(int page)
{
    if (page < 1 || page > totalPages()) return;
    m_currentPage = page;
    updateButtons();
}

int PaginationWidget::totalRecords() const { return m_totalRecords; }
int PaginationWidget::pageSize() const { return m_pageSize; }
int PaginationWidget::currentPage() const { return m_currentPage; }

int PaginationWidget::totalPages() const
{
    if (m_totalRecords == 0) return 0;
    // 关键公式：ceil(total / pageSize)
    // C++ 整数除法技巧：(total + size - 1) / size  ≡  ceil(total / size)
    return (m_totalRecords + m_pageSize - 1) / m_pageSize;
}

// ════════════════════════════════════════════════════════════════
// 槽函数
// ════════════════════════════════════════════════════════════════
void PaginationWidget::onPageButtonClicked()
{
    // sender() 返回发射信号的按钮对象
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn) return;

    int page = btn->text().toInt();  // 按钮的文字就是页码数字
    setCurrentPage(page);
    emit pageChanged(m_currentPage);
}

void PaginationWidget::onPrevButtonClicked()
{
    if (m_currentPage > 1) {
        setCurrentPage(m_currentPage - 1);
        emit pageChanged(m_currentPage);
    }
}

void PaginationWidget::onNextButtonClicked()
{
    if (m_currentPage < totalPages()) {
        setCurrentPage(m_currentPage + 1);
        emit pageChanged(m_currentPage);
    }
}

// ════════════════════════════════════════════════════════════════
// ★ 核心方法：根据 currentPage 重新生成并刷新所有按钮
// ════════════════════════════════════════════════════════════════
void PaginationWidget::updateButtons()
{
    int total = totalPages();    // 总页码数

    // ── 更新信息标签 ──
    int start = (m_currentPage - 1) * m_pageSize + 1;
    int end = qMin(m_currentPage * m_pageSize, m_totalRecords);
    if (m_totalRecords == 0) {
        m_infoLabel->setText("共 0 条记录");
    } else {
        m_infoLabel->setText(
            QString("显示 %1-%2 条，共 %3 条记录")
                .arg(start).arg(end).arg(m_totalRecords));
    }

    // ── 删除旧的页码按钮 ──
    for (QPushButton *btn : m_pageButtons) {
        m_buttonLayout->removeWidget(btn);
        delete btn;
    }
    m_pageButtons.clear();

    // ── 如果只有 0 或 1 页，不显示页码按钮 ──
    if (total <= 1) {
        m_prevButton->setEnabled(false);
        m_nextButton->setEnabled(false);
        return;
    }

    // ── 生成新的页码按钮 ──
    // 策略：始终显示 7 个按钮（含 "..." 占位）
    // 少于 7 页 → 全显示
    // 多于 7 页 → 首尾 + 中间省略

    QList<int> pages;  // 要显示哪些页码
    if (total <= 7) {
        for (int i = 1; i <= total; ++i) pages << i;
    } else {
        pages << 1;  // 始终显示第一页
        if (m_currentPage > 3) pages << -1;   // -1 表示 "..."

        for (int i = qMax(2, m_currentPage - 1);
                i <= qMin(total - 1, m_currentPage + 1); ++i)
            pages << i;

        if (m_currentPage < total - 2) pages << -1;  // "..."
        pages << total;  // 始终显示最后一页
    }

    // ── 创建按钮 ──
    for (int page : pages) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(32, 32);
        btn->setCursor(Qt::PointingHandCursor);

        if (page == -1) {
            // "..." 占位 —— 不可点击
            btn->setText("···");
            btn->setEnabled(false);
            QString dotStyle =
                "QPushButton {"
                "  background: transparent;"
                "  border: none;"
                "  color: #737686;"
                "  font-size: 13px;"
                "}";
            btn->setStyleSheet(dotStyle);
        } else {
            btn->setText(QString::number(page));
            if (page == m_currentPage) {
                // ★ 当前页：蓝色实心，和 HTML 的 .bg-primary 一致
                QString activeStyle =
                    "QPushButton {"
                    "  background-color: #003FB1;"
                    "  color: #ffffff;"
                    "  border: none;"
                    "  border-radius: 4px;"
                    "  font-size: 13px;"
                    "  font-weight: 500;"
                    "}";
                btn->setStyleSheet(activeStyle);
            } else {
                // 默认页：白色底、灰色边框
                QString normalStyle =
                    "QPushButton {"
                    "  background-color: #ffffff;"
                    "  color: #191b23;"
                    "  border: 1px solid #E2E8F0;"
                    "  border-radius: 4px;"
                    "  font-size: 13px;"
                    "  font-weight: 500;"
                    "}"
                    "QPushButton:hover {"
                    "  background-color: #f3f3fe;"
                    "}";
                btn->setStyleSheet(normalStyle);
            }
            connect(btn, &QPushButton::clicked,
                    this, &PaginationWidget::onPageButtonClicked);
        }

        // 插入到布局中（上一页按钮后面、下一页按钮前面）
        // 每次插入到 m_buttonLayout 的倒数第二个位置（在 m_nextButton 之前）
        m_buttonLayout->insertWidget(m_buttonLayout->count() - 1, btn);
        m_pageButtons.append(btn);
    }

    // ── 更新 ← / → 按钮的启用/禁用状态 ──
    m_prevButton->setEnabled(m_currentPage > 1);
    m_nextButton->setEnabled(m_currentPage < total);

    // ── 给 ← → 按钮设样式（和页码按钮风格一致） ──
    QString navStyle =
        "QPushButton {"
        "  background-color: #ffffff;"
        "  color: #191b23;"
        "  border: 1px solid #E2E8F0;"
        "  border-radius: 4px;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #f3f3fe;"
        "}"
        "QPushButton:disabled {"
        "  color: #c0c0c0;"
        "  border-color: #E2E8F0;"
        "  background-color: #fafafa;"
        "}";
    m_prevButton->setStyleSheet(navStyle);
    m_nextButton->setStyleSheet(navStyle);
}
