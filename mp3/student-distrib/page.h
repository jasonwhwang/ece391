#ifndef PAGE_H
#define PAGE_H

#include "types.h"
#include "lib.h"
#include "syscalls.h"


#define FOUR_MB 0x0400000

//actual addresses
#define VIDEO 0xB8000

//page indexes needed for multi terms
#define IDX_1  1000
#define IDX_2  1001
#define IDX_3  1002

//page addresses needed for multi terms
#define ADDR_1  0xb9000
#define ADDR_2  0xba000
#define ADDR_3  0xbb000

#define VIDEO_T1    0x3e8000
#define VIDEO_T2    0x3e9000
#define VIDEO_T3    0x3ea000

// #define VIDEO_T1    VIDEO+(FOUR_KB*2)
// #define VIDEO_T2    VIDEO+(FOUR_KB*4)
// #define VIDEO_T3    VIDEO+(FOUR_KB*6)


#define VIDEO_T4    VIDEO+(FOUR_KB*1)
#define VIDEO_T5    VIDEO+(FOUR_KB*3)
#define VIDEO_T6    VIDEO+(FOUR_KB*5)

//page table indices
#define VIDEO_IDX   0xB8
#define VIDEO_IDX1  0xBA
#define VIDEO_IDX2  0xBC
#define VIDEO_IDX3  0xBE

#define VIDEO_IDX4  0xB9
#define VIDEO_IDX5  0xBB
#define VIDEO_IDX6  0xBD

#define PD_SET 0x00000082
#define NUM_ENTRIES 1024
#define CR0_SETTING 0x80000000
#define CR4_SETTING 0x00000010
#define KERNEL_SET 0x400083

//creates a page directory
uint32_t page_directory[NUM_ENTRIES] __attribute__((aligned(4096)));
//create the page_table, also with 1024 entries
uint32_t page_table[NUM_ENTRIES] __attribute__((aligned(4096)));
uint32_t video_table[NUM_ENTRIES] __attribute__((aligned(4096)));
//function for initializing paging
void init_page();

//set Control Register functions
extern void setCRs();

#endif
