/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  size = 0;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  // inform the master controller that the interrupt is handled
  Machine::outportb(0x20, 0x20); // for round robin scheduler
  if(Machine::interrupts_enabled())  
    Machine::disable_interrupts();
  // if the queue isn't empty the thread at the top of the queue is dispatched
  if (size != 0) {
    size--;
    Thread* current_thread = ready_queue.dequeue();
    Thread::dispatch_to(current_thread);
  }

  if(!Machine::interrupts_enabled())  
    Machine::enable_interrupts();  
}

void Scheduler::resume(Thread * _thread) {
  if(Machine::interrupts_enabled())  
    Machine::disable_interrupts();

  ready_queue.enqueue(_thread);
  size++; 
}

void Scheduler::add(Thread * _thread) {
  if(Machine::interrupts_enabled())  
    Machine::disable_interrupts();

  ready_queue.enqueue(_thread);
  size++;
}

void Scheduler::terminate(Thread * _thread) {
  for (int i = 0; i < size; i++) {
    Thread * temp = ready_queue.dequeue();
    if (temp->ThreadId() == _thread->ThreadId()) {
      size--;
    } else {
      ready_queue.enqueue(temp);
    }
  } 
}

