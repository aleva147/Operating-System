#ifndef _scheduler_h_
#define _scheduler_h_

#include "../h/pcb.hpp"
#include "../h/memAllocator.hpp"



class Scheduler {
private:
    // Dvostruko ulancana lista spremnih niti, da bismo pri stavljanju niti u red spremnih mogli da azuriramo onu koja joj prethodi.
    PCB* readyHead = nullptr;
    PCB* readyTail = nullptr;

    // Privatan konstruktor, samo ga getInstance() poziva jedanput.
    Scheduler() {}
    // Onemogucavanje kopiranja objekta ove klase (kopirajuceg konstr, premestajuceg konstr, operatora dodele).
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

public:
    /// Singleton:
    static Scheduler* getInstace() {
        static Scheduler* sched = nullptr;  // Samo prvi put ce se izvrsiti jer je definicija staticke promenljive u pitanju.

        if (!sched) {
            sched = new Scheduler();
            sched->readyHead = nullptr;
            sched->readyTail = nullptr;
        }

        return sched;
    }

    /// Put i Get:
    PCB* get();
    void put(PCB* t);

    /// Operatori:
    void* operator new(size_t size);
    void operator delete(void* p);

    /// Ostalo:
    int remove(PCB* t);
};


#endif