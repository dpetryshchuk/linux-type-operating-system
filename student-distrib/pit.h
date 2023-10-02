#ifndef PIT_H_
#define PIT_H_

#include "types.h"
#include "idt.h"
#include "lib.h"
#include "i8259.h"
#include "syscalls.h"

#define PIT_PORT 0x40
#define PIT_CMD 0x43
#define IRQ0_FREQUENCY 100
#define MAX_PIT_FREQ 1193182
#define PIT_IRQ_NUM 0

void pit_init(void);
extern void pit_handler(void);


#endif 
