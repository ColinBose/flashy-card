#ifndef STUDYSESSION_H
#define STUDYSESSION_H
#include "globalincludes.h"
#include "card.h"
#include "cardlist.h"
#include "koreancard.h"
#include "grammarpuller.h"
class StudySession
{
public:
    StudySession();
    void setDeck(QString deck);
    QString getDeckName();
    CardList session;
    bool setSession(DataManager db);
    bool getNext(Card ** c);
    QString loadHint(Card c, bool sentence);
    QString generalHint(Card c);
    int remain();
    QString evalAnswer(Card c, QString answer, bool sentence, int * ret, QString hint);
    QString generalSentenceEval(Card c, QString answer, int *ret, QString hint);
    void remove();
    int getInterval(Card * c, int ans);
    bool isSentences();
    int getAverage(Card * c, int ans);
    void loadGrammar(DataManager db);
    QString padHint(QString answer, QString hint);
    QString flipString(QString curHint, QString answer);
    void clean();
    void setMaxInterval(int interval);
    int maxInterval;
    bool checkSentence(QString back);
private:
    QString deckName;
    GrammarPuller gram;

    int computeAverage(int past, int numPast, int cur, int maxPast);
};

#endif // STUDYSESSION_H
