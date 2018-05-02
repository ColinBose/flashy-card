#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "datamanager.h"
#include <QTableWidget>
#include "deckmanager.h"
#include <QFileDialog>
#include <QMessageBox>
#include "studysession.h"
#include "graph.h"
#include "studyclock.h"
#include "statinfo.h"
#include "network.h"
#include "multimanager.h"
#include "friends.h"
#include "loginmenu.h"
#define MAINPAGE 0
#define DECKMANAGER 1
#define ADDCARD 2
#define CREATEDECKPAGE 3
#define IMPORTPAGE 4
#define DECKSUMMARY 5
#define STUDY 6
#define COLLECTION 7
#define EXPORT 8
#define IMPORT 9
#define MULTIPAGE 10
#define GAME 11

class LoginMenu;
class Friends;
struct importDeckData{
    QString deckID;
    QString creator;
    QString numCards;
    QString desc;
    QString language;

};
struct minUser{
    QString name;
    int startScore;
    int score;
};

struct roomData;
namespace Ui {

class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT


    struct miniCard{
        QString front;
        bool active = false;
        int unit;
    };
public:
    explicit MainWindow(QWidget *parent = 0);
    void singleStudyLoadQuestion();
    void passToRegister(int code);
    void passFriendAdd(int code);
    void passFriendUpdate(QString online, QString offline);
    void passFriendRemove(int code);
    void passPm(QString friendName, QString message);
    void passSentPm(QString friendName, QString message);
    void passUserName();
    ~MainWindow();

    QString loggedInName = "";
public slots:
    void updateImportList(QStringList list);
    void addSecToClock();
    void passToLogin(int code);
    void removeSecFromClock();
    void updateRoomList();
    void joinMultiStudy();
    void roomFull();
    void roomGone();
    void disconnectFromMulti();
    void newMultiCreated();
    void multiLoadQuestion(int cardNum);
    void serverDied();
    void updateScores();
    void addChat(QString message);
    void updateGameBoard(int index, int first, int second);
    void loadGame();
    void returnToStudy();
    void newPm();

private slots:


    void on_mainDeckManager_2_clicked();

    void on_createDeckButton_clicked();

    void on_addCardButton_clicked();

    void on_mainDeckManager_clicked();

    void on_deckManagerAddCard_clicked();

    void on_inputFileSelect_clicked();

    void on_deckManagerImport_clicked();

    void on_inputCheckInput_clicked();

    void on_importSelector_currentIndexChanged(int index);

    void on_importImportButotn_clicked();

    void on_mainStudy_clicked();

    void on_deckSummaryStudy_clicked();

    void on_studyAnswerEdit_returnPressed();

    void on_studyAnswerButton_clicked();

    void on_studyHintButton_clicked();

    void on_studyScoreSlider_sliderMoved(int position);

    void on_deckManagerCollection_clicked();

    void on_collectionUnitList_clicked(const QModelIndex &index);



    void on_studyAnswerEdit_textEdited(const QString &arg1);

   // void on_studyMainMenu_clicked();



    void on_collectionActivate_clicked();

    void on_collectionDeactivate_clicked();

    void on_mainStatsButton_clicked();



    void on_mainConnect_clicked();

    void on_deckManagerExport_clicked();

    void on_exportExportButton_clicked();

    void on_deckManagerImportFromServer_clicked();
    void on_importGetDeck_clicked();
    void on_multiStudyButton_clicked();
    void on_mainMulti_clicked();
    void on_multiCreateRoom_clicked();


    void on_studySendChat_returnPressed();

    void on_game0_clicked();

    void on_game1_clicked();

    void on_game2_clicked();

    void on_game3_clicked();

    void on_game4_clicked();

    void on_game5_clicked();

    void on_game6_clicked();

    void on_game8_clicked();

    void on_game7_clicked();

    void on_game9_clicked();

    void on_game10_clicked();

    void on_game11_clicked();

    void on_game12_clicked();

    void on_game13_clicked();

    void on_game14_clicked();

    void on_game15_clicked();

    void on_mainLogin_clicked();

    void on_friendButton_clicked();

    void on_mainMainReturn_clicked();

private:
    Ui::MainWindow *ui;
    DataManager db;
    DeckManager deckM;
    StudySession study;
    Card * curCard;
    bool question;
    QList<miniCard> miniList;
    QTime studyTime;
    QTime countDown;
    int attempted, correct, avgPercent, newCards;
    QString curDeck;
    QList<importDeckData> importData;
    LoginMenu * menu = 0;
    Friends * friendMenu = 0;


    QString serverIp, serverPort;
    int gameFirstGuess, gameSecondGuess;
    bool guessMade = false;
    bool multiStudy = false;
    bool connected;
    bool answerLock = false;
    void testFunction(char buff[]);
    void doBasicStudySetup();
    void updateAverage(int curScore);
    void updateUnitList(int unit, bool act);
    void updateMiniList(int unit, bool act);
    void setTodayStat(QSqlQuery result);
    QString addHintToHint(QString hint, QString cardHint, QString answer);
    void displayMessage(QString message);
    void returnToMain();
    int studyImproved, studyDeclined;
    int lastRowSelected;
    bool updateScoringDue(int ans);
    void loadDeckSummary();
    void setCountdown(int sec);
    void setPage(int i);
    bool isCurrentlyChecked(int place);
    void gameClickLogic(int guess, bool clicked);
    void populateATable(QTableWidget * table, QStringList headers, QStringList items, QSqlQuery result, bool stretch);
    void setDue();
    void popDeckTable();
    void loadGameButtons();
    void displayGameAnswer(int first, int second, int index);
    void loadQuestion(Card * c);
    void loadCollection(QString deck);
    void hideStudyAnswer();
    void showStudyAnswer();
    void dealWithAnswer(QString answer, int ansVal, QString answerString);
    void colourAnswer(QString answer);
    void incrementTime(int ans);
    QStringList setActiveInactive(QStringList units, QList<miniCard> cards);
    void populateUnitList(QStringList units, QStringList actives);
    QString greenString(QString cur);
    QString yellowString(QString cur);
    QString redString(QString cur);
    void populateCardList(int index);
    void yellowBack(QTableWidgetItem * widge);
    void greenBack(QTableWidgetItem * widge);
    void redBack(QTableWidgetItem * widge);
    void randomBack(QTableWidgetItem * widge, int index);
    void setGameClicked(QPushButton *button);
    void updateStats();
    void setupScoreTable();
    void clearAllStudy();
    void loadConfig();

};
void threadStarter();
void * clockThread(void * args);
void * cdThread(void * args);
void * networkWorker(void * args);
#endif // MAINWINDOW_H
