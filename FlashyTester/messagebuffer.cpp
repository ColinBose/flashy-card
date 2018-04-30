#include "messagebuffer.h"
MessageBuffer::MessageBuffer()
{
    memset(&buffer, 'Z', MAXCLIENTS * MAXMESSAGE);
    memset(&pointerLocation, 0, MAXCLIENTS * sizeof(int));
}
QStringList MessageBuffer::putData(char buff[], int len, int sock){
    int copyLength = MAXMESSAGE - pointerLocation[sock];
    int remainingDataLen = 0;
    if(copyLength < len){
        //have parts of two different packets
        remainingDataLen = len - copyLength;
        len = copyLength;
    }
    memcpy(buffer+ (MAXMESSAGE*sock + pointerLocation[sock]), buff, len);
    pointerLocation[sock] += len;
    QStringList retPackets;
    int endData =1;
    while(endData != 0){
        if(pointerLocation[sock] == 0)
            break;

        //Gets the first occurance of '\0' - finds the end of the packet - if endData = 0 then no end was found
        QString indvPacket  = getPacketData(buffer + (MAXMESSAGE*sock), pointerLocation[sock], &endData);
        if(indvPacket.length() > 0)
            retPackets.push_back(indvPacket);
        //A packet was found, so lets see if there is more data?
        if(endData != 0){
            //Move the socket pointer back by the length of the found data
            pointerLocation[sock] -= endData;
            //Copy the back half of the message(second packet), to the front of the buffer
            memcpy(buffer + MAXMESSAGE*sock, buffer + (MAXMESSAGE*sock + endData), pointerLocation[sock]);

            //copy the rest of the buffer into the buffer...generally this will do absolutely nothing.
            memcpy(buffer + (MAXMESSAGE*sock+pointerLocation[sock]), buff, remainingDataLen);
            pointerLocation[sock] += remainingDataLen;

        }
    }

    return retPackets;

}
QString MessageBuffer::getData(int sd){
    char buff[MAXMESSAGE] = {0};
    memcpy(&buffer, buffer + (MAXMESSAGE * sd), pointerLocation[sd]);
    pointerLocation[sd] = 0;
    QString ret(buffer);
    return ret;
}
QString MessageBuffer::getPacketData(char * buffer, int len, int * end){
    int last = 0;
    QString ret;

    char buff[MAXMESSAGE];
    for(int i = 0; i < len; i++){
        if(buffer[i] == '\0'){
            last = i+1;
            memcpy(&buff, buffer, last);
            QString str(buff);
            ret = str;
            break;
        }
    }
    *end = last;
    return ret;
}
void MessageBuffer::resetDataPointer(int sd){
    pointerLocation[sd] = 0;
}
