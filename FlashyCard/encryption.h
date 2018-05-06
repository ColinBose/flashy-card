#ifndef ENCRYPTION_H
#define ENCRYPTION_H
#include <QString>
#include <QByteArray>
#define MAXSOCK 200000
#define MASTERLENGTH 5000
#define SEED 10
class Encryption
{
public:
    Encryption();
    void addSocket(int sock, int key);
    QByteArray encrypt(QByteArray input, int sock);
   // QByteArray decrypt(QByteArray input, int sock);
    void decrypt(char * buffer, int sock, int len);
    void genKey(char * key, int sock, int len, bool send);
private:
    int sentLocation[MAXSOCK];
    int recvLocation[MAXSOCK];
    int keys[MAXSOCK];
    int MASTERKEY[5000];

};

#endif // ENCRYPTION_H
