#ifndef RTC_H
#define RTC_H

#include "types.h"
#include "lib.h"

#define IRQ_NUM                 8
#define REGISTER_A              0x8A
#define REGISTER_B              0x8B
#define REGISTER_C              0x8C

#define RTC_PORT                0x70
#define CMOS                    0x71


extern void rtc_handler(void);
void rtc_init(void);

int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);



#endif
