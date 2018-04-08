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

    session.clear();
    QSqlQuery result = db.getSession(deckName);
    while(result.next()){
        //QString code, QString front, QString back, QString audio, QString hint,
        //QDate due, int interval, int past, int numDone, int stage){
        id = result.value("_id").toString();
        front = result.value("Front").toString();
        back = result.value("Back").toString();
        audio = result.value("Sound").toString();
        hint = result.value("Hint").toString();
        type = result.value("Type").toString();

        interval = result.value("UpdateInterval").toInt();
        past = result.value("PastGrade").toInt();
        numDone = result.value("NumDone").toInt();
        stage = result.value("Stage").toInt();

        Card c(id, front,back,audio,hint,interval,past,numDone,stage, type);
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
void StudySession::setMaxInterval(int interval){
    maxInterval = interval;
}

QString StudySession::loadHint(Card c, bool sentence){
    if(!sentence)
        return generalHint(c);
    if(c.stage >= 2)
        return generalHint(c);
    QStringList testParts = c.back.split(" ");
    if(testParts.length() < 2)
        return generalHint(c);
    QStringList parts = c.back.split(" ");
    QString retString = "";
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
    int curInt = c->interval;
    int curAvg = c->past;
    int numDone = c->numDone;
    int newInt;
    int newAvg = computeAverage(curAvg, numDone, ans,4);
    double pastPart, futurePart;
    pastPart = PASTINTERVALWEIGHT * curInt;
    futurePart = newAvg*curInt*CURRENTAVERAGEWEIGHT/100;
    return (int)round(pastPart + futurePart);
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
}
bool StudySession::checkSentence(QString back){
    for(int i = 0; i < back.length(); i++){
        if(back[i] ==  ' ')
            return true;
    }
    return false;
}
