#ifndef OUTGOINGQUEUE_H
#define OUTGOINGQUEUE_H

struct outEvent{
    int index = 0;
    int currentRound = -1;
};

class OutgoingQueue
{
public:
    void addEvent(int index, int round);
    void pollForEvents();
    OutgoingQueue();
};

#endif // OUTGOINGQUEUE_H
