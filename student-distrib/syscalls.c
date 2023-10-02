#include "syscalls.h"
operations_table terminal_stdin = {
    .open = terminal_open,
    .close = terminal_close,
    .read = terminal_read,
    .write = NULL
};
operations_table terminal_stdout = {
    .open = terminal_open,
    .close = terminal_close,
    .read = NULL,
    .write = terminal_write
};
operations_table fs_ops = {
  .open = f_open,
  .close = f_close,
  .read = f_read,
  .write = f_write
};
operations_table dir_ops = {
    .open = dir_open,
    .read = dir_read,
    .write = dir_write,
    .close = dir_close
};
operations_table rtc_ops = {
    .open = rtc_open,
    .close = rtc_close,
    .read = rtc_read,
    .write = rtc_write
};
uint32_t process_array[MAX_PROCESS]; // keeps track of the pid of each process
static uint32_t processes_running;
/* pcb_t* get_pcb()
 *   Inputs: none
 *   Return Value: pcb_t* - pointer to the pcb of the current process
 *   Function: returns the pcb of the current process 
*/
pcb_t* get_pcb() {
    return (pcb_t*) (KERNEL_STACK_BASE - ((processes_running) * KERNEL_STACK_SIZE));
}

/* pcb_t* get_pcb_by_pid(uint32_t pid)
 *   Inputs: uint32_t pid - the pid of the process to get the pcb of
 *   Return Value: pcb_t* - pointer to the pcb of the process with the given pid
 *   Function: returns the pcb of the process with the given pid 
*/
pcb_t* get_pcb_by_pid(uint32_t pid) {
    return (pcb_t*) (KERNEL_STACK_BASE - ((pid+1) * KERNEL_STACK_SIZE));
}

