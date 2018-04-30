#include "multimanager.h"
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

bool RUNNING = false;
MultiManager::MultiManager()
{
    connected = false;
    sock = -1;
}
void MultiManager::setConnected(int socket){
    sock = socket;
    connected = true;
}
void MultiManager::setDisconnected(){
    connected = false;
    sock = -1;
}
void MultiManager::setData(DataManager *d, MainWindow * m){
    db = d;
    mw = m;
}
bool MultiManager::handleIncomingEvent(QString packet){
    if(packet.length() == 0)
        return false;
    QStringList parts = packet.split('~');
    if(parts[0] == PACKDESC[EXPRSP]){
        handleExportResponse(packet);
    }
    else if(parts[0] == PACKDESC[INPDAT]){
       handleImportData(packet);
    }
    else if(parts[0] == PACKDESC[DCKLISTRSP]){
        handleDeckListResponse(packet);
    }
    else if(parts[0] == PACKDESC[IMPDAT]){
        handleImportData(packet);
    }
    else if(parts[0] == PACKDESC[MLTRSP]){
        handleRoomList(packet);
    }
    else if(parts[0] == PACKDESC[MLTDAT]){
        joinStudyRoom(packet);
    }
    else if(parts[0] == PACKDESC[MLTFULL]){
        roomFull();
    }
    else if(parts[0] == PACKDESC[MLTCRT]){
        roomCreated(packet);
    }
    else if(parts[0] == PACKDESC[MLTNEXT]){
        handleNextCard(packet);
    }
    else if(parts[0] == PACKDESC[USERJOIN]){
        newUserJoined(packet);
    }
    else if(parts[0] == PACKDESC[MLTEND]){
        endSession(packet);
    }
    else if(parts[0] == PACKDESC[USERLEAVE]){
        handleUserLeave(packet);
    }
    else if(parts[0] == PACKDESC[INDTICK]){
        handleTick(packet);
    }
    else if(parts[0] == PACKDESC[MLTGONE]){
        multiNoLongerExists(packet);
    }
    else if(parts[0] == PACKDESC[INCHAT]){
        handleIncomingChat(packet);
    }
    else if(parts[0] == PACKDESC[GAMEANSWER]){
        handleGameUpdate(packet);
    }
    else if(parts[0] == PACKDESC[GAMESTART]){
        handleGameStart(packet);
    }
    else if(parts[0] == PACKDESC[LOGINRESPONSE]){
        handleLogin(packet);
    }
    else if(parts[0] == PACKDESC[REGISTERRESPONSE]){
        handleRegister(packet);
    }
    else if(parts[0] == PACKDESC[FRIENDRESPONSE]){
        handleFriendResponse(packet);
    }
    else if(parts[0] == PACKDESC[FRIENDUPDATE]){
        handleFriendUpdate(packet);
    }
    else if(parts[0] == PACKDESC[REMOVEFRIEND]){
        handleFriendRemove(packet);
    }
    else if(parts[0] == PACKDESC[PMRECV]){
        handlePm(packet);
    }
    else if(parts[0] == PACKDESC[PMSEND]){
        handleSentPm(packet);
    }
    else if(parts[0] == PACKDESC[WELCOME]){
        handleWelcome(packet);
    }
    return true;
}

bool MultiManager::doExport(QString deckName, QString desc, QString language){
    if(!connected)
        return false;
    QList<aCard> cards;
    QSqlQuery result = db->getCardsForExport(deckName);
    while(result.next()){
        aCard aC;
        aC.front = result.value("Front").toString();
        aC.back = result.value("Back").toString();
        aC.cardNum = result.value("CardNum").toString();
        cards.push_back(aC);
    }
    if(cards.length() == 0)
        return false;

    curDeck = deckName;
    sendExportRequest(deckName, desc, language);
    //exportAllCards(deckName, ret, cards);


}
void MultiManager::sendExportRequest(QString deck, QString desc, QString lang){
    QString sendString = deck + "~" + desc + "~" + lang;
    sendString = packForSend(sendString,0);
    sendData(sock, sendString);
    //QString response = readResponse(sock);
//    return response;

}
QString MultiManager::packForSend(QString data, int type){
    QString totalPack = PACKDESC[type];
    totalPack += "~";
    totalPack += data;
    totalPack += '\0';
    return totalPack;

}

