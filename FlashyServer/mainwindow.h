#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "server.h"
#include <QMainWindow>
#include <QGraphicsScene>
#define GRAPHSTEPS 10
class Server;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateDisplay(int rooms, int users, int networkMs, int cpuMs, int totalPackets);
private slots:
    void on_startServer_clicked();


private:
    Ui::MainWindow *ui;
    Server * serv;
    int networkTimes[GRAPHSTEPS];
    int cpuTimes[GRAPHSTEPS];

    QGraphicsScene * networkScene;
    QGraphicsScene * cpuScene;
    QBrush brushes;
    QPen pens[3];
    QPen pens2;
    int totalDraws = 0;
    void setGraphics();
    void drawGraph(bool network, QGraphicsScene *scene, int numArray[]);
    void setGraphLabels(int r, bool network);
    int getColour(int r);
};

#endif // MAINWINDOW_H
