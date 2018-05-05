#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startButton_clicked()
{
    int delay = 0;
    delay = ui->delayEdit->text().toInt();
    if(delay <= 0)
        return;
    serv.startServer(1,1, delay);
    ui->startButton->setEnabled(false);
}
