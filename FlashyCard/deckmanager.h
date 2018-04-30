#ifndef DECKMANAGER_H
#define DECKMANAGER_H
#include <QString>
#include "globalincludes.h"
#include <QFileInfo>
class DeckManager
{
public:
    DeckManager();
    void loadDeck(QString deck);
    QString getDeck();
    bool addCard(DataManager db, QString front, QString back);
    void setFileName(QString fName);
    QStringList checkLine(QChar seperator);

    void doImport(DataManager db, int front, int back, int unit, int sound);
private:
    QString deckName;
    QString importFileName;
    QStringList allLines;
    QStringList nonMatching;
    int lineLength;
    QChar sep;
    void writeBadMatching(QStringList list, QChar sep);
};

#endif // DECKMANAGER_H
