#include "statinfo.h"

StatInfo::StatInfo()
{
    strong = 0;
    learning = 0;
    longLearning = 0;
    longStruggle = 0;
    newStrong = 0;
    newWeak = 0;
}

void StatInfo::getCardLevels(QList<statCard> cards){
    int level;
    for(int i = 0; i < cards.length(); i++){
        setLevel(cards[i].avaerage, cards[i].interval, cards[i].numDone);
    }
}
void StatInfo::setLevel(int average, int interval, int attempts){
    if(average >= 90){
        if(interval > 15){
            strong++;
            return;
        }
        else{
            newStrong++;
            return;
        }
    }
    else if(average >= 75){
        if(interval > 15){
            learning++;
            return;
        }
        else{
            longLearning++;
            return;
        }
    }
    else{
        if(attempts > 10){
            longStruggle++;
            return;
        }
        else{
            newWeak++;
            return;
        }
   }
}

int StatInfo::getLevel(int average, int interval, int attempts){
    if(average >= 90){
        if(interval > 15){
            //strong
            return 0;
        }
        else{
            //newStrong
            return 1;
        }
    }
    else if(average >= 75){
        if(interval > 15){
            //learning
            return 2;
        }
        else{
            //longLearning
            return 3;
        }
    }
    else{
        if(attempts > 10){
            //longStruggle
            return 4;
        }
        else{
            //newWeak
            return 5;
        }
   }
}