/* int32_t halt(uint8_t status)
 *   Inputs: uint8_t status - the status to return to the parent process
 *   Return Value: int32_t - 0 
 *   Function: halts the current process and returns to the parent process 
*/
int32_t halt(uint8_t status) {

    int i;
    pcb_t* cur_pcb = get_pcb_by_pid(terminals[scheduled_tid].active_task_pid);
    processes_running--;    
    process_array[cur_pcb->curr_pid] = 0;
    
    for (i = 0; i < MAX_FILES; i++) {
        cur_pcb -> fd_array[i].flags = 0;
        cur_pcb -> fd_array[i].file_position = 0;
        cur_pcb -> fd_array[i].inode = 0;
        cur_pcb -> fd_array[i].table_ptr = NULL;   
    }
    if(cur_pcb->parent_pid == -1 ) {
        // if the parent pid is -1, then we are at base shell
        // we need to spin up a new shell
        terminals[scheduled_tid].active_task_pid = -1;
        execute((uint8_t*)"shell");
        return 0;
    }
    else{
        tss.esp0 = KERNEL_STACK_BASE - (cur_pcb->parent_pid) * KERNEL_STACK_SIZE - 0x4; // set the esp0 to the parent's kernel stack 
        execute_paging(cur_pcb->parent_pid); // set the paging to the parent's page directory
        switch_vmem_paging(scheduled_tid);
        terminals[scheduled_tid].active_task_pid = cur_pcb->parent_pid;
        //printf("%d\n",cur_pcb->parent_pid);
        asm volatile(
            "mov %0, %%esp;"
            "mov %1, %%ebp;"
            "mov %2, %%eax;"
            "leave;"
            "ret;"
            :
            : "r"(cur_pcb->parent_esp), "r"(cur_pcb->parent_ebp), "r"((uint32_t)status)
            : "memory"
        );
    }
    return 0;

}
/* int32_t execute(const uint8_t* command)
 *   Inputs: const uint8_t* command - the command to execute
 *   Return Value: int32_t - 0 on success, -1 on failure
 *   Function: executes the command given by the user,
 *          parses the command and sets up the pcb and paging
 *          then loads the file into memory and jumps to the entry point, by using the iret instruction in execute_context_switch
 *      
*/
int32_t execute(const uint8_t* command) {
    cli();
    if(command == NULL) return -1;
    /* FILE NAME */
    // file name is the first word in the command, null terminated
    uint8_t file_command[FILE_NAME_SIZE]; 
    int k = 0;
    while(command[k] == ' ') k++; // skip over any spaces at the beginning of the command
    int i = 0;
    int start_of_args = 0;
    while (command[i+k] != ' ' && command[i+k] != '\0' && i-k < FILE_NAME_SIZE) {
        file_command[i] = command[i+k];
        i++;
        start_of_args++;
    }
    int j;
    for(j = i; j < FILE_NAME_SIZE; j++) {
        file_command[j] = '\0';
    }
    //printf("FILE NAME:\"%s\" : PASS\n", file_command);
    /* ARGUMENTS */
    uint8_t arguments[128]; // args are the rest of the command, null terminated
    for(i = 0; i < 128; i++){
            arguments[i] = 0;
    }
    //get arguments and skip initial space
    if(command[start_of_args] == ' '){
        start_of_args++; 
        i = 0;
        while(command[i + start_of_args] != '\n' && command[i + start_of_args] != '\0'){
                arguments[i] = command[i + start_of_args];
                i++;
        }
    }
    /* EXECUTABLE CHECK */
    dentry_t dentry;
    int32_t result = read_dentry_by_name(file_command, &dentry);
    //printf("FILE NODE (decimal): %d\n", dentry.inode_count);
    if (result == -1) return -1;
    
    uint8_t magic_num[4]; // 4 magic numbers
    result = read_data(dentry.inode_count, 0, magic_num, 4);
    if (result == -1) return -1;
    if (magic_num[0] != FILE_MAGIC_NO_1 || magic_num[1] != FILE_MAGIC_NO_2 || magic_num[2] != FILE_MAGIC_NO_3 || magic_num[3] != FILE_MAGIC_NO_4) {
        return -1;
    }
    if(dentry.file_type != 2) return -1; // if it is not an executable (2) 
    //printf("EXECUTABLE: PASS\n");
    /* PID */
    uint32_t pid = 0;
    while (process_array[pid] != 0 && pid < MAX_PROCESS) {
        pid++; 
    }
    if (pid == MAX_PROCESS) {
        set_max_proc_flag(1);
        return 0; // no more processes can be run
    }
    else{
        process_array[pid] = 1; // set the pid to 1 to indicate that it is in use
        processes_running++;
    }    
    //printf("SHELL PID: %d\n", pid);
    /* SETUP PAGING */
    execute_paging(pid);
    /* LOAD FILE INTO MEMORY */
    uint32_t file_size = get_file_size(dentry.inode_count);
    //printf("FILE SIZE: %x\n", file_size);
    uint8_t * program_addr = (uint8_t*) USR_PROGRAM_ADDR; // where the program will be loaded into memory
    // overwrites the current program in memory
    result = read_data(dentry.inode_count, 0, program_addr, file_size);
    if (result == -1) return -1;
    /* SET UP PCB */
    pcb_t * pcb = get_pcb(); // get the pcb for the process
    pcb->curr_pid = pid; // set the pid
    if(pid < 3) { // if it is a base shell, set the parent pid to -1 so they spin up if halted
        pcb->parent_pid = -1;
    }
    else {
        pcb->parent_pid = terminals[scheduled_tid].active_task_pid; // set the parent pid to the parent pid of the terminal
    }
    terminals[scheduled_tid].active_task_pid = pid; // set the active pid for the terminal
    pcb->terminal_id = scheduled_tid; // set the terminal id for the pcb
    execute_init_fdarray(pcb); // initialize the file descriptor array in the pcb
    strncpy((int8_t*)pcb->arg, (int8_t*)(arguments), 128); // add arguments to pcb
    // we need to save current esp and ebp into the pcb 
    asm volatile(
        "movl %%esp, %0;"
        "movl %%ebp, %1;"
        :"=r"(pcb->parent_esp), "=r"(pcb->parent_ebp)
    );
    /* CONTEXT SWITCH */
    execute_context_switch(pid); // switch to the new process
    return 0;

}
/* uint32_t get_file_size(uint32_t inode_num)
 *   Inputs: uint32_t inode_num - the inode number of the file
 *   Return Value: uint32_t - the size of the file
 *   Function: gets the size of the file 
*/
uint32_t get_file_size(uint32_t inode_num){
    inode_t node = inode_ptr[inode_num];
    return node.data_len;
}

