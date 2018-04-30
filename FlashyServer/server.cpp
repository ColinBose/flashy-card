#include "server.h"
#define UTILITYWORKERS 4
int epoll_fd;
sem_t readReady;
sem_t utilitySem;
MultiManager manager;
ReadEvents readQueue;
bool utilitySigns[UTILITYWORKERS];
bool RUNNING = true;
static struct epoll_event events[MAXCLIENTS], event;
int currentUsers;

Server::Server()
{
    sem_init(&utilitySem,0,0);
}
void Server::startServer(int workers, int events){
    pthread_t acceptThr, pollThr, multiPoll, statPoll;
    sem_init(&readReady,0,0);

    epoll_fd = epoll_create(MAXCLIENTS);
    if (epoll_fd == -1){
        qDebug() << "Error creating EPOLL";
        exit(1);
    }
    events = 1;
    workers = 1;
    currentUsers = 0;
    pthread_create(&acceptThr, 0, epollAcceptThread, (void *)0);
    pthread_create(&pollThr, 0, pollThread, (void *)0);
    for(int i =0 ; i < workers; i++){
        pthread_t work;
        pthread_create(&work, 0, workerThread, (void *)0);
    }
    for(int i =0 ; i < events; i++){
        pthread_t work;
        pthread_create(&work, 0, eventHandler, (void *)0);
    }
    int passVals[UTILITYWORKERS];

    for(int i  = 0; i < UTILITYWORKERS; i++){
        passVals[i] = i;
        pthread_t util;
        pthread_create(&util, 0, utilityThread, (void *)&passVals[i]);
    }
    pthread_create(&multiPoll, 0, startMultiPoll, (void *)0);
    pthread_create(&statPoll, 0, statsPoll, (void *)0);

}

void * epollAcceptThread(void * args){
    struct	sockaddr_in server, client;
    int sd;
    socklen_t client_len;
    int z;

    sd = setupListen(PORT, &server);

    listen(sd, 5);
    client_len = sizeof(client);
    int dif;
    int someSock;
    while (1){
        if((someSock = accept (sd, (struct sockaddr *)&client, &client_len)) == -1){
            fprintf(stderr, "Can't accept client\n");
            fprintf(stderr, "socket() failed: %s\n", strerror(errno));
            exit(1);
        }
        if(fcntl (someSock, F_SETFL, O_NONBLOCK | fcntl(someSock, F_GETFL, 0)) == -1)
            fprintf(stderr, "socket() failed: %s %d\n", strerror(errno), errno);
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = someSock;
            manager.sendWelcomeMessage(someSock);
            currentUsers++;

        if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, someSock, &event) == -1)
            fprintf(stderr, "socket() failed: %s\n", strerror(errno));

    }

}


void * pollThread(void * args){
    int num_fds;
    while (1){
        num_fds = epoll_wait (epoll_fd, events, MAXCLIENTS, -1);
        if (num_fds < 0){
            qDebug() << "Error in epoll_wait!";
            fprintf(stderr, "socket() failed: %s\n", strerror(errno));
            fflush(stdout);

        }
        for (int i = 0; i < num_fds; i++){

                // Case 1: Error condition
            if (events[i].events & (EPOLLHUP | EPOLLERR)){
                manager.removeClient(events[i].data.fd, true);
                currentUsers--;
                //close(events[i].data.fd);
                //deal with leaving client
                continue;
            }
            if(events[i].events & EPOLLIN){
                readQueue.addEvent(events[i].data.fd);
                sem_post(&readReady);
            }
        }
    }
}
void epollRemove(int fd){

    close(fd);
}

void * workerThread(void * args){

    int sock;
    while(RUNNING){
        sem_wait(&readReady);
        sock = readQueue.getEvent();
        if(sock == -1)
            continue;
        readData(sock, &manager);

    }
}
void * eventHandler(void * args){
    while(RUNNING){
        manager.handleEvent();
    }
}
void * startMultiPoll(void *args){
    manager.pollOutEvents();
}
void * statsPoll(void * args){
    QTime lastThread = QTime::currentTime();
    int started = 0;
    while(RUNNING){
        std::pair<int,int> eventStuff = readQueue.getElapsed();
        std::pair<int, int> netCpu = manager.statTick(currentUsers, eventStuff.first, eventStuff.second);
        if(started < UTILITYWORKERS){
            if(lastThread < QTime::currentTime()){
                if(netCpu.first > 5){

                    //Network
                    utilitySigns[started] = true;
                    started++;
                    lastThread = QTime::currentTime();
                    lastThread = lastThread.addSecs(10);
                    sem_post(&utilitySem);
                }
                if(netCpu.second > 5){

                    //Cpu
                    utilitySigns[started] = false;
                    started++;
                    lastThread = QTime::currentTime();
                    lastThread = lastThread.addSecs(10);
                    sem_post(&utilitySem);
                }
            }
        }
        else{
            if(lastThread < QTime::currentTime()){
                if(netCpu.first - netCpu.second < -5){
                    //cpu lagging
                    lastThread = QTime::currentTime();
                    lastThread = lastThread.addSecs(10);
                    for(int i = 0; i < UTILITYWORKERS; i++){
                        if(utilitySigns[i]){
                            utilitySigns[i] = false;
                            break;
                        }
                    }
                }
                else if(netCpu.first - netCpu.second > 5){
                    //network lagging
                    lastThread = QTime::currentTime();
                    lastThread = lastThread.addSecs(10);
                    for(int i = 0; i < UTILITYWORKERS; i++){
                        if(!utilitySigns[i]){
                            utilitySigns[i] = true;
                            break;
                        }
                    }
                }
            }
        }

        sleep(1);
    }
}
void Server::passMw(MainWindow *m){
    manager.setMw(m);
}
void * utilityThread(void *args){
    int num = *(int *)args;
    int sock;
    qDebug() << "Utility #: " + QString::number(num);
    sem_wait(&utilitySem);
    qDebug() << "Utility Thread Started";
    if(utilitySigns[num])
        qDebug() << "Network Thread";
    else
        qDebug() << "CPU Thread";

    while(RUNNING){
        if(utilitySigns[num]){
            sem_wait(&readReady);
            sock = readQueue.getEvent();
            if(sock == -1)
                continue;
            readData(sock, &manager);
        }else{
            manager.handleEvent();
        }
    }
}
