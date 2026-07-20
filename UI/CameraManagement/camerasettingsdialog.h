#ifndef CAMERASETTINGSDIALOG_H
#define CAMERASETTINGSDIALOG_H

#include <QDialog>
#include <QList>
#include <QPoint>

namespace Ui {
class CameraSettingsDialog;
}

class QButtonGroup;
class CameraInfo;

class CameraSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraSettingsDialog(QWidget *parent = nullptr);
    ~CameraSettingsDialog();

signals:
    void camerasUpdated();
    void cameraConfigChanged();

private slots:
    void onSave();

private:
    void setupWindow();
    void loadSettings();
    void buildCameraRows();
    QWidget* createCameraRow(int index, const CameraInfo &info);

    Ui::CameraSettingsDialog *ui;
    QList<QWidget*> m_cameraRows;
    QButtonGroup *m_roleGroup = nullptr;
    bool m_dragging = false;
    QPoint m_dragPosition;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // CAMERASETTINGSDIALOG_H
