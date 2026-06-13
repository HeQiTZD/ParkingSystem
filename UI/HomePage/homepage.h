#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>

namespace Ui {
class HomePage;
}

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

private slots:
    void on_btnPlay_clicked();
    void on_btnPause_clicked();
    void on_btnCapture_clicked();
    void on_btnVehicleIn_clicked();
    void on_btnVehicleOut_clicked();

private:
    Ui::HomePage *ui;
};

#endif // HOMEPAGE_H
