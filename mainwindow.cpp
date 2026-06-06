#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(InitFile* config, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , config(config)
{
    ui->setupUi(this);

    //设置窗口标题
    setWindowTitle(QStringLiteral("停车场管理系统"));

    //设置窗口大小
    resize(800,600);

    dbManager = new DatabaseManager();

    //连接数据库状态信号
    connect(dbManager,&DatabaseManager::connectionStatusChanged,
            this,&MainWindow::on_DatabaseConnectionChanged);

    //初始化数据库
    initDatabase();
}

MainWindow::~MainWindow()
{
    delete dbManager;
    dbManager = nullptr;
    delete ui;
}

void MainWindow::on_loginButton_clicked()
{
    //获取输入框的值
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();

    //验证输入
    if(username.isEmpty() || password.isEmpty()){
        QMessageBox::warning(this,"警告",QStringLiteral("用户名和密码不能为空！"));
        return;
    }

    //对密码进行SHA256加密
    QString encryptedPassword = encryptPassword(password);

    //TODO:连接数据库验证
    QString userRole;
    //使用加密后的密码进行验证
    if(dbManager->validateUser(username,encryptedPassword,userRole)){
        QMessageBox::information(this,"成功",QStringLiteral("登录成功！欢迎%1").arg(userRole));
        //TODO根据不同用户角色跳转到不同界面
    }else{
        QMessageBox::warning(this,"失败",QStringLiteral("用户名或密码错误！"));
    }
}

void MainWindow::on_registerButton_clicked()
{
    //创建注册窗口
    if(!registerWindow){
        registerWindow = new RegisterWindow(this,dbManager);

        //连接信号
        connect(registerWindow,&RegisterWindow::registerSuccess,
                this,&MainWindow::onRegisterSuccess);
        connect(registerWindow,&RegisterWindow::returnToLogin,
                this,&MainWindow::onReturnToLogin);
    }

    //隐藏登录窗口，显示注册窗口
    this->hide();
    registerWindow->show();
}

void MainWindow::on_DatabaseConnectionChanged(bool connected)
{
    updateConnectionStatus();
}

void MainWindow::onRegisterSuccess()
{
    //注册成功后返回登录界面
    registerWindow->hide();
    this->show();

    //清空输入框
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();

    QMessageBox::information(this,"提示",QStringLiteral("请使用新账号登录"));
}

void MainWindow::onReturnToLogin()
{
    //返回登录界面
    registerWindow->hide();
    this->show();
}

void MainWindow::initDatabase()
{
    QString host = config->getDbHost();
    int port = config->getDbPort();
    QString dbname = config->getDbName();
    QString username = config->getDbUsername();
    QString password = config->getDbPassword();

    if(!dbManager->connectDatabase(host,port,dbname,username,password)){
        QMessageBox::warning(this,"警告","数据库连接失败，请检查配置信息！");
    }

    updateConnectionStatus();
}

void MainWindow::updateConnectionStatus()
{
    //更新状态栏显示连接状态
    if(dbManager->isConnected()){
        statusBar()->showMessage("数据库已连接",3000);
    }else{
        statusBar()->showMessage("数据库未连接",3000);
    }
}






