#include <linux/fs.h>
#include "super.h"

/* Superblock functions */
static void lab5fs_put_super(struct super_block *sb)
{
  return;
}

int lab5fs_fill_sb(struct super_block *sb,
                   void *data,
                   int silent)
{
  struct inode *root;

  /* Set sizes, etc. */
  sb->s_op = &lab5fs_ops;
  sb->s_blocksize = 4096;
  sb->s_blocksize_bits = 12;

  /* Get root / */
  root = iget(sb, ROOT_INO);
  sb->s_root = d_alloc_root(root);
  if (!sb->s_root) {
    printk(KERN_INFO "lab5fs could not allocate root\n");
    return -EINVAL;
  }
  return 0;
}

struct super_block *lab5fs_get_sb(struct file_system_type *fs_type,
                                  int flags,
                                  const char *dev_name,
                                  void *data)
{
  return get_sb_nodev(fs_type, flags, data, lab5fs_fill_sb);
}

/* Superblock operations struct */
static struct super_operations lab5fs_ops = {
  .read_inode = lab5fs_read_inode,
  .put_super = lab5fs_put_super
};
