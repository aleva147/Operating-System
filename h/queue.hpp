#ifndef _queue_hpp_
#define _queue_hpp_

#include "../h/memAllocator.hpp"

class PCB;



class Queue {
private:
    PCB* blockedHead = nullptr;
    PCB* blockedTail = nullptr;

public:
    Queue() {}

    /// Put i get:
    PCB* get();
    void put(PCB* t);

    /// Getter:
    PCB* getBlockedHead() { return blockedHead; }

    /// Operatori:
    void* operator new(size_t size);
    void operator delete(void* p);

    /// Ostalo:
    int remove(PCB* t);
};

#endif