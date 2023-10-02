#include "keyboard.h"



char scan_codes[128][2] = { 
    {0,0},        
    {0,0},
    {'1','!'},
    {'2','@'},
    {'3','#'},
    {'4','$'},
    {'5','%'},
    {'6','^'},
    {'7','&'},
    {'8','*'},
    {'9','('},
    {'0',')'},
    {'-','_'},
    {'=','+'},
    {0x8,0x8},        //backspace '\b' is = 0x8
    {2,2},            //tab
    {'q','Q'},
    {'w','W'},
    {'e','E'},
    {'r','R'},
    {'t','T'},
    {'y','Y'},
    {'u','U'},
    {'i','I'},
    {'o','O'},
    {'p','P'},
    {'[','{'},
    {']','}'},
    {0x10,0x10}, // enter '\n' is = 0x10
    {4,4}, //control        
    {'a','A'},
    {'s','S'},
    {'d','D'},
    {'f','F'},
    {'g','G'},
    {'h','H'},
    {'j','J'},
    {'k','K'},
    {'l','L'},
    {';',':'},
    {'\'', '\"'},          
    {'`','~'},
    {5,5}, // left shift
    {'\\','|'},
    {'z','Z'},
    {'x','X'},
    {'c','C'},
    {'v','V'},
    {'b','B'},
    {'n','N'},
    {'m','M'},
    {',','<'},
    {'.','>'},
    {'/','?'},
    {6,6}, // right shift        
    {0,0},
    {0,0},
    {' ',' '},
    {7,7}, // caps lock 
    {9,9}, // F1
    {10,10}, // F2
    {11,11} // F3

    //everything else is unused
}; 

int shift_flag, ctrl_flag, capslock_flag, capslock, alt_flag; 
int max_proc_flag = 0; // flag to check if we are at max processes

/* void keyboard_init(void)
 * Input: void
 * Output: N/A
 * High IRQ (1) to initialize keyboard in PIC
*/
void keyboard_init(void) {
    shift_flag = 0; // init all flags to be 0
    ctrl_flag = 0;
    capslock_flag = 0;
    capslock = 0;
    alt_flag = 0;
    enable_irq(KEYBOARD_IRQ_NUM);
}

/* void keyboard_handler(void)
 * Input: void
 * Output: N/A
 * Adds input to video memory
*/
void keyboard_handler(void) {
    cli();
    send_eoi(KEYBOARD_IRQ_NUM); // send end of interrupt
    if(!terminals[displayed_tid].writeable) { // if we are at the end of the screen
        sti();
        return;
    }

    uint8_t scancode = inb(KEYBOARD_PORT); // scancode is inputted to KEYBOARD_PORT
    char * key_pressed = scan_codes[scancode];    
    int modifier = 1; // flag to indicate if it's a modifier key
    switch(scancode) {
        case 0x3A: // capslock
            capslock_flag = 1;
            capslock = 1 - capslock; // swap value in capslock
            break;
        case 0xBA: // capslock release
            capslock_flag = 0;
            break;
        case 0x2A: // left and right shift presses
        case 0x36:
            shift_flag = 1;
            break;
        case 0xAA: // left and right shift releases
        case 0xB6:
            shift_flag = 0;
            break;
        case 0x1D: // left ctrl press
            ctrl_flag = 1;
            break;
        case 0x9D: // left ctrl release
            ctrl_flag = 0;
            break;
        case 0x38: // left alt press 
            alt_flag = 1;
            break;
        case 0xB8: // left alt release   
            alt_flag = 0;
            break;
        default:
            modifier = 0;
    }
    if(modifier == 1) { // if it was a modifier key, end handler
        sti();
        return;
    }   
    if((terminals[displayed_tid].nchars >= 0x80 - 1) && key_pressed[0] != ENTER && key_pressed[0] != BACKSPACE && !(ctrl_flag && key_pressed[0] == 'l') && !alt_flag) { // if we are at the end of the screen
        sti();
        return;
    }   

    if(scancode < 64) { // ignore released character inputs
        
        switch_vmem_paging(displayed_tid); // switch to video memory of displayed terminal
        if(key_pressed[0] == ENTER) { // if enter is pressed
            terminal_new_line();   
        }
        else if (key_pressed[0] == BACKSPACE) { // if backspace is pressed
            terminal_backspace();
        }
        else if(key_pressed[0] == TAB) { // if tab is pressed
            terminal_tab();
        }
        else if(ctrl_flag && key_pressed[0] == 'l') { // if CTRL-L pressed
            clear(); // clear screen
            reset_cursor();
            puts("391OS> ");
        }
        // else if(ctrl_flag && key_pressed[0] == 'c') {
        //     printf("AHHH ctrl-c recieved\n");
        // }
        else if(alt_flag){
            if(key_pressed[0] == F1){
                switch_terminal(0);
            }
            else if(key_pressed[0] == F2){
                switch_terminal(1);
            }
            else if(key_pressed[0] == F3){
                switch_terminal(2);
            }
        }
        else{
            if (key_pressed[0] >= 'a' && key_pressed[0] <= 'z'){ // if it is a letter
                if (capslock != shift_flag){ // if capslock xor shift is pressed
                    terminal_putc(key_pressed[1]);
                }
                else{
                    terminal_putc(key_pressed[0]);
                }
            } 
            else{ // if it is a number  
                if(shift_flag){ // we only care if shift is pressed
                    terminal_putc(key_pressed[1]);
                }
                else{
                    terminal_putc(key_pressed[0]);        
                }
            }
        }   
        update_cursor();
        switch_vmem_paging(scheduled_tid); // switch back to current terminal
    }


    sti();
    return;
}

