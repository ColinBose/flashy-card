#include "grammarpuller.h"
int vals[] = {3,5,7,11,13,17,19,23,29,31,33};
#define VALSMOD 10
GrammarPuller::GrammarPuller()
{

}
void GrammarPuller::loadLists(QStringList types, QString deckFile){
    bool standard = false;
    if(deckFile.length() == 0)
        standard = true;
    for(int i = 0 ; i < types.length(); i++){
        if(types[i] == "GEN")
            loadGen(deckFile, standard);
        else if(types[i] == "KOR")
            loadKor(deckFile, standard);
        else if(types[i] == "JPN")
            loadJpn(deckFile, standard);
    }
}
void GrammarPuller::loadGen(QString file, bool useString){
    QString fileString;
    if(!useString)
        fileString = file;
    else
        fileString = ENFILE;

    QFile inputFile(fileString);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          QString line = in.readLine();
          QString word = getFirstWord(line);
          QString makeWord = "";
          for(int i = 0; i < word.length(); i++){
              makeWord += word[i];
              int hash = hashWord(makeWord);
              addWord(enList, hash, makeWord);
          }
       }
       inputFile.close();
    }
}
void GrammarPuller::loadKor(QString file, bool useString){
    QString fileString;
    if(!useString)
        fileString = file;
    else
        fileString = KOREANFILE;

    QFile inputFile(fileString);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       while (!inputFile.atEnd())
       {
          QString line = inputFile.readLine();
          QString word = getFirstWord(line);
          QString makeWord = "";
          for(int i = 0; i < word.length(); i++){
              makeWord += word[i];
              int hash = hashWord(makeWord);
              addWord(krList, hash, makeWord);
          }
       }
       inputFile.close();
    }
}
void GrammarPuller::loadJpn(QString file, bool useString){
    QString fileString;
    if(!useString)
        fileString = file;
    else
        fileString = JAPANFILE;

    QFile inputFile(fileString);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          QString line = in.readLine();
          QString word = getFirstWord(line);
          QString makeWord = "";
          for(int i = 0; i < word.length(); i++){
              makeWord += word[i];
              int hash = hashWord(makeWord);
              addWord(jpList, hash, makeWord);
          }
       }
       inputFile.close();
    }
}
QString GrammarPuller::getFirstWord(QString line){
    QString ret = "";
    for(int i = 0; i < line.length();i++){
        if(line[i].isSpace())
            break;
        ret += line[i];
    }
    return ret;
}
int GrammarPuller::hashWord(QString word){
    long val = 0;
    int ret;
    for(int i = 0; i < word.length(); i++){
        val+= word[i].unicode() * vals[i%VALSMOD];
    }
    ret = val%LISTSIZE;
    return ret;
}
void GrammarPuller::addWord(QStringList *list, int hash, QString word){
    if(list[hash].contains(word))
        return;
    else
        list[hash].push_back(word);
}
bool GrammarPuller::checkExists(QString type, QString word){
    if(type == "KOR")
        return checkEx(krList, word);
    else if(type == "JPN")
        return checkEx(jpList, word);
    else if(type == "GEN")
        return checkEx(enList, word);
    else
        return false;
}
bool GrammarPuller::checkEx(QStringList *list, QString word){
    int hash = hashWord(word);
    return list[hash].contains(word);
}
void GrammarPuller::cleanUp(){
    for(int i = 0; i < LISTSIZE; i++){
        krList[i].clear();
        enList[i].clear();
        jpList[i].clear();
    }
}
