/*
 * Ovaj fajl predstavlja C API na koji ce se CPP API oslanjati.
 *   C API poziva odgovarajuce sistemske pozive za realizaciju zahteva.
 */


#include "../lib/hw.h"
#include "../h/syscall_c.h"

//extern "C" void call_universal();       // Odustao od ove ideje.
//uint64 CALL_ID;

extern "C" const uint64 CONST_MEM_ALLOC = 0x01;
extern "C" const uint64 CONST_MEM_FREE = 0x02;
extern "C" const uint64 CONST_THREAD_INITIALIZE = 0x16;
extern "C" const uint64 CONST_THREAD_START = 0x17;
extern "C" const uint64 CONST_THREAD_KILL = 0x18;
extern "C" const uint64 CONST_THREAD_CREATE = 0x11;
extern "C" const uint64 CONST_THREAD_EXIT = 0x12;
extern "C" const uint64 CONST_THREAD_DISPATCH = 0x13;
extern "C" const uint64 CONST_THREAD_DEALLOCATE = 0x15;
extern "C" const uint64 CONST_SEM_OPEN = 0x21;
extern "C" const uint64 CONST_SEM_CLOSE = 0x22;
extern "C" const uint64 CONST_SEM_WAIT = 0x23;
extern "C" const uint64 CONST_SEM_SIGNAL = 0x24;
extern "C" const uint64 CONST_PUTC = 0x3;
extern "C" const uint64 CONST_GETC = 0x4;



/// Alokacija:
void* mem_alloc(size_t size) {
    size_t sizeInBlks = size / MEM_BLOCK_SIZE;        // Izrazavanje date velicine u bajtovima u velicinu u blokovima.
    if (size % MEM_BLOCK_SIZE) sizeInBlks++;
    asm volatile ("mv a1, %0" : : "r" (sizeInBlks));  // Prosledjivanje velicine u blokovima sistemskom pozivu.
    asm volatile ("mv a0, %0" : : "r" (CONST_MEM_ALLOC));
    asm volatile ("ecall");

    void* res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}

int mem_free(void *addr) {
    asm volatile ("mv a1, %0" : : "r" (addr));
    asm volatile ("mv a0, %0" : : "r" (CONST_MEM_FREE));
    asm volatile ("ecall");

    int res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}


/// Niti:
int thread_initialize(thread_t *handle, void (*start_routine)(void *), void *arg) {
    /// Korisnicki stek:
    size_t stackSize = DEFAULT_STACK_SIZE * sizeof(uint64);
    uint64* userStack = (uint64*)mem_alloc(stackSize);
    if (!userStack) return -1;           // Nije bilo memorije da se alocira korisnicki stek, neusepsno kreiranje niti.

    /// Sistemski poziv za thread_initialize:
    asm volatile ("mv a4, %0" : : "r" (userStack));    // Korisnicki stek je poslednji parametar ABI f-je.
    asm volatile ("mv a0, %0" : : "r" (CONST_THREAD_INITIALIZE));
    asm volatile ("mv a1, %0" : : "r" (handle));
    asm volatile ("mv a2, %0" : : "r" (start_routine));
    asm volatile ("mv a3, %0" : : "r" (arg));

    asm volatile ("ecall");

    int res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}

int thread_start(thread_t myHandle) {
    asm volatile ("mv a1, a0");
    asm volatile ("mv a0, %0" : : "r" (CONST_THREAD_START));

    asm volatile ("ecall");

    int res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}

// Dve promene konteksta do kraja ove f-je (kad se alocira stek, kad se ujedno inicijalizuje i stavi u red spremnih).
int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg) {
    /// Korisnicki stek:
    size_t stackSize = DEFAULT_STACK_SIZE * sizeof(uint64);
    uint64* userStack = (uint64*)mem_alloc(stackSize);
    if (!userStack) return -1;         // Nije bilo memorije da se alocira korisnicki stek, neusepsno kreiranje niti.

    /// Poziv ABI thread_initialize:
    asm volatile ("mv a4, %0" : : "r" (userStack));    // Korisnicki stek je poslednji parametar ABI f-je.
    asm volatile ("mv a0, %0" : : "r" (CONST_THREAD_CREATE));
    asm volatile ("mv a1, %0" : : "r" (handle));
    asm volatile ("mv a2, %0" : : "r" (start_routine));
    asm volatile ("mv a3, %0" : : "r" (arg));

    asm volatile ("ecall");

    int res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}

void thread_dispatch() {
    asm volatile ("mv a0, %0" : : "r" (CONST_THREAD_DISPATCH));
    asm volatile ("ecall");
}

int thread_exit() {
    asm volatile ("mv a0, %0" : : "r" (CONST_THREAD_EXIT));
    asm volatile ("ecall");

    return 0;   // Nikad se nece izvrsiti. Nece se desiti neuspeh sistemskog poziva.
}

void thread_deallocate(thread_t handle) {
    asm volatile ("mv a1, a0");
    asm volatile ("mv a0, %0" : : "r" (CONST_THREAD_DEALLOCATE));
    asm volatile ("ecall");
}

int thread_kill(thread_t handle) {
    asm volatile ("mv a1, a0");
    asm volatile ("mv a0, %0" : : "r" (CONST_THREAD_KILL));
    asm volatile ("ecall");

    int res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}


/// Semafori:
int sem_open(sem_t *handle, unsigned int init) {
    asm volatile ("mv a2, a1");
    asm volatile ("mv a1, a0");
    asm volatile ("mv a0, %0" : : "r" (CONST_SEM_OPEN));

    asm volatile ("ecall");

    int res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}

int sem_wait(sem_t id) {
    asm volatile ("mv a1, a0");
    asm volatile ("mv a0, %0" : : "r" (CONST_SEM_WAIT));

    asm volatile ("ecall");

    int res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}

int sem_signal(sem_t id) {
    asm volatile ("mv a1, a0");
    asm volatile ("mv a0, %0" : : "r" (CONST_SEM_SIGNAL));

    asm volatile ("ecall");

    int res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}

// Oslobađa semafor sa datom ručkom. Sve niti koje su se zatekle da čekaju na semaforu se deblokiraju, pri čemu njihov
//   wait vraća grešku. U slučaju uspeha vraća 0, a u slučaju neuspeha vraća negativnu vrednost (kôd greške)
int sem_close(sem_t handle) {
    asm volatile ("mv a1, a0");
    asm volatile ("mv a0, %0" : : "r" (CONST_SEM_CLOSE));

    asm volatile ("ecall");

    int res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}


/// Konzola:
void putc(char chr) {
    asm volatile ("mv a1, a0");
    asm volatile ("mv a0, %0" : : "r" (CONST_PUTC));

    asm volatile ("ecall");
}

char getc() {
    asm volatile ("mv a0, %0" : : "r" (CONST_GETC));

    asm volatile ("ecall");

    char res;
    asm volatile ("mv %0, a0" : "=r" (res));
    return res;
}
