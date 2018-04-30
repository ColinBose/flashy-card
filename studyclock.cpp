#include "studyclock.h"
#include <chrono>
#include <thread>
#define TIMEOUT -7
StudyClock::StudyClock(MainWindow * main)
{
    mw = main;
    running = false;
}
void StudyClock::startClock(){

    running = true;
    QDateTime curTime;
    while(running){
        curTime = QDateTime::currentDateTime();
        curTime = curTime.addSecs(TIMEOUT);
        if(curTime < lastPressed || !forward){
            if(forward)
                QMetaObject::invokeMethod(mw,"addSecToClock", Qt::QueuedConnection);
            else
                QMetaObject::invokeMethod(mw,"removeSecFromClock", Qt::QueuedConnection);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
void StudyClock::stopClock(){
    running = false;
}
void StudyClock::keyPushed(){
    lastPressed = QDateTime::currentDateTime();

}

void StudyClock::setDown(){
    forward = false;
}
