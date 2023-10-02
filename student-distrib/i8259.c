/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */


/* codes referenced from linux/i8259.c and O  */

/* void rtc_init(void);
 * Inputs: void
 * Return Value: none
 * Function: Initialize both PICs slave and master */
void i8259_init(void) {
	 /* mask interrupts */
	outb(master_mask, MASTER_DATA);
    outb(slave_mask, SLAVE_DATA);

    /* ICW1: select 8259A-1 init */
	/* ICW1: select 8259A-2 init */
	outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

	/* ICW2: 8259A-1 IR0 mapped to 0x20 + 1*/
	/* ICW2: 8259A-2 IR0 mapped to 0xA0 + 1 */
    outb(ICW2_MASTER, MASTER_DATA);
    outb(ICW2_SLAVE, SLAVE_DATA);

	/* ICW3: 8259A-1 (the master) has a slave on IR2 */
	/* ICW3: 8259A-2 is a slave on master's IR2 */
    outb(ICW3_MASTER, MASTER_DATA);
    outb(ICW3_SLAVE, SLAVE_DATA);

    /* ICW4: Enable 8086 Mode */
    outb(ICW4, MASTER_DATA);
    outb(ICW4, SLAVE_DATA);

    /* restore masks */
    outb(master_mask, MASTER_DATA);
	outb(slave_mask, SLAVE_DATA);
    enable_irq(0x02); // enable 2nd ir on master pic
 
}

/* void rtc_init(void);
 * Inputs: void
 * Return Value: none
 * Function:  Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    
	/* check if irq# is within [0,15], otherwise invalid*/
	if(irq_num < 0 || irq_num > 15){
		return;
	}

	/* if desired IRQ is on slave PIC, offset -8 and send to slave*/
	if(irq_num > 7){
		irq_num -= 8;
		slave_mask = inb(SLAVE_DATA) & ~(1 << irq_num);
		outb(slave_mask, SLAVE_DATA);
	/* otherwise send to master */
	} else { 
		master_mask = inb(MASTER_DATA) &  ~(1 << irq_num);
		outb(master_mask, MASTER_DATA);
	} 
   

}

/* void rtc_init(void);
 * Inputs: void
 * Return Value: none
 * Function:  Disable (mask) the specified IRQ */
 void disable_irq(uint32_t irq_num) {
    /* check if irq# is within [0,15], otherwise invalid*/
	if(irq_num < 0 || irq_num > 15){
		return;
	}

	/* if desired IRQ is on slave PIC, offset -8 and send to slave*/
	if(irq_num > 7){
		irq_num -= 8;
		slave_mask = inb(SLAVE_DATA) | (1 << irq_num);
		outb(slave_mask, SLAVE_DATA);
	/* otherwise send to master */
	} else { 
		master_mask = inb(MASTER_DATA) | (1 << irq_num);
		outb(master_mask, MASTER_DATA);
	} 
   
}

/* void rtc_init(void);
 * Inputs: void
 * Return Value: none
 * Function:  Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num < 0 || irq_num > 15) {
        return;
    }
	/* if IRQ is on slave PIC, then send EOI to both*/
    if(irq_num > 7){ 
		irq_num -= 8;
		outb(EOI | irq_num, SLAVE_8259_PORT);
		outb(EOI | 2, MASTER_8259_PORT); // slave is inputted on the 2nd ir pin of the master
    } else {
		outb(EOI | irq_num, MASTER_8259_PORT);
	}
     
}
