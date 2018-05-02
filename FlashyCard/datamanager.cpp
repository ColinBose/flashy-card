#include "datamanager.h"
QString languageArr[] = {"GEN", "KOR", "JPN" };
#define GEN 0
#define KOR 1
#define JPN 2

DataManager::DataManager()
{
    QDir databasePath;
    QString path = databasePath.currentPath()+"/FlashyDB.db";
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    db.open();
    tableCreation();




}
bool DataManager::createCardTable(){
    QSqlQuery query;
    if (!query.exec("CREATE TABLE `CardTable` ("
                        "`_id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                        "`Deck`	TEXT NOT NULL,"
                        "`CardNum`	INTEGER NOT NULL,"
                        "`Front`	TEXT,"
                        "`Back`	TEXT,"
                        "`Due`	TEXT,"
                        "`Active`	TEXT,"
                        "`UpdateInterval`	INTEGER,"
                        "`Hint`	TEXT,"
                        "`PastGrade`	INTEGER,"
                        "`NumDone`	INTEGER,"
                        "`Stage`	INTEGER,"
                        "`Unit`	INTEGER,"
                        "`Sound` TEXT,"
                        "`Type` TEXT,"
                        "`Inactive` TEXT DEFAULT 'NO')"
                    ))
    {
         qDebug() << "SQL error: "<< query.lastError().text() << endl;
         return false;
    }
    return true;
}
bool DataManager::createDeckTable(){
    QSqlQuery query;
    if(!query.exec("CREATE TABLE `Deck` ("
                       "`_id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                       "`DeckID`	TEXT NOT NULL,"
                       "`DeckName`	TEXT NOT NULL,"
                       "`LastUpdated`	TEXT,"
                       "`UpdatesLeft`	INTEGER,"
                       "`Daily`	INTEGER,"
                       "`MaxInterval` INTEGER,"
                       "`Dictionary` TEXT)"
                   )){
        qDebug() << "SQL error: "<< query.lastError().text() << endl;
        return false;
    }
    return true;
}
bool DataManager::createStatTable(){
    QSqlQuery query;
    if (!query.exec("CREATE TABLE `Stats` ("
                        "`Date`	TEXT PRIMARY KEY,"
                        "`Attempted` INTEGER,"
                        "`Correct`	INTEGER,"
                        "`AvgPercent`	INTEGER,"
                        "`New`	INTEGER,"
                        "`Time`	TEXT,"
                        "`Strong`	INTEGER,"
                        "`Learning`	INTEGER,"
                        "`LongLearning`	INTEGER,"
                        "`LongStruggle`	INTEGER,"
                        "`NewStrong`	INTEGER,"
                        "`NewWeak`	INTEGER)"
                    ))
    {
         qDebug() << "SQL error: "<< query.lastError().text() << endl;
         return false;
    }
    return true;
}
void DataManager::addDaysToStats(){
    QSqlQuery query(db);
    QString currentDate = getDate();
    query.prepare("SELECT * FROM Stats WHERE Date = ?");
    QStringList daysToReAdd;
    QDate firstDate;
    query.bindValue(0, currentDate);
    query.exec();
    if(query.next())
        return;

    query.prepare("SELECT Date FROM Stats");
    if(!query.exec()){
        qDebug() << "Error in adddaytostats";
        printError(query);
    }
    QDate startDate;
    if(!query.next()){
        startDate = QDate::currentDate();
    }
    else{
        QString lastString = query.value(0).toString();
        while(query.next()){
            lastString = query.value(0).toString();
        }
        startDate = QDate::fromString(lastString, Qt::ISODate);
        firstDate = startDate;

    }
    QDate current = QDate::currentDate();
    while(startDate <= current){
        query.prepare("INSERT INTO Stats (Date) VALUES(?)");
        query.bindValue(0,startDate.toString(Qt::ISODate));
        if(startDate != current)
            daysToReAdd.push_back(startDate.toString(Qt::ISODate));
        query.exec();
        startDate = startDate.addDays(1);
    }
    reFillStats(daysToReAdd, firstDate);
}
void DataManager::reFillStats(QStringList days, QDate first){
    QSqlQuery query(db);
    QString date = first.toString(Qt::ISODate);
    query.prepare("SELECT Strong, Learning, LongLearning, LongStruggle, NewStrong, NewWeak FROM Stats WHERE Date = ? ");
    query.bindValue(0, date);
    int strong,learning,longlearning,longstruggle,newstrong,newweak;
    if(!query.exec()){
        qDebug() << "ERror in refill stats";
        printError(query);
        return;
    }
    query.next();
    strong = query.value(0).toInt();
    learning = query.value(1).toInt();
    longlearning= query.value(2).toInt();
    longstruggle= query.value(3).toInt();
    newstrong = query.value(4).toInt();
    newweak= query.value(5).toInt();

    for(int i = 0; i < days.length(); i++){
        query.prepare("UPDATE Stats SET Strong = ?, Learning = ?, LongLearning = ?, LongStruggle = ?, NewStrong = ?, NewWeak = ? WHERE Date = ?");
        query.bindValue(0,strong);
        query.bindValue(1,learning);
        query.bindValue(2,longlearning);
        query.bindValue(3,longstruggle);
        query.bindValue(4,newstrong);
        query.bindValue(5,newweak);
        query.bindValue(6,days[i]);
        if(!query.exec()){
            qDebug() << "Error back updating stats";
            printError(query);
        }
    }

}

