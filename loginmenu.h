#ifndef LOGINMENU_H
#define LOGINMENU_H

#include <QDialog>
#include "mainwindow.h"
#include "multimanager.h"
class Mainwindow;
class MultiManager;
namespace Ui {
class LoginMenu;
}

class LoginMenu : public QDialog
{
    Q_OBJECT

public:
    explicit LoginMenu(QWidget *parent = 0, MainWindow * m = 0, MultiManager * networked = 0);
    ~LoginMenu();
public slots:
    void handleLogin(int code);
    void handleRegister(int code);

private slots:
    void on_loginLoginButton_clicked();

    void on_loginPass_returnPressed();

    void on_loginLoginButton_2_clicked();

    void on_registerCheckPass_returnPressed();

    void on_loginRegisterButton_clicked();

    void on_registerBackToLogin_clicked();

private:
    Ui::LoginMenu *ui;
    MainWindow * mw;
    MultiManager * multiManager;
    bool checkName(QString name);
    bool checkPass(QString pass);
};

#endif // LOGINMENU_H
