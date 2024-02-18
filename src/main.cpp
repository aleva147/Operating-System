#include "../lib/hw.h"
#include "../lib/console.h"
#include "../h/memAllocator.hpp"
#include "../h/pcb.hpp"
#include "../h/scheduler.hpp"
#include "../h/userMain.hpp"
#include "../h/semaphore.hpp"

const uint64 CONST_SIST_C_U = 0x08;
const uint64 CONST_SIST_C_S = 0x09;

const uint64 CONST_TIMER = 0x8000000000000001;
const uint64 CONST_CONSOLE = 0x8000000000000009;

const uint64 CONST_MEM_ALLOC = 0x01;         // Nije mogla da se uveze ovakva postojeca konstanta iz syscall_c,
const uint64 CONST_MEM_FREE = 0x02;          // buni se za visestruke definicije, a problem je sto je const i mora se definisati
const uint64 CONST_THREAD_INITIALIZE = 0x16; // pa se ne moze samo deklarisati kao extern u h fajlu, moralo bi i da se definise.
const uint64 CONST_THREAD_START = 0x17;
const uint64 CONST_THREAD_KILL = 0x18;
const uint64 CONST_THREAD_CREATE = 0x11;
const uint64 CONST_THREAD_EXIT = 0x12;
const uint64 CONST_DISPATCH = 0x13;
const uint64 CONST_THREAD_DEALLOCATE = 0x15;
const uint64 CONST_SEM_OPEN = 0x21;
const uint64 CONST_SEM_CLOSE = 0x22;
const uint64 CONST_SEM_WAIT = 0x23;
const uint64 CONST_SEM_SIGNAL = 0x24;
const uint64 CONST_PUTC = 0x3;
const uint64 CONST_GETC = 0x4;

int depthCnt = 0;       // Extern promenljiva iz pcb.hpp.



extern "C" void supervisorTrap();