QSqlQuery DataManager::getAllActive(){
    QSqlQuery query(db);
    query.prepare("SELECT UpdateInterval, PastGrade, NumDone FROM CardTable WHERE Active='YES' AND NumDone > 0 AND Inactive='NO'");
    if(!query.exec()){
        qDebug() << "Error in getAllactive";
        printError(query);
    }
    return query;
}

bool DataManager::tableCreation(){

    bool check = true;
    if(!db.tables().contains("CardTable")){
        qDebug() << "Creating CardTable";
        check = createCardTable();
    }
    if(!check){
        qDebug() << "Error creating Card Table";
        return false;
    }
    if(!db.tables().contains("Deck")){
        qDebug() << "Creating DeckTable";
        check = createDeckTable();
    }
    if(!check){
        qDebug() << "Error creating Deck Table";
        return false;
    }
    if(!db.tables().contains("Stats")){
        qDebug() << "Creating Stats Table";
        check = createStatTable();
    }
    if(!check){
        qDebug() << "Error creating Stat Table";
        return false;
    }
    return true;

}
QString DataManager::getDate(){
    QDate cur = QDate::currentDate();
    return cur.toString(Qt::ISODate);
}
QSqlQuery DataManager::getDecks(){
    QSqlQuery query(db);
    QString date = getDate();
    QString q = "SELECT DeckName, DName FROM Deck LEFT JOIN (SELECT Count(Deck) DName, Deck FROM CardTable WHERE ACTIVE = 'YES' AND Due <= '" + date +"'  AND Inactive='NO' GROUP BY Deck) as C ON  DeckName = c.Deck";
    query.exec(q);
    return query;
}


bool DataManager::createDeck(QString name, int maxInterval, int dailyUpdates){
    QSqlQuery query(db);
    QString code;
    QString date = getDate();
    if(deckExists(name))
        return false;
    code = getCode("Deck", "DeckID");
    query.prepare("INSERT INTO Deck (DeckID, DeckName, LastUpdated, UpdatesLeft, Daily, MaxInterval) VALUES(?,?,?,?,?,?)");
    query.bindValue(0, code);
    query.bindValue(1, name);
    query.bindValue(2, date);
    query.bindValue(3, dailyUpdates);
    query.bindValue(4, dailyUpdates);
    query.bindValue(5, maxInterval);

    if(!query.exec()){
        printError(query);
        return false;
    }
    return true;
}
void DataManager::updateMaxInterval(QString deck){
    QSqlQuery query(db);
    query.prepare("SELECT MaxInterval FROM Deck WHERE DeckName = ?");
    query.bindValue(0,deck);
    query.exec();
    query.next();
    MAXINTERVAL = query.value(0).toInt();

}

