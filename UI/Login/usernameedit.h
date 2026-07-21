#ifndef USERNAMEEDIT_H
#define USERNAMEEDIT_H
#include <QLineEdit>
#include <QPaintEvent>
#include <QFocusEvent>
class UsernameEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit UsernameEdit(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
private:
    bool m_hasFocus;
};
#endif