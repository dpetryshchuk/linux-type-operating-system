#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "i8259.h"
#include "keyboard.h"
#include "file_system.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 19 IDT entries are not NULL except for idt[15]
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 19; ++i){
		if ((i != 15) && (idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	if ((idt[15].offset_15_00 != NULL) && (idt[15].offset_31_16 != NULL)){
		assertion_failure();
		result = FAIL;
	}

	return result;
}

/* Divide by Zero
 *
 * Test divide by zero exception
 * Inputs: None
 * Outputs: None
 * Side Effects: freeze in while loop
 * Coverage: IDT exception handler
 * Files: idt.c
 */
void division_by_zero_test(){
	TEST_HEADER;
	int x;
	int one = 1;
	int zero = 0;
	x = one/zero;
}
/* system_call_test
 * 
 * Testing a system call
 * Inputs: None
 * Outputs: none
 * Side Effects: If syscall is working properlly, sticks the os 
 * Coverage: system call idt entry
 * Files: idt.c idt.h
 */
void system_call_test(){
	TEST_HEADER;
	asm volatile("int $0x80");
}


/* PIC Test
 * 
 * Testing suite for functions in i8259.c (except send_eoi)
 * Inputs: None
 * Outputs: PASS/FAIL, IMR's are printed
 * Side Effects: None
 * Coverage: enable/disable IRQs
 * Files: i8259.c, i8259.h
 */
int pic_test(){
	TEST_HEADER;
	int result = PASS;

	uint32_t i = 0;
	/* enable all interrupts test */
	for(i = 0; i <= 15; i++){
		enable_irq(i);
	}
	if(inb(MASTER_8259_PORT+1) != 0x00 || inb(SLAVE_8259_PORT+1) != 0x00){
		printf("MIMR: %x, SIMR: %x\n",inb(MASTER_8259_PORT+1), inb(SLAVE_8259_PORT+1));
		result = FAIL;
	}

	/* disable all interrupts (except IRQ2) test */
	for(i = 0; i <= 15; i++){
		if(i == 2){
			continue;
		}
		disable_irq(i);
	}
	if(inb(MASTER_8259_PORT+1) != 0xFB || inb(SLAVE_8259_PORT+1) != 0xFF){
		printf("MIMR: %x, SIMR: %x\n",inb(MASTER_8259_PORT+1), inb(SLAVE_8259_PORT+1));
		result = FAIL;
	}
	
	printf("MIMR: %x, SIMR: %x\n",inb(MASTER_8259_PORT+1), inb(SLAVE_8259_PORT+1));

	for(i = 0; i <= 15; i++){
		enable_irq(i);
	}
	return result;
}

/* Paging Test
 * 
 * Testing suite for functions in paging.h
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Attempts to access bad memory
 * Coverage: access kernel address bounts, and NULL adress
 * Files: paging.c, paging.h, enablePaging.S
 */

int deref (int * a) {
    return *a;
}

int paging_test(){
    TEST_HEADER;
    int result = FAIL;
    int i;

    for(i = 0; i < 10; i++){
        printf("PAGING TEST!\n");
	}
    printf("dereferencing kernel address + 20\n");
    deref((int * ) 0x400000 + 20);
	printf("no exception!\n");
    printf("dereferencing kernel address\n");
    deref((int * ) 0x400000);
	printf("no exception!\n");
	printf("dereferencing kernel address + 4MB\n");
    deref((int * ) 0x400000 + (1024 * 1000));
	printf("no exception!\n");
    printf("dereferencing null, RESULT = PASS if exception occurs\n");
    deref(NULL);
    
    return result;
}




/* Checkpoint 2 tests */
/*
 * terminal_write_test
 * 
 * Testing suite for functions in terminal.c
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal_write
 * Files: terminal.c, terminal.h
*/
int terminal_driver_test(void) {
	TEST_HEADER;
	char buf[128];
	int nbytes;
	printf("Should print TESTING AHHHHH: -> ");
	terminal_write(0, (uint8_t*)"TESTING AHHHHH", 16);

	printf("\n");
	printf("Enter a string to test normal behavior (buf size = 12) : ");
    nbytes = terminal_read(0, buf, 12);
    terminal_write(0, buf, nbytes);
	printf("\n");
	printf("Enter a string to test buffer overflow (buffer size = 5) : ");
	nbytes = terminal_read(0, buf, 5);
	terminal_write(0, buf, nbytes);
	printf("\n");

	return 1;
}

/* dir_read_test()
 * 
 * Testing suite for dir_read() and read_dentry_by_index
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints all file names/info to screen
 * Coverage: accesses all dentrys in filesystem
 * Files: file_system.c file_system.h
 *
int dir_read_test(){
	TEST_HEADER;
	int result = PASS;
	int i = 1;

	dentry_t* dentry = dentry_ptr;
	
	uint8_t buf1[(inode_ptr + dentry->inode_count)->data_len];
	dir_read(i, buf1, i, dentry); 

	return result;
}
*/

/* f_read_test()
 * 
 * Testing suite for  f_read() and read_data
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints a buffer of bytes to the screen
 * Coverage: prints what is discovered at data blocks based on input
 * Files: file_system.c file_system.h
 
int f_read_test(){
	TEST_HEADER;
	int result = PASS;
	int i = 3;

	dentry_t* dentry3 = dentry_ptr;
	read_dentry_by_name((const uint8_t*) "frame0.txt", dentry3);

	file_descriptors[i].inode = dentry3->inode_count;
	file_descriptors[i].file_position = 0;

	uint32_t numbytes = (inode_ptr + dentry3->inode_count)->data_len;
	uint8_t buf[numbytes];

	if(numbytes != f_read((const uint8_t *) "frame0.txt", i, buf, numbytes)){
		assertion_failure();
		result = FAIL;
	}

	for(i = 0; i <  numbytes; i++){
		printf("%c", buf[i]);
	}

	return result;
}
*/

/* rtc_open_close_test()
 * 
 * Testing suite for rtc_read(), rtc_open(), rtc_close()
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: prints at rtc frequency
 * Files: rtc.c, rtc.h
 */
int rtc_open_close_test(){
	TEST_HEADER;

	uint32_t closed;
	uint32_t count = 0;
	uint32_t buffer;

	rtc_open(0);
	while (count < 5){
		rtc_read(0, &buffer,4);
		printf("%d", count);
		count++;
	}
	printf("\n");
	
	closed = rtc_close(0);
	if (!closed)
		printf("rtc_close success\n");
		return 1;
	return 0;
}


/* rtc_read_write_test()
 * 
 * Testing suite for rtc_read(), rtc_write
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: prints at changing rtc frequency
 * Files: rtc.c, rtc.h
 */
int rtc_read_write_test(){
	TEST_HEADER;
	uint32_t freq;
    uint32_t print;
    int32_t errors = 0;

    errors += rtc_open(0);

    for(freq = 2; freq <= 1024; freq*=2) {

        errors += rtc_write(0, &freq, 4);

        printf("%d Hz\n", freq);

        for(print = 0; print < freq; print++) {

            errors += rtc_read(0, 0, 0);
            printf("*");

        }
        printf("\n");
    }
    if(errors == 0) {
		printf("YESSSSIR :D\n");
		return 1;
    } else {
        printf("depression\n");
		return 0;
    }
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// launch your tests here

	/* CP1 */
	//TEST_OUTPUT("pic_test", pic_test());
	//TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("idt_test", idt_test());	
	//division_by_zero_test(); 
	//system_call_test();

	/* CP2 */
	//TEST_OUTPUT("rtc_open_close_test", rtc_open_close_test());
	//TEST_OUTPUT("rtc_read_write_test", rtc_read_write_test());
	//TEST_OUTPUT("dir_read_test", dir_read_test());
	//TEST_OUTPUT("f_read_test", f_read_test());
	//TEST_OUTPUT("terminal_driver_test", terminal_driver_test());
	
	/* CP3 */
}
