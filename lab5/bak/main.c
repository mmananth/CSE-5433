#include <linux/module.h>
#include "super.h"
#include "inode.h"

/* Declare file system type */
static struct file_system_type lab5fs_type = {
  .name = "lab5fs",
  .get_sb = lab5fs_get_sb,
  .kill_sb = kill_anon_super /* temp */
};

/* Initialization and exiting of the module */
static int __init init_lab5fs_fs(void)
{
  register_filesystem(&lab5fs_type);
  printk(KERN_INFO "lab5fs module loaded and fs registered\n");
  return 0;
}

static int __exit exit_lab5fs_fs(void)
{
  unregister_filesystem(&lab5fs_type);
  printk(KERN_INFO "lab5fs module unloaded and fs unregistered\n");
  return 0;
}

module_init(init_lab5fs_fs);
module_exit(exit_lab5fs_fs);
