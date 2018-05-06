#ifndef MESSAGEBUFFER_H
#define MESSAGEBUFFER_H
#define MAXCLIENTS 200000
#define MAXMESSAGE 1024
#include <QString>
#include <stdio.h>
#include <QStringList>
#include <semaphore.h>
class MessageBuffer
{
public:
    MessageBuffer();
    QString getData(int sd);
    QStringList putData(char buff[], int len, int sock);
    QString getPacketData(char * buffer, int len, int *end);
    void resetDataPointer(int sd);
private:
    char buffer[MAXCLIENTS * MAXMESSAGE];
    int pointerLocation[MAXCLIENTS];
    sem_t dataLock;

};

#endif // MESSAGEBUFFER_H
