#ifndef PAGING_H
#define PAGING_H

#ifndef ASM

#include "types.h"

/*constants for paging*/
#define   KB            1024  
#define   FOURKB		(KB * 4)
#define   VIDMEM_ADDR   0xB8000   
#define   VIDMEM_USER_ADDR 0xB7000 // virtual address that is always mapped to video memory
#define   KERNEL_ADDR   0x400000       

/* paging directory struct */
typedef struct __attribute__((packed)) dir_entry_desc{
    uint32_t present            : 1;
    uint32_t read_write         : 1;
    uint32_t user_super         : 1;
    uint32_t write_through      : 1;
    uint32_t cache_disable      : 1;
    uint32_t accessed           : 1;
    uint32_t available_bit_6    : 1;
    uint32_t page_size          : 1; 
    uint32_t available_bits_11_8: 4;
    uint32_t table_addr_31_12   : 20;
}dir_entry_desc_t;

/*paaging table struct */
typedef struct __attribute__((packed)) table_entry_desc{
    uint32_t present            : 1;
    uint32_t read_write         : 1;
    uint32_t user_super         : 1;
    uint32_t write_through      : 1;
    uint32_t cache_disable      : 1;
    uint32_t accessed           : 1;
    uint32_t dirty              : 1;
    uint32_t pat                : 1;
    uint32_t global             : 1;
    uint32_t available_bits_11_9: 3;
    uint32_t page_addr_31_12    : 20;
}table_entry_desc_t;

/*arrays for paging */
dir_entry_desc_t pagingDirectory[KB] __attribute__((aligned (FOURKB)));
table_entry_desc_t pagingTable[KB] __attribute__((aligned (FOURKB)));
table_entry_desc_t pagingTableVideo[KB] __attribute__((aligned (FOURKB)));

extern void paging_init();

#endif 
#endif

