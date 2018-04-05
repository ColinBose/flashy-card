#ifndef GRAPH_H
#define GRAPH_H

#include <QDialog>
#include "datamanager.h"
#include <QtGui>
#include <QGraphicsScene>
namespace Ui {
class Graph;
}

class Graph : public QDialog
{
    Q_OBJECT

public:
    explicit Graph(QWidget *parent = 0);
    ~Graph();
    QGraphicsScene * scene;
    int COLPART;
    int SPACEPART;
    int HEIGHT;
    double TOTALSPACE;
    void setData(DataManager * data);

private slots:
    void on_graphTimeSelecter_currentIndexChanged(int index);
    void drawGraph(QList<statData> dataList);
    void graphPercent(QList<statData> dataList);
    void graphCards(QList<statData> dataList);
    void graphNew(QList<statData> dataList);
    void graphComplete(QList<statData> dataList4);
    void getDimensions();
    void setSpacers(int space);
    void graphTime(QList<statData> dataList);
    void doGraph(int index);
    void setSpacersTime(int space);
    void on_radioComplete_clicked();

    void on_radioAverage_clicked();

    void on_radioNew_clicked();

    void on_radioTimeSpent_clicked();

    void on_radioCardStrength_clicked();

private:
    Ui::Graph *ui;
    DataManager * db;
};

#endif // GRAPH_H
