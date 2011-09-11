#ifndef TASKING_H
#define TASKING_H

#include <util/cpp.h>
#include <memory/AddressSpace.h>

// This structure defines a 'task' - a process.
typedef struct task
{
   int id;                // Process ID.
   u32int esp;       // Stack and base pointers.
   u32int eip;            // Instruction pointer.
   AddressSpace *page_directory; // Page directory.
   struct task *next;     // The next task in a linked list.
} task_t;

// Initialises the tasking system.
void initialise_tasking();

// Called by the timer hook, this changes the running process.
void switch_task();

// Forks the current process, spawning a new one with a different
// memory space.
int fork();

// Causes the current process' stack to be forcibly moved to a new location.
extern "C" void move_stack(void *new_stack_start, u32int size);

// Returns the pid of the current process.
int getpid();

#endif
