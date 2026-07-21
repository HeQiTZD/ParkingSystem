#ifndef NOTIFICATION_GLOBAL_H
#define NOTIFICATION_GLOBAL_H
#include <QWidget>
#include "toastwidget.h"
#include "notificationdialog.h"
inline void notifySuccess(QWidget *parent, const QString &msg, int ms = 3000) {
    ToastWidget::show(parent, msg, ToastWidget::Success, ms);
}
inline void notifyFailure(QWidget *parent, const QString &msg, int ms = 3500) {
    ToastWidget::show(parent, msg, ToastWidget::Failure, ms);
}
inline void notifyInfo(QWidget *parent, const QString &msg, int ms = 3000) {
    ToastWidget::show(parent, msg, ToastWidget::Info, ms);
}
inline void notifyToastWarning(QWidget *parent, const QString &msg, int ms = 3500) {
    ToastWidget::show(parent, msg, ToastWidget::Warning, ms);
}
inline bool notifyWarning(QWidget *parent, const QString &title, const QString &msg) {
    return NotificationDialog::warning(parent, title, msg);
}
inline void notifyError(QWidget *parent, const QString &title, const QString &msg) {
    NotificationDialog::error(parent, title, msg);
}
inline bool notifyConfirm(QWidget *parent, const QString &title, const QString &msg) {
    return NotificationDialog::question(parent, title, msg);
}
#endif