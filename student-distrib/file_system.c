#include "file_system.h"
#include "syscalls.h"

void init_fs(uint32_t base_addrs) {
  //init file_system pointers
  boot_ptr = (boot_block_t*)(base_addrs);
  dentry_ptr = boot_ptr->dentry_block;
  inode_ptr = (inode_t* )(boot_ptr + 1); 
  data_ptr = (uint8_t*)(inode_ptr + (boot_ptr->inode_nums)); 
}

/* 
 * read_dentry_by_name
 * Input: fname, dentry
 * Output: return 0 = success, return 1 = fail
 * 
 * Searches filesytem for a dentry that has a matching filename string
 * 
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){ 
  uint32_t file_length;
  uint32_t dentry_length;
  int i; //iterator
  for(i = 0; i < 64; i++) {
    
    file_length = strlen((int8_t*) fname);
    dentry_length = strlen((int8_t*) boot_ptr->dentry_block[i].file_name);
    
    if(file_length > FILE_NAME_SIZE){
      file_length = FILE_NAME_SIZE;
    }
    if(dentry_length > FILE_NAME_SIZE){
      dentry_length = FILE_NAME_SIZE;
    }
    uint8_t * file_name_cmp = (boot_ptr->dentry_block[i]).file_name;
    //printf("%c\n", *file_name_cmp);
    if(file_length == dentry_length && strncmp((int8_t*) file_name_cmp,(int8_t*) fname, 32) == 0){
      read_dentry_by_index(i, dentry);
      return 0;
    }
  }
  return -1;
}

/* 
 * read_dentry_by_index
 * Input: fname, dentry
 * Output: return 0 = success, return 1 = fail
 * 
 * Searches filesytem for a dentry that has a matching index node
 * 
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){

  if(index < DENTRY_NUMS){
    strncpy((int8_t*)dentry->file_name, (int8_t*)dentry_ptr[index].file_name,FILE_NAME_SIZE);
    dentry->file_type = dentry_ptr[index].file_type;
    dentry->inode_count = dentry_ptr[index].inode_count;
    return 0;
  }
  return -1;
}

/* 
 * read_data
 * Input: inode, offset, buf, length
 * Output: return 0 = success, return length = success
 * 
 * Searches data_blocks and copies desired bytes to buffer one at a time.
 * - Inode determines which data blocks are searched,
 * - offset indicates the byte offset from start of the file to start reading
 * - length is # of bytes to be read
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {

    inode_t* inode_block_ptr = (inode_t*)(inode_ptr + inode); //ptr to inode in question

    uint32_t block_index = offset / BLOCK_SIZE;         //current block being read
    uint32_t byte_index = offset % BLOCK_SIZE;          //byte being read inside current block_index
    uint32_t total_bytes_read = 0;                      //# of bytes read oveall
    uint32_t bytes_to_read = length;                    //# of bytes to be read
    uint32_t file_length = inode_block_ptr->data_len;

    //our two variables that will be used for reading and copying bytes
    uint32_t data_block = inode_block_ptr->data_inode_block_nums[block_index]; //the data block in question
    uint8_t* data_byte_ptr = (uint8_t*)(data_ptr + (BLOCK_SIZE * data_block)); //ptr to the byte being copied

    uint32_t i; //iterator

  //check if the offset is bigger then the actual file
    if(offset >= inode_block_ptr->data_len){
      return 0;
    }
    
    //check if the inode is valid relative to the filesystem
    if (inode < 0 || inode >= boot_ptr->inode_nums) {
      return -1;
    }

    for (i = 0; i < length; i++) {
      //if we have read all the bytes we want to, stop
      if (total_bytes_read >= bytes_to_read) {
          break;
      }

      //check for if we reached the end of the file
      if(offset + i >= file_length){
        break;
      }

      //if we reach the end of the block, move into the next one
      if(byte_index >= BLOCK_SIZE){
        byte_index = 0;
        block_index++;

        //check if our new block is invalid
        if(inode_block_ptr->data_inode_block_nums[block_index] >= boot_ptr->data_block_nums){
        return -1;
        }
      }
      
      //copy current byte into the buffer
      data_block = inode_block_ptr->data_inode_block_nums[block_index];
      data_byte_ptr = (uint8_t*)(data_ptr + (BLOCK_SIZE * data_block) + byte_index);
      memcpy(buf, data_byte_ptr,1);

      //increment  iterators
      total_bytes_read++;
      buf++;
      byte_index++;
    }

    return total_bytes_read;
}

/*
* f_open
* input: filename, fd
* outout: 0 = success
*
*/
int32_t f_open(const uint8_t * filename){
  return 0;
}
/*
* f_close
* input: filename, fd
* outout: 0 = success
*
*/
int32_t f_close(int fd){
  return 0;
}

/* 
 * f_read
 * Input: filename, fd, buf, nbytes
 * Output: return nbytes = success
 * 
 * Using read_data, copies data from data_blocks to buffer
 * 
 */
int32_t f_read(int fd , void * buf, int32_t nbytes){

    pcb_t * pcb = get_pcb();                        //grab the pcb
    fd_t * cur_fd = pcb->fd_array;                  //grab the current fd_array

    //check for end of file
    if(cur_fd[fd].file_position >= (inode_ptr + cur_fd[fd].inode)->data_len){
      return 0;
    }

    //read the data, retunr it
    int32_t bytes_read = read_data(cur_fd[fd].inode, cur_fd[fd].file_position, buf, nbytes);
    if(bytes_read == -1){
      return 0; 
    }
    //increment the file position
    else cur_fd[fd].file_position += bytes_read;
    return bytes_read;

}

/* 
 * f_write
 * Input: filename, fd, buf, nbytes
 * Output: returns -1;
 * 
 */
int32_t f_write(int fd , const void * buf, int32_t nbytes){
  return -1;
}

/* 
 * dir_read
 * Input: fd, buf, nbytes, dentry
 * Output: return nbytes = success
 * 
 * prints all directories/files in input directory
 * 
 */
int32_t dir_read(int32_t fd, void * buf, int32_t nbytes){
  
  dentry_t dentry;
  pcb_t * pcb = get_pcb();   //grab the pcb
  fd_t * cur_fd = pcb->fd_array;                  //grab the current fd_array
  
  //check if directory exists, read in to dentry;
  if(-1 == read_dentry_by_index(cur_fd->file_position, &dentry)){
    return -1;
  }

  //copy name of directory into buffer
  strncpy((int8_t*) buf, (const int8_t *) dentry.file_name,(uint32_t) nbytes);

  //increment the file position for the next 
  cur_fd->file_position++;

  //if we read a long file name, return 32 read bytes
  if(strlen((const int8_t *) dentry.file_name) > FILE_NAME_SIZE){
    return FILE_NAME_SIZE;
  }
  //otherwise return bytes read
  else {
    return strlen((const int8_t *) dentry.file_name);
  }
}
/* 
 * dir_write
 * Input: fd, buf, nbytes
 * Output: -1
 * 
 * file system is read only, unused
 * 
 */
int32_t dir_write(int fd , const void * buf, int32_t nbytes){
  return -1;
}
/* 
 * dir_open
 * Input: filename
 * Output: 0, success
 * 
 */
int32_t dir_open(const uint8_t * filename){
  return 0;
}
/* 
 * dir_close
 * Input: filename
 * Output: 0, success
 * 
 */
int32_t dir_close(int fd){
  return 0;
}
