#include "logindialog.h"
#include "ui_logindialog.h"
#include <QFile>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    // 加载QSS样式文件
    QFile styleFile(":/styles/login.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        setStyleSheet(styleSheet);
        styleFile.close();
    }
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
