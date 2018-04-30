#ifndef READEVENTS_H
#define READEVENTS_H

#include <semaphore.h>
struct node{
    node * next;
    int fd;
};
class ReadEvents
{
public:
    ReadEvents();
    void addEvent(int fd);
    int getEvent();
private:
    node * head;
    node * tail;
    sem_t lock;
};

#endif // READEVENTS_H
