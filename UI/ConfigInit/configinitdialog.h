#ifndef CONFIGINITDIALOG_H
#define CONFIGINITDIALOG_H
#include <QDialog>
#include <QPoint>
namespace Ui {
class ConfigInitDialog;
}
class ConfigInitDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigInitDialog(QWidget *parent = nullptr);
    ~ConfigInitDialog();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void on_btnSubmit_clicked();
    void on_btnCancel_clicked();
    void on_btnMinimize_clicked();
    void on_btnClose_clicked();
private:
    Ui::ConfigInitDialog *ui;
    bool m_dragging;
    QPoint m_dragPosition;
    bool validateInputs();
    void saveConfig();
    void setupWindowFlags();
    void setupShadow();
    void setupTitleBar();
};
#endif