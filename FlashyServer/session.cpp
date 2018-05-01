#include "session.h"

Session::Session()
{
    id = -1;

    for(int i = 0; i < 10; i++){
        fbCard f;
        f.front = "GameTest" + QString::number(i);
        f.back = "GameAnswer" + QString::number(i);
        reviewList.push_back(f);
    }
    sem_init(&someLock, 0, 1);
    sem_init(&outsideLock, 0, 1);

}
bool Session::isOpen(){
    return userList.length() < MAXUSERS;

}
void Session::lock(){
    sem_wait(&outsideLock);
}
void Session::unLock(){
    sem_post(&outsideLock);
}

void Session::setMaxCard(){
    int max = 0;
    for(int i = 0; i < MAXCARD; i++){
        if(cardBits.test(i))
            max = i;
    }
    maxCard = max;
}
void Session::joinSession(QList<int> newUserCards){
    QList<fbCard> newList;
    sem_wait(&someLock);
    for(int i = 0; i < cardList.length(); i++){
        int index = cardList[i].cardNum.toInt();
        bool found = false;
        for(int j = 0; j < newUserCards.length(); j++){
            if(index == newUserCards[j]){
                found = true;
                break;
            }
            if(newUserCards[j] > index)
                break;
        }
        if(found)
            newList.push_back(cardList[i]);
    }
    cardList = newList;
    sem_post(&someLock);

}
bool Session::full(){
    sem_wait(&someLock);
    bool ret = userList.length() >= MAXUSERS;
    sem_post(&someLock);
    //return userList.length() >=MAXUSERS;
    return ret;
}
bool Session::overFull(){
    return userList.length() > MAXUSERS;
}
int Session::numUsers(){
    return userList.length();
}

void Session::addUser(QString name, int sock){
    sem_wait(&someLock);
    user u;
    u.name = name;
    u.sock = sock;
    userList.push_back(u);
    sem_post(&someLock);
}
void Session::userLock(){
    sem_wait(&someLock);
}
void Session::userUnlock(){
    sem_post(&someLock);
}

bool Session::updateRound(int user, int answer, int remove, int cardN){
    sem_wait(&someLock);
    roundResponses++;
    if(remove)
        usersRemoving++;
    for(int i = 0; i < userList.length(); i++){
        if(userList[i].sock == user){
            userList[i].lastScore = answer;
            break;
        }
    }
    //qDebug() << "Responses: " + QString::number(roundResponses) + " This round req: " + QString::number(thisRoundUsers);
    sem_post(&someLock);
    return roundResponses >= thisRoundUsers;
}
bool Session::checkRound(int round){
    return currentRound == round;
}
QString Session::getReturnResponse(int * next){
    sem_wait(&someLock);
    QString ret;
    updateScoring();
    double removePerc = (double)usersRemoving / thisRoundUsers;
    int length = cardList.length();
    if(length > curIndex){
        if(removePerc >= 0.5){
            cardBits[cardList[curIndex].cardNum.toInt()] = 0;
            if(cardList.length() > curIndex){
                if(checkForReview(cardList[curIndex].back)){
                     reviewList.push_back(cardList[curIndex]);
                }
                cardList.removeAt(curIndex);
            }

        }
        else{
            bumpTime();
        }
    }
    QString userPart;
    for(int i = 0; i < userList.length(); i++){
        userPart += userList[i].name;
        userPart += '+';
        userPart += QString::number(userList[i].score);
        userPart += '+';
        if(userList[i].lastScore == -2){
            userPart += QString::number(-1);
        }
        else{
            userPart += QString::number(userList[i].currentRoundScore);
        }
        userPart += '\t';
    }
    QString nextCardNum = getNextCard();
    if(nextCardNum == ""){
        *next = -1;
    }
    else{
        *next = nextCardNum.toInt();
    }
    ret += nextCardNum;
    ret += '~';
    ret += userPart;
    ret += '~';
    ret += QString::number(currentRound);
//    qDebug() << "Current round is set to: " + QString::number(currentRound);

    sem_post(&someLock);
    return ret;
}
void Session::updateScoring(){
    int max = 1;
    int place = userList.length();
    while(max > 0){
        max = 0;
        for(int i = 0; i < userList.length(); i++){
            if(userList[i].lastScore > max)
                max = userList[i].lastScore;
        }
        int found = 0;
        for(int i = 0; i < userList.length(); i++){
            if(userList[i].lastScore == max){
                double tot = round((double)place/userList.length() * 10.0);
                userList[i].currentRoundScore = (int)tot;
                userList[i].score += tot;
                userList[i].lastScore = -1;
                found ++;
            }

        }
        place-= found;
    }
    for(int i = 0; i < userList.length(); i++){
        if(userList[i].lastScore == -1)
            userList[i].currentRoundScore = 0;
    }

}