void DataManager::printError(QSqlQuery query){
    qDebug() << "SQL error: "<< query.lastError().text() << endl;

}

QString DataManager::getCode(QString table, QString row){
    QSqlQuery query(db);
    QString code;
    int seed = time(NULL);
    srand(seed);
    int i = 0;
    while(i++ < 100){
        code = genCode();
        QString q = "SELECT * FROM " + table + " WHERE " + row + " = '" + code + "'";

        if(!query.exec(q)){
            qDebug() << "Error in getCode";
            qDebug() << "SQL error: "<< query.lastError().text() << endl;
        }
        if(!query.next())
            return code;
    }
}
QString DataManager::genCode(){
    QString code = "";
    for(int i = 0; i < CODELENGTH; i++){
        int t = rand() % 26;
        t+=65;
        code += (char)t;
    }
    return code;
}
int DataManager::fastIndex(QString deck){
    int index, throwAway;
    getLastIndex(deck, &index, &throwAway);
    return index;
}
int DataManager::getInterval(QString deck){
    QSqlQuery query(db);
    query.prepare("SELECT MaxInterval FROM Deck WHERE DeckName = ?");
    query.bindValue(0,deck);
    if(!query.exec()){
        printError(query);
        return -1;
    }
    if(!query.next()){

        printError(query);
        return -1;
    }
    return query.value(0).toInt();
}

int DataManager::getType(QString back){

    int kor, jpn, gen;
    kor = 0;
    jpn = 0;
    gen = 0;
    for(int i =0; i < back.length(); i++){
        int c = back.at(i).unicode();
        if(c >= 44032 && c <= 55171)
            kor++;
        else if(c >= 65 && c <=122)
            gen++;
        else if(c >= 12352 && c <= 12543)
            jpn++;
        else if(c >= 19968 && c <= 40895)
            jpn++;

        //19968 - 40895
        //12352 - 12447
        //12448 - 12543


    }
    if(kor >= gen)
        if(kor >= jpn)
            return KOR;
        else
            return JPN;
    if(gen > jpn)
        return GEN;
    return JPN;
}

bool DataManager::fastAddCard(QString deck, QString front, QString back, QString sound, int unit, int index){
    QSqlQuery query(db);

    QString date = getDate();
    int cardType = getType(back);

    query.prepare("INSERT INTO CardTable (Deck, CardNum, Front, Back, Due, Active, UpdateInterval, Hint, PastGrade, NumDone, Stage, Unit, Sound, Type, Inactive)"
                  " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) ");
    query.bindValue(0,deck);
    query.bindValue(1,index);
    query.bindValue(2,front);
    query.bindValue(3, back);
    query.bindValue(4,date);
    query.bindValue(5,"NO");
    query.bindValue(6, 1);
    query.bindValue(7, "");
    query.bindValue(8, 0);
    query.bindValue(9, 0);
    query.bindValue(10, 0);
    query.bindValue(11, unit);
    query.bindValue(12, sound);
    query.bindValue(13, languageArr[cardType]);
    query.bindValue(14, "NO");
    return query.exec();
}

