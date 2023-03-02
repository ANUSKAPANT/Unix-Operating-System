/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"
#include "thread.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/
extern Scheduler* SYSTEM_SCHEDULER;

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
    disk_queue_size = 0;
    this->disk_queue =  new Queue();
}

void BlockingDisk::wait_until_ready() {
    if (!SimpleDisk::is_ready()) {
        Thread *current_thread = Thread::CurrentThread();
        this->disk_queue->enqueue(current_thread);
    	disk_queue_size++;
        SYSTEM_SCHEDULER->yield();
    }
}

bool BlockingDisk::is_ready() {
    return SimpleDisk::is_ready();
}


/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
   SimpleDisk::read(_block_no, _buf);
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
    SimpleDisk::write(_block_no, _buf);
}
