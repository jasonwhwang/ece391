#include "syscalls.h"
#include "types.h"
#include "filesystem.h"
#include "keyboard.h"
#include "rtc.h"
#include "page.h"
#include "lib.h"
#include "x86_desc.h"

//Global processes array
uint8_t processes[MAX_PROCESSES] = {0,0,0,0,0,0};

// processes_init();
// uint32_t z;
// for (z = 0; z < MAX_PROCESSES; z++){
//   printf("%d",processes[z]);
// }

PCB_t temp = { .process_num = 0 };
PCB_t* tPCB = &temp;

//jump tables for open, close, read, write
file_ops_t std_operations = {term_open, term_close, term_read, term_write};
file_ops_t dir_operations = {dir_open, dir_close, dir_read, dir_write};
file_ops_t file_operations = {file_open, file_close, file_read, file_write};
file_ops_t rtc_operations = {rtc_open, rtc_close, rtc_read, rtc_write};


uint8_t* get_processes(){
  return processes;
}

PCB_t* get_pcb()
{
  PCB_t* pcb_position;
  //bit mask the esp to get the current pcb
  asm volatile("andl %%esp, %%ebx"
              :"=b"(pcb_position)
              :"b"(bit_mask)
              );
  return pcb_position;
}

//called from jump table in assembly linkage
// dummy_jmp, sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close,
//sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn

int32_t sys_halt(uint8_t status){
  cli();

  int32_t ret = (int32_t)(status & 0xFF);

  // 1. Close process files
  PCB_t* pcb = get_pcb();
  if(pcb == NULL)
    return -1;
  int file_array_idx;
  // close all files, max of 8 files in fd, set fd entry values to null
  for(file_array_idx = 0; file_array_idx < MAX_FILES; file_array_idx++) {
    pcb->fd_array[file_array_idx].inode = -1;
    pcb->fd_array[file_array_idx].flags = 0;
    pcb->fd_array[file_array_idx].file_position = 0;
    pcb->fd_array[file_array_idx].file_ops_table = NULL;
  }

  processes[pcb->process_num] = 0;

  if(pcb->process_num <= 2)
  {
      sys_execute((uint8_t*)"shell");
  }
  // Check this, referenced from sys_execute to get the PCB from process_num
  //PCB_t* parent_PCB = (PCB_t*) (EIGHT_MB - (EIGHT_KB * (pcb->parent_process_num + 1)));

  // 2. Restore Parent data
  int32_t esp, ebp;
  esp = pcb->esp;
  ebp = pcb->ebp;

  // 3. Restore Parent paging
  page_directory[USER_ENTRY] = ((pcb->parent_process_num*FOUR_MB) + EIGHT_MB) | DIR_SET;
  //flush tlb
  asm volatile("mov %%cr3, %%eax \n\
                mov %%eax, %%cr3 \n\
               "
               :
               :
               : "memory"
               );

  tss.esp0 = EIGHT_MB - (EIGHT_KB * (pcb->parent_process_num)) - 4;
  //tss.esp0 = parent_PCB->esp;
  tss.ss0 = KERNEL_DS;


  sti();


  // 4. Jump to execute return
	asm volatile("mov %0, %%esp \n\
                mov %1, %%ebp \n\
                mov %2, %%eax \n\
                jmp BACK_TO_EXECUTE \n\
               "
               :
               : "r"(esp), "r" (ebp), "r"(ret)
               : "%eax"
               );
  return status;
}


