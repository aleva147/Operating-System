#ifndef _semaphore_hpp_
#define _semaphore_hpp_

#include "../h/queue.hpp"

class Queue;
class Scheduler;



namespace kernel {

    class Semaphore {
    private:
        int val;
        Queue *blocked = new Queue();
        volatile bool destroyed = false;

        Semaphore(unsigned init) : val(init) {}

    protected:
        /// Pomocni:
        void deblock();

    public:
        static Semaphore *createSemaphore(unsigned init) {
            return new Semaphore(init);
        }

        /// Getter:
        bool getDestroyedStatus() const { return destroyed; }

        /// Wait i Signal:
        int wait();
        int signal();

        /// Destruktor:
        ~Semaphore();

        /// Operatori:
        void *operator new(size_t size);
        void operator delete(void *p);

        /// Ostalo:
        int remove(PCB* t);
    };

}

#endif