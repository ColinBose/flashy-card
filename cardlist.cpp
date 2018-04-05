#include "cardlist.h"

CardList::CardList()
{
    index = 0;
    len = 0;
    current = NULL;
    head = NULL;
    tail = NULL;

}
void CardList::pushFront(Card c){
    len++;
    node * n = new node();
    int s = sizeof(node);
    int p = sizeof(c);
   // n->c = c;
    n->card.front = c.front;
    n->index = index++;
    if(index == 1){
        head = n;
        tail = n;
        n->next = NULL;

    }
    else{
        n->next = head;
        head = n;
    }
}
void CardList::pushBack(Card c){
    len++;
    int s = sizeof(node);
    node * n = new node();
    n->card = c;
    n->index = index++;
    if(head == NULL){
        head = n;
        tail = n;
        n->next = NULL;

    }
    else{
        tail->next = n;
        tail = n;
    }
}
void CardList::display(){
    node * n = head;
    while(n->next != NULL){
        qDebug() << n->card.front;
        n = n->next;
    }
}
bool CardList::empty(){
    return head == NULL;
}
bool CardList::next(Card ** c){
    if(current == NULL){
        if(head == NULL)
            return false;
        *c = &head->card;
        curIndex = head->index;
        current = head;
        return true;
    }
    else{
        if(current->next == NULL)
            return false;
        *c = &current->next->card;
        curIndex = current->next->index;
        current = current->next;
        return true;
    }
}
void CardList::getHead(Card **c){
    *c = &head->card;
    curIndex = head->index;

}
void CardList::setIndex(QString code){
    node * c = head;
    while(c->next != NULL){
        if(c->card.code == code){
            curIndex = c->index;
            return;
        }
        c = c->next;
    }
    curIndex = c->index;
}

void CardList::clear(){
    node * next;
    node * cur;
    len = 0;
    tail = NULL;
    if(head == NULL)
        return;
    cur = head;
    while(cur->next != NULL){
        next = cur->next;
        free(cur);
        cur = next;
    }
    free(cur);
    head = NULL;
}
void CardList::reset(){
    current = NULL;
    index = -1;
}
void CardList::setCurrentNull(){
    current = NULL;
}

int CardList::size(){
    return len;
}
void CardList::removeCur(){
    node * cur;
    node * hold;
    cur = head;
    current = NULL;
    //at head
    if(cur->index == curIndex){
        if(cur->next != NULL){
            head = cur->next;
            qDebug() << "Removing: " + cur->card.front;
            free(cur);
            len--;
            return;
        }else{
            qDebug() << "Removing: " + cur->card.front;
            free(cur);
            head = NULL;
            tail = NULL;
            len = 0;
            return;
        }
    }

    //in middle

    while(cur->next != NULL){
        if(cur->next->index == curIndex){
            if(cur->next->next == NULL){
                qDebug() << "Removing: " + cur->card.front;
                //tail
                len--;
                tail = cur;
                free(cur->next);
                cur->next = NULL;
                return;
            }
            qDebug() << "Removing: " + cur->card.front;
            hold = cur->next;
            cur->next = cur->next->next;
            free(hold);
            len--;
            return;
        }
        cur = cur->next;



    }
}
bool CardList::checkFirstFew(){
    int checked = 0;
    node * n = head;
    int sentence, nonSentence;
    while(n->next != NULL){
        checked++;
        QString line = n->card.back;
        QStringList parts = line.split(' ');
        if(parts.length() > 1)
            sentence++;
         else
            nonSentence++;
        n = n->next;

    }
    double perc = (double)sentence / nonSentence;
    if(perc > 0.3)
        return true;
    return false;
}
QStringList CardList::checkTypes(){
    QStringList types;
    node * n = head;
    if(n == NULL)
        return types;
    types.push_back(n->card.type);
    while(n->next != NULL){
        QString type = n->card.type;
        if(!types.contains(type))
            types.push_back(type);
        n = n->next;
    }
    return types;
}
