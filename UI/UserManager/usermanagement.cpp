#include "usermanagement.h"
#include "ui_usermanagement.h"
#include "src/database/databasemanager.h"
#include <QFile>

UserManagement::UserManagement(QWidget *parent, DatabaseManager *db)
    : QWidget(parent)
    , ui(new Ui::UserManagement)
    , m_db(db)
{
    ui->setupUi(this);

    QFile styleFile(":/styles/userManagement.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }
}

UserManagement::~UserManagement()
{
    delete ui;
}
