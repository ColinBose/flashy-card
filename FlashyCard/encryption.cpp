#include "encryption.h"
#include <QDebug>
Encryption::Encryption()
{
    srand(SEED);
    for(int i = 0; i < MASTERLENGTH; i++){
        MASTERKEY[i] = rand() % 256;
    }
}
void Encryption::addSocket(int sock, int key){
    keys[sock] = key;
    sentLocation[sock] = 0;
    recvLocation[sock] = 0;
    qDebug() << "Encryption KEY IS SET TO : " + QString::number(key);
}
QByteArray Encryption::encrypt(QByteArray input, int sock){

    int len = input.length();
    char * thisKey = (char *)malloc(len);
   // qDebug() << "Encrypting with key: " + QString(thisKey);
   // qDebug() << "Encrypting Length = " + QString::number(len);
    genKey(thisKey, sock, len, true);
    unsigned int left, right;
    for(int i = 0; i < len; i++){
        //get halves
        unsigned char c;
        c = input[i];
        left = c/16;
        right = c%16;
        //swap halves
        input[i] = left + right*16;
        //xor
        //char c = input[i];
        c = input.at(i);
        c ^= thisKey[i];
        char t = thisKey[i];
        input[i] = c;
    }
    free(thisKey);
    return input;
}
void Encryption::decrypt(char * buffer, int sock, int len){
    char * thisKey = (char *)malloc(len);
    genKey(thisKey, sock, len, false);
   // qDebug() << "Decrypting with key: " + QString(thisKey);
   // qDebug() << "Decrypting Length = " + QString::number(len);
    unsigned int left, right;
    for(int i = 0; i < len; i++){
        unsigned char c = buffer[i];
        buffer[i] = c ^= thisKey[i];
        //get halves
        c = buffer[i];
        left = c/16;
        right = c%16;
       // left = floor(buffer[i]/16);
        //right = buffer[i] % 16;
        //swap halves
        buffer[i] = left + right*16;

    }
    free(thisKey);
}
/*
QByteArray Encryption::decrypt(QByteArray input, int sock){
    int len = input.length();
    char * thisKey = (char *)malloc(len);
    genKey(thisKey, sock, len, false);
    unsigned int left, right;
    for(int i = 0; i < len; i++){
        unsigned char c = input[i];
        input[i] = c ^= thisKey[i];
        //get halves
        c = input[i];
        left = c/16;
        right = c%16;
       // left = floor(input[i]/16);
        //right = input[i] % 16;
        //swap halves
        input[i] = left + right*16;

    }
    free(thisKey);
    return input;
}
*/
void Encryption::genKey(char *key, int sock, int len, bool send){
    int skip = keys[sock];
    int start;
    if(send)
        start = sentLocation[sock];
    else
        start = recvLocation[sock];

    if(send){
        qDebug() << "Encrypting with start location: " + QString::number(start);
    }
    else{
        qDebug() << "Decrypting with start location: " + QString::number(start);
    }
    for(int i = 0; i < len; i++){
        key[i] = MASTERKEY[start % MASTERLENGTH];
        start += skip;
    }
    start = start % MASTERLENGTH;
    if(send)
        sentLocation[sock] = start;
    else
        recvLocation[sock] = start;
}
