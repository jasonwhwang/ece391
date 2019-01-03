#include "page.h"


void init_page(){
  int i;
  //Makes all entries supervisor, can read/write, and marked as not present,258
  for(i=0;i<NUM_ENTRIES;i++){page_directory[i] = PD_SET;}
  //for(i=0;i<NUM_ENTRIES;i++){page_directory[i] = 0x00000087;}

  //page align the values and set 12 bits to 0 to be used for later attributes
  for(i=0;i<NUM_ENTRIES;i++){page_table[i] = 0;}
    //for(i=0;i<NUM_ENTRIES;i++){page_table[i] = (i * 0x1000) | 7;}

  //Mark the actual page table video memory to read/write, supervisor and present
  page_table[VIDEO_IDX] = VIDEO|3;
  //page_table[VIDEO] = 0xB8007;

  page_table[IDX_1] = ADDR_1 + 0x3;
  page_table[IDX_2] = ADDR_2 + 0x3;
  page_table[IDX_3] = ADDR_3 + 0x3;

  // page_table[VIDEO_IDX1] = (VIDEO)|3;
  // page_table[VIDEO_IDX2] = (VIDEO_T2)|3;
  // page_table[VIDEO_IDX3] = (VIDEO_T3)|3;
  //
  // page_table[VIDEO_IDX4] = (VIDEO_T4)|3;
  // page_table[VIDEO_IDX5] = (VIDEO_T5)|3;
  // page_table[VIDEO_IDX6] = (VIDEO_T6)|3;

  //Mark the video section as present
  page_directory[0] = ((uint32_t)page_table)|3;
  //page_directory[0] = ((uint32_t)page_table)|7;

  page_directory[2] = ((uint32_t)page_table)|3;
  page_directory[3] = ((uint32_t)page_table)|3;
  page_directory[4] = ((uint32_t)page_table)|3;

  //Mark kernel section as present
  page_directory[1] = KERNEL_SET;
  //page_directory[1] = 0x400087;
  setCRs();
      asm volatile("mov %%cr3, %%eax \n\
                mov %%eax, %%cr3 \n\
               "
               :
               :
               : "memory"
               );
}


//Set the control register bits to their proper values
void setCRs(){
  asm volatile(
  "mov %%eax, %%cr3             \n\
  mov %%cr4, %%eax             \n\
  or  $0x00000010, %%eax       \n\
  mov %%eax, %%cr4             \n\
  mov %%cr0, %%eax             \n\
  or $0x80000001, %%eax        \n\
  mov %%eax, %%cr0             \n\
  "
  :
  : "a"(page_directory)
  : "memory"
  );
}
