#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "types.h"
#include "file_system.h"
#include "x86_desc.h"
#include "paging.h"
#include "keyboard.h"
#include "rtc.h"
#define MAX_FILES 8 // max number of files that can be opened at once
#define MAX_PROCESS 6 // max number of processes that can be run at once
#define FILE_HEADER_SIZE 40 // size of the file header in bytes
#define MAX_USER_DEFINED_FILE 2 //index to the start of user-opened files

#define FILE_MAGIC_NO_1 0x7f // these magic numbers are used to identify the file type
#define FILE_MAGIC_NO_2 0x45 // if the file is an executable, it will have these magic numbers
#define FILE_MAGIC_NO_3 0x4c
#define FILE_MAGIC_NO_4 0x46

#define USER_VMEM 0x08000000 // 128MB
#define USR_PROGRAM_PAGE_SIZE 0x400000 // 4MB
#define USR_PROGRAM_ADDR 0x08048000 // virtual address of the program
#define KERNEL_STACK_BASE 0x800000 // 8MB
#define KERNEL_STACK_SIZE 0x2000 // 8KB
#define VIDEO_DIRECTORYIDX 34   //index of vidmap page
#define VIDMAP_MEM 0x8800000    //virtual address for text-mode memory

/* operations jump table*/
typedef struct {
    int32_t (*read)(int fd , void * buf, int32_t nbytes);
    int32_t (*write)(int fd , const void * buf, int32_t nbytes);
    int32_t (*open)(const uint8_t * filename);
    int32_t (*close)(int fd);
} operations_table;

/* File descriptor struct */
typedef struct {
    operations_table * table_ptr;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags;
} fd_t;

typedef struct{
    fd_t fd_array[MAX_FILES];

    uint32_t curr_pid;
    uint32_t parent_pid;

    uint32_t parent_esp;
    uint32_t parent_ebp;

    uint32_t schedule_esp;
    uint32_t schedule_ebp;
    uint32_t schedule_esp0;
    
    uint32_t terminal_id; // terminal id of the process

    uint8_t arg[128]; // max filename
    
} pcb_t;

void switch_task(pcb_t * dest_pcb);

void execute_init_fdarray(pcb_t* pcb);
void execute_init_pcb(pcb_t* pcb);
pcb_t* get_pcb();
pcb_t* get_pcb_by_pid(uint32_t pid);
void flush_tlb();
void execute_context_switch(uint32_t pid);
void execute_paging();
uint32_t get_file_size(uint32_t inode_num);
void schedule();
int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

#endif