//EXPDAT~DECKNAME~DECKID~CARDDATA~CARDDATA~...
//CardDate -> nun\tfront\tback~
void MultiManager::exportAllCards(QString deck, QString deckID, QList<aCard> cards){
    QString data;
    int sendLength = 0;
    int MAXLEN = MAXLENGTH - (PACKDESC[1].length() + 3 + deck.length() + deckID.length());

    QString header = PACKDESC[1] + "~" + deck + "~" + deckID + "~" + '\0';
    int headerLen;
    QByteArray iNeedToFixThis = header.toLocal8Bit();
    headerLen = iNeedToFixThis.length();
    bool lastSend = false;
    QString send;
    int wowLength;
    MAXLEN = MAXLENGTH - headerLen;
    for(int i = 0; i < cards.length(); i++ ){
        lastSend = false;
        QString temp = cards[i].cardNum + '\t' + cards[i].front + '\t' + cards[i].back + "~";
        int realLength;
        QByteArray thisIsReallyStupid = temp.toLocal8Bit();
        realLength = thisIsReallyStupid.length();
        if((sendLength + realLength) >= MAXLEN){
            lastSend = true;
            send = deck + "~" + deckID + "~";
            send += data;
            send = packForSend(send, EXPDATA);
            QByteArray iCantBelieveThis = send.toLocal8Bit();
            wowLength = iCantBelieveThis.length();

            sendData(sock, send);
            data = "";
            data += temp;
            sendLength = realLength;
        }else{
            sendLength += realLength;
            data += temp;
        }

    }
    if(!lastSend){
        send = deck + "~" + deckID + "~";
        send += data;
        send = packForSend(send, EXPDATA);
        QByteArray iCantBelieveThis = send.toLocal8Bit();
        wowLength = iCantBelieveThis.length();

        sendData(sock, send);
    }
}

void MultiManager::requestDeckList(){
    QString pack = packForSend("", DECKLIST);
    int err = sendData(sock, pack);
    if(err < 0){
        //function to deal with broken socket
    }




}
void MultiManager::readLoop(){
    RUNNING = true;
    while(RUNNING){
        QString packet = readUntilEmpty(sock);
        if(packet.length() == 1){
            if(packet == "D"){
                //deal with socket cleanup/ui updates here

                qDebug() << "Server disconected";
                registerSocket(sock, 15);
                QMetaObject::invokeMethod(mw,"serverDied", Qt::QueuedConnection);


            }
            break;
        }

        if(!handleIncomingEvent(packet))
            RUNNING = false;
    }
}


