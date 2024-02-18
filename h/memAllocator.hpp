#ifndef _memAllocator_hpp_
#define _memAllocator_hpp_

#include "../lib/hw.h"

// sizeof(imeKlase) - racunaju se samo nestaticki atributi. Ako klasa nema nestatickih atributa, vratice 1 umesto 0 kako ne bi bilo
//                      problema u koriscenju.

class MemoryAllocator {
private:
    struct FreeSegment {
        FreeSegment* next;
        size_t size;
    };
    FreeSegment* freeSegHead = nullptr;

    // Privatan konstruktor, samo ga getInstance() poziva jedanput.
    MemoryAllocator() {}
    // Onemogucavanje kopiranja objekta ove klase (kopirajuceg konstr, premestajuceg konstr, operatora dodele).
    MemoryAllocator(const MemoryAllocator&) = delete;
    MemoryAllocator& operator=(const MemoryAllocator&) = delete;
    MemoryAllocator(MemoryAllocator&&) = delete;
    MemoryAllocator& operator=(MemoryAllocator&&) = delete;

public:
    // Staticka klasa koja prvim pozivanjem stvara jedan objekat klase MemoryAllocator i vraca pok na njega, a ubuduce samo vraca pok na taj stvoreni objekat.
    static MemoryAllocator* getInstace() {
        static MemoryAllocator* memoryAllocator = nullptr;  // Samo prvi put ce se izvrsiti jer je definicija staticke promenljive u pitanju.
        if (!memoryAllocator) {
            memoryAllocator = (MemoryAllocator*)HEAP_START_ADDR;    // Ovaj jedinstven objekat klase MemoryAllocator stavljamo na pocetak slobodnog prosotra.
            // Posto je freeSegHead inicijalizovano na nullptr, potrebno najpre postaviti freeSegHead da ukazuje na mesto u memoriji nakon MemoryAllocator.
            memoryAllocator->freeSegHead = (FreeSegment*)((char*)HEAP_START_ADDR + sizeof(MemoryAllocator));
            memoryAllocator->freeSegHead->next = nullptr;
            memoryAllocator->freeSegHead->size = ((char*)HEAP_END_ADDR - ((char*)HEAP_START_ADDR + sizeof(MemoryAllocator))); // bez +1 jer je END-1 posl dostupna lokacija
                // Velicina segmenta mi uracunava i velicinu zaglavlja segmenta.
        }

        return memoryAllocator;
    }

    void* allocate(size_t size);
    int deallocate(void* address);
};

#endif