bool DataManager::addCard(QString deck, QString front, QString back){
    QSqlQuery query(db);
    int index, unit;
    QString date = getDate();
    int cardType = getType(back);
    getLastIndex(deck, &index, &unit);
    if(index == -1){
        qDebug() << "Error adding card";
        return false;
    }

    query.prepare("INSERT INTO CardTable (Deck, CardNum, Front, Back, Due, Active, UpdateInterval, Hint, PastGrade, NumDone, Stage, Unit, Sound, Type, Inactive)"
                  " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) ");
    query.bindValue(0,deck);
    query.bindValue(1,index);
    query.bindValue(2,front);
    query.bindValue(3, back);
    query.bindValue(4,date);
    query.bindValue(5,"NO");
    query.bindValue(6, 1);
    query.bindValue(7, "");
    query.bindValue(8, 0);
    query.bindValue(9, 0);
    query.bindValue(10, 0);
    query.bindValue(11, unit);
    query.bindValue(12, "");
    query.bindValue(13, languageArr[cardType]);
    query.bindValue(14, "NO");
    return query.exec();

}
void DataManager::getLastIndex(QString deck, int *index, int *unit){
    QSqlQuery query(db);
    query.prepare("SELECT * FROM CardTable WHERE Deck = ? ORDER BY CardNum DESC LIMIT 1");
    query.bindValue(0, deck);
    if(!query.exec())
        *index = -1;
    if(!query.next()){
        *index = 0;
        *unit = 0;
    }
    else{
        *index = query.value("CardNum").toInt()+1;
        *unit = query.value("Unit").toInt();
    }
}
int DataManager::getCurrentDue(QString deck){
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(Deck) FROM CardTable WHERE Deck = ? AND Active ='YES' AND NumDone = 0 AND Inactive='NO'" );
    query.bindValue(0, deck);
    if(!query.exec())
        return -1;
    if(!query.next())
        return -2;
    return query.value(0).toInt();

}
int DataManager::getDailyDue(QString deck){
    QSqlQuery query(db);
    query.prepare("SELECT UpdatesLeft FROM Deck WHERE DeckName = ?");
    query.bindValue(0, deck);
    if(!query.exec()){
        printError(query);
        return -1;
    }
    if(!query.next()){
        printError(query);
        return -2;
    }
    return query.value(0).toInt();
}
void DataManager::setDailyDue(QString deck, int numDue){
    QSqlQuery query(db);
    query.prepare("UPDATE CardTable SET Active = 'YES' WHERE _id IN"
                  "(SELECT _id FROM CardTable WHERE Deck = ? AND Active = 'NO' AND Inactive = 'NO' LIMIT ?)");
    query.bindValue(0, deck);
    query.bindValue(1, numDue);
    if(!query.exec())
        printError(query);
    int updated = query.numRowsAffected();
    updateNumLeft(deck, numDue-updated);
}
void DataManager::updateNumLeft(QString deck, int newNum){
    QSqlQuery query(db);
    QString date = getDate();
    query.prepare("UPDATE Deck SET UpdatesLeft = ?, LastUpdated = ? WHERE DeckName = ?");
    query.bindValue(0, newNum);
    query.bindValue(1, date);
    query.bindValue(2, deck);
    if(!query.exec()){
        qDebug() << "Error in updateNumLeft";
        printError(query);
    }
}

