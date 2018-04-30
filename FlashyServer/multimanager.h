#ifndef MULTIMANAGER_H
#define MULTIMANAGER_H
#include "session.h"
#include "datamanager.h"
#include "network.h"
#include "messagebuffer.h"
#include "mainwindow.h"
#define BITSETSIZE 8000
struct matchReturn{
    int id;
    int matches;
    int members;
    int interval;
    int independant;
    int unique;
};
struct loggedUser{
    QList<loggedUser> onlineFriends;
    QStringList allFriends;
    int sock;
    QString name;
};

struct userData{
    QString name;
    QList<int> currentCards;
    QString randomData;
    int studyIndex = -1;
    bool loggedIn = false;
};
struct outEvent{
    int index;
    int round = -1;
    QTime sendTime;
};

class MainWindow;
class MultiManager
{
public:
    MultiManager();
    QStringList putData(char buff[], int len, int sock);
    void handleEvent();
    QString userList[MAXCLIENTS];
    void pushEvent(QString packet, int socket);
    void random();
    void removeClient(int sd, bool remove);
    void pollOutEvents();
    void sendWelcomeMessage(int sock);
    std::pair<int, int> statTick(int current, int totalMessage, int totalElapsed);
    void setMw(MainWindow * m);


private:
    MessageBuffer messages;
    DataManager data;
    sem_t inEvent;
    sem_t eventLock;
    QStringList testList;
    Session studySessions[MAXROOMS];
    userData userDat[MAXCLIENTS];
    QList<loggedUser> currentUsers;
    QList<outEvent> outQueue;
    sem_t queueLock;
    int totalSessions = 1;
    MainWindow * mw;
    int totalEvents = 0;
    int totalEventTime = 0;

    void addLoggedUser(QString name, int sock);
    void addEvent(outEvent o);
    void handleExportRequest(int sock, QStringList parts);
    void handleExportData(int sock, QStringList parts);
    void sendDeckList(int sock);
    void sendCardData(int sock, QStringList parts);
    void handleMultiRequest(int sock, QStringList parts);
    void createMultiSession(int sock, QStringList parts);
    int addSession(Session newSession);
    QList<matchReturn> getBestMatches(std::bitset<BITSETSIZE> cur, QString deckID);
    void handleStudyJoin(int sock, QStringList parts);
    std::bitset<BITSETSIZE> setBitset(char buff[], int len);
    void setUserCards(std::bitset<BITSETSIZE> bits, int sock);
    bool checkStillValid(QString deckID, int index, int unique);
    QString getNewName();
    void handleAnswer(int sock, QStringList parts);
    void sendNext(int index);
    void handleLeavingClient(int sock);
    void cleanSession(int index);
    void handleIndTick(int sock);
    void handleDoneUser(int sock);
    void handleChat(int sock, QStringList parts);
    void handlePm(int sock, QStringList parts);
    void handleGameGuess(int sock, QStringList parts);
    void handleLogin(int sock, QStringList parts);
    void handleRegister(int sock, QStringList parts);
    void handleRemoveFriend(int sock, QStringList parts);
    void handleAddFriend(int sock, QStringList parts);
    void sendGameList(int index);
    void sendFriendUpdate(int sock, loggedUser *l);
    void removeLoggedIn(int sock);
    void newFriendOnline(int sock);
};

#endif // MULTIMANAGER_H
