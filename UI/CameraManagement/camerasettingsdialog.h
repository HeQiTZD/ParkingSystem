#ifndef CAMERASETTINGSDIALOG_H
#define CAMERASETTINGSDIALOG_H

#include <QDialog>
#include <QList>

namespace Ui {
class CameraSettingsDialog;
}

class CameraCardWidget;

class CameraSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraSettingsDialog(QWidget *parent = nullptr);
    ~CameraSettingsDialog();

private:
    void loadCameraCards();
    void onSave();

    Ui::CameraSettingsDialog *ui;
    QList<CameraCardWidget *> m_cards;
};

#endif // CAMERASETTINGSDIALOG_H
