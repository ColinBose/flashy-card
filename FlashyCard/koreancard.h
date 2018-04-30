#ifndef KOREANCARD_H
#define KOREANCARD_H
#include "card.h"

class KoreanCard
{
public:
    KoreanCard();
    QString pullGrammar(Card c);
    QString vocabEval(Card c, QString answer, int *ret, QString hint);
    int breakChar(int c, int * a);
    QString addColours(QChar c, bool correct, bool wrong);
private:
    QString padHint(int len, QString hint);

};
QString addRed(const QChar c);
QString addGreen(const QChar c);
QString addYellow(const QChar c);
QString addBlue(const QChar c);
#endif // KOREANCARD_H
