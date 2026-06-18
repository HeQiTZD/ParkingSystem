#ifndef PASSWORDEDIT_H
#define PASSWORDEDIT_H

#include <QLineEdit>
#include <QPaintEvent>
#include <QFocusEvent>
class PasswordEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit PasswordEdit(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    bool m_hasFocus;
};

#endif // PASSWORDEDIT_H
