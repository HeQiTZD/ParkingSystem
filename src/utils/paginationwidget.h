#ifndef PAGINATIONWIDGET_H
#define PAGINATIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QList>

class PaginationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PaginationWidget(QWidget *parent = nullptr);

    // ── 设置数据 ──
    void setTotalRecords(int total);   // 总记录数，如 128
    void setPageSize(int size);        // 每页条数，如 10
    void setCurrentPage(int page);     // 跳到指定页（从 1 开始）

    int totalRecords() const;
    int pageSize() const;
    int currentPage() const;
    int totalPages() const;            // 自动计算：ceil(total / pageSize)

signals:
    // 翻页时发射，通知外界"用户想看第 page 页了"
    void pageChanged(int page);

private slots:
    void onPageButtonClicked();        // 页码按钮被点击
    void onPrevButtonClicked();        // 上一页
    void onNextButtonClicked();        // 下一页

private:
    void updateButtons();              // ★ 核心方法：根据 currentPage 重新生成按钮

    QLabel *m_infoLabel;               // 左侧："显示 1-10 条，共 128 条记录"
    QPushButton *m_prevButton;         // "<" 上一页
    QList<QPushButton *> m_pageButtons; // 中间的页码按钮们
    QPushButton *m_nextButton;         // ">" 下一页
    QHBoxLayout *m_buttonLayout;       // 容纳页码按钮的水平布局

    int m_totalRecords = 0;
    int m_pageSize = 10;
    int m_currentPage = 1;
};

#endif // PAGINATIONWIDGET_H