/*
| --------------------------------
| TERMINAL FUNCTIONS START HERE
| --------------------------------
*/




/* void terminal_putc(uint8_t c)
 * Input: const uint8_t* filename
 * Output: N/A
 * Description: 
*/
int32_t terminal_open(const uint8_t* filename) {
    clear();
    reset_cursor();
    return 0;
}

/* int32_t terminal_close(int32_t fd);
 * Inputs: int32_t fd
 * Return Value: returns 0 if file descriptor closed, closing an invalid file descriptor returns -1
 * Function: Closes the specified file descriptor and makes it available for return from later calls to open. 
 *              Does not allow the user to close the defualt descriptors (0 for inptu and 1 for output)
 * */
int32_t terminal_close(int32_t fd) {
    return 0;
}
/* int32_t terminal_read(int32_t fd, void * buf, int32_t nbytes);
 * Inputs: int32_t fd = file descriptor, void * buf = buffer, int32_t nbytes = number of bytes
 * Return Value: number of bytes read, -1 on fail
 * Function: reads data from one line that has been terminated by pressing enter or 
 *              as much as fits in the buffer from one such line
 *              line returned should include the line feed character */
int32_t terminal_read(int32_t fd, void * buf, int32_t nbytes) {
    if(buf == NULL) return -1; // invalid buffer
    if(nbytes == 0) return 0; // invalid number of bytes but still return 0
    if(nbytes < 0 ) return -1; // invalid number of bytes
    int nbytes_local = nbytes;
    int i = 0;
    terminals[scheduled_tid].enter_flag = 0;

    sti();
    while(!terminals[scheduled_tid].enter_flag) {} // wait until enter is pressed
    cli(); // once enter is pressed, disable interrupts
    terminals[scheduled_tid].enter_flag = 0;
    for(i = 0; i < nbytes && i < 128; i++) {
        ((char*)buf)[i] = terminals[scheduled_tid].line_buffer[i]; // copy the line buffer to the buffer
        terminals[scheduled_tid].line_buffer[i] = ' ';
        terminals[scheduled_tid].tab_buffer[i] = ' ';

        if(((char*)buf)[i] == '\n') { // if we have reached the end of the line
            nbytes_local = i + 1; // set the number of bytes to the number of bytes read
            break;
        }
    }
    terminals[scheduled_tid].nchars = 0;

    sti(); 
    return nbytes_local;
}

/* int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes);
 * Inputs: int32_t fd = file descriptor, const void * buf = buffer, int32_t nbytes = number of bytes
 * Return Value: number of bytes written, -1 on failure
 * Function: writes data to the terminal, all data is displayed to the screen immediately*/
