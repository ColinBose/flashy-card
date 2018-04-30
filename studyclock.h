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
    void setDown();
private:
    QDateTime lastPressed;
    MainWindow * mw;
    bool forward = true;
    bool running;
};

#endif // STUDYCLOCK_H
