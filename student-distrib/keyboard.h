#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "i8259.h"
#include "lib.h"
#include "syscalls.h"

#define NUM_TERMINALS 3
#define VIDEO 0xB8000
// VIDEO MEMORY IS A LIB.H definition at B8000

#define _4KB 0x1000
typedef struct {
    char line_buffer[128]; // 128 characters per line, buffer to hold the current line
    char tab_buffer[128]; // buffer that holds the same as the line buffer but tabs are \t instead of spaces
    unsigned int nchars; // number of characters in the current line

    int32_t active_task_pid; // the task that is currently running on this terminal
    int screen_x;
    int screen_y;
    uint32_t terminal_vmem;
    int enter_flag;
    int writeable;
} terminal_t;
// Terminal DRIVER
extern int32_t terminal_open(const uint8_t* filename);
extern int32_t terminal_read(int32_t fd, void * buf, int32_t nbytes);
extern int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes);
extern int32_t terminal_close(int32_t fd);
void terminals_init();
void terminal_new_line();
void terminal_backspace();
void terminal_putc(char c);
void terminal_reset_cursor();
void terminal_tab(void);
void switch_terminal(int32_t terminal_id);
void switch_vmem_paging(int target_tid);
terminal_t terminals[NUM_TERMINALS];
int32_t scheduled_tid; // schedulid tid
int32_t displayed_tid;


// Keyboard DRIVER

#define KEYBOARD_PORT           0x60
#define KEYBOARD_IRQ_NUM         1
void keyboard_init(void);
void keyboard_handler(void);
void set_max_proc_flag(int flag);

#define BACKSPACE 0x8
#define ENTER 0x10
#define TAB 0x2
#define CTRL 0x4
#define LSHIFT 0x5
#define RSHIFT 0x6
#define CAPSLOCK 0x7
#define F1 0x09
#define F2 0x0A
#define F3 0x0B

#endif
