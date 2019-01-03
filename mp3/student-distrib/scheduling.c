#include "scheduling.h"
#include "lib.h"
#include "page.h"
#include "i8259.h"
#include "syscalls.h"
#include "x86_desc.h"
#include "rtc.h"

int int_flag = 0;
uint32_t int_process = 0;
uint32_t shell_ct = 0;
uint32_t val_arry[3] = {0,0,0};

// from http://www.osdever.net/bkerndev/Docs/pit.htm
void init_pit(){
    int divisor =  PIT_FREQ/20;    // calculate divisor value
    outb(COM_BYTE, PIT_PORT);             // Set command byte 0x36
    outb(divisor & LOW_MASK, CHAN_0);   // Set low byte of divisor
    outb(divisor >> EIGHT, CHAN_0);     // Set high byte of divisor
    enable_irq(0);
}

void pit_handler(){
  send_eoi(PIT_IRQ);

  cli();

  //grab current pcb
  PCB_t* pcb = get_pcb();
  int32_t esp, ebp;

  // save esp, ebp, into variables
  asm volatile("mov %%esp, %%eax \n\
               mov %%ebp, %%ebx \n\
             "
             : "=a" (esp), "=b" (ebp)
             );
  val_arry[int_process] = esp;
  pcb->esp = esp;
  pcb->ebp = ebp;
  int_process++;
  int_process = int_process%3;
  //launch two more shells
  if(shell_ct < 2)
  {
     shell_ct++;
     sys_execute((uint8_t*)("shell"));
  }

  asm volatile("mov %0, %%esp"
               :
               : "r"(val_arry[int_process])
             );

  pcb = get_pcb();

  tss.esp0 = EIGHT_MB - (EIGHT_KB*pcb->process_num) - 4;
  tss.ss0 = KERNEL_DS;

  //set up paging
  page_directory[USER_ENTRY] = ((pcb->parent_process_num*FOUR_MB) + EIGHT_MB) | DIR_SET;


  asm volatile("movl %0, %%ebp \n\
               movl %1, %%esp \n\
               "
              :
              : "r" (pcb->ebp), "r" (pcb->esp)
              );

    //flush tlb
    asm volatile("mov %%cr3, %%eax \n\
                  mov %%eax, %%cr3 \n\
                 "
                 :
                 :
                 : "memory"
                 );
  sti();

  //execute shell for term2 and term3
  //context switching

  //save esp, ebp for this process into pcb
  //int32_t esp, ebp;

  //curr_pcb->esp = ;
  //curr_pcb->ebp = ;
  //then set paging up for next process
  //set tss esp0 for the next process
  //get ebp for process we're switching into (set up from scheduling)
}

/*uint8_t* processes = get_processes();
PCB_t *curr_pcb = get_pcb();
PCB_t *future_pcb;
int32_t i;
int32_t proc_id = curr_pcb->process_num;
for(i=0;i<MAX_PROCESSES;i++){
  if(processes[i] == 1)
    future_pcb = (PCB_t*)(EIGHT_MB - (EIGHT_KB * (i+1)));
    switch(future_pcb->term_num):

  }
}*/
