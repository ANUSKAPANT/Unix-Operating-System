/*
 File: vm_pool.C
 
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

#include "vm_pool.H"
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
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    region_number = 0;
    allocated_region* vm_regions = (allocated_region *) base_address;
    page_table->register_pool(this);
}

unsigned long VMPool::allocate(unsigned long _size) {
    if(region_number == 0) {
        vm_regions[0].base_address= base_address;
        vm_regions[0].size = PageTable::PAGE_SIZE;
        region_number++; 
        return base_address + PageTable::PAGE_SIZE;
    }
    // assert false when size if greater than availabe
    assert(!(_size > PageTable::PAGE_SIZE - region_number));

    // allocate continuous memory in size of pages
    unsigned long frames = _size / (PageTable::PAGE_SIZE);
    frames += (_size % (PageTable::PAGE_SIZE)) > 0 ? 1 : 0;

    vm_regions[region_number].base_address = vm_regions[region_number-1].base_address +  vm_regions[region_number-1].size;
    vm_regions[region_number].size = frames*PageTable::PAGE_SIZE;
    region_number++;
     
    Console::puts("Allocated region of memory.\n");
    return vm_regions[region_number-1].base_address;
}

void VMPool::release(unsigned long _start_address) {
    int reg_no = -1;

    // find the region to be released
    for (int i = 0; i < region_number; i++) {
        if (vm_regions[i].base_address == _start_address) {
            reg_no = i;
            break;
        }
    }

    assert(!(reg_no < 0));

    // freeing page table entries
    unsigned int allocated_pages = (vm_regions[reg_no].size) / PageTable::PAGE_SIZE;

    for (int i = 0 ; i < allocated_pages ;i++) {
        page_table->free_page(_start_address);
        _start_address += PageTable::PAGE_SIZE;
    }

    // removing the current region from vm_regions.
    for (int i = reg_no; i < region_number - 1; i++) {
        vm_regions[i] = vm_regions[i+1];
    }
    region_number--;
    page_table->load();
}

bool VMPool::is_legitimate(unsigned long _address) {
    if((_address >= base_address) && (_address < base_address + size))
        return true;
    return false;
}

