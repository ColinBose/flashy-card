#include "readevents.h"
#define NULL 0


ReadEvents::ReadEvents()
{
    sem_init(&lock,0,1);
    head = NULL;
    tail = NULL;
}
void ReadEvents::addEvent(int fd){
    sem_wait(&lock);
    node * n = new node();
    n->fd = fd;
    n->next = NULL;
    n->ms = QDateTime::currentMSecsSinceEpoch();
    if(head == NULL){
        head = n;
        tail = n;
    }
    else{
        tail->next = n;
        tail = n;
    }
    sem_post(&lock);
}
int ReadEvents::getEvent(){
    sem_wait(&lock);
    if(head == NULL){
        sem_post(&lock);

        return -1;
    }
    int ret = head->fd;
    int elapsed = QDateTime::currentMSecsSinceEpoch() - head->ms;
    totalMessages++;
    totalDelay += elapsed;
    if(head->next == NULL){
        head = NULL;
        tail = NULL;
    }
    else{
        head = head->next;
    }
    sem_post(&lock);
    return ret;
}
std::pair<int, int> ReadEvents::getElapsed(){
    int hold = totalDelay;
    int holdMess = totalMessages;
    totalDelay = 0;
    totalMessages = 0;
    std::pair<int, int> ret;
    ret.first = holdMess;
    ret.second = hold;
    return ret;
}