extern "C" void handleSupervisorTrap() {
    uint64 callId;
    uint64 param0, param1, param2, param3, param4;
    uint64 res;
    volatile uint64 varSepc;
    volatile uint64 varSstatus;
    uint64 varScause;

    asm volatile ("ld %0, 10*8(fp)" : "=r" (callId));       //asm volatile ("mv %0, a0" : "=r" (callId));
    asm volatile ("csrr %0, scause" : "=r" (varScause));
    asm volatile ("csrr %0, sepc" : "=r" (varSepc));
    asm volatile ("csrr %0, sstatus" : "=r" (varSstatus));


    /// Prekid izazvan sistemskim pozivom iz korisnickog rezima:
    if (varScause == CONST_SIST_C_U) {
        switch (callId) {
            case CONST_MEM_ALLOC:
                asm volatile ("mv %0, a1" : "=r" (param0));
                res = (uint64)MemoryAllocator::getInstace()->allocate(param0);
                asm volatile ("mv a0, %0" : : "r" (res));     // Povratna vrednost obrade prekida u a0.
                asm volatile ("sd a0, 10*8(fp)");   // Upis preko sacuvane vrednosti a0 na steku, kako bi se po povratku iz prek rut
                                                    // ova povratna vrednost stavila u a0 umesto one sa kojom smo usli u prekidnu rutinu.
                break;
            case CONST_MEM_FREE:
                asm volatile ("mv %0, a1" : "=r" (param0));

                res = MemoryAllocator::getInstace()->deallocate((void*)param0);
                asm volatile ("mv a0, %0" : : "r" (res));
                asm volatile ("sd a0, 10*8(fp)");
                break;


            case CONST_THREAD_INITIALIZE:
                asm volatile ("ld %0, 11*8(fp)" : "=r" (param1));
                asm volatile ("ld %0, 12*8(fp)" : "=r" (param2));
                asm volatile ("ld %0, 13*8(fp)" : "=r" (param3));
                asm volatile ("ld %0, 14*8(fp)" : "=r" (param4));

                (*(PCB**)param1) = PCB::createThread((void (*) (void*))param2, (void*)param3, (uint64*)param4);
                if (*(PCB**)param1 == nullptr) res = -1;
                else res = 0;

                asm volatile ("mv a0, %0" : : "r" (res));
                asm volatile ("sd a0, 10*8(fp)");

                break;
            case CONST_THREAD_START:
                asm volatile ("ld %0, 11*8(fp)" : "=r" (param1));
                res = ((PCB*)param1)->activate();

                asm volatile ("mv a0, %0" : : "r" (res));
                asm volatile ("sd a0, 10*8(fp)");
                break;
            case CONST_THREAD_CREATE:
                asm volatile ("ld %0, 11*8(fp)" : "=r" (param1));
                asm volatile ("ld %0, 12*8(fp)" : "=r" (param2));
                asm volatile ("ld %0, 13*8(fp)" : "=r" (param3));
                asm volatile ("ld %0, 14*8(fp)" : "=r" (param4));

                (*(PCB**)param1) = PCB::createThread((void (*) (void*))param2, (void*)param3, (uint64*)param4);
                if (*(PCB**)param1 == nullptr) res = -1;
                else {
                    res = (*(PCB**)param1)->activate();
                }

                asm volatile ("mv a0, %0" : : "r" (res));
                asm volatile ("sd a0, 10*8(fp)");
                break;
            case CONST_DISPATCH:
                break;
            case CONST_THREAD_EXIT:
                PCB::running->setFinished(true);    // Tekuca nit se nece staviti u red spremnih po promeni konteksta.
                break;
            case CONST_THREAD_DEALLOCATE:
                asm volatile ("ld %0, 11*8(fp)" : "=r" (param1));   // param1 = handle
                delete ((PCB*)param1);
                break;
            case CONST_THREAD_KILL:
                asm volatile ("ld %0, 11*8(fp)" : "=r" (param1));   // param1 = handle
                res = ((PCB*)param1)->kill();
                asm volatile ("mv a0, %0" : : "r" (res));
                asm volatile ("sd a0, 10*8(fp)");
                break;


            case CONST_SEM_OPEN:
                asm volatile ("ld %0, 11*8(fp)" : "=r" (param1));
                asm volatile ("ld %0, 12*8(fp)" : "=r" (param2));

                (*(kernel::Semaphore**)param1) = kernel::Semaphore::createSemaphore((unsigned)param2);
                if (*(kernel::Semaphore**)param1 == nullptr) res = -1;
                else res = 0;

                asm volatile ("mv a0, %0" : : "r" (res));
                asm volatile ("sd a0, 10*8(fp)");

                break;
            case CONST_SEM_WAIT:
                asm volatile ("ld %0, 11*8(fp)" : "=r" (param1));

                ((kernel::Semaphore *) param1)->wait(); // Ukoliko treba, samo stavlja nit u red blokiranih na semaforu,
                                                        //  pa se nece staviti u red spremnih po promeni konteksta.
                asm volatile ("csrr %0, sepc" : "=r" (varSepc));
                asm volatile ("csrr %0, sstatus" : "=r" (varSstatus));
                PCB::dispatch();
                asm volatile ("csrw sstatus, %0" : : "r" (varSstatus));
                asm volatile ("csrw sepc, %0" : : "r" (varSepc));

                res = ((kernel::Semaphore *) param1)->getDestroyedStatus();
                asm volatile ("mv a0, %0" : : "r" (res));
                asm volatile ("sd a0, 10*8(fp)");

                return;     // Nije break kako se ne bi ponovo uradila promena konteksta.
            case CONST_SEM_SIGNAL:
                asm volatile ("ld %0, 11*8(fp)" : "=r" (param1));

                res = ((kernel::Semaphore*)param1)->signal();
                asm volatile ("mv a0, %0" : : "r" (res));
                asm volatile ("sd a0, 10*8(fp)");

                break;
            case CONST_SEM_CLOSE:
                asm volatile ("ld %0, 11*8(fp)" : "=r" (param1));
                delete ((kernel::Semaphore*)param1);        // U a0 ce se upisati uspesnost dealociranja..
                asm volatile ("sd a0, 10*8(fp)");
                break;


            case CONST_PUTC:
                asm volatile ("ld %0, 11*8(fp)" : "=r" (param1));
                __putc((char) param1);

                break;
            case CONST_GETC:
                res = __getc();

                asm volatile ("csrw sstatus, %0" : : "r" (varSstatus));
                asm volatile ("csrw sepc, %0" : : "r" (varSepc));

                asm volatile ("mv a0, %0" : : "r" (res));
                asm volatile ("sd a0, 10*8(fp)");

                break;
        }

        /// Svaki sistemski poziv radi i promenu konteksta:
        asm volatile ("csrr %0, sepc" : "=r" (varSepc));
        varSepc += 4;
        asm volatile ("csrr %0, sstatus" : "=r" (varSstatus));

        PCB::dispatch();

        asm volatile ("csrw sstatus, %0" : : "r" (varSstatus));
        asm volatile ("csrw sepc, %0" : : "r" (varSepc));
    }

    /// Prekid izazvan sistemskim pozivom iz sistemskog rezima: (samo main predaje procesor drugoj niti)
    else if (varScause == CONST_SIST_C_S) {
        asm volatile ("csrr %0, sepc" : "=r" (varSepc));
        varSepc += 4;
        asm volatile ("csrr %0, sstatus" : "=r" (varSstatus));

        PCB::dispatch();

        asm volatile ("csrw sstatus, %0" : : "r" (varSstatus));
        asm volatile ("csrw sepc, %0" : : "r" (varSepc));
        return;
    }

    /// Prekid izazvan tajmerom:
    else if (varScause == CONST_TIMER) {
        __asm__ volatile("csrc sip, 0x02"); // Kraj obrade softverskog prekida (oznacava se upisom nule na prvi bit registra sip).
    }

    /// Prekid izazvan konzolom:
    else if (varScause == CONST_CONSOLE) {
        console_handler();
    }
}


