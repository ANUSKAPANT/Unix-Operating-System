#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;
VMPool * PageTable::pool_head = NULL;

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    current_page_table = this;
    current_page_table->page_directory = (unsigned long *) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);
    // Recursive implementation making entry 1023 of page directory valid
    page_directory[1023] = (unsigned long)(page_directory ) | 3;
    
    unsigned long *page_table = (unsigned long *) (process_mem_pool->get_frames(1) * PAGE_SIZE);

    // holds the physical address of where a page is
    unsigned long address = 0;

    // map the first 4MB of memory
    for( unsigned int i = 0; i < 1024; i++) {
        // attribute set to: supervisor level, read/write, present(011 in binary)
        page_table[i] = address | 3;
        address += 4096; // 4096 = 4KB
    };

    // fill the first entry of the page directory
    // attribute set to: supervisor level, read/write, present(011 in binary)
    page_directory[0] = (unsigned long) page_table | 3;

    for(unsigned int i = 1; i < 1023; i++) {
        current_page_table->page_directory[i] = 0 | 2;
    }

    Console::puts("Constructed Page Table object\n");
}

void PageTable::load()
{
    current_page_table = this;
    // load the page directory address to cr3 register
    write_cr3((unsigned long)current_page_table->page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    paging_enabled = 1;
    // set the paging bit in CR0 to 1, i.e the 31st bit
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    // read the page fault address from cr2 register
   // read the page fault address from cr2 register
    unsigned long address = read_cr2();
    unsigned int page_present = 0;
    VMPool *temp = PageTable::pool_head;
    while (temp != NULL){
      if(temp->is_legitimate(address)) {
        page_present = 1;
        break;
      }
      temp = temp->next;
    }

    if(page_present == 0 && temp!= NULL)
	{
      Console::puts("INVALID ADDRESS \n");
      assert(false);	  	
	}

    unsigned long *page_directory = current_page_table->page_directory;
    unsigned long *page_table;
    unsigned long *page_table_entry;

    // extract the most significant 10 bits in a 32 bit address using right shift 22 times
    unsigned long page_dir_index = address >> 22;
    // extract the 10 bit page number in a 32 bit address using right shift 12 times and clearing the other bits
    unsigned long page_table_index = (address >> 12) & 0X3FF;

    // check if the present bit is 0
    if((page_directory[page_dir_index] & 1) == 0) {
        page_table = (unsigned long *)(process_mem_pool->get_frames(1) * PAGE_SIZE);
        // fill the index and set attribute to: supervisor level, read/write, present(011 in binary)
        unsigned long *page_directory_entry = (unsigned long *)(0xFFFFF << 12);
        page_directory_entry[page_dir_index] = (unsigned long)page_table | 3;
    }

    page_table_entry = (unsigned long*)(process_mem_pool->get_frames(1) * PAGE_SIZE);
    unsigned long *page_entry = (unsigned long *)((0x3FF<< 22)| (page_dir_index <<12));
    page_entry[page_table_index] = (unsigned long)page_table_entry | 3;

    Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
    if( PageTable::pool_head == NULL ) {
        PageTable::pool_head = _vm_pool;
    } else {
        VMPool *temp = PageTable::pool_head;
        while(temp->next != NULL) temp = temp->next;
        temp->next = _vm_pool;
    }
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    // shifting right 22 times to extract page directory index
    unsigned long page_dir_index = _page_no >> 22;
    // shifting and reseting bits to extract page table index
    unsigned long page_table_index = (_page_no >> 12) & 0X3FF;
    // page table page for mmu but PDE
    unsigned long *page_table = (unsigned long *) (0XFFC00000 | (page_dir_index << 12));
    // entries are 4 byte long
    unsigned long frame_no = page_table[page_table_index] / PAGE_SIZE;

    process_mem_pool->release_frames(frame_no);
    // reset the present bit
    page_table[page_table_index] |= 2;
    Console::puts("freed page\n");

    //Flushing TLB
    load();
}
