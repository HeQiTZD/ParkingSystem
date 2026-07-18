#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
class DatabaseManager;

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr, DatabaseManager *db = nullptr);
    ~RegisterDialog();

    QString getUserName() const;
    QString getPassword() const;

private slots:
    void on_btnRegister_clicked();
    void on_btnBack_clicked();

protected:
    // 无边框窗口手动拖动
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    // 绘制圆角白色背景（WA_TranslucentBackground 下 QSS background 不生效）
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::RegisterDialog *ui;
    bool validateInputs();

private:
    DatabaseManager *m_db;

    bool m_dragging = false;   // 是否正在拖动
    QPoint m_dragPos;          // 拖动起始偏移
    QRect  m_dragArea;         // 可拖动区域（顶部高度）
};

#endif // REGISTERDIALOG_H
