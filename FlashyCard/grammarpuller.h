#ifndef GRAMMARPULLER_H
#define GRAMMARPULLER_H
#define LISTSIZE 10000
#include <QString>
#include <QFile>
#include <QTextStream>
const QString KOREANFILE = "krDict.txt";
const QString ENGLISHFILE = "engDict.txt";
const QString JAPANFILE = "jpnDict.txt";
class GrammarPuller
{
public:
    GrammarPuller();
    void loadLists(QStringList types, QString deckFile);
    void loadGen(QString file, bool useString);
    void loadKor(QString file, bool useString);
    void loadJpn(QString file, bool useString);
    bool checkExists(QString type, QString word);
    void cleanUp();

private:
    QStringList krList[LISTSIZE];
    QStringList enList[LISTSIZE];
    QStringList jpList[LISTSIZE];

    QString getFirstWord(QString line);
    int hashWord(QString word);
    void addWord(QStringList * list, int hash, QString word);
    bool checkEx(QStringList * list, QString word);

};

#endif // GRAMMARPULLER_H
