#ifndef ENCRYPTION_H
#define ENCRYPTION_H
#include <QString>
#include <QByteArray>
#include <semaphore.h>
#define MAXSOCK 200000
#define MASTERLENGTH 5000
#define SEED 10
class Encryption
{
public:
    Encryption();
    void addSocket(int sock, int key);
    void encrypt(char *buffer, QByteArray data, int sock, int len);
    void decrypt(char *buffer, int sock, int len);
    void genKey(char * key, int sock, int len, bool send);
private:
    int sentLocation[MAXSOCK];
    int recvLocation[MAXSOCK];
    int keys[MAXSOCK];
    int MASTERKEY[MASTERLENGTH];
    sem_t encLock;

};

#endif // ENCRYPTION_H
