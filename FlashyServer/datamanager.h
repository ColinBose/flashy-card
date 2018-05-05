#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QtSql>
#include <QtDebug>
#include <QList>
#include <semaphore.h>
struct fbCard{
    QString front;
    QString back;
    QString cardNum;
    QDateTime lastTry;
};

class DataManager
{
public:
    DataManager();
    QString addDeck(QString deck, QString desc, QString lang, QString user);
    QString genCode();
    bool deckExists(QString code);
    void printError(QSqlQuery query);
    void addCard(QString deckID, QString cardNum, QString front, QString back);
    QStringList getDeckList();
    void addCards(QString deckID, QStringList cards);
    QList<fbCard> getAllCards(QString deckID);
    QString getDeckName(QString deckID);
    bool registerUser(QString name, QString pass);
    bool userAlreadyExists(QString name);
    QString getUser(QString name);
    QStringList getFriends(QString name);
    bool addFriend(QString name, QString friendName);
    bool userExists(QString name);
    bool removeFriend(QString name, QString friendName);
private:
    QSqlDatabase db;
    QSqlQuery * testQuery;

    void updateCardCount(QString deckId);
    bool createDeckTable();
    bool createUserTable();
    bool createCardTable();
    bool tableCreation();
    void doBasicTestSetup();
    sem_t dbLock;
};

#endif // DATAMANAGER_H
