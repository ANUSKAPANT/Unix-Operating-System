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
   unsigned long *page_table = (unsigned long *) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);

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
 
   for(unsigned int i = 1; i < 1024; i++) {
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
   unsigned long address = read_cr2();
   unsigned long *page_directory = current_page_table->page_directory;
   unsigned long *page_table;
   unsigned long *page_table_entry;

   // extract the most significant 10 bits in a 32 bit address using right shift 22 times
   unsigned long page_dir_index = address >> 22;
   // extract the 10 bit page number in a 32 bit address using right shift 12 times and clearing the other bits
   unsigned long page_table_index = (address >> 12) & 0X3FF;

   // check if the present bit is 0
   if((page_directory[page_dir_index] & 1) == 0) {
      page_table = (unsigned long *)(kernel_mem_pool->get_frames(1) * PAGE_SIZE);
      // fill the index and set attribute to: supervisor level, read/write, present(011 in binary)
      page_directory[page_dir_index] = (unsigned long)page_table | 3;
      page_directory[page_dir_index] |= 3;

      for(int i = 0; i < 1024; i++){
         page_table[i] = 0 | 2;
      }
   }

   // assign the entry in page directory to page table and resetting the attributes
   page_table = (unsigned long *) (page_directory[page_dir_index] & 0xFFFFF000);
   page_table_entry = (unsigned long*)(kernel_mem_pool->get_frames(1) * PAGE_SIZE);
   page_table[page_table_index] = (unsigned long)page_table_entry | 3;
  
   Console::puts("handled page fault\n");
}

