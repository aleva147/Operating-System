#include "../h/regFuns.hpp"

void RegFuns::getBackSppSpie() {
    asm volatile ("csrw sepc, ra");

    // Prelazak u korisnicki rezim:
    uint64 a = 0x100;
    asm volatile ("csrc sstatus, %0" : : "r" (a));

    asm volatile ("sret");
}