int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes) {
    if(buf == NULL) return -1; // invalid buffer

    int i;
    cli();
    switch_vmem_paging(scheduled_tid);
    if(max_proc_flag) {
        puts("MAX PROCESSES REACHED\n");
        max_proc_flag = 0; // reset the flag
    }
    for(i = 0; i < nbytes; i++) { // write the buffer to the screen
        putc(((char*)buf)[i]);
    }
    if(scheduled_tid == displayed_tid) update_cursor();
    get_screen_x_y(&terminals[scheduled_tid].screen_x, &terminals[scheduled_tid].screen_y);
    switch_vmem_paging(displayed_tid);
    sti();
    return nbytes;
}

/* void terminals_init(); 
 * Inputs: none
 * Return Value: none
 * Function: initializes the three terminals
*/
void terminals_init(){
    // initializes our 3 terminals
    int i, j;
    // clear the line buffer
    scheduled_tid = 0;
    displayed_tid = 0;
    uint32_t global_vidmem = VIDEO;

    for(i = 0; i < NUM_TERMINALS; i++) {
        // all terminals load in with a shell
        terminals[i].active_task_pid = -1; // set the active task pid to 0
        for(j = 0; j < 128; j++) {
            terminals[i].line_buffer[i] = '\0';
            terminals[i].tab_buffer[i] = '\0';
        }
        terminals[i].nchars = 0;
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;
        terminals[i].terminal_vmem = (global_vidmem + ((i+1) * _4KB));
        terminals[i].active_task_pid = -1;  
        terminals[i].enter_flag = 0; 
        terminals[i].writeable = 1;
    }
}
/* void terminal_new_line();
 * Inputs: none
 * Return Value: none
 * Function: resets the number of characters and adds a new line to the line buffer and updates the cursor position
*/
void terminal_new_line(){
    uint32_t nchars = terminals[displayed_tid].nchars;
    if(terminals[displayed_tid].nchars >= 0x80) {// if the number of chars is greater than 128
        terminals[displayed_tid].line_buffer[0x80-1] = '\n'; // add a new line
        terminals[displayed_tid].tab_buffer[0x80-1] = '\n';
    }
    else{
        terminals[displayed_tid].line_buffer[nchars] = '\n'; // add a new line
        terminals[displayed_tid].tab_buffer[nchars] = '\n'; // add a new line
    }
    terminals[displayed_tid].enter_flag = 1; // set enter flag to 1    
    terminals[displayed_tid].nchars = 0;
    putc('\n');
    get_screen_x_y(&terminals[displayed_tid].screen_x, &terminals[displayed_tid].screen_y); // update the cursor position
    update_cursor();
}

/* void terminal_backspace();
 * Inputs: none
 * Return Value: none
 * Function: decrements the number of characters and removes a char from the line and updates the cursor position
*/
void terminal_backspace(){
    int i;
    uint32_t nchars = terminals[displayed_tid].nchars;
    //printf("nchars: %d, active term: %u", nchars, displayed_tid);
    if(nchars > 0){
        //printf("nchars: %d", nchars);
        if(terminals[displayed_tid].tab_buffer[nchars-1] == '\t') {
            if(nchars >= 4) terminals[displayed_tid].nchars -= 4; // if there are 4 or more chars on the line, decrement by 4
            else terminals[displayed_tid].nchars = 0; // else set the number of chars to 0
            for(i = 0; i < 4; i++) { // print 4 backspaces
                putc('\b'); 
            }
        }
        else{    
            terminals[displayed_tid].nchars--; 
            putc('\b'); // print a backspace
        }
    }
    get_screen_x_y(&terminals[displayed_tid].screen_x, &terminals[displayed_tid].screen_y); // update the cursor positio
    update_cursor(); 
}
/* void terminal_putc(char c);
 * Inputs: char c = character to be printed
 * Return Value: none
 * Function: prints a character to the screen and adds it to the line buffer 
*/
void terminal_putc(char c){
    uint32_t nchars = terminals[displayed_tid].nchars;
    if(nchars == 0x80 - 1) { // if number of chars on the line is one less than the max
        terminals[displayed_tid].line_buffer[nchars] = '\n'; // set last char to new line character
        terminals[displayed_tid].tab_buffer[nchars] = '\n'; // set last char to new line character

    }
    else if(nchars < 0x80 - 1) { // if the number of chars is greater than 128
        terminals[displayed_tid].line_buffer[nchars] = c;
        terminals[displayed_tid].tab_buffer[nchars] = c; // set last char to new line character

        terminals[displayed_tid].nchars++;
    }
    else{ // if the number of chars is greater than 128 buffer is full so just return  
        return;
    }
    putc(c);
    get_screen_x_y(&terminals[displayed_tid].screen_x, &terminals[displayed_tid].screen_y); // update the cursor position
    update_cursor();
    return;
}
/* void terminal_tab();
 * Inputs: none
 * Return Value: none
 * Function: increments the number of characters and adds a tab to the line and updates the cursor position 
*/
void terminal_tab(void){
    int i;
    uint32_t nchars = terminals[displayed_tid].nchars; // temp variable for number of chars
    for(i = 0; i < 4; i++) {
        if(nchars == 0x80 - 1) { // if number of chars on the line is one less than the max
            terminals[displayed_tid].line_buffer[nchars] = '\n'; // set last char to new line character
            terminals[displayed_tid].tab_buffer[nchars] = '\n'; // set last char to new line character

        }
        else if(nchars < 0x80 - 1) { // if the number of chars is greater than 128
            terminals[displayed_tid].line_buffer[nchars] = ' ';
            terminals[displayed_tid].tab_buffer[nchars] = '\t';
            terminals[displayed_tid].nchars++;
            nchars++;
        }
        else{ // if the number of chars is greater than 128 buffer is full so just return  
            break;
        }

    }
    putc('\t');
    get_screen_x_y(&terminals[displayed_tid].screen_x, &terminals[displayed_tid].screen_y); // update the cursor position
    update_cursor();
}

