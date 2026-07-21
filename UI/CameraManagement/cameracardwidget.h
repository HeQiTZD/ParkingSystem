#ifndef CAMERACARDWIDGET_H
#define CAMERACARDWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include "src/camera/camerainfo.h"

class CameraCardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CameraCardWidget(int index, QWidget *parent = nullptr);

    void setCameraInfo(const CameraInfo &info);
    CameraInfo cameraInfo() const;

private:
    int m_cameraIndex;
    QLineEdit *m_nameEdit;
    QLineEdit *m_locationEdit;
    QRadioButton *m_entryRadio;
    QRadioButton *m_monitorRadio;
    QComboBox *m_resolutionCombo;
    QComboBox *m_fpsCombo;
};

#endif // CAMERACARDWIDGET_H
