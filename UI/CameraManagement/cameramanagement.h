#ifndef CAMERAMANAGEMENT_H
#define CAMERAMANAGEMENT_H

#include <QWidget>
#include <QList>

namespace Ui { class CameraManagement; }

class CameraWindow;
class QFrame;
class QGridLayout;
class QLabel;
class QPushButton;

class CameraManagement : public QWidget
{
    Q_OBJECT

public:
    explicit CameraManagement(QWidget *parent = nullptr);
    ~CameraManagement();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void buildCameraWindows();
    void updateGridLayout();
    int  gridCols(int count) const;
    void updateCountLabel();

    Ui::CameraManagement *ui;
    
    QFrame       *m_videoContainer  = nullptr;
    QGridLayout *m_gridLayout      = nullptr;
    QLabel       *m_emptyLabel     = nullptr;
    QLabel       *m_countLabel     = nullptr;
    QPushButton *m_startAllBtn     = nullptr;
    QPushButton *m_stopAllBtn      = nullptr;
    QPushButton *m_settingsBtn     = nullptr;

    QList<CameraWindow*> m_windows;
    bool m_soloMode  = false;
    int  m_soloIndex = -1;
};

#endif // CAMERAMANAGEMENT_H
