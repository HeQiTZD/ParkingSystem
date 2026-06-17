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

#endif // USERNAMEEDIT_H

/*
    Qt 中专门处理键盘焦点（Keyboard Focus）的事件类。
    当一个控件获得焦点（FocusIn）或失去焦点（FocusOut）时，Qt 会生成该事件并传递给对应的控件。
*/