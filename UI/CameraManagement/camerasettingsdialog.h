#ifndef CAMERASETTINGSDIALOG_H
#define CAMERASETTINGSDIALOG_H

#include <QDialog>
#include <QList>
#include <QPoint>

class QCloseEvent;
class QButtonGroup;

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

signals:
    void camerasUpdated();
    void cameraConfigChanged();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onSave();

private:
    void loadGlobalParams();
    void loadCameraCards();

    Ui::CameraSettingsDialog *ui;
    QList<CameraCardWidget *> m_cards;
    QButtonGroup *m_roleGroup = nullptr;
    bool m_dirty = false;
    QPoint m_dragPos;
};

#endif // CAMERASETTINGSDIALOG_H
