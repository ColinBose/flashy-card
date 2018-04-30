#include "card.h"
#include "koreancard.h"
Card::Card()
{

}
Card::Card(QString code, QString front, QString back, QString audio, QString hint, int interval, int past, int numDone, int stage, QString type, int num){
    this->code = code;
    this->front = front;
    this->back = back;
    this->audio = audio;
    this->hint = hint;
    this->stage = stage;
    this->type = type;
    this->interval = interval;
       this->past = past;
    this->numDone = numDone;
    if(this->numDone > 0)
        newCard = false;
    else
        newCard = true;
    this->lastTry = QDateTime::currentDateTime();
    this->score = 100;
    this->prevDone = false;
    initLevel = -1;
    cardNum = num;
    this->level = genLevel(this->past, this->interval, this->numDone);
}
Card::Card(QString test){
    this->front = test;
}


//0 = best, 5 = worst;
int
genLevel(int avg, int interval, int attempts){
    if(attempts == 0)
        return 6;
    if(avg >= 90){
        if(interval > 15){
            return 0;
        }
        else{
            return 1;
        }
    }
    else if(avg >= 75){
        if(interval > 15){
            return 2;
        }
        else{
            return 3;
        }
    }
    else{
        if(attempts > 10){
            return 4;
        }
        else{
            return 5;
        }
   }

}