/* void execute_paging(uint32_t pid)
 *   Inputs: uint32_t pid - the pid of the process
 *   Return Value: none
 *   Function: sets up the paging for the process
 * 
*/
void execute_paging(uint32_t pid) {
    // 22 shifts it so that we're left with a 10 bit index number
    uint32_t pagedir_index = (uint32_t) (USER_VMEM >> 22); // get page directory index


    //offset into user program page,
    // 0x000FFFFF is the mask for the offset
    // shift 12 to get the offset into the physical
    uint32_t paging_offset = (0x000FFFFF) & ((KERNEL_STACK_BASE + (pid) * USR_PROGRAM_PAGE_SIZE) >> 12);
    // set up page directory
    pagingDirectory[pagedir_index].present = 1;
    pagingDirectory[pagedir_index].read_write = 1;
    pagingDirectory[pagedir_index].user_super = 1; // user level
    pagingDirectory[pagedir_index].write_through = 0; 
    pagingDirectory[pagedir_index].cache_disable = 0; 
    pagingDirectory[pagedir_index].accessed = 0; 
    pagingDirectory[pagedir_index].available_bit_6 = 0;
    pagingDirectory[pagedir_index].page_size = 1; // 4MB page
    pagingDirectory[pagedir_index].available_bits_11_8 = 0;
    pagingDirectory[pagedir_index].table_addr_31_12 = paging_offset; 
    //printf("pagingDirectory physical address offset: %x\n", pagingDirectory[pagedir_index].table_addr_31_12);
    
    flush_tlb(); // flush the tlb
}
/* void execute_init_fdarray(pcb_t * pcb)
 *   Inputs: pcb_t * pcb - the pcb of the process
 *   Return Value: none
 *   Function: initializes the file descriptor array in the pcb
 *
*/
void execute_init_fdarray(pcb_t * pcb)  {
    //init fd array
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        pcb->fd_array[i].flags = 0;
        pcb->fd_array[i].table_ptr = NULL;
        pcb->fd_array[i].inode = 0;
        pcb->fd_array[i].file_position = 0;    
    }


    pcb->fd_array[0].flags = 1;
    pcb->fd_array[0].table_ptr = &terminal_stdin;
    pcb->fd_array[0].inode = 0;
    pcb->fd_array[0].file_position = 0; 

    pcb->fd_array[1].flags = 1;
    pcb->fd_array[1].table_ptr = &terminal_stdout;
    pcb->fd_array[1].inode = 0;
    pcb->fd_array[1].file_position = 0; 

}
/* void execute_context_switch(uint32_t pid)
 *   Inputs: uint32_t pid - the pid of the process
 *   Return Value: none
 *   Function: context switches to the new process through iret
 *  
*/
void execute_context_switch(uint32_t pid) {
    
    // set up stack for iret (top to bottom)
    // EIP, CS, EFLAGS, ESP, SS 
    // register ds has to point to user data segment
    
    // we need to set up the stack for the new process
    uint32_t esp = USER_VMEM + USR_PROGRAM_PAGE_SIZE - 0x4; // 4 is for fence
    tss.esp0 = KERNEL_STACK_BASE - KERNEL_STACK_SIZE * (pid) - 0x04; // set the kernel stack for the new process

    uint8_t* entry_point = (uint8_t*) (USR_PROGRAM_ADDR + 24); // 24 is the offset of the entry point in the file header
    uint32_t eip = * (uint32_t*) entry_point;
    

    uint32_t usercs = USER_CS; 
    uint32_t ss = USER_DS;
    // printf("CONTEXT SWITCH VALUES (eip pushed last):\n EIP:%x\n CS:%x\n ESP:%x\n SS:%x\n", eip, USER_CS, esp, USER_DS);
    sti();
        // push the values onto the stack/
    // $0x200 selects the interrupt flag
    asm volatile("\
        pushl %%eax         ;\
        pushl %%ebx         ;\
        pushfl              ;\
        popl  %%eax         ;\
        orl   $0x0200, %%eax;\
        pushl %%eax         ;\
        pushl %%ecx         ;\
        pushl %%edx         ;\
        iret                ;\
        "
        :
        : "a"(ss), "b"(esp), "c"(usercs), "d" (eip)
        : "memory"
    );
    //  movl %0, %%ebx   ;
    //     andl $0x00FF, %%ebx;
    //     movw %%bx, %%ds  ;
    return;
}

