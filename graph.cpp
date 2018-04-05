#include "graph.h"
#include "ui_graph.h"
#include <QDate>
#include "math.h"
#define HH 500
#define MAXNEW 40.0
#define WW 900

Graph::Graph(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Graph)
{
    ui->setupUi(this);
    this->db = db;
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, ui->graphView->width(), ui->graphView->height());
    ui->graphView->setScene(scene);

}
void Graph::setData(DataManager *data){
    db = data;
}

Graph::~Graph()
{
    delete ui;
}
void Graph::doGraph(int index){
    QDate range = QDate::currentDate();
    if(index == 0){
        range = range.addDays(-7);
    }
    else if(index ==1){
        range = range.addDays(-30);
    }
    else{
        range = range.addDays(-90);
    }
    QList<statData> dataList = db->getHistory(range);
    //graphPercent(len, arr);
    if(ui->radioComplete->isChecked()){
        graphComplete(dataList);
    }
    else if(ui->radioAverage->isChecked()){
        graphPercent(dataList);
    }
    else if(ui->radioNew->isChecked()){
        graphNew(dataList);
    }
    else if(ui->radioTimeSpent->isChecked()){
        graphTime(dataList);
    }
    else if(ui->radioCardStrength->isChecked()){
        graphCards(dataList);
    }
    else{
        return;
    }
}

void Graph::on_graphTimeSelecter_currentIndexChanged(int index)
{
   doGraph(index);
}
void Graph::getDimensions(){
    COLPART = ui->graphView->width() * 0.7;
    SPACEPART = ui->graphView->width() * 0.3;
    HEIGHT = ui->graphView->height();

}
void Graph::graphCards(QList<statData> dataList){
    scene->clear();
    getDimensions();
    if(dataList.length() == 0)
        return;
    int len = dataList.length();
    double colSpace, colWidth;
    int thingSpacer;

    colWidth = COLPART / len;
    colSpace = SPACEPART / len;
    TOTALSPACE = colWidth + colSpace;
    int max = 0;
    max += dataList[len-1].learning;
    max += dataList[len-1].longlearning;
    max += dataList[len-1].longstruggle;
    max += dataList[len-1].newStrong;
    max += dataList[len-1].newWeak;
    max += dataList[len-1].strong;

    double t = (double)max / 30.0;
    t = ceil(t);
    t *= 30;
    if(t == 0)
        t = 1;
    thingSpacer = t / 10;
    setSpacers(thingSpacer);
    double strongP, learningP, longlearningP, longstruggleP, newstrongP, newweakP;
    QBrush brush(Qt::blue);
    QBrush brush2(Qt::red);

    for(int i = 0; i < dataList.length(); i++){

        strongP = dataList[i].strong / t * HH;
        newstrongP = (double)dataList[i].newStrong / t * HH + strongP;
        longlearningP = (double)dataList[i].longlearning / t * HH + newstrongP;
        learningP = (double)dataList[i].learning / t * HH + longlearningP;
        newweakP = (double)dataList[i].newWeak / t * HH + learningP;
        longstruggleP = (double)dataList[i].longstruggle/ t * HH + newweakP;


        scene->addRect(0+i*TOTALSPACE,HEIGHT- longstruggleP,colWidth,longstruggleP, QPen(Qt::black), QBrush(QColor(180, 35, 0)));
        scene->addRect(0+i*TOTALSPACE,HEIGHT- newweakP,colWidth,newweakP, QPen(Qt::black), QBrush(QColor(254, 90, 50)));
        scene->addRect(0+i*TOTALSPACE,HEIGHT- learningP,colWidth,learningP, QPen(Qt::black), QBrush(QColor(254, 200, 50)));
        scene->addRect(0+i*TOTALSPACE,HEIGHT- longlearningP,colWidth,longlearningP, QPen(Qt::black), QBrush(QColor(254, 254, 0)));
        scene->addRect(0+i*TOTALSPACE,HEIGHT- newstrongP,colWidth,newstrongP, QPen(Qt::black), QBrush(QColor(33, 254, 33)));
        scene->addRect(0+i*TOTALSPACE,HEIGHT- strongP,colWidth,strongP, QPen(Qt::black), QBrush(QColor(0, 99, 0)));




    }
}

void Graph::graphPercent(QList<statData> dataList){
    scene->clear();
    getDimensions();
    int len = dataList.length();
    if(len == 0)
        return;
    double colSpace, colWidth;
    colWidth = COLPART / len;
    colSpace = SPACEPART / len;
    TOTALSPACE = colWidth + colSpace;
    double max = -1;
    for(int i = 0; i < len; i++){
        if(dataList[i].pastAvg > max)
            max = dataList[i].pastAvg;
    }

    setSpacers(10);
    double perc;
    QBrush brush(Qt::blue);
    for(int i = 0; i < len; i++){
        perc = dataList[i].pastAvg / 100 * HH;


        scene->addRect(0+i*TOTALSPACE,HH - perc,colWidth,perc, QPen(Qt::black), brush);

    }
}
void Graph::graphTime(QList<statData> dataList){
    scene->clear();
    getDimensions();

    int len = dataList.length();
    if(len == 0)
        return;
    double colSpace, colWidth;
    colWidth = COLPART / len;
    colSpace = SPACEPART / len;
    TOTALSPACE = colWidth + colSpace;
    int max = -1;
    for(int i = 0; i < len; i++){
        int el = QTime(0,0,0).secsTo(dataList[i].studyTime);
        if(el > max)
            max = el;
    }
    setSpacersTime(max / 10);
    double perc;
    QBrush brush(Qt::blue);
    for(int i = 0; i < len; i++){
        int el = QTime(0,0,0).secsTo(dataList[i].studyTime);

        perc = (double)el / max * HH;


        scene->addRect(0+i*TOTALSPACE,HH - perc,colWidth,perc, QPen(Qt::black), brush);

    }
}

