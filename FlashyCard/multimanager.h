#ifndef MULTIMANAGER_H
#define MULTIMANAGER_H

#include <QString>
#include "datamanager.h"
#include "network.h"
#include "mainwindow.h"

struct roomData{
    QString id;
    QString members;
    QString interval;
    QString matches;
    int independant;
    QString uniqueId;
};
struct users{
    QString name;
    int score;
    int startScore;
    int lastScore;
};

#define MAXLENGTH 1024
class MainWindow;
struct aCard{
    QString front;
    QString back;
    QString cardNum;
};


class MultiManager
{
public:
    QList<roomData> availableRooms;
    QList<users> userList;
    bool connected;
    void setConnected(int socket);
    void setData(DataManager * d, MainWindow * m);
    void setDisconnected();
    MultiManager();
    bool doExport(QString deckName, QString desc, QString language);
    void requestDeckList();
    void readLoop();
    void sendLogin(QString userName, QString pass);
    void sendRegisterRequest(QString userName, QString pass);
    void sendImportRequest(QString deckID, QString deckName);
    void sendMultiRequest(QString deckName);
    bool createRoom(QString deckName, int interval, int games, int indep);
    void sendMultiJoinRequest(QString deckName, int index);
    void roomFull();
    void roomCreated(QString packet);
    QList<int> getCardList();
    bool isSolo();
    void closeConnect();
    bool isIndependant();
    void sendAnswer(int cardNum, int answer, char remove);
    void sendExit();
    void sendIndTick();
    void sendUserDone();
    bool isMe(QString n);
    void sendFakeResponse();
    void sendChat(QString message);
    void sendGameGuess(int first, int second);
    void sendFriendAdd(QString name);
    void sendFriendRemove(QString name);
    void sendPm(QString message, QString friendName);


    int interval;
    QStringList gameList;

private:
    MainWindow * mw;
    int sock;
    QString curDeck ="";
    QString curId = "";
    DataManager *db;
    QString curName;
    QString currentRound = "0";
    bool sessionOver = false;
    QList<int> currentCards;
    bool independant;
    bool playingGame = false;
    void sendExportRequest(QString deck, QString desc, QString lang);
    QString packForSend(QString data, int type);
    void exportAllCards(QString deck, QString deckID, QList<aCard> cards);
    bool handleIncomingEvent(QString packet);
    void handleExportResponse(QString packet);
    void handleImportData(QString packet);
    void handleDeckListResponse(QString packet);
    void handleRoomList(QString packet);
    void newUserJoined(QString packet);
    void joinStudyRoom(QString packet);
    void handleNextCard(QString packet);
    void endSession(QString packet);
    QByteArray cardsToBits(QList<aCard> cardList);
    QList<aCard> bitsToCards(QByteArray data);
    void handleTick(QString packet);
    void handleUserLeave(QString packet);
    void multiNoLongerExists(QString packet);
    void copyAndSendScores();
    void handleIncomingChat(QString packet);
    void handleGameUpdate(QString packet);
    void handleGameStart(QString packet);
    void handleLogin(QString packet);
    void handleRegister(QString packet);
    void handleFriendResponse(QString packet);
    void handleFriendUpdate(QString packet);
    void handleFriendRemove(QString packet);
    void handlePm(QString packet);
    void handleSentPm(QString packet);
    void handleWelcome(QString packet);
};

#endif // MULTIMANAGER_H
