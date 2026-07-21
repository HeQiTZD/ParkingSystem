#include "paginationwidget.h"
#include <QHBoxLayout>
#include <QSpacerItem>
PaginationWidget::PaginationWidget(QWidget *parent)
    : QWidget(parent)
{

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 12, 16, 12);

    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet("color: #737686; font-size: 13px;");
    mainLayout->addWidget(m_infoLabel);

    mainLayout->addStretch(1);

    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(4);

    m_prevButton = new QPushButton("←", this);
    m_prevButton->setFixedSize(32, 32);
    m_prevButton->setCursor(Qt::PointingHandCursor);
    m_buttonLayout->addWidget(m_prevButton);

    m_nextButton = new QPushButton("→", this);
    m_nextButton->setFixedSize(32, 32);
    m_nextButton->setCursor(Qt::PointingHandCursor);
    m_buttonLayout->addWidget(m_nextButton);
    mainLayout->addLayout(m_buttonLayout);

    connect(m_prevButton, &QPushButton::clicked,
            this, &PaginationWidget::onPrevButtonClicked);
    connect(m_nextButton, &QPushButton::clicked,
            this, &PaginationWidget::onNextButtonClicked);

    m_infoLabel->setText("共 0 条记录");
    updateButtons();
}
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

    return (m_totalRecords + m_pageSize - 1) / m_pageSize;
}
void PaginationWidget::onPageButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn) return;
    int page = btn->text().toInt();
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
void PaginationWidget::updateButtons()
{
    int total = totalPages();

    int start = (m_currentPage - 1) * m_pageSize + 1;
    int end = qMin(m_currentPage * m_pageSize, m_totalRecords);
    if (m_totalRecords == 0) {
        m_infoLabel->setText("共 0 条记录");
    } else {
        m_infoLabel->setText(
            QString("显示 %1-%2 条，共 %3 条记录")
                .arg(start).arg(end).arg(m_totalRecords));
    }

    for (QPushButton *btn : m_pageButtons) {
        m_buttonLayout->removeWidget(btn);
        delete btn;
    }
    m_pageButtons.clear();

    if (total <= 1) {
        m_prevButton->setEnabled(false);
        m_nextButton->setEnabled(false);
        return;
    }

    QList<int> pages;
    if (total <= 7) {
        for (int i = 1; i <= total; ++i) pages << i;
    } else {
        pages << 1;
        if (m_currentPage > 3) pages << -1;
        for (int i = qMax(2, m_currentPage - 1);
                i <= qMin(total - 1, m_currentPage + 1); ++i)
            pages << i;
        if (m_currentPage < total - 2) pages << -1;
        pages << total;
    }

    for (int page : pages) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(32, 32);
        btn->setCursor(Qt::PointingHandCursor);
        if (page == -1) {
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

        m_buttonLayout->insertWidget(m_buttonLayout->count() - 1, btn);
        m_pageButtons.append(btn);
    }

    m_prevButton->setEnabled(m_currentPage > 1);
    m_nextButton->setEnabled(m_currentPage < total);

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