void MultiManager::handleExportResponse(QString packet){
    QList<aCard> cards;
    QStringList parts = packet.split('~');
    QString deckID = parts[1];
    db->updateDeckID(deckID, curDeck);
    QSqlQuery result = db->getCardsForExport(curDeck);
    while(result.next()){
        aCard aC;
        aC.front = result.value("Front").toString();
        aC.back = result.value("Back").toString();
        aC.cardNum = result.value("CardNum").toString();
        cards.push_back(aC);
    }
    exportAllCards(curDeck,deckID ,cards);
}
void MultiManager::handleDeckListResponse(QString packet){
    QStringList parts = packet.split('~');
    for(int i = 1; i < parts.length(); i++){
        qDebug() << "NEW DECK: " + parts[i];
    }
    parts.pop_front();
    mw->updateImportList(parts);
}
void MultiManager::sendImportRequest(QString deckID, QString deckName){
    QString send = deckID;
    QString dat = packForSend(send, IMPREQ);
    curName = deckName;

    sendData(sock, dat);
}
void MultiManager::handleImportData(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 3)
        return;
    QString deckName;
    QList<shortCard> cardList;
    deckName = parts[2];
    for(int i = 3; i < parts.length(); i++){
        QStringList line = parts[i].split('\t');
        if(line.length() < 3)
            continue;
        shortCard c;
        c.cardNum = line[0];
        c.front = line[1];
        c.back = line[2];
        cardList.push_back(c);

    }
    db->addALLCard(cardList, curName);

}
void MultiManager::sendMultiRequest(QString deckName){
    QString deckID = db->getDeckId(deckName);
    if(deckID.length() == 0)
        return;
    QSqlQuery result = db->getSession(deckName);
    QString send = "";
    send += PACKDESC[MLTREQ];
    send += '~';
    send += deckID;
    send += '~';
    char buff[1000] = {0};
    int place, bit;
    while(result.next()){
        int cardNum = result.value("CardNum").toInt();
        place = cardNum / 7;
        bit = cardNum % 7;
        buff[place] += pow(2,bit);
    }
    for(int i = 0; i <= place; i++){
        buff[i] += 128;
    }
    QString bits = QString::fromLatin1(buff);

    send += bits;
    send += '\0';
    sendData(sock, send);
}
bool MultiManager::createRoom(QString deckName, int interval, int games, int indep){
    sessionOver = false;
    currentRound = "0";
    QString send;
    QString deckID = db->getDeckId(deckName);
    QSqlQuery result = db->getSession(deckName);
    this->interval = interval;
    if(indep == 1)
        independant = true;
    else
        independant = false;
    send += PACKDESC[MLTNEW];
    send += '~';
    send += deckID;
    send += '~';
    send += QString::number(interval);
    send += '~';
    send += QString::number(games);
    send += '~';
    send += QString::number(independant);
    send += '~';
    char buff[1000] = {0};
    int place, bit;
    while(result.next()){
        int cardNum = result.value("CardNum").toInt();
        currentCards.push_back(cardNum);
        place = cardNum / 7;
        bit = cardNum % 7;
        buff[place] += pow(2,bit);
    }
    for(int i = 0; i <= place; i++){
        buff[i] += 128;
    }
    QString bits = QString::fromLatin1(buff);

    send += bits;


    send += '\0';
    sendData(sock, send);


}
void MultiManager::handleRoomList(QString packet){
    availableRooms.clear();
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    for(int i =0; i < parts.length(); i++){
        QStringList rDat = parts[i].split('\t');
        if(rDat.length() < 6)
            continue;
        roomData r;
        r.matches = rDat[0];
        r.id = rDat[1];
        r.members = rDat[2];
        r.interval = rDat[3];
        r.independant = rDat[4].toInt();
        r.uniqueId = rDat[5];
        availableRooms.push_back(r);

    }
    //mw->updateRoomList(availableRooms);
    //QMetaObject::invokeMethod(mw,"updateRoomList", Q_ARG(QList<roomData>, availableRooms));
    QMetaObject::invokeMethod(mw,"updateRoomList", Qt::QueuedConnection);
}
void MultiManager::sendMultiJoinRequest(QString deckName, int index){
    QString deckID = db->getDeckId(deckName);
    interval = availableRooms[index].interval.toInt();
    QString send = PACKDESC[MLTJOIN];
    send += '~';
    send += deckID;
    send += '~';
    send += availableRooms[index].id;
    send += '~';
    send += availableRooms[index].uniqueId;
    send += '\0';
    independant = false;
    if(availableRooms[index].independant == 1)
        independant = true;
    sendData(sock, send);
}

void MultiManager::joinStudyRoom(QString packet){
    sessionOver = false;
   // independant = false;
    userList.clear();
    QStringList parts = packet.split('~');
    if(parts.length() < 4)
        return;
    QString userPart = parts[1];
    QString cardPart = parts[2];
    curName = parts[3];
    users n;
    n.lastScore = 0;
    n.name = curName;
    n.score = 0;
    n.startScore = 0;
    userList.push_back(n);
    QStringList allUsers = userPart.split('\t');
    for(int i = 0; i < allUsers.length(); i++){
        users newUser;
        QStringList scores = allUsers[i].split('+');
        if(scores.length() < 2)
            continue;
        newUser.name = scores[0];
        newUser.startScore = scores[1].toInt();
        newUser.score = scores[1].toInt();
        userList.push_back(newUser);
    }

    QStringList cards = cardPart.split('\t');
    for(int i = 0; i < cards.length(); i++){
        if(cards[i] == "")
            continue;
        currentCards.push_back(cards[i].toInt());
    }
    qDebug() << "Current users;";
    for(int i = 0; i < userList.length(); i++){
        qDebug() << userList[i].name;
    }

    QMetaObject::invokeMethod(mw, "joinMultiStudy",Qt::QueuedConnection);

}


