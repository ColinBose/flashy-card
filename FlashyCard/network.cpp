/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Network.cpp
--
-- PROGRAM: FileTransfer
--
-- FUNCTIONS:
        int sendData(int socket, char * message, int buffsize);
        int connectTCPSocket(int port, char * ip);
        int readSock(int sd, int buffSize, char * buff);
        int setupListen(int port, sockaddr_in * server);
--
-- DATE: 10/03/2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Colin Bose
--
-- PROGRAMMER: Colin Bose
--
-- NOTES: Provides network functions for file transfer - Setting up send/listen sockets and sending data
----------------------------------------------------------------------------------------------------------------------*/
#include "network.h"
#include <errno.h>
#include <QDebug>
#include "encryption.h"
Encryption e;
int totalSent = 0;
int totalErrs = 0;
Network::Network()
{
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendData
--
-- DATE: 10/03/2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Colin Bose
--
-- PROGRAMMER: Colin Bose
--
-- INTERFACE: int sendData(int socket, char * message, int buffSize
                        int socket - sokcet to send to
                        char * message - buffer to write to the socket
                        int buffSize - size of the buffer
--
-- RETURNS: int
--
-- NOTES: Call to send a buffer of data to the specified socket
----------------------------------------------------------------------------------------------------------------------*/

int sendData(int socket, QString message){
    int ret;
    if(message[message.length()-1] != '\0')
        message += '\0';
    QByteArray data = message.toLocal8Bit();
    int sent = data.length();

    data = e.encrypt(data,socket);
    int m = 0;
    int times = 0;

   // memcpy(&message, data.data(), buffSize);
    while (sent > 0) {
           if ( (ret = write(socket, data.data() + m, sent)) <= 0) {
               if (ret < 0 && errno == EINTR)
                   ret = 0;       /* and call write() again */
               else
                   return(-1);         /* error */
           }

           sent -= ret;
           m += ret;
       }



}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connectTCPSocket
--
-- DATE: 10/03/2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Colin Bose
--
-- PROGRAMMER: Colin Bose
--
-- INTERFACE: int connectTCPSocket(int port, char * ip)
--
-- RETURNS: -1 for failure, or a socket descriptor on success
--
-- NOTES: Call to attempt to connect to a specified ip/port. Returns socket on success
----------------------------------------------------------------------------------------------------------------------*/
int connectTCPSocket(int port, char * ip){
    int sock;
    struct hostent  * host;
    struct sockaddr_in server;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Cannot create socket");
        return -1;
    }
    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if ((host = gethostbyname(ip)) == NULL){
        fprintf(stderr, "Unknown server address\n");
        return -1;
    }
    bcopy(host->h_addr, (char *)&server.sin_addr, host->h_length);
    if (connect (sock, (struct sockaddr *)&server, sizeof(server)) == -1){
        fprintf(stderr, "Can't connect to server\n");
        printf("socket() failed: %s\n", strerror(errno));

        return -1;
    }

    return sock;

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readSock
--
-- DATE: 10/03/2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Colin Bose
--
-- PROGRAMMER: Colin Bose
--
-- INTERFACE: int readSock(int sd, int buffSize, char * buff)
                    int sd - socket to read
                    int buffSize - size of buffer
                    char * buff - buffer to write to
--
-- RETURNS: returns 0 on client disconnect
--
-- NOTES: Call to read a specified amount of data from socket and stores it in buffer
----------------------------------------------------------------------------------------------------------------------*/


int readSock(int sd, int buffSize, char * buff){
    int n;
    memset(buff, 0, buffSize);
    //("Reading SOCKET: %d \n", sd);
    //fflush(stdout);
    int bytesLeft = buffSize;
    while((n = recv(sd, buff, bytesLeft, 0)) < buffSize){
        if(n == -1){
            if(errno == EAGAIN)
                continue;

            return -1;
        }
        if(n == 0){
            return 0;


        }
    buff += n;
    bytesLeft -= n;
    if(bytesLeft ==0)
        return 1;

  }

  return 1;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setupListen
--
-- DATE: 10/03/2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Colin Bose
--
-- PROGRAMMER: Colin Bose
--
-- INTERFACE: setupListen(int port, sockaddr_in * server)
                            int port - port to bind to
                            sockaddr_in * server - sockaddr structure to populate
--
-- RETURNS: returns bound socket or -1 on fail
--
-- NOTES: Call to bind a tcp socket
----------------------------------------------------------------------------------------------------------------------*/

int setupListen(int port, sockaddr_in * server){

    int sock;



    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("Can't create a socket");
        return -1;
    }
    int z;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &z, sizeof(int)) < 0)
        qDebug() << "setsockopt(SO_REUSEADDR) failed";
    // Bind an address to the socket
    bzero((char *)server, sizeof(struct sockaddr_in));
    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    server->sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

    if (bind(sock, (struct sockaddr *)server, sizeof(*server)) == -1)
    {
        printf("socket() failed: %s\n", strerror(errno));

        perror("Can't bind name to socket");
        exit(1);
    }
    return sock;
}
int  setUdp(int port, const char * servIp, sockaddr_in * serveraddr){
    int sd;
   if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
       perror ("Can't create a socket\n");
       exit(1);
    }
    memset((char *) serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr->sin_family = AF_INET;
    serveraddr->sin_port = htons(port);
    if (bind(sd, (struct sockaddr*)serveraddr, sizeof(*serveraddr))==-1){
        printf("error binding");
        printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));

        fflush(stdout);
    }

    if (inet_aton(servIp, &serveraddr->sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    return sd;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readSock
-- DATE: 29/10/2016
-- REVISIONS: (Date and Description)
-- DESIGNER: Colin Bose
-- PROGRAMMER: Colin Bose
-- INTERFACE: readSock()
-- RETURNS: int - data read or error no
--
-- NOTES: Call to read a udp socket and store data in given buffer
----------------------------------------------------------------------------------------------------------------------*/

int readSock(int sd, int buffSize, char * buff, sockaddr_in * serveraddr){
    socklen_t slen = sizeof(*serveraddr);
    int err;
    if (err = (recvfrom(sd, buff, buffSize, 0, (struct sockaddr*)serveraddr, &slen))< 0){
        printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));

        return 0;
    }
       return 1;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendDataTo
-- DATE: 29/10/2016
-- REVISIONS: (Date and Description)
-- DESIGNER: Colin Bose
-- PROGRAMMER: Colin Bose
-- INTERFACE:
-- RETURNS: int - 0 = error
--
-- NOTES: call to send UDP socket
----------------------------------------------------------------------------------------------------------------------*/

int sendDataTo(int sd, int buffSize, char * buff, sockaddr_in * serveraddr){
    int err = 1;
    if(err = (sendto(sd, buff, buffSize, 0, (struct sockaddr*)serveraddr, sizeof(*serveraddr))) < 0){
        printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));

        return 0;
    }



     return err;
}


QString readResponse(int sock){
    char buffer[STDMESSAGE];
    int remain = STDMESSAGE;
    int err = 0;
    int total = 0;
    while((err = recv(sock, &buffer[total], remain, 0)) > 0){
        if(err == 0)
            return QString("D");
        else if(err <0)
            return QString("E");
        remain -= err;
        total += err;
        if(total == STDMESSAGE)
            break;
    }
    return QString(buffer);


}
void registerSocket(int sock, int seed){
    e.addSocket(sock, seed);
}

QString readUntilEmpty(int sock){
    char buffer[STDMESSAGE] = {0};
    QByteArray bytes;
    int err;
    while((err = recv(sock, &buffer, STDMESSAGE,0 ))){
        if(err == 0)
            return QString("D");
        else if(err<0)
            return QString("E");

            //QByteArray bytes();
        e.decrypt(buffer, sock, err);
        for(int i = 0; i < err; i++)
            bytes.append(buffer[i]);
        if(buffer[err-1] == '\0'){
            QString ret = QString::fromUtf8(bytes);
            return ret;
        }
        memset(buffer, '\0', STDMESSAGE);
    }
    return "D";
}
