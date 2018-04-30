#include "multimanager.h"
#include "stdlib.h"
#include "math.h"
QString PACKDESC[] = {"EXPREQ", "EXPDAT", "DCKLST", "EXPRSP", "INPDAT", "DCKRSP", "IMPREQ",
                      "IMPDAT", "MLTREQ", "MLTRSP", "MLTNEW", "MLTJON", "MLTDAT", "MLTFLL",
                      "MLTCRT", "MLTANS", "MLTNXT", "USRJON", "USRLVE", "MLTEND", "INDTCK",
                      "MLTGNE", "USRDNE", "OUTCHT", "INTCHT", "GMEGSS", "GMEANS", "GMESTA",
                      "LOGREQ", "LOGRSP", "REGREQ", "REGRSP", "FRNADD", "FRNREM", "FRNUPD",
                      "FRNRSP", "PMSEND", "PMRECV", "WELCME"};
#define EXPDATA 1
#define DECKLIST 2
#define EXPRSP 3
#define INPDAT 4
#define DCKLISTRSP 5
#define IMPREQ 6
#define IMPDAT 7
#define MLTREQ 8
#define MLTRSP 9
#define MLTNEW 10
#define MLTJOIN 11
#define MLTDAT 12
#define MLTFULL 13
#define MLTCRT 14
#define MLTANS 15
#define MLTNEXT 16
#define USERJOIN 17
#define USERLEAVE 18
#define MLTEND 19
#define INDTICK 20
#define MLTGONE 21
#define USERDONE 22
#define OUTCHAT 23
#define INCHAT 24
#define GAMEGUESS 25
#define GAMEANSWER 26
#define GAMESTART 27
#define LOGIN 28
#define LOGINRESPONSE 29
#define REGISTER 30
#define REGISTERRESPONSE 31
#define ADDFRIEND 32
#define REMOVEFRIEND 33
#define FRIENDUPDATE 34
#define FRIENDRESPONSE 35
#define PMSEND 36
#define PMRECV 37
#define WELCOME 38


#define SENDDIFF 3
#define SENDMIN 2
//#define REALDECKID "FZYHPS"


MultiManager::MultiManager()
{
    sem_init(&inEvent,0,0);
    sem_init(&queueLock, 0, 1);
    sem_init(&eventLock, 0, 1);

    srand(time(NULL));
}
void MultiManager::pollOutEvents(){
    QTime cur;
    int rem;
    while(1){
    sem_wait(&queueLock);
        rem = 0;
        cur = QTime::currentTime();
        QList<outEvent> send;
        //qDebug() <<"Queue length: " + QString::number(outQueue.length());
        if(outQueue.length() > 0){
            while(outQueue[0].sendTime < cur){
                send.push_back(outQueue[0]);
                outQueue.removeAt(0);
                if(outQueue.length() == 0)
                    break;

            }
        }
    sem_post(&queueLock);
    //qDebug() << "Send length : " + QString::number(send.length());
        for(int i = 0; i < send.length();i ++){
           sendAnswer(send[i].sock, send[i].curCard, send[i].curRound);
        }
        send.clear();;
        usleep(50000);
    }

}
void MultiManager::addEvent(outEvent o){
    ////qDebug() << "Sock : " + QString::number(o.sock) + " Registered an event";

    sem_wait(&queueLock);
        if(outQueue.length() == 0){
            outQueue.push_back(o);
            sem_post(&queueLock);

            return;
        }
        int low = 0;
        int high = outQueue.length() -1;
        int index = -1;
        int lOrR;
        int mid = 1;
        while(low <= high){
            mid = low + (high - low)/2;
            if(outQueue[mid].sendTime == o.sendTime){
                index = mid;
                break;
            }
            else if(outQueue[mid].sendTime > o.sendTime){
               high = mid-1;
               lOrR = 0;
            }
            else{
                low = mid + 1;
                lOrR = 1;
            }

        }
        if(index == -1){
            outQueue.insert(low-1+lOrR, o);

        }else{
            outQueue.insert(index,o);
        }
    sem_post(&queueLock);
}
void MultiManager::pushEvent(QString packet, int socket){
    inputEvent e;
    e.packet = packet;
    e.sock = socket;
    sem_wait(&eventLock);
        events.push_back(e);
    sem_post(&eventLock);
    sem_post(&inEvent);
}
QStringList MultiManager::putData(char buff[], int len, int sock){
    return messages.putData(buff, len, sock);
}
void MultiManager::removeClient(int sd, bool remove){

    if(remove){
        epollRemove(sd);
        //messages.resetDataPointer(sd);
    }



}
void MultiManager::handleEvent(){
   //QString packet = messages.getData(sd);

    //decrypt and stuff here maybe


    sem_wait(&inEvent);
    sem_wait(&eventLock);
        inputEvent cur = events[0];
        events.pop_front();
    sem_post(&eventLock);
    QString packet = cur.packet;
    int sd = cur.sock;


    QStringList parts = packet.split("~");
    if(parts.length() == 0){
        ////qDebug() << "Bad packet";
        return;
    }
    else if(parts[0] == PACKDESC[WELCOME]){
        handleWelcome(packet, sd);
    }
    else if(parts[0] == PACKDESC[MLTRSP]){
        handleMultiResponse(packet, sd);
    }
    else if(parts[0] == PACKDESC[MLTCRT]){
        roomCreated(packet, sd);
    }
    else if(parts[0] == PACKDESC[MLTNEXT]){
        handleNextCard(packet,sd);
    }


}

