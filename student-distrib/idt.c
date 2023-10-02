#include "idt.h"

char* exception_names[20] = {
    "Division Error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception"
}; 

/* macro to create all of the exception handlers*/
#define EXCEPTION_HANDLER_CREATE(name, id) \
void name(void) {                          \
    exception_handler(id);                 \
}

EXCEPTION_HANDLER_CREATE         (division_error,               0x00);
EXCEPTION_HANDLER_CREATE         (debug,                        0x01);
EXCEPTION_HANDLER_CREATE         (non_maskable_interrupt,       0x02);
EXCEPTION_HANDLER_CREATE         (breakpoint,                   0x03);
EXCEPTION_HANDLER_CREATE         (overflow,                     0x04);
EXCEPTION_HANDLER_CREATE         (bound_exceeded,               0x05);
EXCEPTION_HANDLER_CREATE         (invalid_opcode,               0x06);
EXCEPTION_HANDLER_CREATE         (device_unavailable,           0x07);
EXCEPTION_HANDLER_CREATE         (double_fault,                 0x08);
EXCEPTION_HANDLER_CREATE         (coprocessor_segment_overrun,  0x09);
EXCEPTION_HANDLER_CREATE         (invalid_tss,                  0x0A);
EXCEPTION_HANDLER_CREATE         (segment_not_present,          0x0B);
EXCEPTION_HANDLER_CREATE         (stack_segment_fault,          0x0C);
EXCEPTION_HANDLER_CREATE         (general_protection_fault,     0x0D);
EXCEPTION_HANDLER_CREATE         (page_fault,                   0x0E);
EXCEPTION_HANDLER_CREATE         (x87_floating_point,           0x10);
EXCEPTION_HANDLER_CREATE         (alignment_check,              0x11);
EXCEPTION_HANDLER_CREATE         (machine_check,                0x12);
EXCEPTION_HANDLER_CREATE         (SIMB_floating_point,          0x13);

/* void exception_handler(uint32_t id)
 * Inputs: uint32_t id
 * Return Value: void
 * Function: prints out the exception when needed */
void exception_handler(uint32_t id) {
    clear();       
    printf("Exception: %s\n", exception_names[id]);
    
    halt(255);
    
}


/* void default_interrupt(void)
 * Inputs: void
 * Return Value: void
 * Function: prints out the default interrupt when needed
*/
void default_interrupt(void) {
    cli();
    printf("Default Interrupt\n");
    sti();

}
/* void syscall_handler()
 * Inputs: void
 * Return Value: void
 * Function: prints out the system call when needed */
void syscall_handler(){
    cli();   
    printf("SYSTEM CALL !!! \n");
    while(1);
    sti();
}
/* void initialize_idt()
 * Inputs: void
 * Return Value: void
 * Function: populates idt with the correct vectors corresponding to our interrupts and intel's exceptions */
void initialize_idt() {
    
    lidt(idt_desc_ptr);
    unsigned int i;
    for(i = 0; i < NUM_VEC; i++) {
        idt[i].reserved4 = 0x0; // constants defined this way in idt descriptor for an interrupt gate
        
        idt[i].reserved3 = 0x1; // first 32 are exceptions 01110
        idt[i].reserved2 = 0x1;   
        idt[i].reserved1 = 0x1;   
        idt[i].size = 0x1; // sets size at 32 bits    
        idt[i].reserved0 = 0x0;

        idt[i].dpl = 0x0; // sets priviledge level of interrupt
        idt[i].present = 0x1; // set to 1 by SET_IDT_ENTRY()

        idt[i].seg_selector = KERNEL_CS; 
        if(i >= 32) {
            idt[i].reserved3 = 0;
            SET_IDT_ENTRY(idt[i], default_interrupt);
        }
        
        
    }
    SET_IDT_ENTRY(idt[0x0], division_error);
    SET_IDT_ENTRY(idt[0x1], debug);
    SET_IDT_ENTRY(idt[0x2], non_maskable_interrupt);
    SET_IDT_ENTRY(idt[0x3], breakpoint);
    SET_IDT_ENTRY(idt[0x4], overflow);
    SET_IDT_ENTRY(idt[0x5], bound_exceeded);
    SET_IDT_ENTRY(idt[0x6], invalid_opcode);
    SET_IDT_ENTRY(idt[0x7], device_unavailable);
    SET_IDT_ENTRY(idt[0x8], double_fault);
    SET_IDT_ENTRY(idt[0x9], coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[0x0A], invalid_tss);
    SET_IDT_ENTRY(idt[0x0B], segment_not_present);
    SET_IDT_ENTRY(idt[0x0C], stack_segment_fault);
    SET_IDT_ENTRY(idt[0x0D], general_protection_fault);
    SET_IDT_ENTRY(idt[0x0E], page_fault); 
    // idt[15] is reserved by INTEL
    SET_IDT_ENTRY(idt[0x10], x87_floating_point);
    SET_IDT_ENTRY(idt[0x11], alignment_check);
    SET_IDT_ENTRY(idt[0x12], machine_check);
    SET_IDT_ENTRY(idt[0x13], SIMB_floating_point);
    //idt[20] - idt[31] reserved by INTEL

    SET_IDT_ENTRY(idt[0x80], syscall_link); // 0x80 is system call idt entry
    idt[0x80].dpl = 0x3;

    SET_IDT_ENTRY(idt[0x28], rtc_link); // rtc
    idt[0x28].reserved3 = 1;
    
    SET_IDT_ENTRY(idt[0x21], keyboard_link); // keyboard
    idt[0x28].reserved3 = 1;

    SET_IDT_ENTRY(idt[0x20], pit_link); // pit
    
    idt[0x21].present = 1;
    idt[0x28].present = 1;
    
}


