#ifndef STUDYCLOCK_H
#define STUDYCLOCK_H
#include "mainwindow.h"
class MainWindow;
class StudyClock
{
public:
    StudyClock(MainWindow * main);
    void startClock();
    void keyPushed();
    void stopClock();
private:
    QDateTime lastPressed;
    MainWindow * mw;
    bool running;
};

#endif // STUDYCLOCK_H
