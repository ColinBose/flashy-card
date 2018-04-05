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
#define MAINPAGE 0
#define DECKMANAGER 1
#define ADDCARD 2
#define CREATEDECKPAGE 3
#define IMPORTPAGE 4
#define DECKSUMMARY 5
#define STUDY 6
#define COLLECTION 7
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
    ~MainWindow();
public slots:
    void addSecToClock();

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

    void on_importReturnMain_clicked();

    void on_createReturnMain_clicked();

    void on_studyAnswerEdit_textEdited(const QString &arg1);

    void on_studyMainMenu_clicked();



    void on_collectionActivate_clicked();

    void on_collectionDeactivate_clicked();

    void on_mainStatsButton_clicked();



private:
    Ui::MainWindow *ui;
    DataManager db;
    DeckManager deckM;
    StudySession study;
    Card * curCard;
    bool question;
    QList<miniCard> miniList;
    QTime studyTime;
    int attempted, correct, avgPercent, newCards;
    QString curDeck;

    void updateAverage(int curScore);
    void updateUnitList(int unit, bool act);
    void updateMiniList(int unit, bool act);
    void setTodayStat(QSqlQuery result);
    QString addHintToHint(QString hint, QString cardHint, QString answer);
    void displayMessage(QString message);
    void returnToMain();
    int studyImproved, studyDeclined;
    int lastRowSelected;
    void updateScoringDue(int ans);
    void loadDeckSummary();
    void setPage(int i);
    void populateATable(QTableWidget * table, QStringList headers, QStringList items, QSqlQuery result, bool stretch);
    void setDue();
    void popDeckTable();
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
    void updateStats();
};
void threadStarter();
void * clockThread(void * args);
#endif // MAINWINDOW_H
