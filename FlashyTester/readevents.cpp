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
