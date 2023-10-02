#define ASM     1

# Interrupt linkage creator function
#define INTERRUPT_LINK(name, handler) \
    .globl name                      ;\
    name:                             ;\
        pushal                        ;\
        pushfl                        ;\
        call handler                  ;\
        popfl                         ;\
        popal                         ;\
        iret

INTERRUPT_LINK(keyboard_link, keyboard_handler);
INTERRUPT_LINK(rtc_link, rtc_handler);
INTERRUPT_LINK(pit_link, pit_handler);
