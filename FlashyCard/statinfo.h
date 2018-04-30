#ifndef STATINFO_H
#define STATINFO_H
#include <QStringList>
#include <QTime>
struct statCard{
    int avaerage;
    int interval;
    int numDone;
};
class StatInfo
{


public:
    StatInfo();

    void getCardLevels(QList<statCard> cards);
    int getLevel(int average, int interval, int attempts);

    int strong, learning, longLearning, longStruggle, newStrong, newWeak;
    int correct, attempted, newCards, pastAvg;
    QTime time;
private:
    void setLevel(int average, int interval, int attempts);
};

#endif // STATINFO_H