void MultiManager::handleWelcome(QString packet, int sock){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    QString numPart = QString(parts[1]);
    int seed = parts[1].toInt();
    registerSocket(sock, seed);
    doJoinProcedure(sock);
}
void MultiManager::doJoinProcedure(int sock){
    QString deckID = REALDECKID;
    QString send = "";
    send += PACKDESC[MLTREQ];
    send += '~';
    send += deckID;
    send += '~';
    char buff[1000] = {0};
    int place, bit;
    int cards = 0;
    while(cards < 100){
        int cardNum = cards;
        place = cardNum / 7;
        bit = cardNum % 7;
        buff[place] += pow(2,bit);
        cards++;
    }
    for(int i = 0; i <= place; i++){
        buff[i] += 128;
    }
    QString bits = QString::fromLatin1(buff);

    send += bits;
    send += '\0';
    sendData(sock, send);


}
void MultiManager::handleMultiResponse(QString packet, int sock){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    QString currentRound = "0";
    if(parts[1].length() == 0){
        //create new room
        QString send;
        QString deckID = REALDECKID;
        send += PACKDESC[MLTNEW];
        send += '~';
        send += deckID;
        send += '~';
        send += QString::number(10);
        send += '~';
        send += QString::number(0);
        send += '~';
        send += QString::number(0);
        send += '~';
        char buff[1000] = {0};
        int place, bit;
        int cards = 0;
        while(cards < 100){
            int cardNum = cards;
            place = cardNum / 7;
            bit = cardNum % 7;
            buff[place] += pow(2,bit);
            cards++;
        }
        for(int i = 0; i <= place; i++){
            buff[i] += 128;
        }
        QString bits = QString::fromLatin1(buff);
        send += bits;
        send += '\0';
        sendData(sock, send);
    }else{
        //join room 1
        //qDebug() << "Joining room!!";
        QString deckID = REALDECKID;
        QString send = PACKDESC[MLTJOIN];
        QStringList rDat = parts[1].split('\t');
        if(rDat.length() < 6)
            return;
        send += '~';
        send += deckID;
        send += '~';
        send += rDat[1];
        send += '~';
        send += rDat[5];
        send += '\0';
        sendData(sock, send);
    }

}
void MultiManager::roomCreated(QString packet, int sock){
   // qDebug() << "Created a new room!!";
    registerNewAnswer(sock, "0", "0");
}
void MultiManager::handleNextCard(QString packet, int sock){

    QStringList parts = packet.split("~");
    if(parts.length() < 4){
        qDebug() << "Error with packet makeup";
        return;
    }
    QString cardNum = parts[1];
    QString currentRound = parts[3];
    //qDebug() << "SOCK : " + QString::number(sock) + " Card num; " + cardNum + " CurrentRound; " + currentRound;
    registerNewAnswer(sock, cardNum, currentRound);
}
void MultiManager::sendAnswer(int sock, QString cardNum, QString currentRound){
    //qDebug() << "SOCK : " + QString::number(sock) + " SENDING ANSWER";
    QString send;
    send += PACKDESC[MLTANS];
    send += '~';
    send += cardNum;
    send += '~';
    send += QString::number(88);
    send += '~';
    send += "0";
    send += '~';
    send += currentRound;
    send += '\0';
    sendData(sock, send);
}
void MultiManager::registerNewAnswer(int sock, QString cardNum, QString currentRound){
    outEvent o;
    QTime cur = QTime::currentTime();
    int delay = rand() % SENDDIFF;
    delay += SENDMIN;

  //  qDebug() << "Current Card Num: " + cardNum + " Current Round Num; " + currentRound;
    cur = cur.addSecs(1);
    o.curCard = cardNum;
    o.sendTime = cur;
    o.curRound = currentRound;
    o.sock = sock;
    addEvent(o);
}
void MultiManager::setFile(QString fileName){
    REALDECKID = fileName;
}
