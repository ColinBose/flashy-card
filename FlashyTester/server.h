#ifndef SERVER_H
#define SERVER_H
#include <pthread.h>
#include <QDebug>
#include <semaphore.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "network.h"
#include <fcntl.h>
#include <assert.h>
#include "limits.h"
#include "readevents.h"
#include "multimanager.h"
#include <QFile>

class Server
{
public:
    Server();
    void startServer(int workers, int events);
    void ff();
private:
};
void * workerThread(void * args);
void * epollAcceptThread(void * args);
void * pollThread(void * args);
void * eventHandler(void * args);
void * startMultiPoll(void * args);
void epollRemove(int fd);
void * joinerThread(void * args);
void loadConfig();
#endif // SERVER_H
