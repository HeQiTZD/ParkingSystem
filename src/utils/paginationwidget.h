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

    void setTotalRecords(int total);
    void setPageSize(int size);
    void setCurrentPage(int page);
    int totalRecords() const;
    int pageSize() const;
    int currentPage() const;
    int totalPages() const;
signals:
    void pageChanged(int page);
private slots:
    void onPageButtonClicked();
    void onPrevButtonClicked();
    void onNextButtonClicked();
private:
    void updateButtons();
    QLabel *m_infoLabel;
    QPushButton *m_prevButton;
    QList<QPushButton *> m_pageButtons;
    QPushButton *m_nextButton;
    QHBoxLayout *m_buttonLayout;
    int m_totalRecords = 0;
    int m_pageSize = 10;
    int m_currentPage = 1;
};
#endif