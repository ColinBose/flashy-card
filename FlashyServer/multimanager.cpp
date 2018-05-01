#include "multimanager.h"
QString PACKDESC[] = {"EXPREQ", "EXPDAT", "DCKLST", "EXPRSP", "INPDAT", "DCKRSP", "IMPREQ",
                      "IMPDAT", "MLTREQ", "MLTRSP", "MLTNEW", "MLTJON", "MLTDAT", "MLTFLL",
                      "MLTCRT", "MLTANS", "MLTNXT", "USRJON", "USRLVE", "MLTEND", "INDTCK",
                      "MLTGNE", "USRDNE", "OUTCHT", "INTCHT", "GMEGSS", "GMEANS", "GMESTA",
                      "LOGREQ", "LOGRSP", "REGREQ", "REGRSP", "FRNADD", "FRNREM", "FRNUPD",
                      "FRNRSP", "PMSEND", "PMRECV"};
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


#define MINMATCHES 30
#define MAXMATCHES 15
#define GAMELENGTH 60
#define MAXINDEPENDANT 5

struct inputEvent{
    QString packet;
    int sock;
    int ms;
};


QList<inputEvent> events;
MultiManager::MultiManager()
{
    sem_init(&inEvent,0,0);
    sem_init(&eventLock, 0, 1);
    sem_init(&queueLock, 0, 1);
    srand(time(NULL));

}
QStringList MultiManager::putData(char buff[], int len, int sock){
    return messages.putData(buff, len, sock);
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
    if(parts.at(0) == PACKDESC[0]){
        //Export Request
        handleExportRequest(sd, parts);

    }
    else if(parts.at(0) == PACKDESC[1]){
        //Card data coming in from an export
        handleExportData(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[2]){
        //Card data coming in from an export
        sendDeckList(sd);
    }
    else if(parts.at(0) == PACKDESC[IMPREQ]){
        sendCardData(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[MLTREQ]){
        handleMultiRequest(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[MLTNEW]){
        createMultiSession(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[MLTJOIN]){
        handleStudyJoin(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[MLTANS]){
        handleAnswer(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[USERLEAVE]){
        handleLeavingClient(sd);
    }
    else if(parts.at(0) == PACKDESC[INDTICK]){
        handleIndTick(sd);
    }
    else if(parts.at(0) == PACKDESC[USERDONE]){
        handleDoneUser(sd);
    }
    else if(parts.at(0) == PACKDESC[OUTCHAT]){
        handleChat(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[GAMEGUESS]){
        handleGameGuess(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[LOGIN]){
        handleLogin(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[REGISTER]){
        handleRegister(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[ADDFRIEND]){
        handleAddFriend(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[REMOVEFRIEND]){
        handleRemoveFriend(sd, parts);
    }
    else if(parts.at(0) == PACKDESC[PMSEND]){
        handlePm(sd, parts);
    }
    totalEventTime += QDateTime::currentMSecsSinceEpoch() - cur.ms;
    totalEvents++;

}
void MultiManager::handleExportRequest(int sock, QStringList parts){
    //invalid data;
    if(parts.length() < 4)
        return;
    QString deck, desc, lang, deckCode, user;

    deck = parts[1];
    desc = parts[2];
    lang = parts[3];
    user = userList[sock];
    if(user.length() == 0)
        user = "Anonymous";
    deckCode = data.addDeck(deck, desc, lang, user);
    if(deckCode.length() == 0)
        return;
    deckCode += '\0';
    QString resp = PACKDESC[EXPRSP];
    resp += '~';
    resp += deckCode;
    sendAllData(sock, resp);
    //int err = sendStandardResponse(sock,resp.toStdString().c_str(), resp.length());

}
void MultiManager::handleExportData(int sock, QStringList parts){
    if(parts.length() < 3)
        return;
    QString deck, deckID;
    deck = parts[1];
    deckID = parts[2];

    data.addCards(deckID, parts);


}
void MultiManager::pushEvent(QString packet, int socket){
    inputEvent e;
    e.packet = packet;
    e.sock = socket;
    e.ms = QDateTime::currentMSecsSinceEpoch();
    sem_wait(&eventLock);
        events.push_back(e);
    sem_post(&eventLock);
    sem_post(&inEvent);
}
void MultiManager::random(){
    qDebug() << "I hope this works soon";


}
void MultiManager::sendDeckList(int sock){
    QStringList decks= data.getDeckList();
    QString sendString;
    sendString += PACKDESC[DCKLISTRSP];
    sendString += '~';
    for(int i = 0; i < decks.length(); i++){
        sendString+= decks[i];
        sendString+= "~";
    }
    sendString+= '\0';
    sendAllData(sock, sendString);
    /*QByteArray realData = sendString.toLocal8Bit();
    int length = realData.length();
    int sent = 0;
    char buff[STANDARDLENGTH];
    while(sent < length){
        memset(buff, '\0', STANDARDLENGTH);
        int maxRead = length - sent;
        if(maxRead > STANDARDLENGTH)
            maxRead = STANDARDLENGTH;
        memcpy(&buff, realData.constData() + sent, maxRead);
        sent+= maxRead;

        int err = sendStandardResponse(sock, realData.toStdString().c_str(), maxRead);

    }*/
}
void MultiManager::sendCardData(int sock, QStringList parts){
    if(parts.length() < 2)
        return;
    QString deckID = parts.at(1);
    QList<fbCard> cardList = data.getAllCards(deckID);
    QString deckName = data.getDeckName(deckID);
    QString retString = "";
    retString += PACKDESC[IMPDAT];
    retString += '~';
    retString += deckID;
    retString += '~';
    retString += deckName;
    retString += '~';
    for(int i = 0; i < cardList.length(); i++){
        fbCard f;
        f = cardList[i];
        retString += f.cardNum;
        retString += '\t';
        retString += f.front;
        retString += '\t';
        retString += f.back;
        retString += '~';

    }
    retString += '\0';
    sendAllData(sock, retString);

}
void MultiManager::handleMultiRequest(int sock, QStringList parts){
    if(parts.length() < 3)
        return;
    QString deckID = parts[1];
    QString cardList = parts[2];

    for(int i = 0; i < cardList.length(); i++){
        char c = cardList[i].toLatin1();
        c -= 128;
        cardList[i] = c;
    }
    std::bitset<BITSETSIZE> curReq = setBitset((char *)cardList.toStdString().c_str(), cardList.length());
    for(int i = 0; i < BITSETSIZE; i++){
        if(curReq.test(i))
            userDat[sock].currentCards.push_back(i);
    }

    QList<matchReturn> bestMatches = getBestMatches(curReq, deckID);
    QString send = "";
    send+= PACKDESC[MLTRSP];
    send+= '~';
    for(int i = 0; i < bestMatches.length(); i++){
        send += QString::number(bestMatches[i].matches);
        send += '\t';
        send += QString::number(bestMatches[i].id);
        send += '\t';
        send += QString::number(bestMatches[i].members);
        send += '\t';
        send += QString::number(bestMatches[i].interval);
        send += '\t';
        send += QString::number(bestMatches[i].independant);
        send += '\t';
        send += QString::number(bestMatches[i].unique);
        send += '~';
    }
    send += '\0';
    sendAllData(sock, send);

}
void MultiManager::createMultiSession(int sock, QStringList parts){
    if(parts.length() < 6)
        return;
    QString deckID = parts[1];
    QString cardList = parts[5];
    int interval = parts[2].toInt();
    int games = parts[3].toInt();
    int independant = parts[4].toInt();
    for(int i = 0; i < cardList.length(); i++){
        char c = cardList[i].toLatin1();
        c -= 128;
        cardList[i] = c;
    }
    std::bitset<BITSETSIZE> curReq = setBitset((char *)cardList.toStdString().c_str(), cardList.length());
    setUserCards(curReq,sock);
    Session newSession;
    newSession.cardBits = curReq;
    QList<fbCard> curCards = data.getAllCards(deckID);
    for(int i = 0; i < curCards.length(); i++){
        int cardNum = curCards[i].cardNum.toInt();
        if(cardNum >= BITSETSIZE)
            continue;
        if(curReq.test(cardNum)){
            curCards[i].lastTry = QDateTime::currentDateTime();
            newSession.cardList.push_back(curCards[i]);

        }
    }
    user u;
    //get name from global list here
    u.score = 0;
    u.sock = sock;
    QString newName = userDat[sock].name;
    if(newName == "")
        newName = getNewName();
    u.name = newName;
    userDat[sock].name = newName;
    newSession.userList.push_back(u);
    newSession.games = games;
    newSession.interval = interval;
    newSession.independant = independant;
    newSession.setMaxCard();
    newSession.uniqueId = totalSessions++;
    newSession.deckID = parts[1];
    userDat[sock].studyIndex = addSession(newSession);
    QString send = PACKDESC[MLTCRT];
    send += '~';
    send += newName;
    send += '~';
    send += QString::number(newSession.uniqueId);
    send += '~';
    send += QString::number(newSession.id);
    send += '\0';
    sendAllData(sock, send);

}
QList<matchReturn> MultiManager::getBestMatches(std::bitset<BITSETSIZE> cur, QString deckID){
    QList<matchReturn> ret;
    QList<matchReturn> independantRooms;
    int cards = 0;
    for(int i = 0; i < cur.size(); i++){
        if(cur.test(i))
            cards++;
    }
    int minPerc = cards/2.5;
    int indRooms = 0;

    QList<matchReturn> possibleReturns;
    for(int i = 0; i < MAXROOMS; i++){
        if(studySessions[i].id == -1)
            continue;
        if(!studySessions[i].isOpen())
            continue;
        if(studySessions[i].deckID != deckID)
            continue;
        if(studySessions[i].independant){
            matchReturn m;
            m.id = i;
            m.matches = 0;
            m.members = studySessions[i].userList.length();
            m.interval = studySessions[i].interval;
            m.independant = 1;
            m.unique = studySessions[i].uniqueId;
            independantRooms.push_back(m);
            indRooms++;
            continue;
        }

        int matches = 0;
        for(int j = 0; j < (studySessions[i].maxCard+1); j++){
            if(studySessions[i].cardBits.test(j) && cur.test(j))
                matches++;
        }
        if(matches > MINMATCHES || matches >minPerc){

            matchReturn m;
            m.id = i;
            m.matches = matches;
            m.members = studySessions[i].userList.length();
            m.interval = studySessions[i].interval;
            m.independant = 0;
            m.unique = studySessions[i].uniqueId;
            possibleReturns.push_back(m);
        }
    }
    if(independantRooms.length() > MAXINDEPENDANT)
        indRooms = 5;
    if((possibleReturns.length() + indRooms )< MAXMATCHES){
        for(int i = 0 ; i < indRooms; i++){
            possibleReturns.push_back(independantRooms[i]);
        }
        return possibleReturns;
    }
    for(int i = 0; i < (MAXMATCHES-indRooms); i++){
        int max = possibleReturns[0].matches;
        int ind = 0;
        for(int j = 0; j < possibleReturns.length(); j++){
            if(possibleReturns[j].matches > max){
                max = possibleReturns[j].matches;
                ind = j;
            }
        }
        ret.push_back(possibleReturns[ind]);
        possibleReturns.removeAt(ind);
    }
    for(int i = 0; i < indRooms; i++){
        ret.push_back(independantRooms[i]);
    }
    return ret;

}
int MultiManager::addSession(Session newSession){
    for(int i=0; i < MAXROOMS; i++){
        if(studySessions[i].id == -1){
            studySessions[i] = newSession;
            studySessions[i].id = i;
            return i;
        }
    }
}
std::bitset<BITSETSIZE> MultiManager::setBitset(char buff[], int len){
    std::bitset<BITSETSIZE> retSet;
    char inbuff[9] = {0};

    memcpy(&inbuff,buff, 9);
    int place = 0;
    for(int i = 0; i < len; i++){
        for(int j = 0; j < 7; j++){

            int bit = (buff[i] & (1 << j));
            if(bit)
                retSet[place] = 1;

            place++;
        }
    }

    return retSet;
}
void MultiManager::handleStudyJoin(int sock, QStringList parts){
    if(parts.length() < 4)
        return;
    QString deckID = parts[1];
    QString unique = parts[3];
    int index = parts[2].toInt();
    QString send;
    if(!checkStillValid(deckID, index, unique.toInt())){
        send = PACKDESC[MLTGONE];
        send += '\0';
        sendAllData(sock, send);
        return;
    }
    QString curUsers;
    if(studySessions[index].full()){
        send = PACKDESC[MLTFULL];
    }
    else{
        studySessions[index].lock();
        //Send join data
        send = PACKDESC[MLTDAT];
        //Get all current users
        studySessions[index].userLock();
        for(int i = 0; i < studySessions[index].userList.length(); i++){
            curUsers += studySessions[index].userList[i].name;
            curUsers += '+';
            curUsers += QString::number(studySessions[index].userList[i].score);
            curUsers += '\t';
        }
        studySessions[index].userUnlock();
        QString cardNums;
        //If not independant and its a single person room, then join the lists for real multi study
        if(studySessions[index].userList.length() == 1 && studySessions[index].independant == 0){
              studySessions[index].joinSession(userDat[sock].currentCards);

        }
        //Get card numbers
        for(int i = 0; i < studySessions[index].cardList.length(); i++){
            cardNums += studySessions[index].cardList[i].cardNum;
            cardNums += '\t';
        }
        //Send the user back his name - either set by login or created at random for this session
        QString joinedName = userDat[sock].name;
        if(joinedName == "")
            joinedName = getNewName();
        userDat[sock].name = joinedName;
        send += '~';
        send += curUsers;
        send += '~';
        send += cardNums;
        send += '~';
        send += joinedName;
        userDat[sock].studyIndex = index;

        studySessions[index].addUser(joinedName, sock);

        send += '\0';
        sendAllData(sock, send);

        //Add user string
        send = "";
        send += PACKDESC[USERJOIN];
        send += '~';
        send += joinedName;
        send += '\0';
        for(int i = 0; i < studySessions[index].userList.length()-1; i++){
            sendAllData(studySessions[index].userList[i].sock, send);
        }
        studySessions[index].unLock();
    }


}
void MultiManager::setUserCards(std::bitset<BITSETSIZE> bits, int sock){
    for(int i =0; i < bits.size(); i++){
        if(bits.test(i))
            userDat[sock].currentCards.push_back(i);
    }
}
bool MultiManager::checkStillValid(QString deckID, int index, int unique){
    if(unique != studySessions[index].uniqueId)
        return false;
    return studySessions[index].deckID == deckID;
}
QString MultiManager::getNewName(){
    QString ret = "Anonymous";
    for(int i = 0; i < 4; i++){
        int r = rand() % 50;
        ret += QString::number(r);
    }
    return ret;
}
void MultiManager::handleAnswer(int sock, QStringList parts){
    int index = userDat[sock].studyIndex;
    if(parts.length() < 5)
        return;
    int answer = parts[2].toInt();
    int remove = parts[3].toInt();
    int currentRound = parts[4].toInt();
    if(!studySessions[index].checkRound(currentRound)){

        return;
    }
    int cardN = parts[1].toInt();
    if(studySessions[index].gameTime()){
        sendGameList(index);
        outEvent o;
        o.index = index;
        o.round = studySessions[index].getRound();
        QTime cur = QTime::currentTime();
        o.sendTime = cur.addSecs(GAMELENGTH);
        addEvent(o);
        return;
    }
    if(studySessions[index].updateRound(sock,answer, remove, cardN)){
        sendNext(index);
    }
}
void MultiManager::sendNext(int index){
    QString send;
    int ret = 0;
    QString resp = studySessions[index].getReturnResponse(&ret);
    if(ret == -1){
        send += PACKDESC[MLTEND];
    }
    else{
        send += PACKDESC[MLTNEXT];
    }
    send += '~';
    send += resp;
    send += '\0';

   // if(index % 50 == 0)
     //   qDebug() << "Sending an update to room:  " + QString::number(index);
    for(int i = 0; i < studySessions[index].userList.length(); i++){
        sendAllData(studySessions[index].userList[i].sock, send.toStdString().c_str());
    }
    if(ret != -1){
        outEvent o;
        o.index = index;
        o.round = studySessions[index].getRound();
        QTime cur = QTime::currentTime();
        o.sendTime = cur.addSecs(studySessions[index].interval + 1);
        addEvent(o);
    }

}
void MultiManager::removeClient(int sd, bool remove){
    if(userDat[sd].loggedIn)
        removeLoggedIn(sd);
    qDebug() << "Removing client";
    int sessionIndex = userDat[sd].studyIndex;
    if(sessionIndex != -1){
        studySessions[sessionIndex].userLeaving(sd);
        QString name = userDat[sd].name;
        userDat[sd].currentCards.clear();
        userDat[sd].name = "";
        userDat[sd].randomData = "";
        userDat[sd].studyIndex = -1;
        QString send;
        send += PACKDESC[USERLEAVE];
        send += '~';
        send += name;
        send += '\0';
        for(int i = 0; i < studySessions[sessionIndex].userList.length(); i++){
            sendAllData(studySessions[sessionIndex].userList[i].sock, send.toStdString().c_str());

        }
        if(studySessions[sessionIndex].isEmpty()){
            cleanSession(sessionIndex);
        }
        if(studySessions[sessionIndex].userLeftCheck())
            sendNext(sessionIndex);
    }
    if(remove){
        epollRemove(sd);
        messages.resetDataPointer(sd);
        qDebug() << "Client Disconected;";
    }



}
void MultiManager::handleLeavingClient(int sock){

    removeClient(sock, false);
}
void MultiManager::cleanSession(int index){
    studySessions[index].cleanUp();
    studySessions[index].id = -1;
    studySessions[index].uniqueId = -1;
}
void MultiManager::handleIndTick(int sock){
    int index = userDat[sock].studyIndex;
    QString name = userDat[sock].name;
    QString send = PACKDESC[INDTICK];
    send += '~';
    send += name;
    send += '\0';
    for(int i = 0; i < studySessions[index].userList.length(); i++){
        if(studySessions[index].userList[i].name == name)
            continue;
        sendAllData(studySessions[index].userList[i].sock, send);
    }
}
void MultiManager::handleDoneUser(int sock){
    int index = userDat[sock].studyIndex;
    for(int i = 0; i < studySessions[index].userList.length(); i++){
        if(studySessions[index].userList[i].sock == sock)
            studySessions[index].userList[i].finished = true;
    }
}
void MultiManager::handleChat(int sock, QStringList parts){
    if(parts.length() < 2)
        return;
    QString message = parts[1];
    QString send = PACKDESC[INCHAT];
    send += '~';
    send += message;
    send += '\0';
    int index = userDat[sock].studyIndex;
    for(int i = 0 ; i < studySessions[index].userList.length(); i++){
        sendAllData(studySessions[index].userList[i].sock, send);
    }
}
void MultiManager::pollOutEvents(){
    QTime cur;
    int rem;
    while(1){
    sem_wait(&queueLock);
        rem = 0;
        cur = QTime::currentTime();
        for(int i =0; i < outQueue.length(); i++){
            if(outQueue[i].sendTime >= cur)
                break;

            rem++;

        }
        QList<outEvent> send;
        for(int i =0; i < rem; i++){
            if(outQueue[0].round == studySessions[outQueue[0].index].getRound()){
                send.push_back(outQueue[0]);
            }
            outQueue.removeAt(0);
        }
    sem_post(&queueLock);
        for(int i = 0; i < send.length();i ++){
            sendNext(send[i].index);
        }
        send.clear();
        usleep(2000000);
    }

}
void MultiManager::addEvent(outEvent o){
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

        }
        else{
            outQueue.insert(index,o);
        }
    sem_post(&queueLock);
}

void MultiManager::handleGameGuess(int sock, QStringList parts){
    if(parts.length() < 3)
        return;
    int first, second;
    first = parts[1].toInt();
    second = parts[2].toInt();
    int index = userDat[sock].studyIndex;
    if(studySessions[index].doGuess(first,second)){
        QString send = PACKDESC[GAMEANSWER];
        send += '~';
        send += userDat[sock].name;
        send += '~';
        send += QString::number(first);
        send += '~';
        send += QString::number(second);
        send += '\0';

        for(int i = 0; i < studySessions[index].userList.length(); i++){
            sendAllData(studySessions[index].userList[i].sock, send);
        }
    }
    if(studySessions[index].gameOver()){
        sendNext(index);
    }
}
void MultiManager::sendGameList(int index){

    QString send;
    send += PACKDESC[GAMESTART];
    send += '~';
    send += studySessions[index].gameString;
    send += '\0';

    for(int i = 0; i < studySessions[index].userList.length(); i++){
        sendAllData(studySessions[index].userList[i].sock, send);
    }

}
void MultiManager::handleLogin(int sock, QStringList parts){
    if(parts.length() < 3)
        return;
    QString send = PACKDESC[LOGINRESPONSE];
    send += '~';
    QString name = parts[1];
    QString passHash = parts[2];
    QString foundHash = data.getUser(name);

    if(foundHash == ""){
        send += "0";

    }
    else if(foundHash != passHash){
        send += "1";
    }
    else{
        send += "2";
        addLoggedUser(name, sock);
        newFriendOnline(sock);
        userDat[sock].loggedIn = true;
    }
    send+= '\0';
    sendAllData(sock, send);
}
void MultiManager::handleRegister(int sock, QStringList parts){
    bool reg = false;
    if(parts.length() >= 3){
        QString name = parts[1];
        QString passHash = parts[2];
        reg = data.registerUser(name, passHash);

    }
    QString send = PACKDESC[REGISTERRESPONSE];
    send += '~';
    if(reg)
        send += "1";
    else
        send += "0";
    send += '\0';
    sendAllData(sock, send);
}
void MultiManager::addLoggedUser(QString name, int sock){
    QStringList friends = data.getFriends(name);
    loggedUser cur;
    cur.allFriends = friends;
    cur.sock = sock;
    cur.name = name;
    for(int i = 0; i < friends.length(); i++){
        for(int j = 0; j < currentUsers.length(); j++){
            if(friends[i] == currentUsers[j].name){
                currentUsers[j].onlineFriends.push_back(cur);
                sendFriendUpdate(currentUsers[j].sock, &currentUsers[j]);

                cur.onlineFriends.push_back(currentUsers[j]);
                break;
            }
        }

    }
    currentUsers.push_back(cur);
    userDat[sock].name = name;

    sendFriendUpdate(sock, &cur);

}
void MultiManager::handleRemoveFriend(int sock, QStringList parts){
    if(parts.length() < 2)
        return;
    QString send = PACKDESC[REMOVEFRIEND];
    send += '~';
    QString name = userDat[sock].name;
    QString friendName = parts[1];
    if(data.removeFriend(name, parts[1])){
        send += "1";
        loggedUser * cur = 0;
        for(int i = 0; i < currentUsers.length(); i++){
            if(currentUsers[i].sock == sock){
                cur = &currentUsers[i];
                break;
            }
        }
        if(cur != 0){
            int index = -1;
            for(int i = 0; i < cur->onlineFriends.length();i++){
                if(cur->onlineFriends[i].name == friendName){
                    index = i;
                    break;
                }
            }
            if(index != -1)
                cur->onlineFriends.removeAt(index);
            cur->allFriends.removeAll(friendName);
            sendFriendUpdate(sock, cur);
        }
    }else{
        send += "0";
    }
    send += '\0';
    sendAllData(sock, send);
}
void MultiManager::handleAddFriend(int sock, QStringList parts){

    if(!userDat[sock].loggedIn)
        return;
    if(parts.length() < 2)
        return;
    loggedUser  * cur = NULL;
    QString send = PACKDESC[FRIENDRESPONSE];
    send += '~';
    QString friendName = parts[1];
    if(data.addFriend(userDat[sock].name, friendName )){
        for(int i = 0; i < currentUsers.length(); i++){
            if(currentUsers[i].sock == sock){
                cur = &currentUsers[i];
                currentUsers[i].allFriends.push_back(friendName);
                for(int j = 0; j < currentUsers.length(); j++){
                    if(currentUsers[j].name == friendName){
                        currentUsers[i].onlineFriends.push_back(currentUsers[j]);
                        break;

                    }
                }
                break;
            }
        }
        if(cur != NULL)
            sendFriendUpdate(sock, cur);
        send += "1";
    }
    else{
        send += "0";
    }
    send += '\0';
    sendAllData(sock, send);
}
void MultiManager::sendFriendUpdate(int sock, loggedUser * l){
    QString send = PACKDESC[FRIENDUPDATE];
    send += '~';
    for(int i = 0; i < l->onlineFriends.length(); i++){
        send += l->onlineFriends[i].name;
        send += '\t';
    }
   // if(l->onlineFriends.length() > 0)
     //   send = send.left(send.length() -1);
    send += '~';
    for(int i = 0; i < l->allFriends.length(); i++){
        bool found = false;
        for(int j = 0; j < l->onlineFriends.length(); j++){
            if(l->allFriends[i] == l->onlineFriends[j].name){
                found = true;
                break;
            }
        }
        if(!found){
            send += l->allFriends[i];
            send += '\t';
        }

    }
    //if(l->allFriends.length() > 0)
        //send = send.left(send.length() -1);
    send += '\0';
    sendAllData(sock, send);
}
void MultiManager::removeLoggedIn(int sock){
    int remInd = -1;
    int remFriend = -1;
    for(int i = 0; i < currentUsers.length(); i++){
        if(currentUsers[i].sock == sock){
            remInd = i;
            continue;
        }
        remFriend = -1;
        loggedUser l = currentUsers[i];
        for(int j = 0; j < l.onlineFriends.length(); j++){
            if(l.onlineFriends[j].sock == sock){
                remFriend = j;
                break;

            }
        }
        if(remFriend != -1){
            currentUsers[i].onlineFriends.removeAt(remFriend);
            sendFriendUpdate(currentUsers[i].sock, &currentUsers[i]);
        }
    }
    if(remInd != -1)
        currentUsers.removeAt(remInd);
}
void MultiManager::newFriendOnline(int sock){
    QString name = userDat[sock].name;
    loggedUser cur;
    for(int i = 0; i < currentUsers.length(); i++){
        if(currentUsers[i].sock == sock){
            cur = currentUsers[i];
            break;
        }
    }
    for(int i = 0; i < currentUsers.length(); i++){
        loggedUser * l = &currentUsers[i];
        for(int j = 0; j < l->allFriends.length(); j++){
            if(l->allFriends[j] == name){
                //l->onlineFriends.push_back(cur);
                sendFriendUpdate(l->sock, l);
            }
        }
    }
}
void MultiManager::handlePm(int sock, QStringList parts){
    if(parts.length() < 2)
        return;
    QString message;
    for(int i = 1; i < parts.length(); i++)
        message += parts[i];

    char c = 1;
    QStringList messageParts = message.split(c);
    if(messageParts.length() < 2)
        return;
    QString messageSend = messageParts[1];
    QString friendName = messageParts[0];
    int sendSock = -1;
    for(int i = 0; i < currentUsers.length(); i++){
        if(currentUsers[i].name == friendName){
            sendSock = currentUsers[i].sock;
            break;
        }
    }
    if(sendSock != -1){
        QString send = PACKDESC[PMRECV];
        send += '~';
        send += userDat[sock].name;
        send += c;
        send += messageSend;
        send += '\0';
        sendAllData(sendSock, send);
        send = PACKDESC[PMSEND];
        send += '~';
        send += friendName;
        send += c;
        send += messageSend;
        send += '\0';
        sendAllData(sock, send);
    }
}
void MultiManager::sendWelcomeMessage(int sock){
    int newSeed = rand() % 30; + 10;
    registerSocket(sock, 15);
    QString send = "WELCME";
    send += '~';
    send += QString::number(newSeed);
    send += '\0';
    sendAllData(sock, send);
    registerSocket(sock, newSeed);

}
void MultiManager::setMw(MainWindow *m){
    mw = m;
}
std::pair<int,int> MultiManager::statTick(int current, int totalMessage, int totalElapsed){
    int rooms = 0;
    for(int i  = 0 ; i < MAXROOMS; i++){
        if(studySessions[i].id != -1)
            rooms++;
    }
    int holdEventMs, holdEventTotal;
    int packetsOut = getTotalPackets();
    int packetIn = getTotalRecv();
    holdEventMs = totalEventTime;
    totalEventTime = 0;
    holdEventTotal = totalEvents;
    totalEvents = 0;
    if(totalMessage== 0)
        totalMessage = 1;
    if(holdEventTotal ==0)
        holdEventTotal = 1;
    int avgNetwork = totalElapsed/totalMessage;
    int avgCpu = holdEventMs/holdEventTotal;
   // qDebug() << "Average Network MS: " + QString::number(totalElapsed/totalMessage )+ " Average CPU Ms: " + QString::number(holdEventMs/holdEventTotal) + " Total ROOMS: " + QString::number(rooms);
   // qDebug() << "Total Packets Out: " +QString::number(packetsOut) + " Total Clients: " + QString::number(current);
    std::pair<int,int> ret;
    ret.first = avgNetwork;
    ret.second = avgCpu;
    QMetaObject::invokeMethod(mw, "updateDisplay",Q_ARG(int, rooms), Q_ARG(int, current), Q_ARG(int, avgNetwork), Q_ARG(int, avgCpu), Q_ARG(int, packetsOut+packetIn));
    return ret;
}
