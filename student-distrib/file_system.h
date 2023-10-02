#ifndef FILE_SYSTEM
#define FILE_SYSTEM

#include "types.h"
#include "lib.h"
#include "syscalls.h"

#define BLOCK_SIZE 4096
#define INODE_BLOCK_NUMS 1023
#define DENTRY_NUMS 63
#define FILE_NAME_SIZE 32

#define DENTRY_RESERVED_SIZE 24
#define BLOCK_RESERVED_SIZE 52

#define MAX_FILES 8

/* inode uint32 */
typedef struct {
    uint32_t data_len;
    uint32_t data_inode_block_nums[INODE_BLOCK_NUMS];
} inode_t;

/* dentry uint8 */
typedef struct {
    uint8_t file_name[FILE_NAME_SIZE];
    uint32_t file_type;
    uint32_t inode_count;
    uint8_t reserved[DENTRY_RESERVED_SIZE];
} dentry_t;

/* boot_block_t uint32 */
typedef struct {
    uint32_t dir_entries;
    uint32_t inode_nums;
    uint32_t data_block_nums;
    uint8_t reserved_block[BLOCK_RESERVED_SIZE];
    dentry_t dentry_block[DENTRY_NUMS];
} boot_block_t;

/* pointers */
boot_block_t* boot_ptr;
dentry_t* dentry_ptr;
inode_t * inode_ptr;
uint8_t * data_ptr;

/* functions */
void init_fs(uint32_t base_addrs);
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t f_read(int fd , void * buf, int32_t nbytes);
int32_t f_write(int fd , const void * buf, int32_t nbytes);
int32_t f_open(const uint8_t * filename);
int32_t f_close(int fd);

int32_t dir_read(int fd , void * buf, int32_t nbytes);
int32_t dir_write(int fd , const void * buf, int32_t nbytes);
int32_t dir_open(const uint8_t * filename);
int32_t dir_close(int fd);

#endif 