int32_t sys_execute(uint8_t* command){
  uint8_t get_command[999];
  uint8_t i;
  uint8_t process_id;
  uint8_t null_check = 0;


  for(process_id = 0; process_id < MAX_PROCESSES; process_id++)
  {
    if(processes[process_id] == 0)
    {
      break;
    }
  }
  //full processes so return -1
  if(process_id == MAX_PROCESSES)
  {
    return -1;
  }

  //set up process block
  PCB_t* process_block = (PCB_t*) (EIGHT_MB - (EIGHT_KB * (process_id + 1)));

  if(command[0] == 92 && command[1] == 'b')
  {
    int q;
    for(q = 0; q < strlen((int8_t*)(command)); q++)
    {
      command[q] = command[q+2];
    }
  }

  //get first word
  for(i = 0; i < strlen((int8_t*)(command)); i++)
  {

    if(command[i] != ' ')
    {
      get_command[i] = command[i];
    }
    else
    {
      get_command[i] = NULL;
      null_check = 1;
      break;
    }
  }
  //NULL terminate get_command word if it never got null terminated (no space in command)
  if(null_check == 0)
  {
    get_command[i] = NULL;
  }

  //fill in the args for current process block
  if(null_check != 0) //if there are spaces in the command
  {
    uint8_t args_index = 0;

    i = i + 1; //iterator stopped at the ' ' earlier when getting command. increment i to get args after space

    while(i < strlen((int8_t*)(command)))
    {
      process_block->args[args_index] = command[i];
      i++;
      args_index++;
    }
    process_block->args[args_index] = NULL; //add null at the end to be safe
  }
/*
  int z=0;
  while(get_command[z] != NULL){
    putc(get_command[z]);
    z++;
  }
  return 0;
*/
  dentry_t exe_direntry; //dentry to pass into read_dentry_by_name
  uint8_t get_header[HEADER_SIZE]; //space to store header
  uint8_t* get_header_ptr = get_header;
  uint32_t hold_entry; //variable to hold entry point

  //check file validity
  if(read_dentry_by_name((uint8_t*)(get_command), &exe_direntry) != 0) //check this type cast
  {
    //non-existent file
    return -1;
  }
  else
  {
    //read data with direntry and buffer, first 40B go into buffer
    //first 40B of file for header to see if executable
    read_data(exe_direntry.inode_num, 0, get_header_ptr, HEADER_SIZE); //check this type cast
    //match with each of first 4 Bytes in appendix C
    if((int)get_header[0] == MAGIC_BYTE_ONE && (int)get_header[1] == MAGIC_BYTE_TWO
       && (int)get_header[2] == MAGIC_BYTE_THREE && (int)get_header[3] == MAGIC_BYTE_FOUR)
    {
      //if matches, then pick out entry point from
      //bytes 24,25,26,27 (is the address) and directly put into address of variable
    read_data(exe_direntry.inode_num, ENTRY_POINT_START, (uint8_t*)(&hold_entry), FOUR_BYTES);

    }
    else{
      //else, return -1
      return -1;
    }
  }

  //modify entry in page directory
  page_directory[USER_ENTRY] = ((process_id*FOUR_MB) + EIGHT_MB) | DIR_SET;
  //flush tlb
  asm volatile("mov %%cr3, %%eax \n\
                mov %%eax, %%cr3 \n\
               "
               :
               :
               : "memory"
               );
  //load file into memory
  read_data(exe_direntry.inode_num, 0, (uint8_t*)(VIRT_ADDR), 999999);

  //get the process number for this process from PCB
  process_block->process_num = process_id;

  if(process_id == 0)
  {
    //shell so don't do anything
  }
  else
  {
	//set parent_process_num of the new PCB to the process_num of the own PCB
    PCB_t* own_pcb = get_pcb();
    process_block->parent_process_num = own_pcb->process_num;
  }

    asm volatile("mov %%esp, %%eax \n\
                mov %%ebp, %%ebx \n\
               "
               : "=a" (process_block->esp), "=b" (process_block->ebp)
               );


  //fill in file descriptor array with stdin stdout
  process_block->fd_array[stdin_index].file_ops_table = &std_operations;
  process_block->fd_array[stdin_index].flags = 1;

  process_block->fd_array[stdout_index].file_ops_table = &std_operations;
  process_block->fd_array[stdout_index].flags = 1;

  //context switch
  //fill in esp0 and ss0 (find value (its given))
  tss.ss0 = KERNEL_DS;

  //esp0 with where kernel stack supposed to be
  //depending on the process number
  tss.esp0 = EIGHT_MB - (EIGHT_KB * process_id) - 4;

  //set process in array
  processes[process_id] = 1;

  if(process_id < 3) {
    process_block->term_num = process_id + 1;
  }
  else
  {
      process_block->term_num = terminal;
  }

  term_open(0);
  //push IRET (entering ring 3 for pushes)
 cli();

  asm volatile("mov $0x2B, %%ax;"
                "mov %%ax, %%ds;"
                "mov %%ax, %%es;"
                "mov %%ax, %%fs;"
                "mov %%ax, %%gs;"
                "pushl $0x2B;"
                //132MB - 1 bottom of user page
                "pushl $0x83FFFFC;"
                "pushfl;"
                "popl %%edx;"
                //OR the IF bit (10th bit) of flags to set to 1 because I cli()ed it
                "orl $0x200, %%edx;"
                "pushl %%edx;"
                //push user code segment
                "pushl $0x23;"
                //push entry point
                "pushl %0;"
                "iret;"
                "BACK_TO_EXECUTE: "
               :
               : "r"(hold_entry)
               : "edx", "eax"
               );

  return 0;
}

