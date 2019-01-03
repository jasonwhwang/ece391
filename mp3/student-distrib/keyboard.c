//Keyboard and terminal support

#include "keyboard.h"
#include "syscalls.h"

// https://wiki.osdev.org/PS/2_Keyboard
// For keyboard layout information
//Regular keyboard map pulled from Bran's kernel development

volatile int clear_term_at_start = 1;

char keyboard_map[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t',     /* Tab */
  'q', 'w', 'e', 'r', /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0,      /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
 '\'', '`',   0,    /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',      /* 49 */
  'm', ',', '.', '/',   0,        /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* 58 Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

//Shift map
char shiftmap[128]={
  0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
'(', ')', '_', '+', '\b', /* Backspace */
'\t',     /* Tab */
'Q', 'W', 'E', 'R', /* 19 */
'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
  0,      /* 29   - Control */
'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
'"', '~',   0,    /* Left shift */
'|', 'Z', 'X', 'C', 'V', 'B', 'N',      /* 49 */
'M', '<', '>', '?',   0,        /* Right shift */
'*',
  0,  /* Alt */
' ',  /* Space bar */
  0,  /* 58 Caps lock */
  0,  /* 59 - F1 key ... > */
  0,   0,   0,   0,   0,   0,   0,   0,
  0,  /* < ... F10 */
  0,  /* 69 - Num lock*/
  0,  /* Scroll Lock */
  0,  /* Home key */
  0,  /* Up Arrow */
  0,  /* Page Up */
'-',
  0,  /* Left Arrow */
  0,
  0,  /* Right Arrow */
'+',
  0,  /* 79 - End key*/
  0,  /* Down Arrow */
  0,  /* Page Down */
  0,  /* Insert Key */
  0,  /* Delete Key */
  0,   0,   0,
  0,  /* F11 Key */
  0,  /* F12 Key */
  0,  /* All other keys are undefined */
};

//Caps lock map
char capmap[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t',     /* Tab */
  'Q', 'W', 'E', 'R', /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', /* Enter key */
    0,      /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', /* 39 */
 '\'', '`',   0,    /* Left shift */
 '\\', 'Z', 'X', 'C', 'V', 'B', 'N',      /* 49 */
  'M', ',', '.', '/',   0,        /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* 58 Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

//global structs for terminal and keyboard
terminal_t term;
keyboard_t keybd;

//initialize keyboard
void init_keyboard() {
    enable_irq(keyboard_irq);
    keybd.shift_FLAG = 0;
    keybd.caps_FLAG = 0;
    //Need to account for the increase in the size before putting keys in
    keybd.alt_FLAG = 0;
    keybd.key_buf_val = 1;
    keybd.buff_size = -1, keybd.buff_size1 = -1, keybd.buff_size2 = -1, keybd.buff_size3 = -1;
    keybd.limit = 128;
    int32_t i;
    for(i=0;i<keybd.limit;i++){keybd.key_buf[i] = 0;}
}

//keyboard handler for keypresses
void keypress(){

  //Get scancode, no need for status bit check if we're using interrupts
  keybd.scancode = inb(keyboard_port);

  //Enter
  if(keybd.scancode == ENTER){
    switch(terminal)
    {
      case 1:
        keybd.T1_ENTER_FLAG = 1;
        break;
      case 2:
        keybd.T2_ENTER_FLAG = 1;
        break;
      case 3:
        keybd.T3_ENTER_FLAG = 1;
        break;
    }
    keybd.buff_size++;
    keybd.key_buf[keybd.buff_size] = keyboard_map[keybd.scancode];
    putc(keyboard_map[keybd.scancode]);
    //set the reset flag
    keybd.res_flag = 1;
  }

  //Handle the rest of the keyboard inputs that aren't ENTER
  else{
    //Reset the reset flag
    if(keybd.res_flag == 1){
      int32_t i;
      for(i=0;i<keybd.limit;i++){keybd.key_buf[i]=NULL;}
      keybd.buff_size = -1;
      keybd.res_flag = 0;
    }

    //CAPS LOCK
    if(keybd.scancode == CAPS_LOCK){
      if(keybd.caps_FLAG == 0) keybd.caps_FLAG = 1;
      else keybd.caps_FLAG = 0;
    }

    // ALT Keys
    if(keybd.scancode == ALT)
      keybd.alt_FLAG = 1;

    if(keybd.scancode == ALT_REL)
      keybd.alt_FLAG = 0;


    //Shifts
    if(keybd.scancode == R_SHIFT || keybd.scancode == L_SHIFT)
      keybd.shift_FLAG = 1;
    if(keybd.scancode == R_REL || keybd.scancode == L_REL)
      keybd.shift_FLAG = 0;

    //Controls
    if(keybd.scancode == L_CTRL)
      keybd.ctrl_FLAG = 1;
    if(keybd.scancode == L_CTRLR)
      keybd.ctrl_FLAG = 0;

    //Regular inputs
    if(keybd.buff_size<(keybd.limit) && keybd.scancode<0x80){

      // Terminal - Support for Multiple Terminals
      // ***ALT adds space to the buffer for some reason, need to debug this
      if(keybd.alt_FLAG == 1 && keybd.scancode == F1 && keybd.key_buf_val != 1) shell_switch(1);
      if(keybd.alt_FLAG == 1 && keybd.scancode == F2 && keybd.key_buf_val != 2) shell_switch(2);
      if(keybd.alt_FLAG == 1 && keybd.scancode == F3 && keybd.key_buf_val != 3) shell_switch(3);

      //Defined keys
      if(keybd.scancode<59){
        unsigned char pressed_representation;

        if(keybd.shift_FLAG == 0 && keybd.caps_FLAG == 0){
          if(keybd.ctrl_FLAG == 1 && keybd.scancode == L){
            clear();
            keybd.buff_size = -1;
          }
          else{
            pressed_representation = keyboard_map[keybd.scancode];
            if(keybd.scancode != L_CTRL && keybd.scancode != L_SHIFT && keybd.scancode != R_SHIFT && keybd.scancode != ALT){
              if(keybd.scancode == BACK && keybd.buff_size >= 0){
                keybd.key_buf[keybd.buff_size] = NULL;
                keybd.buff_size--;
                putc(pressed_representation);
              }
              else{
                keybd.buff_size++;
                keybd.key_buf[keybd.buff_size] = pressed_representation;
                putc(pressed_representation);
              }
            }
          }
        }

        else if(keybd.shift_FLAG == 1){
          if(keybd.caps_FLAG == 1) pressed_representation = keyboard_map[keybd.scancode];
          else pressed_representation = shiftmap[keybd.scancode];
          if(keybd.scancode != L_SHIFT && keybd.scancode != R_SHIFT && keybd.scancode != ALT){
            if(keybd.scancode == BACK && keybd.buff_size >= 0){
              keybd.key_buf[keybd.buff_size] = NULL;
              keybd.buff_size--;
              putc(pressed_representation);
            }
            else{
              keybd.buff_size++;
              keybd.key_buf[keybd.buff_size] = pressed_representation;
              putc(pressed_representation);
            }
          }
        }

        else if(keybd.caps_FLAG == 1){
          if(keybd.shift_FLAG == 1) pressed_representation = keyboard_map[keybd.scancode];
          else pressed_representation = capmap[keybd.scancode];
          if(keybd.scancode != ALT && keybd.scancode != CAPS_LOCK && keybd.scancode != L_SHIFT && keybd.scancode != R_SHIFT){
            if(keybd.scancode == BACK && keybd.buff_size >= 0){
              keybd.key_buf[keybd.buff_size] = NULL;
              keybd.buff_size--;
              putc(pressed_representation);
            }
            else{
              keybd.buff_size++;
              keybd.key_buf[keybd.buff_size] = pressed_representation;
              putc(pressed_representation);
            }
          }
        }
      }

      //Undefined keys, do nothing
      else if(keybd.scancode<0x80 && keybd.scancode>=59){}
    }
  }
  send_eoi(keyboard_irq);
}


int32_t shell_switch(int32_t terminal) {
  switch(keybd.key_buf_val) {
    case 1:
      keybd.buff_size1 = keybd.buff_size;
      memcpy(keybd.key_buf1, keybd.key_buf, sizeof(keybd.key_buf));
      break;
    case 2:
      keybd.buff_size2 = keybd.buff_size;
      memcpy(keybd.key_buf2, keybd.key_buf, sizeof(keybd.key_buf));
      break;
    case 3:
      keybd.buff_size3 = keybd.buff_size;
      memcpy(keybd.key_buf3, keybd.key_buf, sizeof(keybd.key_buf));
      break;
  }

  switch(terminal) {
    case 1:
      memcpy(keybd.key_buf, keybd.key_buf1, sizeof(keybd.key_buf1));
      keybd.buff_size = keybd.buff_size1;
      break;
    case 2:
      memcpy(keybd.key_buf, keybd.key_buf2, sizeof(keybd.key_buf2));
      keybd.buff_size = keybd.buff_size2;
      break;
    case 3:
      memcpy(keybd.key_buf, keybd.key_buf3, sizeof(keybd.key_buf3));
      keybd.buff_size = keybd.buff_size3;
      break;
  }
  switchTerminal(terminal);
  keybd.key_buf_val = terminal;

  return 0;
}


////////////////////////////////////////////////////////////////////////////
/////////////////////////// TERMINAL CODE//////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//Initializes terminal variables, returns 0

int32_t term_open(const uint8_t* filename){
  if (clear_term_at_start){
    clear_term_at_start = 0;
    clear();
  }

  term.buf_size = 128;
  int32_t i;
  for(i=0;i<term.buf_size;i++){keybd.key_buf[i] = NULL;}
  return 0;
}

//Clears terminal variables, returns 0.
int32_t term_close(int32_t fd){
  return 0;
}

/*
  *Reads from the keyboard buffer into terminal buffer.
  *buf is the buffer that we want to write to
  *Returns # of bytes read on success or -1 on failure.
*/
int32_t term_read(int32_t fd, void* buf, int32_t nbytes){
  int32_t num_bytes = -1;
  uint8_t* castbuffer = (uint8_t* )buf;
  int32_t i;

  PCB_t* term_pcb = get_pcb();
  //check which enter flag is pressed

  while(1){
    if(term_pcb->term_num == 1 && keybd.T1_ENTER_FLAG == 1){
      break;
    }else if(term_pcb->term_num == 2 && keybd.T2_ENTER_FLAG == 1){
      break;
    }else if(term_pcb->term_num == 3 && keybd.T3_ENTER_FLAG == 1){
      break;
    }
  }
  keybd.scancode = 0;
  for(i=0; i<=keybd.buff_size; i++){
    castbuffer[i] = keybd.key_buf[i];
  }
  num_bytes = i;
  //reset enter flags
  keybd.T1_ENTER_FLAG = 0;
  keybd.T2_ENTER_FLAG = 0;
  keybd.T3_ENTER_FLAG = 0;
  return num_bytes;
}

/*
 *Writes to the screen from the terminal buffer.
 *Returns number of bytes written on success or -1 on failure.
*/
int32_t term_write(int32_t fd, const void* buf, int32_t nbytes){
  int32_t num_bytes = -1;
  uint8_t *castbuffer = (uint8_t *)buf;
  PCB_t* cur_pcb = get_pcb();
  int32_t i,x;
  if(castbuffer != NULL){
    for(i=0;i<nbytes; i++){
      if(castbuffer[i] == '\n'){
        x = i;
        i = x;
      }
      if(cur_pcb->term_num == terminal) {
        putc(castbuffer[i]);
      }
      else {
        putmemc(castbuffer[i], cur_pcb->term_num);
      }
    }
    num_bytes = i;
  }
  return num_bytes;
}
