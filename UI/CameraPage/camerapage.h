#ifndef CAMERAPAGE_H
#define CAMERAPAGE_H

#include <QWidget>

namespace Ui {
class CameraPage;
}

class CameraPage : public QWidget
{
    Q_OBJECT

public:
    explicit CameraPage(QWidget *parent = nullptr);
    ~CameraPage();

private slots:
    void on_btnInit_clicked();
    void on_btnOpen_clicked();
    void on_btnRelease_clicked();
    void on_btnPause_clicked();

private:
    Ui::CameraPage *ui;
};

#endif // CAMERAPAGE_H