int32_t sys_read(int32_t fd, void* buf, int32_t nbytes){
  PCB_t* pcb = get_pcb();
  if(fd < 0 || nbytes < 0) //error checking
    return -1;
  int test = pcb->fd_array[fd].file_ops_table->read_op(fd,buf,nbytes);
  return test;
}
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes){
  PCB_t* pcb = get_pcb(); //do error checks
  if(fd < 0 || nbytes < 0)
    return -1;
  return pcb->fd_array[fd].file_ops_table->write_op(fd, buf, nbytes);
}
int32_t sys_open(const uint8_t* filename){
  PCB_t* pcb = get_pcb();
  int32_t i;
  for(i=0;i<8;i++){if(pcb->fd_array[i].flags == 0) break;}
  //change depending on if you want dir_ops, rtc_ops, or file_ops
  //can only have one at a time
  dentry_t dir_ent;
  //check for error on this guy
  read_dentry_by_name(filename, &dir_ent);
  if(dir_ent.filetype == 0)
  {
    pcb->fd_array[i].file_ops_table = &rtc_operations;
  }
  else if(dir_ent.filetype == 1)
  {
    pcb->fd_array[i].file_ops_table = &dir_operations;
  }
  else if(dir_ent.filetype == 2)
  {
    pcb->fd_array[i].file_ops_table = &file_operations;
  }

  //pcb->fd_array[i].file_ops_table = &file_operations;
  pcb->fd_array[i].inode = dir_ent.inode_num;
  pcb->fd_array[i].file_position = 0;
  pcb->fd_array[i].flags = 1;

  //return pcb->fd_array[i].file_ops_table->open_op(filename);
  return i;
}
int32_t sys_close(int32_t fd){
	PCB_t* pcb = get_pcb();

	//change depending on if you want dir_ops, rtc_ops, or file_ops
	//can only have one at a time
	pcb->fd_array[fd].file_ops_table = &dir_operations;
	pcb->fd_array[fd].flags = 1;
	if(fd < 0)
		return -1;
	return pcb->fd_array[fd].file_ops_table->close_op(fd);
}

int32_t sys_getargs(uint8_t* buf, int32_t nbytes){
	//check whether buffer is valid
  if(buf == NULL || nbytes <= 0)
  {
    return -1;
  }
	//check whether PCB has any arguments, if no arguments return -1
  PCB_t* pcb = get_pcb();
  //puts(pcb->args);
  if(pcb->args[0] == NULL)
  {
    return -1;
  }
	//copy args to buffer with memcpy and nbytes
  memcpy(buf, pcb->args, nbytes);
	//"cat <filename>" to test get_args
  return 0;
}


int32_t sys_vidmap(uint8_t** screen_start){
	//error checking. (*screen_start should be within 128-132 MB)
  uint32_t i;
  uint8_t* mem = (uint8_t*)(MB_128+FOUR_MB);
  //puts(screen_start);
  //if(**screen_start <= 128 || **screen_start >= 132) return -1;

    if((uint32_t)screen_start < MB_128 || (uint32_t)screen_start > (MB_128+FOUR_MB)) return -1;


  //Make sure all values for video_table are set, don't want garbage values
  for(i=0;i<max_values;i++){video_table[i]=(i * MULT_ONE) | BIT_OR_6;}

  video_table[0] = VIDEO|BIT_OR_7;
  page_directory[page_dir_max] = ((uint32_t)video_table)|BIT_OR_7;

	//set up paging s.t. virtual address refers to physical memory
	*screen_start = mem;
  return MB_128+FOUR_MB;
}
int32_t sys_set_handler(int32_t signum, void* handler_address){
  return 0;
}
int32_t sys_sigreturn(void){
  return 0;
}

int32_t dummy_jmp()
{
  return -1;
}
