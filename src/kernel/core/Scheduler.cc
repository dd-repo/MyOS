#include <core/Scheduler.h>
#include <kutils.h>


Scheduler::Scheduler() {
    queue = NULL;
}

void Scheduler::init() {
    queue = new LinkedList<Thread*>();
}

void Scheduler::addThread(Thread* t) {
    queue->insert(t, 1);
}

void Scheduler::removeThread(Thread* t) {
    queue->remove(t);
}

Thread *Scheduler::pickThread() {
    if (queue->length() == 0)
        return NULL;
    Thread *r = queue->remove(0);
    queue->insertLast(r);
    return r;
}