//fill these out tomorrow maybe
QByteArray MultiManager::cardsToBits(QList<aCard> cardList){
    QByteArray ret;
    int place, bit;
    for(int i = 0; i < cardList.length(); i++){
        int cardNum = cardList[i].cardNum.toInt() / 7;
        place = cardNum / 7;
        bit = cardNum % 7;
        if(bit == 0)
            ret += char(0);
        ret[place] = ret.at(place) + pow(2,bit);

    }
    return ret;
}
QList<aCard> MultiManager::bitsToCards(QByteArray data){
    QList<aCard> ret;

}
void MultiManager::roomFull(){
    QMetaObject::invokeMethod(mw, "roomFull", Qt::QueuedConnection);
}
void MultiManager::roomCreated(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    curId = parts[1];
    curName = parts[1];

    users n;
    n.lastScore = 0;
    n.name = curName;
    n.score = 0;
    n.startScore = 0;
    userList.push_back(n);
    QMetaObject::invokeMethod(mw, "newMultiCreated", Qt::QueuedConnection);

}
QList<int> MultiManager::getCardList(){
    return currentCards;
}
bool MultiManager::isSolo(){
    return userList.length() < 1;
}
bool MultiManager::isIndependant(){
    return independant;
}
void MultiManager::sendAnswer(int cardNum, int answer, char remove){
    if(sessionOver)
        return;
    QString send;
    send += PACKDESC[MLTANS];
    send += '~';
    send += QString::number(cardNum);
    send += '~';
    send += QString::number(answer);
    send += '~';
    send += remove;
    send += '~';
    send += currentRound;
    send += '\0';
    sendData(sock, send);
}
void MultiManager::handleNextCard(QString packet){
    QStringList parts = packet.split("~");
    if(parts.length() < 4)
        return;
    if(playingGame){
        playingGame = false;
        QMetaObject::invokeMethod(mw, "returnToStudy");

    }
    QString cardNum = parts[1];
    QString userDataPart = parts[2];
    currentRound = parts[3];
    QStringList indvUsers = userDataPart.split('\t');
    for(int i = 0; i < indvUsers.length();i++){
        QStringList indvUsersData = indvUsers[i].split('+');
        if(indvUsersData.length() < 3)
            continue;
        QString name, score,lastScore;
        //Will still need to update this for real in user list somewhere;
        name = indvUsersData[0];
        score = indvUsersData[1];
        lastScore = indvUsersData[2];
        for(int j = 0; j < userList.length(); j++){
            if(userList[j].name == name){
                userList[j].score = score.toInt();
                userList[j].lastScore = lastScore.toInt();
            }
        }
        qDebug() << "The user: " + name + " Scored " + lastScore + " Last round, total score = " + score;
        qDebug() << "The next card should be: " + cardNum;
    }
    qDebug() << "Current round is set to: " + currentRound;
    //mw->multiLoadQuestion(cardNum.toInt());
    copyAndSendScores();

    QMetaObject::invokeMethod(mw, "multiLoadQuestion", Q_ARG(int,cardNum.toInt() ));
}
void MultiManager::newUserJoined(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    QString newUser = parts[1];
    if(newUser == "")
        return;
    users u;
    u.name = newUser;
    u.score = 0;
    u.startScore = 0;
    userList.push_back(u);

    qDebug() << "New user joined! " + newUser;
}
void MultiManager::endSession(QString packet){
    QStringList parts = packet.split("~");
    sessionOver = true;
    independant = true;
    if(parts.length() < 4)
        return;
    int cardNum = -1;
    QString userDataPart = parts[2];

    QStringList indvUsers = userDataPart.split('\t');
    for(int i = 0; i < indvUsers.length();i++){
        QStringList indvUsersData = indvUsers[i].split('+');
        if(indvUsersData.length() < 3)
            continue;
        QString name, score,lastScore;
        //Will still need to update this for real in user list somewhere;
        name = indvUsersData[0];
        score = indvUsersData[1];
        lastScore = indvUsersData[2];
        for(int j = 0; j < userList.length(); j++){
            if(userList[j].name == name){
                userList[j].score = score.toInt();
                userList[j].lastScore = lastScore.toInt();
                break;
            }
        }
    }
    QMetaObject::invokeMethod(mw, "multiLoadQuestion", Q_ARG(int,cardNum));
}
void MultiManager::closeConnect(){
    close(sock);
    qDebug() << "Closed the socket";
    sock = -1;

}
void MultiManager::sendIndTick(){
    QString send = PACKDESC[INDTICK];
    send += '\0';
    sendData(sock, send);

}