void DataManager::checkDailyReset(){
    QSqlQuery query(db);
    QSqlQuery updateQuery(db);
    QString date = getDate();

    query.prepare("SELECT DeckName, Daily from Deck WHERE LastUpdated < ?");
    query.bindValue(0, date);
    if(!query.exec()){
        qDebug() << "Error with select in checkDailyReset";
        printError(query);
    }
    while(query.next()){
        QString deck = query.value("DeckName").toString();
        int daily = query.value("Daily").toInt();
        updateQuery.prepare("UPDATE Deck SET UpdatesLeft = ?, LastUpdated = ? WHERE DeckName = ?");
        updateQuery.bindValue(0, daily);
        updateQuery.bindValue(1, date);
        updateQuery.bindValue(2, deck);
        if(!updateQuery.exec()){
            qDebug() << "Error in checkDailyReset";
            printError(updateQuery);
        }

    }
}
int DataManager::getCardTotal(QString deck){
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(_id) FROM CardTable WHERE Deck = ?");
    query.bindValue(0,deck);
    if(!query.exec()){
        qDebug() << "Error in getCardTotal";
        printError(query);
        return -1;

    }
    if(!query.next()){
        return -2;
    }
    return query.value(0).toInt();

}
int DataManager::getDueTotal(QString deck){
    QSqlQuery query(db);
    QString date = getDate();
    query.prepare("SELECT COUNT(_id) FROM CardTable WHERE Deck = ? AND Due <= ? AND Active='YES' AND Inactive='NO'");
    query.bindValue(0, deck);
    query.bindValue(1, date);
    if(!query.exec()){
        qDebug() << "Error in getDueTotal";
        printError(query);
        return -1;

    }
    if(!query.next()){
        return -2;
    }
    return query.value(0).toInt();
}
int DataManager::getSeenTotal(QString deck){
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(_id) FROM CardTable WHERE Deck = ? AND Active='YES' AND NumDone > 0");
    query.bindValue(0,deck);
    if(!query.exec()){
        qDebug() << "Error in getSeenTotal";
        printError(query);
        return -1;

    }
    if(!query.next()){
        return -2;
    }
    return query.value(0).toInt();
}
QSqlQuery DataManager::getSession(QString deck){
    QSqlQuery query(db);
    QString date = getDate();
    query.prepare("SELECT * FROM CardTable WHERE Deck = ? AND Due <= ? AND Active='YES' AND Inactive='NO' ORDER BY CardNum ASC");
    query.bindValue(0,deck);
    query.bindValue(1,date);

    if(!query.exec()){
        qDebug() << "Error in getSession";
        printError(query);
    }
    return query;
}
bool DataManager::updateDue(int newAverage, int newInterval, int newAttempts, int newStage, QString id){
    QDate newDue = QDate::currentDate();
    QSqlQuery query(db);
    if(newInterval > MAXINTERVAL)
        newInterval = MAXINTERVAL;
    newDue = newDue.addDays(newInterval);
    QString dueString = newDue.toString(Qt::ISODate);
    query.prepare("UPDATE CardTable SET PastGrade = ? , UpdateInterval = ? , NumDone = ? , Stage = ? , Due = ? WHERE _id = ?");
    query.bindValue(0,newAverage);
    query.bindValue(1,newInterval);
    query.bindValue(2,newAttempts);
    query.bindValue(3,newStage);
    query.bindValue(4,dueString);
    query.bindValue(5,id);
    if(!query.exec()){
          printError(query);
          return false;
    }
    return true;

}
bool DataManager::updateHint(QString hint, QString code){
    QSqlQuery query(db);
    query.prepare("UPDATE CardTable SET Hint = ? WHERE _id = ?");
    query.bindValue(0, hint);
    query.bindValue(1,code);
    return query.exec();
}
QSqlQuery DataManager::getCardsForExport(QString deck){
    QSqlQuery query(db);
    query.prepare("SELECT Front, Back, CardNum FROM CardTable WHERE Deck = ?");
    query.bindValue(0, deck);
    if(!query.exec()){
          printError(query);
          qDebug() << "Error in getAllCards";
    }
    return query;
}