/* 
 * read
 * Input: fd, buf, nbytes
 * Output: return 0 = success, return 1 = fail
 * 
 * reads nbytes of data from read() function in a file_descriptor to a buffer
 * 
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    pcb_t * pcb = get_pcb();                        //grab the pcb
    fd_t * cur_fd = pcb->fd_array;                  //grab the current fd_array

    //check if inputs are valid
    if(fd < 0 || nbytes < 0 || fd > 7 || buf == NULL || fd == 1){
        return -1;
    }

    //check if entry exists
    if(!cur_fd[fd].flags){
        return -1;
    }
    //read the desired bytes to the buffer and return # of bytes read
    return (cur_fd[fd].table_ptr)->read(fd, buf, nbytes);
}
/* 
 * write
 * Input: fd, buf, nbytes
 * Output: return 0 = success, return 1 = fail
 * 
 * writes nbytes of data from a buffer to a write() function in a file_descriptor
 * 
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    pcb_t * pcb = get_pcb();   //grab the pcb
    fd_t * cur_fd = pcb->fd_array;                  //grab the current fd_array

    //check if inputs are valid
    if(fd < 0 || nbytes < 0 || fd > MAX_FILES || buf == NULL || fd == 0){
        return -1;
    }

    //check if entry exists
    if(!cur_fd[fd].flags){
        return -1;
    }

    //read the desired bytes to the buffer and return # of bytes read
    return (cur_fd[fd].table_ptr)->write(fd, buf, nbytes);
    return -1;
}
/* 
 * open
 * Input: filename
 * Output: return 0 = success, return 1 = fail
 * 
 * opens desired file, loads file-descriptor with file-specific data and functions
 * 
 */
int32_t open(const uint8_t* filename) {
    pcb_t * pcb = get_pcb();                        //grab the pcb
    dentry_t dentry;                              //dentry for file data
    int i;                                          //iterator
    fd_t * fd = pcb->fd_array;                      //grab the current fd_array

    // check if filename is valid
    if(strlen((const int8_t*) filename) == NULL){
        return -1;
    }
    
    //find the file (if it exists)
    if(-1 == read_dentry_by_name(filename, &dentry)){
        return -1;
    }

    //find a vacant file descriptor index
    for(i = 0; i < MAX_FILES; i++){

        //set members of the fd_t struct
        if(fd[i].flags == 0){
            
            /*  if we opened the RTC (filetype 0) then give user access to the
            *   RTC's file operations table */
            if(dentry.file_type == 0){
                fd[i].table_ptr = &rtc_ops; 
                fd[i].inode = 0;
            }

            //if we are opening the directory (filetype 1), give it the dir operations table 
            else if(dentry.file_type == 1){

                fd[i].table_ptr = &dir_ops;
                fd[i].inode = 0;
            }

            //in the case of a normal file, set the inode and give it the file operations table
            else if(dentry.file_type == 2){
                fd[i].table_ptr = &fs_ops;
                fd[i].inode = dentry.inode_count;
            }

            //set fd flag to "in use" (true)
            fd[i].flags = 1;

            //set file position to beginning
            fd[i].file_position = 0;

            //success, return index of file descriptor;
            return i;
        }

    }

    //fd array was full, failure
    return -1;


}

/* 
 * close
 * Input: fd
 * Output: return 0 = success, return 1 = fail
 * 
 * closes desired file-descriptor index
 * 
 */
int32_t close(int32_t fd) {

    pcb_t * pcb = get_pcb();   //grab the pcb                                     //iterator
    fd_t * cur_fd = pcb->fd_array;                  //grab the current fd_array
    
    //check if desired file descriptor is off-limits or out of range
    if(fd < MAX_USER_DEFINED_FILE || fd > MAX_FILES){
        return -1;
    }

    //free the file descsriptor
    if(cur_fd[fd].flags){
        cur_fd[fd].flags = 0;
        return 0;
     }

    //file wasn't open? failure
    return -1;

}
/* 
 * getargs
 * Inputs: buf - buffer to which argument will be written 
 *         nbytes - number of bytes to write into
 * Output: 0 if success, -1 if failure */