void MultiManager::handleUserLeave(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    QString name = parts[1];
    int remIndex = -1;
    for(int i = 0; i < userList.length();i++ ){
        if(userList[i].name == name){
            remIndex = i;
            break;
        }
    }
    if(remIndex != -1)
        userList.removeAt(remIndex);
    //if(userList.length() == 0)
      //  independant = true;
    //DEAL WITH ACTUAL USER CLEANUP HERE
}
void MultiManager::sendExit(){
    QString send = PACKDESC[USERLEAVE];
    send += '\0';
    sendData(sock, send);
    //close(sock);
}
void MultiManager::handleTick(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    QString name = parts[1];
    for(int i  =0 ; i < userList.length(); i++){
        if(userList[i].name == name){
            userList[i].score++;
            //update list here
            qDebug() << name + " Score a point!";
        }
    }
    //QMetaObject::invokeMethod(mw, "updateScores", Q_ARG(QList<users>, userList));
    copyAndSendScores();

}
void MultiManager::multiNoLongerExists(QString packet){
    qDebug() << "Room gone";
    QMetaObject::invokeMethod(mw, "roomGone", Qt::QueuedConnection);

}
void MultiManager::sendUserDone(){
    QString send = PACKDESC[USERDONE];
    send += '\0';
    sendData(sock, send);
}
void MultiManager::sendFakeResponse(){
    QString send;
    send += PACKDESC[MLTANS];
    send += '~';
    send += QString::number(-1);
    send += '~';
    send += QString::number(-1);
    send += '~';
    send += "1";
    send += '~';
    send += currentRound;
    send += '\0';
    sendData(sock, send);
}
void MultiManager::copyAndSendScores(){

    QMetaObject::invokeMethod(mw, "updateScores", Qt::QueuedConnection);

}
bool MultiManager::isMe(QString n){
    return n == curName;
}
void MultiManager::sendChat(QString message){
    QString send;
    send += PACKDESC[OUTCHAT];
    send += '~';
    send += curName;
    send += '>';
    send+= message;
    send += '\0';
    if(send.length() > 1000)
        return;
    sendData(sock, send);

}
void MultiManager::handleIncomingChat(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    QString message = parts[1];
    QMetaObject::invokeMethod(mw, "addChat", Q_ARG(QString, message));
}
void MultiManager::handleGameUpdate(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 4)
        return;
    QString name = parts[1];
    int first, second;
    first = parts[2].toInt();
    second = parts[3].toInt();
    int index = 0;
    for(int i = 0; i < userList.length(); i++){
        if(userList[i].name == name){
            index = i;
            break;
        }
    }
    QMetaObject::invokeMethod(mw, "updateGameBoard", Q_ARG(int, index), Q_ARG(int, first), Q_ARG(int, second));

}
void MultiManager::sendGameGuess(int first, int second){
    if(first < 0 || first > 15)
        return;
    if(second < 0 || second > 15)
        return;

    QString send = PACKDESC[GAMEGUESS];
    send += '~';
    send += QString::number(first);
    send += '~';
    send += QString::number(second);
    send += '\0';
    sendData(sock, send);

}
void MultiManager::handleGameStart(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    playingGame = true;
    QStringList wordPart = parts[1].split('\t');
    for(int i = 0; i < wordPart.length(); i++){
        gameList.push_back(wordPart[i]);
    }
    QMetaObject::invokeMethod(mw, "loadGame");

}
void MultiManager::sendLogin(QString userName, QString pass){
    QString send = PACKDESC[LOGIN];
    send += '~';
    send += userName;
    send += '~';
    send += pass;
    send += '\0';
    sendData(sock, send);
}
void MultiManager::sendRegisterRequest(QString username, QString pass){
    QString send = PACKDESC[REGISTER];
    send += '~';
    send += username;
    send += '~';
    send += pass;
    send += '\0';
    sendData(sock, send);
}