QSqlQuery DataManager::getAllCards(QString deck){
    QSqlQuery query(db);
    query.prepare("SELECT Front, Unit, Inactive FROM CardTable WHERE Deck = ? ORDER BY Unit ASC");
    query.bindValue(0, deck);
    if(!query.exec()){
          printError(query);
          qDebug() << "Error in getAllCards";
    }
    return query;
}
QStringList DataManager::getUnits(QString deck){
    QSqlQuery query(db);
    QStringList ret;
    query.prepare("SELECT DISTINCT Unit FROM CardTable WHERE Deck = ? ORDER BY Unit ASC");
    query.bindValue(0,deck);
    if(!query.exec()){
          printError(query);
          qDebug() << "Error in getUnit";
          return ret;
    }
    while(query.next()){
        ret.push_back(query.value(0).toString());
    }
    return ret;
}
QString DataManager::getDictFile(QString deck){
    QSqlQuery query(db);
    query.prepare("SELECT Dictionary FROM Deck WHERE DeckName = ?");
    query.bindValue(0, deck);
    if(!query.exec()){
          printError(query);
          qDebug() << "Error in getFile";
          return "";
    }
    query.next();
    return query.value("Dictionary").toString();
}
void DataManager::updateCardLevels(int strong, int learning, int longLearning, int longStruggle, int newStrong, int newWeak){
    QSqlQuery query(db);
    QString curDate = getDate();
    query.prepare("UPDATE Stats SET Strong = ?, Learning = ?, LongLearning = ?, LongStruggle = ?, NewStrong = ?, NewWeak = ? WHERE Date = ?");
    query.bindValue(0,strong);
    query.bindValue(1,learning);
    query.bindValue(2,longLearning);
    query.bindValue(3,longStruggle);
    query.bindValue(4,newStrong);
    query.bindValue(5,newWeak);
    query.bindValue(6,curDate);
    if(!query.exec()){
        qDebug() << "Error updating stats table";
        printError(query);
    }
}
QSqlQuery DataManager::getTodaysStats(){
    QSqlQuery query(db);
    QString date = getDate();
    query.prepare("SELECT Attempted, Correct, AvgPercent, New, Time FROM Stats WHERE Date = ?");
    query.bindValue(0, date);
    if(!query.exec()){
        qDebug() << "Error pulling past stats";
        printError(query);
    }
    return query;
}
void DataManager::setActive(QString deck, int unit){
    QSqlQuery query(db);
    query.prepare("UPDATE CardTable SET Inactive = 'NO' WHERE Deck = ? AND Unit = ?");
    query.bindValue(0, deck);
    query.bindValue(1, unit);
    if(!query.exec()){
        qDebug() << "Error with set active";
        printError(query);
    }
}
void DataManager::setInactive(QString deck, int unit){
    QSqlQuery query(db);
    query.prepare("UPDATE CardTable SET Inactive = 'YES' WHERE Deck = ? AND Unit = ?");
    query.bindValue(0, deck);
    query.bindValue(1, unit);
    if(!query.exec()){
        qDebug() << "Error with set inactive";
        printError(query);
    }
}
void DataManager::updateStats(int attempted, int correct, int avgPercent, int newCards, QTime studyTime){
    QSqlQuery query(db);
    QString time = studyTime.toString(Qt::ISODate);
    QString day = QDate::currentDate().toString(Qt::ISODate);
    checkTodayStat(day);
    query.prepare("UPDATE Stats SET Attempted = ?, Correct = ?, AvgPercent = ?, New = ?, Time = ? WHERE Date = ?");
    query.bindValue(0,attempted);
    query.bindValue(1,correct);
    query.bindValue(2,avgPercent);
    query.bindValue(3,newCards);
    query.bindValue(4,time);
    query.bindValue(5,day);
    if(!query.exec()){
        qDebug() << "Error updating stats";
        printError(query);
    }
}
void DataManager::checkTodayStat(QString date){
    QSqlQuery query(db);
    query.prepare("SELECT Date FROM Stats WHERE Date = ?");
    query.bindValue(0, date);
    if(!query.exec()){
        qDebug() << "Error checking todays stats";
    }
    if(!query.next()){
        query.prepare("INSERT INTO Stats (Date) VALUES(?)");
        query.bindValue(0,date);
        if(!query.exec()){
            qDebug() << "Error adding new row to stats";
        }
    }

}
QList<statData> DataManager::getHistory(QDate start ){
    QSqlQuery query(db);
    QList<statData> retStats;
    QString startDate = start.toString(Qt::ISODate);
    query.prepare("SELECT * FROM Stats WHERE Date >= ?");
    query.bindValue(0, startDate);
    if(!query.exec()){
        qDebug() << "Error with getHistory";
        printError(query);
    }
    while(query.next()){
        statData data;
        data.attempted = query.value("Attempted").toInt();
        data.completed = query.value("Correct").toInt();
        data.pastAvg = query.value("AvgPercent").toInt();
        data.newCards = query.value("New").toInt();
        data.studyTime = QTime::fromString(query.value("Time").toString(), Qt::ISODate);

        data.strong = query.value("Strong").toInt();
        data.learning = query.value("Learning").toInt();
        data.longlearning = query.value("LongLearning").toInt();
        data.longstruggle = query.value("LongStruggle").toInt();
        data.newStrong = query.value("NewStrong").toInt();
        data.newWeak = query.value("NewWeak").toInt();


        retStats.push_back(data);
    }

    return retStats;
}

