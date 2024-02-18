#include "../h/scheduler.hpp"



/// Put i Get:
PCB* Scheduler::get() {
    Scheduler* s = getInstace();
    PCB *t = s->readyHead;
    if (s->readyHead->next) {
        s->readyHead->next->prev = nullptr;
        s->readyHead = s->readyHead->next;
    }
    else {
        s->readyHead = nullptr;
        s->readyTail = nullptr;
    }
    return t;
}

void Scheduler::put(PCB* t) {
    Scheduler* s = getInstace();
    // Ako je lista spremnih prazna, ovo ce postati jedina u listi:
    if (!s->readyTail) {
        t->next = t->prev = nullptr;
        s->readyHead = s->readyTail = t;
    }
    // Ako nije prazna, stavljamo na kraj liste
    else {
        t->next = nullptr;
        t->prev = s->readyTail;
        t->prev->next = t;
        s->readyTail = t;
    }
}

/// Operatori:
void *Scheduler::operator new(size_t size)  {
    size_t sizeInBlks = size / MEM_BLOCK_SIZE + (size % MEM_BLOCK_SIZE ? 1 : 0);
    return MemoryAllocator::getInstace()->allocate(sizeInBlks);
}
void Scheduler::operator delete(void *p) {
    MemoryAllocator::getInstace()->deallocate(p);
}


/// Ostalo:
int Scheduler::remove(PCB* t) {
    PCB* cur = readyHead;
    for (; cur; cur = cur->next) {
        if (cur == t) {             // Nadjena je nit koju treba izbaciti iz reda.
            if (cur->prev) cur->prev->next = cur->next;
            else readyHead = cur->next;               // Izbacuje se prva nit (treba azurirati readyHead)

            if (cur->next) cur->next->prev = cur->prev;
            else readyTail = cur->prev;               // Izbacuje se poslednja nit (treba azurirati readyTail)

            return 0;
        }
    }
    return -1;
}