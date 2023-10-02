#include "pit.h"


volatile int counter = 0;
volatile int num_terminals_active = 0;
/*
 * pit_init
 *   DESCRIPTION: initializes PIT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes PIT
 */
void pit_init(void) {

    uint32_t freq = IRQ0_FREQUENCY; // 100 Hz
    uint32_t divisor = MAX_PIT_FREQ / freq; // calculate divisor
    outb(PIT_CMD , 0x36); // set command byte to channel 0, lobyte/hibyte, mode 3
    outb(PIT_PORT, divisor & 0xFF); // set low byte of divisor
    outb(PIT_PORT, divisor >> 8);  // set high byte of divisor

    enable_irq(PIT_IRQ_NUM); // enable IRQ 0 (PIT channel 0 is connected to IRQ 0)
    return;
}


/*
 * pit_handler
 *   DESCRIPTION: handles PIT interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handles PIT interrupts
 */
void pit_handler(void) {
    cli();
    send_eoi(PIT_IRQ_NUM); // send end of interrupt
    int target_terminal = (scheduled_tid + 1) % NUM_TERMINALS;
    schedule(target_terminal);    
    sti();

}
