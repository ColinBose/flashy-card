#ifndef ENCRYPTION_H
#define ENCRYPTION_H
#include <QString>
#include <QByteArray>
#define MAXSOCK 100000
#define MASTERLENGTH 5000
#define SEED 10
class Encryption
{
public:
    Encryption();
    void addSocket(int sock, int key);
    //void encrypt(char *buffer, QByteArray data, int sock, int len);
    QByteArray encrypt(QByteArray input, int sock);
    void decrypt(char *buffer, int sock, int len);
    void genKey(char * key, int sock, int len, bool send);
private:
    int sentLocation[MAXSOCK];
    int recvLocation[MAXSOCK];
    int keys[MAXSOCK];
    int MASTERKEY[MASTERLENGTH];

};

#endif // ENCRYPTION_H
