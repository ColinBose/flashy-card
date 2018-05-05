#include "datamanager.h"
#define CODELENGTH 6
DataManager::DataManager()
{
    QDir databasePath;
    QString path = databasePath.currentPath()+"/ServerDB.db";
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    db.open();
    sem_init(&dbLock, 0, 1);
    tableCreation();
    testQuery = new QSqlQuery(db);
}

bool DataManager::tableCreation(){
    bool firstTime = false;
    bool check = true;
    if(!db.tables().contains("CardTable")){
        qDebug() << "Creating CardTable";
        check = createCardTable();
        firstTime =true;
    }
    if(!check){
        qDebug() << "Error creating Card Table";
        return false;
    }
    if(!db.tables().contains("DeckTable")){
        qDebug() << "Creating DeckTable";
        check = createDeckTable();
        firstTime =true;
    }
    if(!check){
        qDebug() << "Error creating Deck Table";
        return false;
    }
    if(!db.tables().contains("UserTable")){
        qDebug() << "Creating Stats Table";
        check = createUserTable();
        firstTime =true;
    }
    if(!check){
        qDebug() << "Error creating Stat Table";
        return false;
    }
    if(firstTime){
        doBasicTestSetup();
    }
    return true;

}
bool DataManager::createCardTable(){
    QSqlQuery query;
    if (!query.exec("CREATE TABLE `CardTable` ("
                        "`_id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                        "`Deck`	TEXT NOT NULL,"
                        "`CardNum`	INTEGER NOT NULL,"
                        "`Front`	TEXT,"
                        "`Back`	TEXT)"
                    ))
    {
         qDebug() << "SQL error: "<< query.lastError().text() << endl;
         return false;
    }
    return true;
}
bool DataManager::createDeckTable(){
    QSqlQuery query;
    if(!query.exec("CREATE TABLE `DeckTable` ("
                       "`_id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                       "`DeckID`	TEXT NOT NULL,"
                       "`DeckName`	TEXT NOT NULL,"
                       "`Creator`	TEXT,"
                       "`Description`	TEXT,"
                       "`Language`	TEXT,"
                       "`TotalCards` INTEGER)"
                   )){
        qDebug() << "SQL error: "<< query.lastError().text() << endl;
        return false;
    }
    return true;
}
bool DataManager::createUserTable(){
    QSqlQuery query;
    if (!query.exec("CREATE TABLE `UserTable` ("
                        "`_id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                        "`UserName`	Text,"
                        "`Pass`	Text,"
                        "`Friends`	Text)"
                    ))
    {
         qDebug() << "SQL error: "<< query.lastError().text() << endl;
         return false;
    }
    return true;
}
QString DataManager::forceAddDeck(QString deck, QString desc, QString lang, QString user){

    QSqlQuery query(db);
    query.prepare("INSERT INTO DeckTable (DeckID, DeckName,Creator,Description,Language,TotalCards) VALUES(?,?,?,?,?,?)");
    query.bindValue(0,deck);
    query.bindValue(1,"TESTING");
    query.bindValue(2,user);
    query.bindValue(3,desc);
    query.bindValue(4,lang);
    query.bindValue(5,0);
    if(!query.exec()){
        qDebug() << "Error with initial setup";
        printError(query);
    }
    return "";
}
QString DataManager::addDeck(QString deck, QString desc, QString lang, QString user){
    QString code;
    int seed = time(NULL);
    srand(seed);
    int i =  100;
    while(i > 0){
        code = genCode();
        qDebug() << code;
        i--;
        if(deckExists(code))
            break;
    }
    QSqlQuery query(db);
    query.prepare("INSERT INTO DeckTable (DeckID, DeckName,Creator,Description,Language,TotalCards) VALUES(?,?,?,?,?,?)");
    query.bindValue(0,code);
    query.bindValue(1,deck);
    query.bindValue(2,user);
    query.bindValue(3,desc);
    query.bindValue(4,lang);
    query.bindValue(5,0);
    if(!query.exec()){
        qDebug() << "Error adding deck";
        printError(query);
        code = "";
    }
    return code;

}
bool DataManager::deckExists(QString code){
    QSqlQuery query(db);
    query.prepare("SELECT * FROM DeckTable WHERE DeckID = ?");
    query.bindValue(0,code);
    if(!query.exec()){
        printError(query);
        qDebug() << "Error in deckExists";
        return false;
    }
    if(query.next())
        return false;
    return true;
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

void DataManager::printError(QSqlQuery query){
    qDebug() << "SQL error: "<< query.lastError().text() << endl;

}
void DataManager::addCard(QString deckID, QString cardNum, QString front, QString back){
    QSqlQuery query(db);
    query.prepare("INSERT INTO CardTable (Deck, CardNum, Front,Back) VALUES(?,?,?,?)");
    query.bindValue(0,deckID);
    query.bindValue(1,cardNum);
    query.bindValue(2,front);
    query.bindValue(3,back);
    if(!query.exec()){
        qDebug() << "Error adding code";
        printError(query);

    }
}
QStringList DataManager::getDeckList(){

    QSqlQuery query(db);
    query.prepare("SELECT * FROM DeckTable");
    if(!query.exec()){
        qDebug() << "Error adding code";
        printError(query);

    }
    QStringList ret;
    while(query.next()){
        QString line = "";
        line += query.value("DeckID").toString();
        line += '\t';
        line += query.value("DeckName").toString();
        line += '\t';
        line += query.value("Creator").toString();
        line += '\t';
        line += query.value("Description").toString();
        line += '\t';
        line += query.value("Language").toString();
        line += '\t';
        line += query.value("TotalCards").toString();
        ret.push_back(line);
    }
    return ret;

}
void DataManager::addCards(QString deckID, QStringList cards){
    sem_wait(&dbLock);
    db.transaction();
    QSqlQuery query(db);
    query.prepare("INSERT INTO CardTable (Deck, CardNum, Front,Back) VALUES(?,?,?,?)");

    for(int i = 3; i < cards.length(); i++){
        QStringList cardData = cards[i].split('\t');
        if(cardData.length() != 3)
            continue;
        QString front, back, code;
        code = cardData[0];
        front = cardData[1];
        back = cardData[2];
        query.bindValue(0,deckID);
        query.bindValue(1,code);
        query.bindValue(2,front);
        query.bindValue(3,back);
        if(!query.exec()){
            qDebug() << "Error in add cards";
            printError(query);
        }
    }
    db.commit();
    sem_post(&dbLock);
    updateCardCount(deckID);
    sem_wait(&dbLock);
}
void DataManager::updateCardCount(QString deckId){
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(_id) FROM CardTable WHERE Deck = ?");
    query.bindValue(0, deckId);
    if(!query.exec()){
        qDebug() << "Error in updateCardCount";
        printError(query);
    }
    int total = 0;
    if(query.next())
        total = query.value(0).toInt();
    query.clear();
    query.prepare("UPDATE DeckTable SET TotalCards = ? WHERE DeckID = ?");
    query.bindValue(0, total);
    query.bindValue(1, deckId);
    if(!query.exec()){
        qDebug() << "Error in updateCardCount";
        printError(query);
    }
    sem_post(&dbLock);
}

QList<fbCard> DataManager::getAllCards(QString deckID){
    sem_wait(&dbLock);
    //QSqlQuery query(db);
    QList<fbCard> retList;
    testQuery->prepare("SELECT CardNum, Front, Back from CardTable WHERE Deck = ?");
    testQuery->bindValue(0, deckID);
    if(!testQuery->exec()){
        qDebug() << "Error in getAllCards";
        //printError(query);
        sem_post(&dbLock);
        return retList;
    }
    while(testQuery->next()){
        fbCard f;
        f.cardNum = testQuery->value(0).toString();
        f.front = testQuery->value(1).toString();
        f.back = testQuery->value(2).toString();
        retList.push_back(f);
    }
    testQuery->clear();
    sem_post(&dbLock);
    return retList;
}
QString DataManager::getDeckName(QString deckID){
    sem_wait(&dbLock);
    QSqlQuery query(db);
    query.prepare("SELECT DeckName FROM DeckTable WHERE DeckID = ?");
    query.bindValue(0, deckID);
    if(!query.exec()){
        qDebug() << "Error in getDeckName";
        printError(query);
        sem_post(&dbLock);
        return "";
    }
    if(!query.next()){
       sem_post(&dbLock);
       return "";
    }

    sem_post(&dbLock);
    return query.value(0).toString();
}
bool DataManager::registerUser(QString name, QString pass){
    if(userAlreadyExists(name))
        return false;
    QSqlQuery query(db);
    query.prepare("INSERT INTO UserTable (UserName, Pass, Friends) VALUES(?,?,?)");
    query.bindValue(0,name);
    query.bindValue(1,pass);
    query.bindValue(2,"");
    if(!query.exec()){
        qDebug() << "Error registering user";
        printError(query);
        return false;
    }
    return true;
}
bool DataManager::userAlreadyExists(QString name){
    QSqlQuery query(db);
    query.prepare("SELECT * FROM UserTable WHERE UserName = ?");
    query.bindValue(0, name);
    if(!query.exec()){
        qDebug() << "Error in userAlready exists";
        printError(query);
        return true;
    }
    return query.next();
}

QString DataManager::getUser(QString name){
    QSqlQuery query(db);
    query.prepare("SELECT * FROM UserTable WHERE UserName = ?");
    query.bindValue(0, name);
    if(!query.exec()){
        qDebug() << "Error in getUser";
        printError(query);
        return "";
    }
    if(!query.next())
        return "";
    return query.value("Pass").toString();

}
QStringList DataManager::getFriends(QString name){
    QSqlQuery query(db);
    QStringList ret;
    query.prepare("SELECT Friends FROM UserTable WHERE UserName = ?");
    query.bindValue(0, name);
    if(!query.exec()){
        qDebug() << "Error in getFriends";
        printError(query);
        ret;
    }
    if(!query.next()){
        return ret;
    }
    QString friendString = query.value("Friends").toString();
    QStringList parts = friendString.split(',');
    for(int i = 0; i < parts.length(); i++){
        if(parts[i].length() > 0){
            ret.push_back(parts[i]);
        }
    }
    return ret;
}
bool DataManager::addFriend(QString name, QString friendName){
    QSqlQuery query(db);
    query.prepare("SELECT Friends FROM UserTable WHERE Username = ?");
    query.bindValue(0, name);
    if(!query.exec()){
        qDebug() << "Error in addFriend";
        printError(query);
        return false;
    }
    if(!query.next()){
        return false;
    }
    QStringList friendSplit = query.value(0).toString().split(',');
    for(int i = 0; i < friendSplit.length(); i++){
        if(friendName == friendSplit[i])
            return false;
    }
    QString friendString = query.value(0).toString();
    if(!userExists(friendName))
        return false;
    friendString += friendName;
    friendString += ',';
    query.clear();
    query.prepare("UPDATE UserTable SET Friends = ? WHERE UserName = ?");
    query.bindValue(0, friendString);
    query.bindValue(1, name);
    if(!query.exec()){
        qDebug() << "Error in addFriend - update list section";
        printError(query);
        return false;
    }
    return true;


}
bool DataManager::userExists(QString name){
    QSqlQuery query(db);
    query.prepare("SELECT _id FROM UserTable WHERE UserName = ?");
    query.bindValue(0, name);
    if(!query.exec()){
        qDebug() << "Error in getUser";
        printError(query);
        return false;
    }
    if(!query.next())
        return false;
    return true;
}
bool DataManager::removeFriend(QString name, QString friendName){
    QSqlQuery query(db);
    query.prepare("SELECT Friends FROM UserTable WHERE Username = ?");
    query.bindValue(0, name);
    if(!query.exec()){
        qDebug() << "Error in addFriend";
        printError(query);
        return false;
    }
    if(!query.next()){
        return false;
    }
    bool ret = false;
    QString friendString;
    QStringList friendSplit = query.value(0).toString().split(',');
    for(int i = 0; i < friendSplit.length(); i++){
        if(friendName == friendSplit[i]){
            ret = true;
            continue;
        }
        friendString += friendSplit[i];
        friendString += ',';
    }

    query.prepare("UPDATE UserTable SET Friends = ? WHERE UserName = ?");
    query.bindValue(0, friendString);
    query.bindValue(1, name);
    if(!query.exec()){
        qDebug() << "Error in addFriend - update list section";
        printError(query);
        return false;
    }
    return true;

}
void DataManager::doBasicTestSetup(){
    QString deckID = "AAAAAA";
    forceAddDeck(deckID, "Test Deck", "TESTLANG", "ADMIN");
    QString front = "test";
    QString back = "ans";
    for(int i = 0; i < 100; i++){
        QString insFront = front;
        insFront += QString::number(i);
        QString insBack= back;
        insBack += QString::number(i);
        addCard(deckID, QString::number(i),insFront, insBack);
    }
    updateCardCount(deckID);
}
