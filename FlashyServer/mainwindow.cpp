#include "mainwindow.h"
#include "ui_mainwindow.h"
#define GRAPHSPACE 0.1f
#define SCENEWIDTH 430
#define SCENEHEIGHT 290

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serv = new Server();
    setGraphics();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startServer_clicked()
{
    ui->startServer->setEnabled(false);
    serv->startServer(1,1);
    serv->passMw(this);
}

void MainWindow::updateDisplay(int rooms, int users, int networkMs, int cpuMs, int totalPackets){
    if(totalDraws < GRAPHSTEPS){
        cpuTimes[totalDraws] = cpuMs;
        networkTimes[totalDraws] = networkMs;
    }
    else{
        for(int i = 0; i < GRAPHSTEPS-1; i++){
            cpuTimes[i] = cpuTimes[i+1];
            networkTimes[i] = networkTimes[i+1];
        }
        cpuTimes[GRAPHSTEPS - 1] = cpuMs;
        networkTimes[GRAPHSTEPS - 1] = networkMs;
    }
    totalDraws++;
    drawGraph(true, networkScene, networkTimes);
    drawGraph(false, cpuScene, cpuTimes);
    ui->mainActiveRooms->setText("Active Rooms: " + QString::number(rooms));
    ui->mainConnectedClients->setText("Connected Clients: " + QString::number(users));
    ui->mainPacketLabel->setText("Packets/s: " + QString::number(totalPackets));


}
void MainWindow::setGraphics(){
    brushes = QBrush(Qt::red);
    pens[0] = QPen(Qt::green);
    pens[0].setWidth(5);
    pens[1] = QPen(Qt::yellow);
    pens[1].setWidth(5);
    pens[2] = QPen(Qt::red);
    pens[2].setWidth(5);
    pens2 = QPen(Qt::blue);

    networkScene = new QGraphicsScene(0,0, SCENEWIDTH, SCENEHEIGHT);
    cpuScene = new QGraphicsScene(0,0, SCENEWIDTH, SCENEHEIGHT);

    ui->networkLatency->setScene(networkScene);
    ui->cpuLatency->setScene(cpuScene);
    for(int i = 0; i < GRAPHSTEPS; i++){
        cpuTimes[i] = 0;
        networkTimes[i] = 0;
    }
    //drawGraph(0);


}
void MainWindow::setGraphLabels(int r, bool network){
    if(network){
        ui->nw1->setText(QString::number(r / 10 * 1) + " ms");
        ui->nw2->setText(QString::number(r / 10 * 2)+ " ms");
        ui->nw3->setText(QString::number(r / 10 * 3)+ " ms");
        ui->nw4->setText(QString::number(r / 10 * 4)+ " ms");
        ui->nw5->setText(QString::number(r / 10 * 5)+ " ms");
        ui->nw6->setText(QString::number(r / 10 * 6)+ " ms");
        ui->nw7->setText(QString::number(r / 10 * 7)+ " ms");
        ui->nw8->setText(QString::number(r / 10 * 8)+ " ms");
        ui->nw9->setText(QString::number(r / 10 * 9)+ " ms");
    }else{
        ui->cpu1->setText(QString::number(r / 10 * 1) + " ms");
        ui->cpu2->setText(QString::number(r / 10 * 2)+ " ms");
        ui->cpu3->setText(QString::number(r / 10 * 3)+ " ms");
        ui->cpu4->setText(QString::number(r / 10 * 4)+ " ms");
        ui->cpu5->setText(QString::number(r / 10 * 5)+ " ms");
        ui->cpu6->setText(QString::number(r / 10 * 6)+ " ms");
        ui->cpu7->setText(QString::number(r / 10 * 7)+ " ms");
        ui->cpu8->setText(QString::number(r / 10 * 8)+ " ms");
        ui->cpu9->setText(QString::number(r / 10 * 9)+ " ms");
    }
}

void MainWindow::drawGraph(bool network, QGraphicsScene * scene, int numArray[10]){

    int scale;
    float lastW, lastH;
    int w;
    float hScale;
    int penType;

    scene->clear();
    for(int i = 1; i < 10; i++){
        QLineF line(0, SCENEHEIGHT/10*i, SCENEWIDTH, SCENEHEIGHT/10*i);
        scene->addLine(line,  pens2);

    }
    int totalSteps = totalDraws;
    if(totalSteps > GRAPHSTEPS)
        totalSteps = GRAPHSTEPS;
    int min = 999999999;
    int max = -1;
    for(int i = 0; i < totalSteps; i++){
        if(numArray[i] < min)
            min = numArray[i];
        if(numArray[i] > max)
            max = numArray[i];
    }

    if(max < 10){
        scale = 10;
    }
    else if(max < 100){
        scale = 100;
    }
    else{
        scale = 500;
    }

    setGraphLabels(scale, network);
    w= 0;

    for(int i = 0; i < totalSteps; i++){
        hScale = (float)numArray[i] / (float)scale;
        if(w==0){
            lastW = 0;
            lastH = SCENEHEIGHT - hScale * (SCENEHEIGHT-10);
            w++;
            continue;
        }
        penType = getColour(numArray[i]);
        QLineF line(lastW,lastH ,w*GRAPHSPACE * SCENEWIDTH, SCENEHEIGHT- hScale * (SCENEHEIGHT-10));
        lastW = w*GRAPHSPACE * SCENEWIDTH;
        lastH = SCENEHEIGHT - hScale * (SCENEHEIGHT-10);
        w++;
        scene->addLine(line,pens[penType]);
    }
    qApp->processEvents();


}
int MainWindow::getColour(int r){
    if(r < 10)
        return 0;
    if(r < 100)
        return 1;
    return 2;
}
