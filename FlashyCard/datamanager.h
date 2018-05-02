#ifndef DATAMANAGER_H
#define DATAMANAGER_H
#include <QtSql>
#include <QtDebug>
#define CODELENGTH 6
#define STANDARDDAILY 20
#define STANDARDINTERVAL 30
struct statData{
    int completed;
    int attempted;
    int newCards;
    int pastAvg;
    QTime studyTime;
    int strong, learning, longlearning, longstruggle, newStrong, newWeak;
};
struct shortCard{
    QString front;
    QString back;
    QString cardNum;
    QString sound;
    int unit;
};

class DataManager
{
public:
    DataManager();
    QSqlQuery getDecks();
    void updateDeckID(QString id, QString deckName);
    bool createDeck(QString name, int maxInterval, int dailyUpdates);
    bool addCard(QString deck, QString front, QString back);
    bool fastAddCard(QString deck, QString front, QString back, QString sound, int unit, int index);
    int fastIndex(QString deck);
    int getCurrentDue(QString deck);
    void importDeckFromFile(QString deckName, QList<shortCard> cardList);
    int getDailyDue(QString deck);
    void setDailyDue(QString deck, int numDue);
    void checkDailyReset();
    QString addImportDeck(QString deckID, QString deckName);
    QSqlQuery getTodaysStats();
    void setInactive(QString deck, int unit);
    void setActive(QString deck, int unit);
    QString getDictFile(QString deck);
    int getDueTotal(QString deck);
    int getCardTotal(QString deck);
    int getSeenTotal(QString deck);
    QSqlQuery getSession(QString deck);
    bool updateDue(int newAverage, int newInterval, int newAttempts, int newStage, QString id);
    void updateMaxInterval(QString deck);
    bool updateHint(QString hint, QString code);
    QSqlQuery getAllCards(QString deck);
    QStringList getUnits(QString deck);
    int getInterval(QString deck);
    void addDaysToStats();
    QSqlQuery getAllActive();
    QSqlQuery getCardsForExport(QString deck);
    void updateCardLevels(int strong, int learning, int longLearning, int longStruggle, int newStrong, int newWeak);
    void updateStats(int attempted, int correct, int avgPercent, int newCards, QTime studyTime);
    QList<statData> getHistory(QDate start );
    int addALLCard(QList<shortCard> cardList, QString deckName);
    QString getDeckId(QString deckName);
    bool firstSetup();
private:
    bool tableCreation();
    QSqlDatabase db;
    bool createDeckTable();
    bool createCardTable();
    bool deckExists(QString deckName);
    bool createStatTable();
    void checkTodayStat(QString date);
    void reFillStats(QStringList days, QDate first);
    QString getCode(QString table, QString row);
    QString genCode();
    QString getDate();
    void printError(QSqlQuery query);
    void getLastIndex(QString deck, int * index, int * unit);
    void updateNumLeft(QString deck, int newNum);
    int getType(QString back);
    int MAXINTERVAL;
    bool firstRun = false;
};

#endif // DATAMANAGER_H