QString DataManager::addImportDeck(QString deckID, QString deckName){

    QSqlQuery query(db);
    QString code;
    QString date = getDate();
    QString deckCheck = deckName;
    int i = 1;
    while(deckExists(deckCheck)){
        deckCheck = deckName;
        deckCheck += "_";
        deckCheck += QString::number(i);
        i++;
    }
    deckName = deckCheck;
    query.prepare("INSERT INTO Deck (DeckID, DeckName, LastUpdated, UpdatesLeft, Daily, MaxInterval) VALUES(?,?,?,?,?,?)");
    query.bindValue(0, deckID);
    query.bindValue(1, deckName);
    query.bindValue(2, date);
    query.bindValue(3, STANDARDDAILY);
    query.bindValue(4, STANDARDDAILY);
    query.bindValue(5, STANDARDINTERVAL);

    if(!query.exec()){
        printError(query);
        return "";
    }

    return deckCheck;
}
int DataManager::addALLCard(QList<shortCard> cardList, QString deckName){
      db.transaction();
    QSqlQuery query(db);
    QString date = getDate();
    query.prepare("INSERT INTO CardTable (Deck, CardNum, Front, Back, Due, Active, UpdateInterval, Hint, PastGrade, NumDone, Stage, Unit, Sound, Type, Inactive)"
                  " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) ");
    for(int i = 0; i < cardList.length(); i++){

        int cardType = getType(cardList[i].back);
        query.bindValue(0,deckName);
        query.bindValue(1,cardList[i].cardNum);
        query.bindValue(2,cardList[i].front);
        query.bindValue(3, cardList[i].back);
        query.bindValue(4,date);
        query.bindValue(5,"NO");
        query.bindValue(6, 1);
        query.bindValue(7, "");
        query.bindValue(8, 0);
        query.bindValue(9, 0);
        query.bindValue(10, 0);
        query.bindValue(11, (int)i/50);
        query.bindValue(12, "");
        query.bindValue(13, languageArr[cardType]);
        query.bindValue(14, "NO");
        query.exec();
    }
    db.commit();
}
bool DataManager::deckExists(QString deckName){
    QSqlQuery query(db);
    query.prepare("SELECT * FROM Deck WHERE DeckName = ?");
    query.bindValue(0, deckName);
    if(!query.exec()){
        qDebug() << "Error in deck Check";
        printError(query);
    }
    return query.next();

}
void DataManager::updateDeckID(QString id, QString deckName){
    QSqlQuery query(db);
    query.prepare("UPDATE Deck SET DeckID = ? WHERE DeckName = ?");
    query.bindValue(0, id);
    query.bindValue(1, deckName);
    if(!query.exec()){
        qDebug() << "Error updating deck ID";
        printError(query);
    }

}
void DataManager::importDeckFromFile(QString deckName, QList<shortCard> cardList){
    db.transaction();
    QSqlQuery query(db);
    QString date = getDate();
    query.prepare("INSERT INTO CardTable (Deck, CardNum, Front, Back, Due, Active, UpdateInterval, Hint, PastGrade, NumDone, Stage, Unit, Sound, Type, Inactive)"
                " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) ");
    for(int i = 0; i < cardList.length(); i++){

      int cardType = getType(cardList[i].back);
      query.bindValue(0,deckName);
      query.bindValue(1, i);
      query.bindValue(2,cardList[i].front);
      query.bindValue(3, cardList[i].back);
      query.bindValue(4,date);
      query.bindValue(5,"NO");
      query.bindValue(6, 1);
      query.bindValue(7, "");
      query.bindValue(8, 0);
      query.bindValue(9, 0);
      query.bindValue(10, 0);
      query.bindValue(11, cardList[i].unit);
      query.bindValue(12, cardList[i].sound);
      query.bindValue(13, languageArr[cardType]);
      query.bindValue(14, "NO");
      query.exec();
    }
db.commit();
}
QString DataManager::getDeckId(QString deckName){
    QSqlQuery query(db);
    query.prepare("SELECT DeckID FROM Deck WHERE DeckName = ?");
    query.bindValue(0, deckName);
    if(!query.exec()){
        qDebug() << "Error updating deck ID";
        printError(query);
    }
    if(!query.next())
        return "";
    return query.value(0).toString();
}
