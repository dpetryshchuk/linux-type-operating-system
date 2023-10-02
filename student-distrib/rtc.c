#include "rtc.h"
#include "i8259.h"

volatile uint32_t rtc_intialized = 0;
volatile uint32_t rtc_interrupt_enabled = 1;

/* rtc_init
 * Inputs: void
 * Return Value: none
 * Function: initializes rtc */
void rtc_init(void) {

    outb(REGISTER_A, RTC_PORT);
    
    outb(REGISTER_B, RTC_PORT);           // select register B, and enable NMI
    char prev = inb(CMOS);                // read the current value of register B        
    outb(REGISTER_B,RTC_PORT);            // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, CMOS);              // write the previous value ORed with 0x40. This turns on bit 6 of register B
    enable_irq(IRQ_NUM);    

    rtc_intialized = 1;
    rtc_interrupt_enabled = 1;           // enable interrupts
}

/* rtc_handler
 * Inputs: void
 * Return Value: none
 * Function: read from register C to handle interrupts  */
void rtc_handler(void) {
    cli();
    //test_interrupts();

    send_eoi(IRQ_NUM);
    outb(REGISTER_C,RTC_PORT);      // select register C
    inb(CMOS);                      // just throw away contents  

    rtc_interrupt_enabled = 0;     // disable interrupts

    sti();
    
}

/* rtc_open
 * Inputs:  filename - rtc device
 * Return Value: 0 on success, -1 on failure
 * Function: Open and/or initialize rtc device  */
int32_t rtc_open(const uint8_t* filename) {

    if (!rtc_intialized){                       // initialize rtc if not initialized
        rtc_init();
    }

    outb(REGISTER_A, RTC_PORT);                 // select register A
    char two_hz = 0x0F;                         // set default frequency to 2Hz
    two_hz = two_hz | (0xF0 & inb(CMOS)); 
    outb(two_hz, CMOS);

    return 0;
}

/* rtc_close
 * Inputs:  fd      - File descriptor number
 * Return Value: 0 on success, -1 on failure
 * Function: Close rtc device  */
int32_t rtc_close(int32_t fd) {
    return 0;                                   // close rtc by returning 0
}


/* rtc_write
 * Inputs:  fd      - File descriptor number
 *          buf     - Input data pointer
 *          nbytes  - Number of bytes
 * Return Value: 0 on success, -1 on failure
 * Function: Write rtc interrupt rate  */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    
    uint32_t rate;

    // return -1 if wrong input
    if(nbytes != 4 || (uint32_t)buf == NULL){
        return -1;
    }

    else {
        
        // determine rate based on frequency
        uint32_t frequency = *(uint32_t*)(buf);

        switch (frequency){
            case 2:     
                rate = 0x0F;
                break;
            case 4:     
                rate = 0x0E;
                break;
            case 8:     
                rate = 0x0D;
                break;
            case 16:  
                rate = 0x0C;
                break;
            case 32:    
                rate = 0x0B;
                break;
            case 64:    
                rate = 0x0A;
                break;
            case 128:   
                rate = 0x09;
                break;
            case 256:   
                rate = 0x08;
                break;
            case 512:   
                rate = 0x07;
                break;
            case 1024:  
                rate = 0x06;
                break;

            default:    return -1;
        }

        //write refresh rate to CMOS
        rate = rate | (0xF0 & inb(CMOS));

        outb(REGISTER_A, RTC_PORT);
        outb(rate, CMOS);
    }
 

    return 0;
}

/* rtc_read
 * Inputs:  fd      - File descriptor number
 *          buf     - Output data pointer
 *          nbytes  - Number of bytes read
 * Return Value: 0 on success, -1 on failure
 * Function: wait until a rtc interrupt is received  */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    
    sti();
    // wait until interrupts disabled
    while (rtc_interrupt_enabled){}

    // enable interrupts once completed
    rtc_interrupt_enabled = 1;

    return 0;
}


