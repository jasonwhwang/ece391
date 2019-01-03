#ifndef _RTC_H
#define _RTC_H

#include "types.h"
/*
#define rtc_irq    0x8
#define SELECT_REG      0x70
#define CMOS_RTC_PORT   0x71
#define REG_A           0x8A // With NMI Disabled 0x80
#define REG_B           0x8B // With NMI Disabled
#define REG_C           0x8C

extern void init_rtc();
extern void rtc();
*/

#define rtc_irq         0x8
#define rtc_irq2        0x2
#define SELECT_REG      0x70
#define CMOS_RTC_PORT   0x71
#define REG_A           0x8A // With NMI Disabled 0x80
#define REG_B           0x8B // With NMI Disabled
#define REG_C           0x8C
#define bit6_mask       0x40

#define HZ_1024         0x06
#define HZ_512          0x07
#define HZ_256          0x08
#define HZ_128          0x09
#define HZ_64           0x0A
#define HZ_32           0x0B
#define HZ_16           0x0C
#define HZ_8            0x0D
#define HZ_4            0x0E
#define HZ_2            0x0F

// Variables

volatile int rtc_interrupt_occurred;

// Functions

extern void init_rtc();
extern void rtc_handler();
int32_t set_rtc_freq(int32_t freq);
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open (const uint8_t* filename);
int32_t rtc_close (int32_t fd);

#endif
