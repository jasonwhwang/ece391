#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "types.h"

//Bytes
#define FOUR_KB 4096
#define EIGHT_KB 8192
#define EIGHT_MB 0x800000
#define FOUR_MB 0x0400000
#define MB_128 0x08000000
#define FOUR_BYTES 4
#define ENTRY_POINT_START 24

//constants for setting up paging
#define DIR_SET 0x87
#define USER_ENTRY 32

#define VIRT_ADDR 0x08048000 //virtual address
#define bit_mask 0x007fe000 //8MB-8KB
#define MAX_PROCESSES 6 //max number of processes
#define MAX_FILES 8 //max number of files in fd
#define HEADER_SIZE 40 //40B size of header

//first four bytes magic number in header
#define MAGIC_BYTE_ONE 0x7f
#define MAGIC_BYTE_TWO 0x45
#define MAGIC_BYTE_THREE 0x4c
#define MAGIC_BYTE_FOUR 0x46

//vidmap definitions
#define max_values 1024
#define vm_MB_128 128 //fix this value?
#define vm_MB_132 132 //fix this value?
#define MULT_ONE 0x1000
#define BIT_OR_7 7
#define BIT_OR_3 3
#define BIT_OR_6 6
#define page_dir_max 33

//indexes in fd array
#define stdin_index 0
#define stdout_index 1

//set up for jump table
typedef int32_t (*read_fn)(int32_t fd, void* buf, int32_t nbytes);
typedef int32_t (*open_fn)(const uint8_t* filename);
typedef int32_t  (*close_fn)(int32_t fd);
typedef int32_t (*write_fn)(int32_t fd, const void* buf, int32_t nbytes);

//list of system calls
int32_t sys_halt(uint8_t status);
int32_t sys_execute(uint8_t* command);
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t sys_open(const uint8_t* filename);
int32_t sys_close(int32_t fd);
int32_t sys_getargs(uint8_t* buf, int32_t nbytes);
int32_t sys_vidmap(uint8_t** screen_start);
int32_t sys_set_handler(int32_t signum, void* handler_address);
int32_t sys_sigreturn(void);

uint8_t* get_processes(void);

//jump table file_ops struct
typedef struct file_ops{
  open_fn open_op;
  close_fn close_op;
  read_fn read_op;
  write_fn write_op;
}file_ops_t;

//file descriptor struct
typedef struct file_desc{
  int32_t inode;
  int32_t flags;
  int32_t file_position;
  file_ops_t* file_ops_table;
}file_desc_t;


typedef struct PCB{
  file_desc_t fd_array[8];
  uint8_t args[128];
  int32_t process_num;
  int32_t parent_process_num;
  int32_t ebp;
  int32_t esp;
  //keep track of which terminal this process belongs to
  int32_t term_num;
}PCB_t;

//get_pcb for grabbing current pcb
PCB_t* get_pcb();



#endif
