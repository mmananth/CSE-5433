#include <linux/fs.h>
#include "inode.h"

/* Superblock operations */
void lab5fs_read_inode(struct inode *inode)
{
  ino_t ino = inode->i_ino;

  /* Set up defaults */
  inode->i_mode = S_IRUGO|S_IWUSR; /* temp */
  inode->i_nlink = 1;
  inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
  inode->i_blocks = 0;
  inode->i_blksize = inode->i_sb->s_blocksize;

  /* Root / special case */
  if (ino == ROOT_INO) {
    printk(KERN_INFO "lab5fs reading root\n");
    inode->i_mode |= S_IFDIR | S_IXUGO;
    inode->i_nlink = 2;
    inode->i_fop = &simple_dir_operations;
    inode->i_op = &simple_dir_inode_operations;
  }
}