/// Prelaz sa korisnickog na sistemski stek niti pri ulasku u prekidnu rutinu.
extern "C" void switchToSsp() {
    asm volatile ("sd a4, -1*8(fp)");       // Tokom izvrsavanja ove f-je, isprljaju se a3, a4 i a5.
    asm volatile ("sd a5, -2*8(fp)");       //  Pre toga ih privremeno cuvam na fp, i vracam sa fp pred izlazak iz f-je.
    asm volatile ("sd a3, -3*8(fp)");

    depthCnt++;
    if (depthCnt == 1) {    // Radimo prelaz sa korisnickog na sistemski stek samo ako se ne radi o ugnezdjenom prekidu.

        uint64 savedSp;
        asm volatile ("mv %0, sp" : "=r" (savedSp));
        PCB::running->saveSp(savedSp);
        uint64 threadSsp = PCB::running->getSsp();

        threadSsp -= 0x10; // Jer ce return iz ove f-je uvecati sp za 0x10 racunajuci da ga je umanjio za 0x10 ulaskom u f-ju kad je stavio ra na stek..
                            // Ali nije vise isti sp kao onaj sa kojim smo usli u f-ju, pa nam to ne odgovara.
        asm volatile ("mv sp, %0" : : "r" (threadSsp));

    }

    asm volatile ("ld a4, -1*8(fp)");
    asm volatile ("ld a5, -2*8(fp)");
    asm volatile ("ld a3, -3*8(fp)");
}
/// Prelaz sa sistemskog na korisnicki stek niti (na kraju prekidne rutine, a u threadWrapper f-ji rucno).
extern "C" void switchToSp() {
    asm volatile ("sd a4, -1*8(fp)");
    asm volatile ("sd a5, -2*8(fp)");
    asm volatile ("sd a3, -3*8(fp)");

    if (depthCnt == 1) {        // Prelaz sa sistemskog na korisnicki stek samo ako se ne radi o ugnezdjenom prekidu.

        uint64 savedSp, threadSsp;
        asm volatile ("mv %0, sp" : "=r" (threadSsp));
        threadSsp += 0x10;  // Pre ovog uvecanja threadSsp nam je umanjen za 0x10 u odnosu na vrednost koja je bila u trap.s, i umanjice se za jos 0x10 kad ga uzmemo
                            //  u switchToSsp, i uvecace se za 0x10 kad iz switchToSsp predje u trap.s
        PCB::running->saveSsp(threadSsp);
        savedSp = PCB::running->getSp();
        asm volatile ("mv sp, %0" : : "r" (savedSp));

    }
    depthCnt--;

    asm volatile ("ld a4, -1*8(fp)");
    asm volatile ("ld a5, -2*8(fp)");
    asm volatile ("ld a3, -3*8(fp)");
}


/// Kernelska nit:
int main() {
    // Ucitavamo odmah pomocne singleton klase u memoriju, kako ne bi npr objekat scheduler bio alociran izmedju dve niti,
    //  pa onda izdelio trajno mem na dva slobodna fragmenta.
    MemoryAllocator::getInstace();
    Scheduler::getInstace();

    // Postavljanje prekidne rutine:
    asm volatile ("csrw stvec, %0" : : "r" (&supervisorTrap));

    // Kreiranje kernelske niti:
    PCB* krnl = PCB::createThread(nullptr, nullptr,nullptr);
    PCB::running = krnl;

    // Kreiranje userMain korisnicke niti:
    size_t stackSize = DEFAULT_STACK_SIZE*sizeof(uint64);
    uint64* usrStack = (uint64*)MemoryAllocator::getInstace()->allocate(stackSize/MEM_BLOCK_SIZE + (stackSize%MEM_BLOCK_SIZE ? 1 : 0));
    PCB* usr = PCB::createThreadNoArg(userMain, usrStack);
    usr->activate();

    // Kernelska nit samo predaje procesor ukoliko userMain nit jos nije zavrsena:
    while (!usr->isFinished()) {
        asm volatile ("ecall");
    }

    // Dealociranje preostalog zauzetog prostora:
    MemoryAllocator::getInstace()->deallocate(usr->getStack());
    MemoryAllocator::getInstace()->deallocate(usr->getPrvlStack());
    MemoryAllocator::getInstace()->deallocate(usr);
    krnl->setFinished(true);
    MemoryAllocator::getInstace()->deallocate(krnl->getPrvlStack());
    MemoryAllocator::getInstace()->deallocate(krnl);

    // Kraj kernelske niti:
    return 0;
}