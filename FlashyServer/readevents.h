#ifndef READEVENTS_H
#define READEVENTS_H
#include <QTime>
#include <semaphore.h>
struct node{
    node * next;
    int fd;
    long ms = 0;
};

class ReadEvents
{
public:
    ReadEvents();
    void addEvent(int fd);
    int getEvent();
    std::pair<int,int> getElapsed();

private:
    node * head;
    node * tail;
    long totalDelay = 0;
    int totalMessages = 0;
    sem_t lock;
};

#endif // READEVENTS_H
