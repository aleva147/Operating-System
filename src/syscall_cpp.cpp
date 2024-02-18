/*
 * Ovaj fajl predstavlja CPP API koji ce biti dostupan korisnicima naseg operativnog sistema koji pravimo.
 *   Za njih jedino CPP API postoji, nemaju nikakav uvid o C API-ju na koji se oslanja, niti o nizim slojevima.
 */

#include "../h/syscall_cpp.hpp"



/// Operatori new i delete:
void* operator new(size_t n){
    return mem_alloc(n);
}

void operator delete(void* p) {
    mem_free(p);
}

void* operator new[](uint64 n) {
    return mem_alloc(n);
}

void operator delete[](void* p) noexcept {
    mem_free(p);
}


/// Klasa Thread:
Thread::Thread(void (*body)(void *), void* arg) {
    thread_initialize(&myHandle, body, arg);
}

void _run(void* t) {
    if (t) ((Thread*)t)->run();
}
Thread::Thread() {
    thread_initialize(&myHandle, _run, this);   // F-ji run ce biti prosledjen argument this, pa ce ona pozivati this->run();
}

Thread::~Thread() {
    thread_deallocate(myHandle);
    myHandle = nullptr;
}

int Thread::start() {
    return thread_start(myHandle);
}

void Thread::dispatch() {
    thread_dispatch();
}

int Thread::sleep(time_t) {
    return 0;
}

// Ako je vec finished, negativna povratna vr.
int Thread::kill() {
    return thread_kill(myHandle);
}


/// Klasa Semaphore:
Semaphore::Semaphore(unsigned int init) {
    if (sem_open(&myHandle, init) < 0) myHandle = nullptr;
}

Semaphore::~Semaphore() {
    if (myHandle) sem_close(myHandle);
}

int Semaphore::wait() {
    return sem_wait(myHandle);;
}

int Semaphore::signal() {
    if (!myHandle) return -1;
    return sem_signal(myHandle);
}


/// Klasa Console:
char Console::getc() {
    return ::getc();
}

void Console::putc(char c) {
    ::putc(c);
}
