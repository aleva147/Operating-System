#ifndef _syscall_c_h_
#define  _syscall_c_h_

#include "../lib/hw.h"
#include "../h/pcb.hpp"

/// Alokacija:
void* mem_alloc (size_t size);

int mem_free (void*);


/// Niti:

class _thread;
typedef _thread* thread_t;

// Stvara objekat PCB-a zadatim argumenitma.
int thread_initialize (thread_t* handle, void(*start_routine)(void*), void* arg);

// Startuje tekucu nit (poziva se iz Thread::start() metoda). Ukoliko je vec startovana, samo nema efekta, i vraca -1 u tom slucaju. U suprotnom, vraca 0.
int thread_start (thread_t myHandle);

// Stvara objekat PCB-a zadatim argumentima i startuje ga.
int thread_create (thread_t* handle, void(*start_routine)(void*), void* arg);

// Gasi tekuću nit. U slučaju neuspeha vraća negativnu vrednost (kôd greške)
int thread_exit ();

// Potencijalno oduzima procesor tekućoj i daje nekoj drugoj (ili istoj) niti
void thread_dispatch ();

void thread_deallocate(thread_t handle);

int thread_kill(thread_t handle);

/// Semafori:

class _sem;
typedef _sem* sem_t;

int sem_open (sem_t* handle, unsigned init);

int sem_close (sem_t handle);

int sem_wait (sem_t id);

int sem_signal (sem_t id);


/// Za javne testove:

void putc(char chr);
char getc();

#endif