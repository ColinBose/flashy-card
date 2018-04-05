#include "koreancard.h"

KoreanCard::KoreanCard()
{

}

QString KoreanCard::pullGrammar(Card c){

}
QString KoreanCard::vocabEval(Card c, QString answer, int * ret, QString hint){
    int numPieces;
    int ans[3], guess[3];
    int check;
    bool correctChar;
    bool wrongChar;
    int right, wrong;
    QString returnString;
    right = 0;
    wrong = 0;
    hint = padHint(answer.length(), hint);
    for(int i = 0; i < answer.length(); i++){
        correctChar = false;
        wrongChar = false;
        numPieces = breakChar(c.back[i].unicode(),ans);
        if(hint[i] != '-'){
            returnString += c.back[i];
            continue;
        }
        if(numPieces == -1){
            returnString += c.back[i];
            continue;
        }
        check = breakChar(answer[i].unicode(), guess);
        if(check == -1){
            returnString += addRed(c.back[i]);
            wrong+= numPieces;
            continue;
        }
        if(numPieces < check){
            wrong += (numPieces - check);
            numPieces = check;
        }
        for(int j = 0; j < numPieces; j++){
            if(ans[j] == guess[j]){
                right++;
                correctChar = true;
            }
            else{
                wrong++;
                wrongChar = true;
            }
        }
        returnString += addColours(c.back[i], correctChar, wrongChar);


    }
    double perc = (double)right/(right+wrong);
    perc*= 100;
    *ret = (int)round(perc);
    return returnString;
}
QString KoreanCard::addColours(QChar c, bool correct, bool wrong){
    if(correct && !wrong)
        return addGreen(c);
    else if(correct && wrong)
        return addYellow(c);
    else return addRed(c);

}

int KoreanCard::breakChar(int c, int * a){
    if(c <44032 || c > 55171)
        return -1;

    int val = c - 44032;
    a[0] = val / 588;
    val %= 588;
    if(val == 0){
        a[1] = 0;
        return 2;
    }
    a[1] = val / 28;
    val %= 28;
    if(val == 0)
        return 2;
    a[2] = val;
    return 3;
}
QString addRed(const QChar c){
    QString ret = "";
    ret += "<font color='red'>";
    ret += c;
    ret += "</font>";
    return ret;
}
QString addGreen(const QChar c){
    QString ret = "";
    ret += "<font color='green'>";
    ret += c;
    ret += "</font>";
    return ret;
}
QString addYellow(const QChar c){
    QString ret = "";
    ret += "<font color='orange'>";
    ret += c;
    ret += "</font>";
    return ret;
}
QString addBlue(const QChar c){
    QString ret = "";
    ret += "<font color='blue'>";
    ret += c;
    ret += "</font>";
    return ret;
}
QString KoreanCard::padHint(int len, QString hint){
    if(hint.length() >= len)
        return hint;
    int added = len - hint.length();
    QString retString = hint;
    for(int i = 0; i < added; i++){
        retString += '-';
    }
    return retString;
}
