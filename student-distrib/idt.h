#ifndef _IDT_H_
#define _IDT_H_

#include "x86_desc.h"
#include "interrupt_linkage.h"
#include "syscall_linkage.h"
#include "rtc.h"
#include "i8259.h"
#include "keyboard.h"
#include "lib.h"


void initialize_idt();
void exception_handler(uint32_t id);
void syscall_handler();

#endif
