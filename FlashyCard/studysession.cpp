#include "studysession.h"
#define IGNOREDLEN 3
#define PASTINTERVALWEIGHT 0.5
#define CURRENTAVERAGEWEIGHT 1.5
#define MINCURRENTAVERAGEWEIGHT 5
char ignoredChars[] = {' ', '?', '-'};
StudySession::StudySession()
{

}
void StudySession::setDeck(QString deck){
    deckName = deck;
}
QString StudySession::getDeckName(){
    return deckName;
}
bool StudySession::setSession(DataManager db){
    int interval, past, numDone, stage;
    QString id, front,back,audio,hint, type;
    int cardNum;
    session.clear();
    QSqlQuery result = db.getSession(deckName);
    while(result.next()){
        //QString code, QString front, QString back, QString audio, QString hint,
        //QDate due, int interval, int past, int numDone, int stage){
        id = result.value("_id").toString();
        front = result.value("Front").toString();
        cardNum = result.value("CardNum").toInt();

        back = result.value("Back").toString();
        audio = result.value("Sound").toString();
        hint = result.value("Hint").toString();
        type = result.value("Type").toString();

        interval = result.value("UpdateInterval").toInt();
        past = result.value("PastGrade").toInt();
        numDone = result.value("NumDone").toInt();
        stage = result.value("Stage").toInt();

        Card c(id, front,back,audio,hint,interval,past,numDone,stage, type,cardNum);
        session.pushBack(c);


    }
    if(session.empty())
        return false;
    return true;
}
bool StudySession::getNext(Card ** c){
    QDateTime currentTime = QDateTime::currentDateTime();
    session.setCurrentNull();
    if(session.empty())
        return false;
    bool found = false;
    Card * minTimeCard;
    session.getHead(&minTimeCard);
    while(session.next(c)){
        if((*c)->lastTry < currentTime){
            return true;
        }
        if((*c)->lastTry < minTimeCard->lastTry)
            minTimeCard = *c;
    }
    if(!found)
        *c = minTimeCard;
    session.setIndex(minTimeCard->code);

   return true;

}
bool StudySession::getMultiNext(Card ** c, int cardNum){
    session.setCurrentNull();
    wasReserve = false;
    if(session.findCard(c, cardNum))
        return true;
    if(reserveList.length() == 0)
        return false;
    wasReserve = true;
    *c = &reserveList[0];
    return true;
}

void StudySession::setMaxInterval(int interval){
    maxInterval = interval;
}

QString StudySession::loadHint(Card c, bool sentence){
    if(!sentence)
        return generalHint(c);
    if(c.stage >= 2)
        return generalHint(c);
    QStringList testParts = c.back.split(" ");
    if(testParts.length() < 2 && c.type != "JPN")
        return generalHint(c);
    QStringList parts = c.back.split(" ");
    QString retString = "";
    if(c.type != "JPN"){
        for(int i = 0; i < parts.length(); i++){
            QString curWord = "";
            for(int j = 0; j < parts[i].length(); j++){
                curWord+=parts[i][j];
                QChar newChar = parts[i][j];
                if(gram.checkExists(c.type, curWord)){
                     retString += "-";
                }
                else{
                    retString+= newChar;
                }
            }
            if(i < (parts.length()-1))
                   retString+= " ";

        }
    }
    else{
        for(int i = 0; i < parts.length(); i++){
            QString curWord = "";
            for(int j = 0; j < parts[i].length(); j++){
                curWord+=parts[i][j];
                QChar newChar = parts[i][j];
                if(gram.checkExists(c.type, curWord)){
                     retString += "-";
                }
                else{
                    retString+= newChar;
                    curWord = "";
                }
            }
            if(i < (parts.length()-1))
                   retString+= " ";

        }
    }
    if(c.stage == 0){

        return retString;
    }
    else{
        return flipString(retString, c.back);
    }
}
QString StudySession::flipString(QString curHint, QString answer){
    if(curHint.length() != answer.length()){
        qDebug() << "Something went wrong with the hint";
        return curHint;
    }
    QString newHint = "";
    for(int i = 0 ; i < curHint.length(); i++){
        if(curHint[i] == '-'){
            newHint+= answer[i];
        }
        else{
            if(curHint[i] == ' ')
                newHint+= ' ';
            else if(newHint[i] == '?')
                newHint+= '?';
            else
                newHint += '-';
        }
    }
    return newHint;

}

QString StudySession::generalHint(Card c){
    QString ret = "";
    for(int i = 0; i < c.back.length(); i++){
        if(c.back.at(i) == ' ')
            ret+=" ";
        else if(c.back.at(i) == '?')
            ret +="?";
        else
            ret += '-';
    }
    return ret;
}
int StudySession::remain(){
    return session.size();
}

