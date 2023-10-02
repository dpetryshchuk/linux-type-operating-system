#include "paging.h"

extern void loadPageDirectory(unsigned int*);
extern void enablePaging();


void paging_init(){

//iterator
  unsigned i;

  //blank the page directory and table with 0s
  for(i = 0; i < KB; i++){
    pagingDirectory[i].present              = 0;
    pagingDirectory[i].read_write           = 0;
    pagingDirectory[i].user_super           = 0;
    pagingDirectory[i].write_through        = 0; 
    pagingDirectory[i].cache_disable        = 0; 
    pagingDirectory[i].accessed             = 0;
    pagingDirectory[i].available_bit_6      = 0;
    pagingDirectory[i].page_size            = 0;
    pagingDirectory[i].available_bits_11_8  = 0; 
    pagingDirectory[i].table_addr_31_12     = 0;

    pagingTable[i].present            = 0;
    pagingTable[i].read_write         = 0;
    pagingTable[i].user_super         = 0;
    pagingTable[i].write_through      = 0;
    pagingTable[i].cache_disable      = 0;
    pagingTable[i].accessed           = 0;
    pagingTable[i].dirty              = 0;
    pagingTable[i].pat                = 0;
    pagingTable[i].global             = 0;
    pagingTable[i].available_bits_11_9= 0;
    pagingTable[i].page_addr_31_12    = 0;

    pagingTableVideo[i].present            = 0;
    pagingTableVideo[i].read_write         = 0;
    pagingTableVideo[i].user_super         = 0;
    pagingTableVideo[i].write_through      = 0;
    pagingTableVideo[i].cache_disable      = 0;
    pagingTableVideo[i].accessed           = 0;
    pagingTableVideo[i].dirty              = 0;
    pagingTableVideo[i].pat                = 0;
    pagingTableVideo[i].global             = 0;
    pagingTableVideo[i].available_bits_11_9= 0;
    pagingTableVideo[i].page_addr_31_12    = 0;
}

  //set up first directory entry that maps the first 4 MB of 4 KB pages
  pagingDirectory[0].present              = 1; //pages are present
  pagingDirectory[0].read_write           = 1; //pages are readable
  pagingDirectory[0].user_super           = 0;
  pagingDirectory[0].write_through        = 0; 
  pagingDirectory[0].cache_disable        = 0; 
  pagingDirectory[0].accessed             = 0;
  pagingDirectory[0].available_bit_6      = 0;
  pagingDirectory[0].page_size            = 0; //4 KB page sizes
  pagingDirectory[0].available_bits_11_8  = 0; 
  //align [0] to a 4KB block to be overwritten by "access bits and such"
  pagingDirectory[0].table_addr_31_12 = ((unsigned int) pagingTable) >> 12;

  //set up paging directory[1] for the kernel page (next 4 MB)
  pagingDirectory[1].present              = 1;
  pagingDirectory[1].read_write           = 1;
  pagingDirectory[1].user_super           = 0; //supervisor bit should be set for kernel page???
  pagingDirectory[1].write_through        = 0;
  pagingDirectory[1].cache_disable        = 0;
  pagingDirectory[1].accessed             = 0;
  pagingDirectory[1].available_bit_6      = 0;
  pagingDirectory[1].page_size            = 1; //kernel page is 4 MB long
  pagingDirectory[1].available_bits_11_8  = 0;
  pagingDirectory[1].table_addr_31_12 = ((unsigned int) KERNEL_ADDR) >> 12;

  //setup the page table for the first 4 KB
  for(i = 0; i < KB; i++){
    //set up video memory pages
    // sets up B7 - BC for video memory pages
    // B7 is a direct mapping of video memory
    // B8 changes but is the global video memory page
    // B9 - BC are background buffers for scheduling use.
    if((i*FOURKB) == VIDMEM_USER_ADDR || (i * FOURKB) == VIDMEM_ADDR 
        || (i * FOURKB) == (VIDMEM_ADDR + FOURKB) 
        || (i * FOURKB) == (VIDMEM_ADDR + 2 * FOURKB)
        || (i * FOURKB) == (VIDMEM_ADDR + 3 * FOURKB)){
      pagingTable[i].present = 1;
      pagingTable[i].read_write = 1;
    }
    if(i * FOURKB == VIDMEM_USER_ADDR){
      pagingTable[i].page_addr_31_12 = VIDMEM_ADDR >> 12;
    }
    else{
      pagingTable[i].page_addr_31_12 = i;
    }
  }
 

  //setup vidmap page
  for(i = 0; i < KB; i++){
    pagingTableVideo[i].read_write = 1;
    pagingTableVideo[i].user_super = 1;
    pagingTableVideo[i].page_addr_31_12 = i;
  }

    //enable paging using extern fxns in "enablePaging.S"
    loadPageDirectory((unsigned int*) pagingDirectory);
    enablePaging();

}

