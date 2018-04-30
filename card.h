#ifndef CARD_H
#define CARD_H
#include "globalincludes.h"

class Card
{
public:
    Card();
    Card(QString test);
    Card(QString code, QString front, QString back, QString audio, QString hint, int interval, int past, int numDone, int stage, QString type, int num);
    QString code, front, back, audio, hint, type;
    QDate due;

    QDateTime lastTry;
    bool first;
    int interval, past, numDone;
    int score;
    int stage;
    bool newCard;
    bool prevDone;
    int initLevel;
    int cardNum;
    int level;

};

int genLevel(int avg, int interval, int attempts);
#endif // CARD_H