void Graph::graphNew(QList<statData> dataList){
    scene->clear();
    getDimensions();
    int len = dataList.length();
    if(len == 0)
        return;
    double colSpace, colWidth;
    colWidth = COLPART / len;
    colSpace = SPACEPART / len;
    TOTALSPACE = colWidth + colSpace;
    int max = -1;
    for(int i = 0; i < len; i++){
        if(dataList[i].newCards > max)
            max = dataList[i].newCards;
    }

    setSpacers(MAXNEW / 10);
    double perc;
    QBrush brush(Qt::blue);
    for(int i = 0; i < len; i++){
        perc = (double)dataList[i].newCards / MAXNEW * HH;


        scene->addRect(0+i*TOTALSPACE,HH - perc,colWidth,perc, QPen(Qt::black), brush);

    }
}
void Graph::graphComplete(QList<statData> dataList){
    scene->clear();
    getDimensions();
    int len = dataList.length();
    if(len == 0)
        return;
    double colSpace, colWidth;
    int thingSpacer;
    colWidth = COLPART / len;
    colSpace = SPACEPART / len;
    TOTALSPACE = colWidth + colSpace;
    int max = -1;
    for(int i = 0; i < len; i++){
        if(dataList[i].attempted > max)
            max = dataList[i].attempted;
    }
    double t = (double)max / 30.0;
    t = ceil(t);
    t *= 30;
    thingSpacer = t / 10;
    setSpacers(thingSpacer);
    double perc, perc2;
    QBrush brush(Qt::blue);
    QBrush brush2(Qt::red);

    for(int i = 0; i < len; i++){
        perc = (double)dataList[i].completed / t * HH;
        perc2 = (double)dataList[i].attempted / t * HH;


        scene->addRect(0+i*TOTALSPACE,HH - perc2,colWidth,perc2, QPen(Qt::black), brush2);
        scene->addRect(0+i*TOTALSPACE,HH - perc,colWidth,perc, QPen(Qt::black), brush);

    }
}
void Graph::setSpacers(int space){
    ui->g1->setText(QString::number(space*1));
    ui->g2->setText(QString::number(space*2));
    ui->g3->setText(QString::number(space*3));
    ui->g4->setText(QString::number(space*4));
    ui->g5->setText(QString::number(space*5));
    ui->g6->setText(QString::number(space*6));
    ui->g7->setText(QString::number(space*7));
    ui->g8->setText(QString::number(space*8));
    ui->g9->setText(QString::number(space*9));
    ui->g10->setText(QString::number(space*10));
}
void Graph::setSpacersTime(int space){
    QTime a(0,0,0);
    a = a.addSecs(space);
    ui->g1->setText(a.toString("HH:mm:ss"));
    a = a.addSecs(space);
    ui->g2->setText(a.toString("HH:mm:ss"));
    a = a.addSecs(space);
    ui->g3->setText(a.toString("HH:mm:ss"));
    a = a.addSecs(space);
    ui->g4->setText(a.toString("HH:mm:ss"));
    a = a.addSecs(space);
    ui->g5->setText(a.toString("HH:mm:ss"));
    a = a.addSecs(space);
    ui->g6->setText(a.toString("HH:mm:ss"));
    a = a.addSecs(space);
    ui->g7->setText(a.toString("HH:mm:ss"));
    a = a.addSecs(space);
    ui->g8->setText(a.toString("HH:mm:ss"));
    a = a.addSecs(space);
    ui->g9->setText(a.toString("HH:mm:ss"));
    a = a.addSecs(space);
    ui->g10->setText(a.toString("HH:mm:ss"));
}

void Graph::drawGraph(QList<statData> dataList){
    QBrush brush(Qt::blue);
    int len = dataList.length();
    if(len == 0)
        len = 1;
        //return;
    double colWidth, colSpace;

    colWidth = COLPART / len;
    colSpace = SPACEPART / len;
    for(int i = 0; i < len; i++){

    }

    //scene->addRect(0,0,30,60, QPen(Qt::black), brush);



}

void Graph::on_radioComplete_clicked()
{
    doGraph(ui->graphTimeSelecter->currentIndex());
}

void Graph::on_radioAverage_clicked()
{
    doGraph(ui->graphTimeSelecter->currentIndex());
}

void Graph::on_radioNew_clicked()
{
    doGraph(ui->graphTimeSelecter->currentIndex());
}

void Graph::on_radioTimeSpent_clicked()
{
    doGraph(ui->graphTimeSelecter->currentIndex());
}

void Graph::on_radioCardStrength_clicked()
{
   doGraph(ui->graphTimeSelecter->currentIndex());
}
