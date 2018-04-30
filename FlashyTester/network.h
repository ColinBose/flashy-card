#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "server.h"
#include "multimanager.h"
#include "encryption.h"
#define STANDARDLENGTH 512
#define MAXMESSAGE 1024
#define PORT 7000
class MultiManager;
class Network
{
public:
    Network();
};
void setUdp(int port, const char * servIp);
int read_echo_to_client (int filedes);
//int sendData(int socket, const char *message, int buffsize);
int sendData(int socket, QString message);
int  setUdp(int port, const char * servIp, sockaddr_in * serveraddr);
int connectTCPSocket(int port, char * ip);
int readSock(int sd, int buffSize, char * buff);
int sendDataTo(int sd, int buffSize, char * buff, sockaddr_in * serveraddr);
int setupListen(int port, sockaddr_in * server);
int readSock(int sd, int buffSize, char * buff, sockaddr_in * serveraddr);
int readData(int sd, MultiManager  * manager);
int sendStandardResponse(int sock, const char message[], int len);
int sendAllData(int sock, QString message);
void registerSocket(int sock, int seed);
#endif // NETWORK_H
