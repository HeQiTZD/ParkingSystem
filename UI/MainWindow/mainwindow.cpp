#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // QFile styleFile(":/styles/main.qss");
    // if (styleFile.open(QFile::ReadOnly)) {
    //     QString styleSheet = QLatin1String(styleFile.readAll());
    //     setStyleSheet(styleSheet);
    //     styleFile.close();
    // }
}

MainWindow::~MainWindow()
{
    delete ui;
}
