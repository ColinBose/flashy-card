#include "deckmanager.h"
DeckManager::DeckManager()
{

}
void DeckManager::loadDeck(QString deck){
    deckName = deck;
}
QString DeckManager::getDeck(){
    return deckName;
}
bool DeckManager::addCard(DataManager db, QString front, QString back){
    db.addCard(deckName, front, back);
}
void DeckManager::setFileName(QString fName){
    importFileName = fName;
}
QStringList DeckManager::checkLine(QChar seperator){
    QFile f(importFileName);
    sep = seperator;
    if(!f.open(QIODevice::ReadOnly)){
        qDebug() << "Error opening file";
        QStringList empty;
        return empty;
    }
    QString line = f.readLine();
    if(line[line.length()-1] == '\n');
        line = line.left(line.length()-1);
    allLines.push_back(line);
    QStringList parts = line.split(seperator);
    lineLength = parts.length();
    while(!f.atEnd()){
        line = f.readLine();
        if(line[line.length()-1] == '\n');
            line = line.left(line.length()-1);
        allLines.push_back(line);
    }
    f.close();
    return parts;


}
void DeckManager::loadOnStartUp(QString file, QChar seperator, int lineLen){
    QFile f(file);
    if(!f.open(QIODevice::ReadOnly)){
        qDebug() << "Error opening file";
        return;
    }
    lineLength = lineLen;
    QString line;
    sep = seperator;
    while(!f.atEnd()){
        line = f.readLine();
        if(line[line.length()-1] == '\n');
            line = line.left(line.length()-1);
        allLines.push_back(line);
    }
    f.close();
}

void DeckManager::doImport(DataManager db, int front, int back, int unit, int sound){
    int totalDone = 0;
    QList<shortCard> cardList;
    int startIndex = db.fastIndex(deckName);
    for(int i = 0; i < allLines.length(); i++){

        QString singleLine = allLines.at(i);
        QStringList indvParts = singleLine.split(sep);

        if(indvParts.length() != lineLength){
            nonMatching.push_back(singleLine);
            continue;
        }

        int incUnit;
        if(unit == 0){
            incUnit = totalDone / 50;
        }
        else{
            incUnit = indvParts.at(unit-1).toInt();
        }
        QString soundString = "";
        if(sound != 0)
            soundString = indvParts.at(sound-1);

        QString frontString = indvParts.at(front-1);
        QString backString = indvParts.at(back-1);
        shortCard sh;
        sh.back = backString;
        sh.front = frontString;
        sh.sound = soundString;
        sh.unit = incUnit;
        cardList.push_back(sh);
        totalDone++;
        /*if(db.fastAddCard(deckName, frontString, backString, soundString, incUnit,startIndex)){
            startIndex++;
            qDebug() << "Card added";
        }
        else{
            qDebug() << "Error adding card";
        }
        */
    }
    db.importDeckFromFile(deckName, cardList);
    if(nonMatching.length() > 0){
      writeBadMatching(nonMatching, sep);
    }
    allLines.clear();
}
void DeckManager::writeBadMatching(QStringList list, QChar sep){

    QString outFile = "badMatches.txt";
    for(int i = 0; i < 50; i++){
        QFileInfo check_file(outFile);
        if(check_file.exists())
            break;
        outFile = "badMatches" + QString::number(i) + ".txt";
    }
    QFile file(outFile);
    if (!file.open(QIODevice::ReadWrite)) {
       qDebug() << "Error opening file;";
       return;
    }
    for(int i = 0; i < list.length(); i++){
        QStringList parts = list[i].split(sep);
        QString output = "";
        for(int j = 0; j < parts.length(); j++){
            output += parts[j];
                if(j != (parts.length()-1))
                output += sep;

             //else
               // output += '\n';

        }
        file.write(output.toUtf8());
    }
}
