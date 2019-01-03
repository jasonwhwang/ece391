#include "x86_desc.h"
#include "tests.h"
#include "keyboard.h"
#include "rtc.h"
#include "exceptions.h"
#include "driver_assembly.h"
#include "sys_linkage.h"


void init_idt(){
    lidt(idt_desc_ptr);
    /* Construct the IDT entries*/
    int i;
    for(i = 0; i < NUM_VEC; i++) {
      idt[i].reserved0 = 0;
      idt[i].reserved1 = 1;
      idt[i].reserved2 = 1;
      idt[i].reserved3 = 0;
      idt[i].reserved4 = 0;

      idt[i].size = 1;
      idt[i].dpl = 0;
      idt[i].present = 0;
      idt[i].seg_selector = KERNEL_CS;


      //make sure reserved3 and present bits are 1
      //for entries under 32
      if(i < NUM_EXC) {
        idt[i].reserved3 = 0;
        idt[i].present = 1;
      }
      if(i == 0x21) {
          idt[i].present = 1;
      }
      if(i == 0x80){
        idt[i].present = 1;
        idt[i].dpl = 3;
      }
    }

    //set up exception entries
    SET_IDT_ENTRY(idt[0], e0);
    SET_IDT_ENTRY(idt[1], e1);
    SET_IDT_ENTRY(idt[2], e2);
    SET_IDT_ENTRY(idt[3], e3);
    SET_IDT_ENTRY(idt[4], e4);
    SET_IDT_ENTRY(idt[5], e5);
    SET_IDT_ENTRY(idt[6], e6);
    SET_IDT_ENTRY(idt[7], e7);
    SET_IDT_ENTRY(idt[8], e8);
    SET_IDT_ENTRY(idt[9], e9);
    SET_IDT_ENTRY(idt[10], e10);
    SET_IDT_ENTRY(idt[11], e11);
    SET_IDT_ENTRY(idt[12], e12);
    SET_IDT_ENTRY(idt[13], e13);
    SET_IDT_ENTRY(idt[14], e14);
    SET_IDT_ENTRY(idt[15], e15);
    SET_IDT_ENTRY(idt[16], e16);
    SET_IDT_ENTRY(idt[17], e17);
    SET_IDT_ENTRY(idt[18], e18);
    SET_IDT_ENTRY(idt[19], e19);
    SET_IDT_ENTRY(idt[20], e20);
    SET_IDT_ENTRY(idt[21], e21);
    SET_IDT_ENTRY(idt[22], e22);
    SET_IDT_ENTRY(idt[23], e23);
    SET_IDT_ENTRY(idt[24], e24);
    SET_IDT_ENTRY(idt[25], e25);
    SET_IDT_ENTRY(idt[26], e26);
    SET_IDT_ENTRY(idt[27], e27);
    SET_IDT_ENTRY(idt[28], e28);
    SET_IDT_ENTRY(idt[29], e29);
    SET_IDT_ENTRY(idt[30], e30);
    SET_IDT_ENTRY(idt[31], e31);

    idt[32].present = 1;
    SET_IDT_ENTRY(idt[32], pit_driver_assembly);
    //Start of Master PIC
    SET_IDT_ENTRY(idt[33], keyboard_driver_assembly); //keyboard interrupt, jump to x86_desc

    SET_IDT_ENTRY(idt[0x80], sys_call_assembly);
    //Start of Slave PIC
    idt[40].present = 1;
    SET_IDT_ENTRY(idt[40], rtc_driver_assembly); //RTC interrupt, jump to x86_desc

}