void Session::bumpTime(){

    QDateTime cur = QDateTime::currentDateTime();
    cur = cur.addSecs(300);
    if(cardList.length() <= curIndex)
        return;
    cardList[curIndex].lastTry = cur;
}
QString Session::getNextCard(){

    if(cardList.length() == 0)
        return "-1";
    QDateTime min = cardList[0].lastTry;
    int minIndex = 0;
    for(int i = 0; i < cardList.length(); i++){
        if(cardList[i].lastTry < min){
            minIndex = i;
            min = cardList[i].lastTry;
        }
    }
    curIndex = minIndex;
    roundResponses = 0;
    usersRemoving = 0;
    currentRound++;
    int doneUsers = 0;
    for(int i =0; i < userList.length(); i++){
        if(userList[i].finished)
            i++;
    }
    thisRoundUsers = userList.length() - doneUsers;
    for(int i = 0; i < userList.length(); i++)
        userList[i].lastScore = -2;
    return cardList[minIndex].cardNum;
}
void Session::userLeaving(int sock){
    int remDex = -1;
    for(int i = 0; i < userList.length(); i++){
        if(userList[i].sock == sock){
            remDex = i;
            break;
        }
    }
    if(remDex != -1){
        if(userList[remDex].lastScore != -2)
            roundResponses--;
        thisRoundUsers--;
        userList.removeAt(remDex);
    }
}
bool Session::isEmpty(){
    if(userList.length() <= 0)
        return true;
    return false;
}
void Session::cleanUp(){
    for(int i = 0; i < MAXCARD; i++){
        cardBits[i] = 0;
        userList.clear();
        cardList.clear();
        currentRound = 0;
        usersRemoving = 0;
        roundResponses = 0;
        curIndex = 0;
        maxCard = 0;
        thisRoundUsers = 0;

    }
}
bool Session::userLeftCheck(){
    return roundResponses >= thisRoundUsers;
}
int Session::getRound(){
    return currentRound;
}
bool Session::doGuess(int first, int second){
    if(gameChecks[first] == second){
        gameMatches++;
        return true;
    }
    return false;
}
bool Session::gameOver(){
    if(gameMatches >= GAMEPIECES){
        return true;
        //might do cleanup here
    }
    return false;
}

bool Session::gameTime(){
    if(games == 0)
        return false;
    if(roundResponses < (thisRoundUsers-1))
        return false;
    int len = reviewList.length();
    if(len < GAMEPIECES)
        return false;
    if(currentRound % GAMEFREQUENCY == 0){
        if(currentRound == 0)
            return false;
        for(int i = 0; i < GAMEPIECES; i++){
            int index = rand() % len;
            gameCardList.push_back(reviewList[index]);
            reviewList.removeAt(index);
            len--;
        }
        setGameString();
        currentRound++;
        gameMatches = 0;
        return true;
    }
    return false;

}
bool Session::checkForReview(QString back){
    for(int i = 0; i < back.length(); i++){
        if(back[i] == ' ')
            return false;
    }
    return true;
}
void Session::setGameString(){
    if(gameCardList.length() < GAMEPIECES)
        return;
    int done = 0;
    QString curArr[GAMEPIECES *2];
    for(int i = 0; i < GAMEPIECES * 2; i++)
        gameChecks[i] = 0;
    while(done < GAMEPIECES){
        int index = rand() % gameCardList.length();
        int front = -1;
        int back = -1;
        while(front == -1){
            int check = rand() % (GAMEPIECES * 2);
            if(gameChecks[check] == 0){
                gameChecks[check] = 1;
                front = check;
            }
        }
        while(back == -1){
            int check = rand() % (GAMEPIECES * 2);
            if(gameChecks[check] == 0){
                gameChecks[check] = 1;
                back = check;
            }
        }
        gameChecks[front] = back;
        gameChecks[back] = front;
        curArr[front] = gameCardList[index].front;
        curArr[back] = gameCardList[index].back;
        gameCardList.removeAt(index);
        done++;
    }
    gameString = "";
    for(int i = 0; i < GAMEPIECES *2; i++){
        gameString += curArr[i];
        gameString += '\t';
    }
    gameString = gameString.left(gameString.length() - 1);

}
