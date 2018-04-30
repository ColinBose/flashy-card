#include "loginmenu.h"
#include "ui_loginmenu.h"
#include "sha256.h"

LoginMenu::LoginMenu(QWidget *parent, MainWindow *m, MultiManager *networked) :
    QDialog(parent),
    ui(new Ui::LoginMenu)
{
    ui->setupUi(this);
    mw = m;
    multiManager = networked;
}

LoginMenu::~LoginMenu()
{
    delete ui;
}

void LoginMenu::on_loginLoginButton_clicked()
{
    QString name = ui->loginUser->text();
    QString pass = ui->loginPass->text();
    qDebug() << "Name: " + name + " Pass: " + pass;
    if(name.length() == 0)
        return;
    if(pass.length() == 0)
        return;
    std::string hash = sha256(pass.toStdString());
    QString passHash = QString::fromStdString(hash);
    multiManager->sendLogin(name, passHash);

}

void LoginMenu::on_loginPass_returnPressed()
{
    on_loginLoginButton_clicked();
}

void LoginMenu::on_loginLoginButton_2_clicked()
{
    ui->registerErrorLabel->setText("");
    QString name, fPass,sPass;
    name = ui->registerUser->text();
    fPass = ui->registerMainPass->text();
    sPass = ui->registerCheckPass->text();
    if(name.length() == 0)
        return;
    if(fPass.length() == 0 || sPass.length() == 0)
        return;
    if(fPass != sPass){
        ui->registerErrorLabel->setText("Passwords do not match");
        return;
    }
    if(!checkName(name)){
        ui->registerErrorLabel->setText("Name constraints: a-z, A-Z, 0-9");
        return;
    }
    if(!checkPass(fPass)){
        return;
    }
    QString hashPass = QString::fromStdString(sha256(fPass.toStdString()));
    multiManager->sendRegisterRequest(name, hashPass);

}

void LoginMenu::on_registerCheckPass_returnPressed()
{
    on_loginLoginButton_2_clicked();
}
bool LoginMenu::checkName(QString name){
    for(int i = 0; i < name.length(); i++){
        if(!name[i].isLetterOrNumber()){
            return false;
        }
    }
    return true;
}
bool LoginMenu::checkPass(QString pass){
    if(pass.length() < 5){
        ui->registerErrorLabel->setText("Password must be at least 5 characters long");
        return false;
    }
    if(pass.length() > 32){
        ui->registerErrorLabel->setText("Password must be under characters long");
        return false;
    }
    //possibly other constraints?
    return true;
}

void LoginMenu::on_loginRegisterButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void LoginMenu::on_registerBackToLogin_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}
void LoginMenu::handleLogin(int code){
    if(code == 0){
        ui->loginErrorLabel->setText("User not found");
    }
    else if(code == 1){
        ui->loginErrorLabel->setText("Incorrect password");
    }
    else{
        ui->loginErrorLabel->setText("Login Successful!");
        mw->loggedInName = ui->loginUser->text();
        ui->loginPass->setText("");
        ui->loginUser->setText("");
        mw->passUserName();
        this->close();
    }
}
void LoginMenu::handleRegister(int code){
    if(code == 1){
        ui->registerErrorLabel->setText("Registration complete!");
        ui->registerCheckPass->setText("");
        ui->registerMainPass->setText("");
        ui->registerUser->setText("");
    }else{

        ui->registerErrorLabel->setText("Username already exists, try another");
    }

}