void MultiManager::handleLogin(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    if(parts[1] == "0"){
        QMetaObject::invokeMethod(mw, "passToLogin", Q_ARG(int, 0));
        //mw->passToLogin(0);
    }
    else if(parts[1] == "1"){
        //mw->passToLogin(1);
        QMetaObject::invokeMethod(mw, "passToLogin", Q_ARG(int, 1));
    }
    else if(parts[1] == "2"){
        QMetaObject::invokeMethod(mw, "passToLogin", Q_ARG(int, 2));
        //mw->passToLogin(2);
    }
    else{
        qDebug() << "Major error parsing login request string";
    }
}
void MultiManager::handleRegister(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    if(parts[1] == "1"){
        qDebug() << "Successful register!";
        mw->passToRegister(1);
    }
    else{
        mw->passToRegister(1);
        qDebug() << "Failed to register ;;";
    }
}
void MultiManager::sendFriendAdd(QString name){
    QString send = PACKDESC[ADDFRIEND];
    send += '~';
    send += name;
    send += '\0';
    sendData(sock, send);
}
void MultiManager::sendFriendRemove(QString name){
    QString send = PACKDESC[REMOVEFRIEND];
    send += '~';
    send += name;
    send += '\0';
    sendData(sock, send);
}
void MultiManager::handleFriendResponse(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    if(parts[1] == "1"){
        mw->passFriendAdd(1);
    }
    else{
        mw->passFriendAdd(0);
    }
}
void MultiManager::handleFriendUpdate(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 3)
        return;



    mw->passFriendUpdate(parts[1], parts[2]);


}
void MultiManager::handleFriendRemove(QString packet){
    QStringList parts = packet.split("~");
    if(parts.length() < 2)
        return;
    if(parts[1] == "1"){
        mw->passFriendRemove(1);
    }else{
        mw->passFriendRemove(0);
    }

}
void MultiManager::sendPm(QString message, QString friendName){
    QString send = PACKDESC[PMSEND];
    char c = 1;
    send += '~';
    send += friendName;
    send += c;
    send += message;
    send += '\0';
    sendData(sock, send);
}
void MultiManager::handlePm(QString packet){
    char c = 1;
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    QString messagePart;
    for(int i = 1; i < parts.length(); i++){
        messagePart += parts[1];
    }
    QStringList messageList = messagePart.split(c);
    if(messageList.length() < 2)
        return;
    QString friendName = messageList[0];
    QString message = messageList[1];
    mw->passPm(friendName, message);
}
void MultiManager::handleSentPm(QString packet){
    char c = 1;
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    QString messagePart;
    for(int i = 1; i < parts.length(); i++){
        messagePart += parts[1];
    }
    QStringList messageList = messagePart.split(c);
    if(messageList.length() < 2)
        return;
    QString friendName = messageList[0];
    QString message = messageList[1];
    mw->passSentPm(friendName, message);
}
void MultiManager::handleWelcome(QString packet){
    QStringList parts = packet.split('~');
    if(parts.length() < 2)
        return;
    int seed = parts[1].toInt();
    registerSocket(sock, seed);
}
