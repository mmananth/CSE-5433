#ifndef _LAB5FS_H_
#define _LAB5FS_H_

#include <linux/types.h>
#include <linux/fs.h>

/* Definitions */
#define LAB5FS_SB_MAGIC_NUM  0xCAFECAFE
#define MAX_FILE_NAME_LEN    256

/* Macros */
#define BYTES_TO_BLOCKS(loc, blocksize) (loc/blocksize)
#define BLOCK_N(n, blocksize)           (n*blocksize)
#define INODE_N(n, bs)                  (BLOCK_N(3,bs)+(n*bs))
#define DATA_N(n, bs)                   (BLOCK_N(129,bs)+(n*bs))

/* Types */
typedef struct lab5fs_sb {
    /* Misc. */
    unsigned long magic_num;

    /* Sizes */
    unsigned long blocksize;
    unsigned long blocksize_bits;
    unsigned long max_bytes;
    unsigned long inode_size;

    /* Block locations */
    unsigned long inode_bitmap_loc;
    unsigned long data_bitmap_loc;
    unsigned long inode_loc;
    unsigned long data_loc;
    unsigned long root_inode_loc;

    unsigned long last_data_block;

    /* Free/etc. counts */
    unsigned long blocks_total;
    unsigned long data_blocks_free;
    unsigned long inode_blocks_free;
    unsigned long data_blocks_total;
    unsigned long inode_blocks_total;
} lab5fs_sb;

typedef struct lab5fs_ino {
    /* Inode data */
    uid_t i_uid;
    gid_t i_gid;
    umode_t i_mode;
    int blocks;
    unsigned long size;

    struct timespec i_atime;
    struct timespec i_mtime;
    struct timespec i_ctime;

    char name[MAX_FILE_NAME_LEN];

    /* For the data blocks */
    unsigned long i_sblock;
    unsigned long i_eblock;

    /* Link data */
    unsigned char is_hard_link;
    unsigned long block_to_link_to;
} lab5fs_ino;

#endif
