#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define keyboard_port   0x60
#define keyboard_irq    0x1
#define keyboard_status keyboard_port+4

#define CAPS_LOCK 0x3A
#define R_SHIFT 0x36
#define L_SHIFT 0x2A
#define R_REL 0xB6
#define L_REL 0xAA
#define L_CTRL 0x1D
#define L_CTRLR 0x9D
#define ALT 0x38
#define ALT_REL 0xB8

#define ENTER 0x1C
#define L 38
#define BACK 0xE
#define SPACE 0x39
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D

#include "lib.h"
#include "i8259.h"

//Keyboard functions

extern void init_keyboard();
extern void keypress();

//Terminal functions

int32_t term_open(const uint8_t* filename);
int32_t term_close(int32_t fd);
int32_t term_read(int32_t fd, void* buf, int32_t nbytes);
int32_t term_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t shell_switch(int32_t terminal);

typedef struct keyboard{
  int32_t shift_FLAG; //1 means on, 0 means off
  int32_t caps_FLAG;
  int32_t ctrl_FLAG;
  int32_t alt_FLAG;
  int32_t key_buf_val;
  uint8_t key_buf[128];
  uint8_t key_buf1[128];
  uint8_t key_buf2[128];
  uint8_t key_buf3[128];
  int32_t T1_ENTER_FLAG;
  int32_t T2_ENTER_FLAG;
  int32_t T3_ENTER_FLAG;
  int32_t buff_size;
  int32_t buff_size1;
  int32_t buff_size2;
  int32_t buff_size3;
  int32_t limit;
  int32_t res_flag; //1 if needed to be reset
  unsigned char scancode;
} keyboard_t;

typedef struct terminal{
  int32_t cursor_x;
  int32_t cursor_y;
  uint8_t term_buf[128];
  int32_t buf_size;
} terminal_t;
#endif
