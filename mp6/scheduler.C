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
  disk = NULL;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {

    if(disk !=NULL && disk->is_ready() && disk->disk_queue_size != 0) {
        Thread *disk_top_thread = disk->disk_queue->dequeue();
        disk->disk_queue_size--;
        Thread::dispatch_to(disk_top_thread);
    } else {
        if (size != 0) {
	    size--;
	    Thread* current_thread = ready_queue.dequeue();
	    Thread::dispatch_to(current_thread);
	  }
    }
}

void Scheduler::resume(Thread * _thread) {
  ready_queue.enqueue(_thread);
  size++; 
}

void Scheduler::add(Thread * _thread) {
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


void Scheduler::addDisk(BlockingDisk * _disk) {
    disk = _disk;
}

