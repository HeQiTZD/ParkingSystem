#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QPoint>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

signals:
    void settingsSaved();
    void dbConfigChanged();

private slots:
    void onTestConnection();
    void onSave();

private:
    void setupWindow();
    void loadSettings();

    Ui::SettingsDialog *ui;
    bool m_dragging = false;
    QPoint m_dragPosition;

    // 保存前快照，用于比对数据库参数是否变更
    QString m_oldHost;
    int     m_oldPort = 0;
    QString m_oldDbName;
    QString m_oldUsername;
    QString m_oldPassword;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // SETTINGSDIALOG_H