/* void switch_terminal(int next_terminal_id);
 * Inputs: next_terminal_id - the terminal id to switch to
 * Return Value: none
 * Function: switches the active terminal to the inputted terminal id, updates the cursor position, and updates the video memory
*/
void switch_terminal(int next_terminal_id) {
    if(next_terminal_id == displayed_tid) return; // already on this terminal

    cli();
    get_screen_x_y(&terminals[displayed_tid].screen_x, &terminals[displayed_tid].screen_y);

    // switch_vmem_paging(next_terminal_id);
    set_screen_x_y(terminals[next_terminal_id].screen_x, terminals[next_terminal_id].screen_y);

    memcpy((void*)terminals[displayed_tid].terminal_vmem, (void*)VIDEO, _4KB);
    memcpy((void*)VIDEO, (void*)terminals[next_terminal_id].terminal_vmem, _4KB);

    // update the cursor position
    update_cursor();
    
    // switch the active terminal
    displayed_tid = next_terminal_id;
    //printf("NOW ON TERMINAL %d\n", displayed_tid);
    sti();
    return;
}

/* void switch_vmem_paging(int target_tid);
 * Inputs: target_tid - the terminal id of the terminal to switch to
 * Return Value: none
 * Function: switches the video memory paging to the target terminal and flushes tlb
*/
void switch_vmem_paging(int target_tid) {
    set_screen_x_y(terminals[target_tid].screen_x, terminals[target_tid].screen_y);
    uint32_t global_vidmem_idx = VIDEO>>12; // get the index of the video memory in the page table
    if(displayed_tid == target_tid) { // if the display terminal is the same as the task terminal
        pagingTable[global_vidmem_idx].page_addr_31_12 = global_vidmem_idx;
        pagingTableVideo[0].page_addr_31_12 = global_vidmem_idx; // page table is stored at index 0 of the video memory page table
        pagingTableVideo[0].present = 1;
        pagingTableVideo[0].user_super = 1;
      

    } else { // if active terminal is not the same as task terminal
        pagingTable[global_vidmem_idx].page_addr_31_12 = (terminals[target_tid].terminal_vmem)>>12;
        pagingTableVideo[0].page_addr_31_12 = (terminals[target_tid].terminal_vmem)>>12;
        pagingTableVideo[0].present = 1;
        pagingTableVideo[0].user_super = 1;
       
    }
    pagingTableVideo[global_vidmem_idx].present = 1;
    flush_tlb();
    return;
}

void set_max_proc_flag(int flag) {
    max_proc_flag = flag;
}
