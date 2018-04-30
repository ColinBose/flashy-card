#include "server.h"

int epoll_fd;
sem_t readReady;
MultiManager manager;
ReadEvents readQueue;
bool RUNNING = true;
#define MAXCLIENTS 50000
#define SERVER "127.0.0.1"
#define PORT "7000"
#define CONFIGFILE "conf.txt"
#define NUMUSERS 25000
QString serverIp;
QString serverPort;
static struct epoll_event events[MAXCLIENTS], event;

Server::Server()
{

}
void Server::startServer(int workers, int events){
    pthread_t acceptThr, pollThr, multiPoll, joiner;
    loadConfig();
    sem_init(&readReady,0,0);

    epoll_fd = epoll_create(MAXCLIENTS);
    if (epoll_fd == -1){
        qDebug() << "Error creating EPOLL";
        exit(1);
    }

   // pthread_create(&acceptThr, 0, epollAcceptThread, (void *)0);
    pthread_create(&pollThr, 0, pollThread, (void *)0);
    for(int i =0 ; i < workers; i++){
        pthread_t work;
        pthread_create(&work, 0, workerThread, (void *)0);
    }
    for(int i =0 ; i < events; i++){
        pthread_t work;
        pthread_create(&work, 0, eventHandler, (void *)0);
    }
    int numStudiers = NUMUSERS;
    pthread_create(&multiPoll, 0, startMultiPoll, (void *)0);
    pthread_create(&joiner, 0, joinerThread, (void *)&numStudiers);
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
               //exit here
                // manager.removeClient(events[i].data.fd, true);
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
void * joinerThread(void * args){
    int * num = (int *) args;
    int totalStudy = *num;
    for(int i = 0; i < totalStudy; i++){
        int sd = connectTCPSocket(serverPort.toInt(),(char *)serverIp.toStdString().c_str());
        if(sd == -1){
            qDebug() << "Error connecting to server";
            continue;
        }
        registerSocket(sd, 15);
        if(fcntl (sd, F_SETFL, O_NONBLOCK | fcntl(sd, F_GETFL, 0)) == -1)
            fprintf(stderr, "socket() failed: %s %d\n", strerror(errno), errno);
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = sd;

        if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, sd, &event) == -1)
            fprintf(stderr, "socket() failed: %s\n", strerror(errno));

        usleep(7500);

    }
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
void loadConfig(){
    QFile f(CONFIGFILE);

    if(!f.open(QIODevice::ReadOnly)){
        qDebug() << "Error loading config file";
        serverPort = PORT;
        serverIp = SERVER;
        return;
    }
    QString line = f.readLine();
    line = line.left(line.length()-1);
    serverIp = line;
    serverIp += '\0';
    line = f.readLine();
    line = line.left(line.length()-1);
    serverPort = line;
    line = f.readLine();
    line = line.left(line.length()-1);
    manager.setFile(line);

    f.close();
}
