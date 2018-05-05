#ifndef MULTIMANAGER_H
#define MULTIMANAGER_H
#include <QTime>
#include <semaphore.h>
#include <unistd.h>
#include "network.h"
#include "messagebuffer.h"
struct outEvent{
    QString curRound;
    QString curCard;
    QTime sendTime;
    int sock;
};
struct inputEvent{
    QString packet;
    int sock;

};
class MultiManager
{
public:
    MultiManager();
    void pollOutEvents();
    void addEvent(outEvent o);
    QStringList putData(char buff[], int len, int sock);
    void pushEvent(QString packet, int socket);
    void handleEvent();
    void removeClient(int sd, bool remove);
    void setFile(QString fileName);
    void setDelay(int d);
private:
    QList<outEvent> outQueue;
    sem_t queueLock;
    int DELAY = 3;
    QList<inputEvent> events;
    sem_t inEvent;
    sem_t eventLock;
    MessageBuffer messages;
    QString REALDECKID = "AAAAAA";
    sem_t waitLock;
    void handleWelcome(QString packet, int sock);
    void doJoinProcedure(int sock);
    void handleMultiResponse(QString packet, int sock);
    void roomCreated(QString packet, int sock);
    void handleNextCard(QString packet, int sock);
    void registerNewAnswer(int sock, QString cardNum, QString currentRound);
    void sendAnswer(int sock, QString cardNum, QString currentRound);
    void signalReady();
};

#endif // MULTIMANAGER_H
