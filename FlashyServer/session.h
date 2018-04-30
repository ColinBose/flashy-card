#ifndef SESSION_H
#define SESSION_H
#include <QString>
#include <bitset>
#include <QList>
#include <semaphore.h>
#include "datamanager.h"
#define MAXCARD 8000
#define MAXROOMS 20000
#define MAXUSERS 10
#define GAMEFREQUENCY 1
#define GAMEPIECES 8
struct user{
    QString name;
    int score = 0;
    int sock;
    int lastScore = -1;
    int currentRoundScore;
    bool finished = false;
};

class Session
{
public:
    Session();
    QList<user> userList;
    QList<fbCard> cardList;
    std::bitset<MAXCARD> cardBits;
    QString deckID;
    QList<fbCard> reviewList;
    QList<fbCard> gameCardList;
    int gameChecks[GAMEPIECES * 2];
    int id;
    int interval = 10;
    int games;
    int uniqueId;
    int maxCard = 0;
    bool isOpen();
    int independant;
    int finishedUsers = 0;
    QString gameString;
    int getRound();
    void setMaxCard();
    void joinSession(QList<int> newUserCards);
    bool full();
    void addUser(QString name, int sock);
    bool updateRound(int user, int answer, int remove, int cardN);
    bool checkRound(int round);
    QString getReturnResponse(int * next);
    void updateScoring();
    void userLeaving(int sock);
    bool isEmpty();
    void cleanUp();
    bool userLeftCheck();
    bool doGuess(int first, int second);
    bool gameTime();
    void setGameString();
    bool gameOver();
    void userLock();
    void userUnlock();
    int currentRound = 0;
    bool overFull();
    int numUsers();
private:
    int usersRemoving = 0;
    int roundResponses = 0;
    sem_t someLock;
    int curIndex = 0;
    int thisRoundUsers = 0;
    int gameMatches;
    void bumpTime();
    bool checkForReview(QString back);
    QString getNextCard();

};

#endif // SESSION_H
