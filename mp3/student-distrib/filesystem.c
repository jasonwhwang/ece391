#include "filesystem.h"
#include "lib.h"
#include "syscalls.h"

inode_t* inode_start;
uint32_t data_block_first;
boot_block_t* boot_block;
dentry_t direntry;

//initialize file system
void fsystem_init(uint32_t fsystem_start) //this will be called with mod->mod_start in kernel.c
{
  boot_block = (boot_block_t*)fsystem_start; //boot block starts at beginning of file system
  inode_start = (inode_t*)(fsystem_start + BLOCK_SIZE); //inode_start begin one block after boot block
  data_block_first = fsystem_start + (boot_block->inode_count)*BLOCK_SIZE + BLOCK_SIZE; //data blocks start after inode_start, get # of inode_start from boot_block
}

//scans directory entries in boot block to find file name
//when successful, fill in dentry_t block with file name, file type, and inode number
//return 0 on success, return -1 on failure, indicating non-existent file
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
  //initialize loop variables
  uint32_t i;
  uint32_t dentry_count = boot_block->dir_count;
  //if(fname[0] == 'Q')
  //{
    //return -1;
  //}

  for(i = 0; i < dentry_count; i++)
  {
    //use strncmp from lib.c, return 0 means strings match
    if(strncmp((int8_t*)fname, (int8_t*)boot_block->direntries[i].filename, FILENAME_LEN) == 0) //if == 0, then found match
    {
        //fill dentry with filename, file type, and inode number
        strncpy((int8_t*)dentry->filename, (int8_t*)boot_block->direntries[i].filename, FILENAME_LEN); //strncopy from lib.c, copies n bytes from source string to destination string
        dentry->filetype = boot_block->direntries[i].filetype;
        dentry->inode_num = boot_block->direntries[i].inode_num;
        return 0;
    }
  }
  return -1;
}

//Use directory entry at given index to fill in dentry_t block with file name, file type, and inode number
//return -1 on failure, indicating invalid index
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
  //return -1 on invalid index
  if(index >= boot_block->dir_count)
    return -1;

  //otherwise, copy contents
  strncpy((int8_t*)dentry->filename, (int8_t*)boot_block->direntries[index].filename, FILENAME_LEN); //strncopy from lib.c, copies n bytes from source string to destination string
  dentry->filetype = boot_block->direntries[index].filetype;
  dentry->inode_num = boot_block->direntries[index].inode_num;
  return 0;
}

//read up to 'length' bytes starting from position 'offset' in file with inode number 'inode'
//return number of bytes read and placed in buffer.
//return 0 on end of file
//return -1 on failure, indicating invalid inode number
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
  uint32_t data_block_begin;
  uint32_t data_index;
  uint8_t* copy_position;
  int i; //initialize counter for bytes read

  //find which data block to start based on offset
  //and find where within datablock
  data_block_begin = offset/BLOCK_BYTES;
  data_index = offset % BLOCK_BYTES;

  // check for out of bounds data block or inode number
  if(inode_start[inode].data_block_num[data_block_begin] >= boot_block->data_count || inode >= boot_block->inode_count)
    return -1;

  //get location to begin read based on first data block plus datablock number to start at plus location within data block
  copy_position = (uint8_t *)(data_block_first + (inode_start[inode].data_block_num[data_block_begin])*BLOCK_SIZE + data_index);

  //loop up until length to read bytes
  for(i = 0; i < length; i++)
  {

    //if index is past block, move to next block
    if(data_index >= BLOCK_SIZE)
    {
      data_index = 0;
      data_block_begin = data_block_begin + 1;

      //return -1 if next datablock is outside bounds of number of data blocks
      if(inode_start[inode].data_block_num[data_block_begin] >= boot_block->data_count )
        return -1;

      //update read location based on next data block
      copy_position = (uint8_t *)(data_block_first + (inode_start[inode].data_block_num[data_block_begin])*BLOCK_SIZE);
    }

    if(inode_start[inode].length <= i + offset) //return if end of file
    {
      return i;
    }
    //put byte into buffer
    buf[i] = *copy_position;

    //increment locations
    data_index++;
    copy_position++;
  }

  return i;
}

//reads 'nbytes' bytes of data from file into 'buf'
int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
  //grab current PCB
  PCB_t* pcb = get_pcb();
  //initialize position to 0
  int position = 0;
  //keep track of position based on bytes read
  position = read_data(pcb->fd_array[fd].inode, pcb->fd_array[fd].file_position, buf, nbytes);
  //update file_position
  pcb->fd_array[fd].file_position = pcb->fd_array[fd].file_position + position;
  if(position < 0)
  {
    return 0;
  }
  else
  {
    return position;
  }
}

//initialize any temporary structures
//return 0
int32_t file_open(const uint8_t* fname)
{
  if(read_dentry_by_name(fname, &direntry) == 0)
    return 0;
  else
    return -1;
}

//undo what you did in open() function
//return 0
int32_t file_close(int32_t fd)
{
  return 0;
}

//do nothing, return -1
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
{
  return -1;
}

//read files filename by filename, include "."
/*FROM MP3 DOCUMENTATION: In the case of reads to the directory, only the filename should be provided (as much as fits, or all 32 bytes), and
subsequent reads should read from successive directory entries until the last is reached, at which point read should
repeatedly return 0.*/
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes)
{

  //grab current PCB
  PCB_t* pcb = get_pcb();
  //initialize variables
  int idx;
  int length = 0;
  //grab file position from pcb to use as index
  idx = pcb->fd_array[fd].file_position;
  pcb->fd_array[fd].file_position += 1;

  //get dentry using idx
  dentry_t d_entry = boot_block->direntries[idx];
  if(read_dentry_by_index(idx, &d_entry) == -1)
  {
    return 0;
  }
  else
  {
    //loop to get length of filename
    while(d_entry.filename[length] != NULL && length < FILENAME_LEN)
    {
      length++;
    }
    //copy filename into buffer
    strncpy(buf, d_entry.filename, FILENAME_LEN);
    return length;
  }


/*
  //CHECKPOINT 2 DIR_READ
  int i;
  int j;
  for(i = 0; i < boot_block->dir_count; i++)
  {
      dentry_t d_entry = boot_block->direntries[i];
      if(read_dentry_by_index(i, &d_entry) == -1) //if invalid index, return -1
      {
        return -1;
      }
      else //otherwise, proceed printing to screen
      {
        //printf("Filename: ");
        for(j = 0; j < 32; j++)
        {
          printf("%c", d_entry.filename[j]);
        }
        printf("\n");
        //printf("     , file_type: %d\n", d_entry.filetype);
      }
  }*/
  //return 0;
}

//opens a directory file (note file types)
//return 0
int32_t dir_open(const uint8_t* fname)
{
  if(read_dentry_by_name(fname, &direntry) == 0)
    return 0;
  else
    return -1;
}

//does nothing
//return -1
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes)
{
  return -1;
}

//does nothing
//return 0
int32_t dir_close(int32_t fd)
{
  return 0;
}
