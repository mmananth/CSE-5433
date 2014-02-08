#ifndef _SUPER_H_
#define _SUPER_H_

struct super_block *lab5fs_get_sb(struct file_system_type *fs_type,
                                  int flags,
                                  const char *dev_name,
                                  void *data);

#endif
