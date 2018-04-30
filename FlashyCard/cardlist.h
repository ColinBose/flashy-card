#ifndef CARDLIST_H
#define CARDLIST_H
#include "card.h"
struct node{
    Card card;
    node * next;
    int index;
};

class CardList
{
public:
    CardList();
    void pushFront(Card c);
    void pushBack(Card c);
    void display();
    bool next(Card ** c);
    void getHead(Card **c);
    void clear();
    bool empty();
    void reset();
    int size();
    void removeCur();
    void setCurrentNull();
    void setIndex(QString code);
    bool checkFirstFew();
    bool findCard(Card **c, int cardNum);
    QStringList checkTypes();


private:
    int index;
    node * head;
    node * tail;
    node * current;
    int curIndex;
    int len;
};


#endif // CARDLIST_H
