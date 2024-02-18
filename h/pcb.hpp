#ifndef _pcb_hpp_
#define _pcb_hpp_

#include "../lib/hw.h"
#include "../h/queue.hpp"
#include "../h/semaphore.hpp"

extern int depthCnt;    // Inicijalizovana u main.cpp na nulu. Pristupa joj se iz f-ja switchToSsp i switchToSp.
                        // Kada prvi put prelazimo na izvrsavanje neke niti, iz threadWrappera se radi sret, pa necemo umanjiti depthCnt u switchToSp
                        //   jer se ne vracamo na ostatak ecall. Pa je potrebno u wrapperu umanjivati depthCnt isto, kako bi buduci ulasci u prekidne
                        //   rutine imali dobru vrednost ove promenljive.
class Scheduler;



class PCB {
public:
    using Body = void (*) (void*);
    using Body2 = void (*) ();          // Za kreiranje niti nad userMain f-jom jer ona nema parametar.
    friend class Scheduler;
    friend class Queue;

    static PCB* running;
private:
    struct Context{
        uint64 ra;
        uint64 sp;
        uint64 ssp; // Pokazivac na sistemski stek ove niti.
    };

    PCB* next = nullptr, *prev = nullptr;

    Body body = nullptr;          // Body i arg se koriste ukoliko se nit ne pravi OOP varijantom, nego se se nit izvrsava nad zadatom f-jom sa zadatim argumentom.
    void* arg = nullptr;
    Body2 bodyNoArg = nullptr;    // Ukoliko je nit pravljena nad f-jom bez parametara, ovde pamtimo adresu te f-je.
    uint64* stack = nullptr;
    uint64* prvlStack = nullptr;

    bool finished = false;
    bool blockedStatus = false;
    Context context = {0, 0, 0 };

    bool hasStarted = false;    // Ispituje activate metod kako se ne bi duplicirala nit koja se vec izvrsava.

    kernel::Semaphore* blockedOnSem = nullptr;


    /// Konstruktori
    explicit PCB(Body givenBody, void* arg, uint64* userStack);
    explicit PCB(Body2 givenBody, uint64* userStack);

    /// Omotac
    static void threadWrapper();

public:
    /// Geteri i Seteri
    bool isFinished() const { return finished; }
    void setFinished(bool val) { finished = val; }
    bool isBlocked() const { return blockedStatus; }
    void setBlockedStatus(bool val) { blockedStatus = val; }
    uint64* getStack() const { return stack; }          // Za dealokaciju userMain i main niti u kodu kernela.
    uint64* getPrvlStack() const { return prvlStack; }  // Za dealokaciju userMain i main niti u kodu kernela.
    void saveSp(uint64 sp) { context.sp = sp; }
    uint64 getSp() { return context.sp; }
    void saveSsp(uint64 ssp) { context.ssp = ssp; }
    uint64 getSsp() { return context.ssp; }

    /// Pozivanje konstruktora
    static PCB* createThread(Body body, void* arg, uint64* userStack);
    static PCB* createThreadNoArg(Body2 body, uint64* userStack);

    /// Destruktor
    ~PCB();

    /// Za promenu konteksta:
    static void yield();    // Za sinhronu promenu konteksta.
    static void dispatch();
    static void contextSwitch(Context* oldC, Context* newC);

    /// Startovanje:
    int activate();

    /// Operatori:
    void* operator new(size_t size);
    void operator delete(void *p);

    /// Ostalo:
    void setBlockedOnSem(kernel::Semaphore* sem) { blockedOnSem = sem; }
    int kill();
};

#endif