QString StudySession::evalAnswer(Card c, QString answer, bool sentence, int * ret, QString hint){
    QString colouredReturn;
    if(c.type == "KOR"){
        KoreanCard k;
        if(sentence){
            return generalSentenceEval(c, answer, ret, hint);

        }
        else{
            if(c.back.length() != answer.length())
                return generalSentenceEval(c, answer, ret, hint);
            else
                return k.vocabEval(c, answer, ret, hint);
        }
    }else if(c.type == "JPN"){
        return generalSentenceEval(c, answer,ret, hint);
    }
    else
        return generalSentenceEval(c, answer,ret, hint);

}
QString StudySession::generalSentenceEval(Card c, QString answer, int * ret, QString hint){
    int right = 0;
    int len = c.back.length();
    bool ignoredFound;
    bool correctFound;
    hint = padHint(c.back,hint);
    QString retString = "";
    for(int i = 0; i < c.back.length(); i++){
        ignoredFound = false;
        correctFound = false;
        for(int p = 0; p < IGNOREDLEN; p++){
            if(c.back.at(i) == ignoredChars[p]){
                len--;
                ignoredFound = true;
                retString += c.back.at(i);
                break;
            }
        }
        if(ignoredFound)
            continue;
        if(hint[i] != '-'){
            len--;
            if(c.back[i] != answer[i])
                retString += addBlue(c.back.at(i));
            else
                retString += c.back.at(i);
            continue;
        }


        for(int j = 0; j < answer.length(); j++){
            if(c.back.at(i) == answer.at(j)){
                right++;
                answer[j] = '~';
                correctFound = true;
                break;
            }
        }
        if(correctFound)
            retString += addGreen(c.back.at(i));
        else
            retString += addRed(c.back.at(i));
    }

    double ans = (double)right/len;
    ans*=100;
    *ret = (int)round(ans);
    return retString;
}
void StudySession::remove(){
    session.removeCur();
}

int StudySession::getAverage(Card *c, int ans){
    return computeAverage(c->past, c->numDone, ans, MINCURRENTAVERAGEWEIGHT);
}
int StudySession::getInterval(Card *c, int ans){
    double curInt = c->interval;
    if(curInt == 0)
        curInt = 1;
    double curAvg = c->past;
    if(c->newCard)
        curInt = 100;
    int scoreDif = ans - curAvg;
    double awayFromPerfect = 100.1 - ans;
    double pastPart = curAvg * 1.35 / 100 * curInt;
    double logScaling = log10(awayFromPerfect * 10) / log10(20) * 0.3;
    double lossPart;
    if(scoreDif < 0){
        lossPart = log10(scoreDif*scoreDif) / 6 * curInt;
    }else{
        lossPart = 0;
    }
    double totalAdjust = pastPart + logScaling - lossPart + 1;
    int ret = round(totalAdjust);
    if(totalAdjust < 1){
        qDebug() << "error in interval formula";
        totalAdjust = 1;

    }
    return ret;
}
int StudySession::computeAverage(int past, int numPast, int cur, int maxPast){
    if(numPast > maxPast)
        numPast = maxPast;
    double total = past*numPast + cur;
    return (int)round(total/(numPast+1));
}
bool StudySession::isSentences(){
    return session.checkFirstFew();
}
void StudySession::loadGrammar(DataManager db){
    QStringList types = session.checkTypes();
    QString file = db.getDictFile(deckName);
    gram.loadLists(types, file);

}
QString StudySession::padHint(QString answer, QString hint){
    if(hint.length() >= answer.length())
        return hint;
    int dif = answer.length() - hint.length();
    QString ret = hint;
    for(int i = 0; i < dif; i++){
        ret += '-';
    }
    return ret;
}
void StudySession::clean(){
    session.clear();
    gram.cleanUp();
    reserveList.clear();
}
bool StudySession::checkSentence(QString back){
    for(int i = 0; i < back.length(); i++){
        if(back[i] ==  ' ')
            return true;
    }
    return false;
}
void StudySession::loadMultiCardList(DataManager db, QList<int> cardList, bool independant){

    int interval, past, numDone, stage;
    QString id, front,back,audio,hint, type;
    int cardNum;

    session.clear();
    QList<Card> current;
    QList<Card> reserve;
    QSqlQuery result = db.getSession(deckName);
    while(result.next()){
        //QString code, QString front, QString back, QString audio, QString hint,
        //QDate due, int interval, int past, int numDone, int stage){
        id = result.value("_id").toString();
        cardNum = result.value("CardNum").toInt();
        front = result.value("Front").toString();
        back = result.value("Back").toString();
        audio = result.value("Sound").toString();
        hint = result.value("Hint").toString();
        type = result.value("Type").toString();
        interval = result.value("UpdateInterval").toInt();
        past = result.value("PastGrade").toInt();
        numDone = result.value("NumDone").toInt();
        stage = result.value("Stage").toInt();

        Card c(id, front,back,audio,hint,interval,past,numDone,stage, type, cardNum);
        if(!independant){
            bool found = false;
            for(int i = 0; i < cardList.length(); i++){
                if(cardList[i] > cardNum)
                    break;
                if(cardList[i] == cardNum)
                    found = true;
            }
            if(found){
                current.push_back(c);
            }
            else{
                reserve.push_back(c);
            }
        }
        else{
            current.push_back(c);
        }



    }
    for(int i = 0; i<current.length();i++){
        session.pushBack(current[i]);
    }
    for(int i = 0; i<reserve.length();i++){
        addReserve(reserve[i]);
    }

}
void StudySession::addReserve(Card c){
    reserveList.push_back(c);
}
bool StudySession::complete(){
    if(session.empty() && reserveList.length() == 0)
        return true;
    return false;
}
