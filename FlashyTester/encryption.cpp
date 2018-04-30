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
}
/*void Encryption::encrypt(char * buffer, QByteArray data, int sock, int len){
    int realLen = data.length();
    if(realLen > 512)
        realLen = 512;
    memcpy(buffer, data.data(), realLen);
    memset(buffer + realLen, '\0', len - realLen);
    char * thisKey = (char *)malloc(len);
    genKey(thisKey, sock, len, true);
   // qDebug() << "Encrypting with key: " + QString(thisKey);
   // qDebug() << "Encrypt Length = " + QString::number(len);
    unsigned int left, right;
    for(int i = 0; i < len; i++){
        //get halves
        unsigned char c;
        c = buffer[i];
        left = c/16;
        right = c%16;
        //swap halves
        buffer[i] = left + right*16;
        //xor
        //char c = buffer[i];
        c = buffer[i];
        c ^= thisKey[i];
        char t = thisKey[i];
        buffer[i] = c;
    }
    free(thisKey);

}*/
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

/*QByteArray Encryption::encrypt(QByteArray input, int sock, int len){

    int dataLen = input.length();
    char * thisKey = (char *)malloc(len);
    char buffer[512] = {0};
    memcpy(&buffer,input.data(), dataLen);
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
*/
void Encryption::decrypt(char * buffer, int sock, int len){
    char * thisKey;// = (char *)malloc(len);
    char keyBuff[512] = {0};
    thisKey = keyBuff;
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
    //free(thisKey);
}

void Encryption::genKey(char *key, int sock, int len, bool send){
    int skip = keys[sock];
    int start;
    if(send)
        start = sentLocation[sock];
    else
        start = recvLocation[sock];



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
