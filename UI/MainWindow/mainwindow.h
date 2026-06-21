#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "circleprogress.h"
#include <QMainWindow>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

private slots:
    void updateTime();

private:
    Ui::MainWindow *ui;
    QTimer *m_timer;
    CircleProgress *m_parkingChart;
    int m_borderWidth = 8;  // 窗口边缘拉伸感应区域宽度
};

#endif // MAINWINDOW_H
