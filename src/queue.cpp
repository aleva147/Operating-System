#include "../h/queue.hpp"
#include "../h/pcb.hpp"



/// Put i get:
PCB* Queue::get() {     // Ako se poziva ovaj metod, trebalo bi sigurno da bude bar jedna nit u listi blokiranih.
    PCB* t = blockedHead;
    if (blockedHead->next) {
        blockedHead->next->prev = nullptr;
        blockedHead = blockedHead->next;
    }
    else {
        blockedHead = blockedTail = nullptr;
    }
    return t;
}

void Queue::put(PCB *t) {
    if (!blockedTail) {
        t->next = t->prev = nullptr;
        blockedHead = blockedTail = t;
    }
    else {
        t->prev = blockedTail;
        t->prev->next = t;      // blockedTail->next = t;
        t->next = nullptr;
        blockedTail = t;
    }
}

/// Operatori:
void *Queue::operator new(size_t size)  {
    size_t sizeInBlks = size / MEM_BLOCK_SIZE + (size % MEM_BLOCK_SIZE ? 1 : 0);
    return MemoryAllocator::getInstace()->allocate(sizeInBlks);
}
void Queue::operator delete(void *p) {
    MemoryAllocator::getInstace()->deallocate(p);
}

/// Ostalo:
int Queue::remove(PCB* t) {
    PCB* cur = blockedHead;
    for (; cur; cur = cur->next) {
        if (cur == t) {             // Nadjena je nit koju treba izbaciti iz reda.
            if (cur->prev) cur->prev->next = cur->next;
            else blockedHead = cur->next;               // Izbacuje se prva nit (treba azurirati blockedHead)

            if (cur->next) cur->next->prev = cur->prev;
            else blockedTail = cur->prev;               // Izbacuje se poslednja nit (treba azurirati blockedTail)

            return 0;
        }
    }
    return -1;
}
