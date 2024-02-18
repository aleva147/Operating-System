#include "../h/semaphore.hpp"
#include "../h/scheduler.hpp"

namespace kernel {
    /// Pomocni:
    void Semaphore::deblock() {
        if (!blocked->getBlockedHead()) return; // Prazan red
        PCB *t = blocked->get();
        t->setBlockedStatus(false);
        t->setBlockedOnSem(nullptr);
        Scheduler::getInstace()->put(t);
    }

    /// Wait i Signal:
    // U PCBu imam i polje 'boolean blockedStatus'. Kad se pozove dispatch metod klase PCB,
    //  running nit se stavlja u red spremnih ako nije finished i ako nije blockedStatus.
    // Kad ta blokirana nit u buducnosti opet dobije kontrolu, zavrsice wait metod (ne radi se vise nista u njemu),
    //  vratiti se iz prek rut i nastaviti gde je stala.
    int Semaphore::wait() {
        if (destroyed) return -1;   // Neuspeh jer se poziva metod wait za objekat semafora koji je unisten.

        val--;
        if (val < 0) {
            blocked->put(PCB::running);
            PCB::running->setBlockedStatus(true);       // Nit se nece staviti u red spremnih po promeni konteksta.
            PCB::running->setBlockedOnSem(this);
        }
        return 0;
    }

    int Semaphore::signal() {
        val++;
        if (!blocked->getBlockedHead()) return -1;
        if (val <= 0) {
            deblock();
        }

        return 0;
    }

    /// Destruktor:
    Semaphore::~Semaphore() {
        /// Oslobadjanje svih niti koje su blokirane na ovom semaforu:
        while (blocked->getBlockedHead()) {
            deblock();
        }

        /// Oslobodjene niti ce po nastavku ispitati destroyed promenljivu semafora kako bi znale da li da vrate negativnu vrednost.
        destroyed = true;

        /// Dealokacija alocirane memorije:
        delete blocked;         // Bice uspesan delete jer smo mi sami napravili Queue blocked i znamo da je ispravna adresa prosledjena alokatoru.
    }

    /// Operatori:
    void *Semaphore::operator new(size_t size) {
        size_t sizeInBlks = size / MEM_BLOCK_SIZE + (size % MEM_BLOCK_SIZE ? 1 : 0);
        return MemoryAllocator::getInstace()->allocate(sizeInBlks);
    }
    void Semaphore::operator delete(void *p) {
        int res = MemoryAllocator::getInstace()->deallocate(p);
        asm volatile ("mv a0, %0" : : "r" (res));           // Ovde moze nastati neuspeh sist poziva sem_close.. Ovde odmah u reg a0 upisujemo povratnu vrednost.
    }


    /// Ostalo:
    int Semaphore::remove(PCB *t) {
        int res = blocked->remove(t);
        if (res >= 0) val++;            // Trazena nit jeste bila na semaforu i sad je izbacena.
        return res;
    }
}