int32_t getargs(uint8_t* buf, int32_t nbytes) {
    
    if (buf == NULL || nbytes <= 0) return -1;
    pcb_t * pcb = get_pcb();                                // grab the pcb

    if (pcb->arg[0] == '\0') return -1;                     //check if no arguments

    strncpy((int8_t*)buf, (int8_t*)(pcb->arg), nbytes);     //copy arguments to user space

    return 0;

}

/* 
 * vidmap
 * Inputs: screen_start - double pointer to start of text-mode video memory
 * Side-effects - establishes a vidmap page for a user-space program to use,
 * sets the screen start to use that memory
 * Output: 0 if success, -1 if failure 
 */
int32_t vidmap(uint8_t** screen_start) {
    //check inputs
    if (screen_start == NULL)
        return -1;
    if ( (uint32_t)screen_start < USER_VMEM || (uint32_t)screen_start > (USER_VMEM + USR_PROGRAM_PAGE_SIZE)) 
        return -1;
    //set up paging directory[2] for the vidmap page
    pagingDirectory[VIDEO_DIRECTORYIDX].present              = 1;
    pagingDirectory[VIDEO_DIRECTORYIDX].read_write           = 1;
    pagingDirectory[VIDEO_DIRECTORYIDX].user_super           = 1;
    pagingDirectory[VIDEO_DIRECTORYIDX].write_through        = 0;
    pagingDirectory[VIDEO_DIRECTORYIDX].cache_disable        = 0;
    pagingDirectory[VIDEO_DIRECTORYIDX].accessed             = 0;
    pagingDirectory[VIDEO_DIRECTORYIDX].available_bit_6      = 0;
    pagingDirectory[VIDEO_DIRECTORYIDX].page_size            = 0; //4 KB
    pagingDirectory[VIDEO_DIRECTORYIDX].available_bits_11_8  = 0;
    pagingDirectory[VIDEO_DIRECTORYIDX].table_addr_31_12 = ((int) pagingTableVideo) >> 12; // 4KB aligned
    switch_vmem_paging(scheduled_tid);
    flush_tlb();
    //set the screen start
    *screen_start = (uint8_t*)(VIDMAP_MEM);
    return 0;
    
}


/*  set_handler
  * Input: None
  * Output: 0 on success, -1 if fails
  */
int32_t set_handler(int32_t signum, void* handler_address) {
    return -1;

}

/*  sigreturn
  * Input: None
  * Output: 0 on success, -1 if fails
  */
int32_t sigreturn(void) {
    return -1;

}

/*  schedule
*   Input: None
*   Output: None
*/
void schedule(int target_terminal) {
    //save the current esp and ebp
    int active_pid = terminals[scheduled_tid].active_task_pid;
    if(active_pid < 0) {
        execute((uint8_t*)"shell");
        return; 
    }
    pcb_t * curr_pcb = get_pcb_by_pid(active_pid);
    asm volatile(
            "movl %%esp, %0 ;"
            "movl %%ebp, %1 ;"
            : "=r" (curr_pcb->schedule_esp) ,"=r" (curr_pcb->schedule_ebp) 
    );
    curr_pcb->schedule_esp0 = tss.esp0;

    scheduled_tid = target_terminal;
    active_pid = terminals[scheduled_tid].active_task_pid;
    if(active_pid < 0) { // if there is no active task
        //target_terminal = (target_terminal + 1) % NUM_TERMINALS;
        //schedule();
        execute((uint8_t*)"shell");
    }
    pcb_t * active_pcb = get_pcb_by_pid(active_pid);
    tss.esp0 = active_pcb->schedule_esp0;
    switch_vmem_paging(scheduled_tid);
    execute_paging(active_pid);
    //set the esp and ebp to the next task's esp and ebp
    asm volatile("         \n\
            movl %0, %%esp \n\
            movl %1, %%ebp \n\
            "
            : : "r" (active_pcb->schedule_esp) ,"r" (active_pcb->schedule_ebp) 
            : "memory"
    );
}
/* void flush_tlb()
 * Inputs: None
 * Outputs: None
 * Side Effects: flushes the tlb
 */
void flush_tlb() {
    asm volatile ("          \n\
        movl %%cr3, %%eax    \n\
        movl %%eax, %%cr3    \n\
        "
        : : : "eax", "cc"
    );
}
