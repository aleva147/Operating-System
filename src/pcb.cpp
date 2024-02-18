#include "../h/pcb.hpp"
#include "../h/scheduler.hpp"
#include "../h/regFuns.hpp"
#include "../h/memAllocator.hpp"



PCB* PCB::running = nullptr;

/// Za promenu konteksta:
void PCB::dispatch() {
    PCB* old = running;
    if (!(old->isFinished() || old->isBlocked())) Scheduler::getInstace()->put(running);
    running = Scheduler::getInstace()->get();

    contextSwitch(&old->context, &running->context);
}

void PCB::yield() {
    uint64 dispCall = 0x13;
    asm volatile ("mv a0, %0" : : "r" (dispCall));
    asm volatile ("ecall");
}

/// Omotac: (Na kraju omotacke f-je se radi sistemski poziv cime se prelazi na sistemski stek)
void PCB::threadWrapper() {
    // Prelazak na korisnicki stek
    asm volatile ("mv %0, sp" : "=r" (PCB::running->context.ssp));
    asm volatile ("mv sp, %0" : : "r" (PCB::running->context.sp));
    depthCnt--;

    // Prelazak u korisnicki rezim
    RegFuns::getBackSppSpie();
    // Izvrsavanje tela korisnicke niti
    if (running->body) running->body(running->arg);
    else if (running->bodyNoArg) running->bodyNoArg();
    // Zavrsavanje niti (yield radi ecall za odg obradu, tad ce se uraditi prelaz na sistemski stek)
    running->setFinished(true);
    PCB::yield();
}

/// Za pravljenje objekta:
PCB* PCB::createThread(Body body, void* arg, uint64* userStack) {
    PCB* p = new PCB(body, arg, userStack);
    if (!p || !p->prvlStack) return nullptr;    // U handleru cemo onda u a0 upisati negativnu vrednost da oznacimo da
                                                //   sist. poz. thread_initialize nije uspeo.
    return p;
}
PCB *PCB::createThreadNoArg(Body2 body, uint64* userStack) {
    PCB* p = new PCB(body, userStack);
    if (!p || !p->prvlStack) return nullptr;    // U handleru cemo onda u a0 upisati negativnu vrednost da oznacimo da
                                                //   sist. poz. thread_initialize nije uspeo.
    return p;
}

/// Konstruktori:
PCB::PCB(Body givenBody, void* argument, uint64* userStack) {
    if (givenBody) {
        body = givenBody;
        arg = argument;
        stack = userStack;  // Napravljen u C API i prosledjen sist pozivu.
        size_t sizeInBlks = (DEFAULT_STACK_SIZE*sizeof(uint64)) / MEM_BLOCK_SIZE + ((DEFAULT_STACK_SIZE*sizeof(uint64)) % MEM_BLOCK_SIZE ? 1 : 0);
        prvlStack = (uint64*) MemoryAllocator::getInstace()->allocate(sizeInBlks);

        context = { (uint64) &threadWrapper, (uint64) &stack[DEFAULT_STACK_SIZE], (uint64) &prvlStack[DEFAULT_STACK_SIZE] };
    }
    else {  // Main nit vec ima svoj korisnicki stek, ali treba da joj dodelimo sistemski.
        size_t sizeInBlks = (DEFAULT_STACK_SIZE*sizeof(uint64)) / MEM_BLOCK_SIZE + ((DEFAULT_STACK_SIZE*sizeof(uint64)) % MEM_BLOCK_SIZE ? 1 : 0);
        prvlStack = (uint64*) MemoryAllocator::getInstace()->allocate(sizeInBlks);

        context = { 0, 0, (uint64) &prvlStack[DEFAULT_STACK_SIZE] };
    }
}
PCB::PCB(PCB::Body2 body, uint64 *userStack) {
    if (body) {
        bodyNoArg = body;
        stack = userStack;
        size_t sizeInBlks = (DEFAULT_STACK_SIZE*sizeof(uint64)) / MEM_BLOCK_SIZE + ((DEFAULT_STACK_SIZE*sizeof(uint64)) % MEM_BLOCK_SIZE ? 1 : 0);
        prvlStack = (uint64*) MemoryAllocator::getInstace()->allocate(sizeInBlks);

        context = { (uint64) &threadWrapper, (uint64) &stack[DEFAULT_STACK_SIZE], (uint64) &prvlStack[DEFAULT_STACK_SIZE] };
    }
}

/// Destruktor:
PCB::~PCB() {
    MemoryAllocator::getInstace()->deallocate(stack);
    MemoryAllocator::getInstace()->deallocate(prvlStack);
}

/// Za startovanje niti:
int PCB::activate() {
    if (!hasStarted) {              // Kako ne bismo stavili vise puta jednu nit u red spremnih.
        Scheduler::getInstace()->put(this);
        hasStarted = true;
        return 0;
    }
    else return -1;    // Kako bismo u C++ apiju imali predstavu da je nit vec startovana i da je poziv bio bez efekta.
}

/// Operatori:
void *PCB::operator new(size_t size) {
    size_t sizeInBlks = size / MEM_BLOCK_SIZE + (size % MEM_BLOCK_SIZE ? 1 : 0);
    return MemoryAllocator::getInstace()->allocate(sizeInBlks);
}
void PCB::operator delete(void *p) {
    MemoryAllocator::getInstace()->deallocate(p);
}

/// Ostalo:
int PCB::kill() {
    if (finished) return -1;// Nit je vec zavrsena, sigurno nista nije blokirano na njoj i sigurno nigde nije blokirana.

    int res;
    if (blockedOnSem) {            // Izbacivanje iz semafora na kome je blokirana (ako je blokirana na nekom semaforu)
        res = blockedOnSem->remove(this);
    }
    else {                         // Izbacivanje iz reda spremnih ako nije blokirana na nekom semaforu.
        res = Scheduler::getInstace()->remove(this);
    }

    // Odblokiranje svih niti koje su blokirane na ovoj. (Ne postoji join pa mi ovo ne treba)

    // Azuriranje finished indikatora.
    finished = true;

    return res;
}
