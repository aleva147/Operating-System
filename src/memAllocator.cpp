#include "../h/memAllocator.hpp"
#include "../lib/hw.h"



// Oslobađa prostor prethodno zauzet pomoću mem_alloc. Vraća 0 u slučaju uspeha, negativnu vrednost u slučaju greške (kôd greške).
//  Argument mora imati vrednost vraćenu iz mem_alloc. Ukoliko to nije slučaj, ponašanje je nedefinisano: jezgro može
//  vratiti grešku ukoliko može da je detektuje ili manifestovati bilo kakvo drugo predvidivo ili nepredvidivo ponašanje
int MemoryAllocator::deallocate(void* address) {
    // Provera neuspeha:
    if (!address) return -1;
    for (FreeSegment* cur = freeSegHead; cur; cur = cur->next) {    // Prosledjena je adresa slobodnog fragmenta.
        if ((char*)cur == (char*)address) return -1;
    }

    // Uvezivanje ovog segmenta u listu slobodnih segmenata:
    FreeSegment* bh = (FreeSegment*)((char*)address - sizeof(FreeSegment));   // Pristup zaglavlju prosledjenog procesa.
    FreeSegment* cur = nullptr;

    // Nalazenje mesta za umetanje u listu slobodnih segmenata tako da ona ostane rastuci uredjena po adresama.
    if (!(!freeSegHead || (char*)bh < (char*)freeSegHead)) {
        for (cur = freeSegHead; cur->next && (char*)bh > (char*)(cur->next); cur = cur->next); // cur se postavlja na slobodan fragment koji prethodi prosledjenom procesu.
    }

    // Ukoliko direktno ispred procesa koji dealociramo imamo slobodan prostor, vrsimo spajanje.
    if (cur && (char*)bh == (char*)cur + cur->size) {
        cur->size += bh->size;
        // Ukoliko i iza procesa koji dealociramo imamo slobodan prostor, vrsimo spajanje.
        if (cur->next && (char*)bh + bh->size == (char*)cur->next){
            cur->size += cur->next->size;
            cur->next = cur->next->next;
        }
        return 0;
    }

    // Ukoliko samo direktno iza slobodnog procesa koji dealociramo imamo slobodan prostor, vrsimo spajanje.
    FreeSegment* nxt = cur ? cur->next : freeSegHead;
    if (nxt && (char*)bh + bh->size == (char*)nxt) {
        bh->size += nxt->size;
        bh->next = nxt->next;
        if (nxt == freeSegHead) freeSegHead = bh;
        else cur->next = bh;
        return 0;
    }

    // Ukoliko nema slobodnog prostora ni direktno iza ni direktno ispred, samo uvezujemo u listu slobodnih fragmenata prostor procesa koji dealociramo.
    if (cur) {
        bh->next = cur->next;
        cur->next = bh;
    }
    else {
        bh->next = freeSegHead;
        freeSegHead = bh;
    }

    return 0;
}

// Alociran prostor je broj trazenih blokova + jedan blok kako bi sigurno stalo i zaglavlje FreeSegment.
//  Zaglavlje je na pocetku alociranog prostora, a odmah nakon njega ce biti struktura koja se alocira
//  (dakle visak slobodnog prostora na kraju alociranog prostora).
//  Pokazivac na alociran prostor koji vracam ukazuje na adresu nakon zaglavlja.
//
//  Razlog je sto se f-ji za dealokaciju prosledjuje samo adresa procesa i ne zna se njegova velicina.
//  Ovako cu u f-ji za dealokaciju od adrese pocetka procesa oduzeti velicinu zaglavlja, konvertovati dobijenu adresu u FreeSegment*,
//  i polje size ce mi dati velicinu prosotra koji je proces zauzeo (racunajuci i ovo zaglavlje) i koji treba osloboditi.
void* MemoryAllocator::allocate(size_t size) {
    if (size <= 0) return nullptr;

    FreeSegment* cur = freeSegHead;
    FreeSegment* prev = nullptr;
    void* retAddr = nullptr;

    size_t sizeInBlocks = size + 1;     // Na pocetku dodatni blok kako bi bilo mesta i za FreeSegment strukturu.
    sizeInBlocks *= MEM_BLOCK_SIZE;

    // Pronalazenje odgovarajuceg slobodnog segmenta.
    for (; cur; prev = cur, cur = cur->next) {
        if (sizeInBlocks <= cur->size) {            // Nadjen je slobodan fragment dovoljne velicine.
            retAddr = (char*)cur + sizeof(FreeSegment);
            if (cur->size - sizeInBlocks >= sizeof(FreeSegment)) {      // Od ostatka prostora izabranog slobodnog fragmenta se formira novi slobodan fragment i uvezuje u listu.
                FreeSegment *newSg = (FreeSegment *) ((char*)cur + sizeInBlocks);
                if (prev) prev->next = newSg;
                else freeSegHead = newSg;
                newSg->next = cur->next;
                newSg->size = cur->size - sizeInBlocks;
            }// Od ostatka prostora izabranog fragmenta ne moze da se napravi novi fragment. Ovaj slucaj se vise ne moze desiti sada kada sam prepravio kod.
            else if (prev) prev->next = cur->next;
            else freeSegHead = cur->next;

            cur->next = nullptr;
            cur->size = sizeInBlocks;   // Bitno da imamo upamcenu velicinu alociranog prosotra u zaglavlju na pocetku samog alociranog prostora zbog kasnije dealokacije.
            break;
        }
    }

    return retAddr;
}
