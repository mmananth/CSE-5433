#include <linux/module.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include "lab5fs.h"
#include <linux/namei.h>

#define ROOT_INO 0
static struct super_operations lab5fs_ops;
static struct file_operations lab5fs_file_operations;
static struct file_operations lab5fs_dir_operations;
static struct inode_operations lab5fs_inode_operations;
static struct address_space_operations lab5fs_aspace_operations;
static struct block_device *g_bdev;
static lab5fs_sb* l5sb;
static struct super_block *g_sb;

/* Define error conditions */
typedef enum _filesystem_errors
{
  FILE_SYSTEM_OUT_OF_RANGE_ERROR = -EIO,
  FILE_SYSTEM_BLOCKSIZE_ERROR = -EIO,
  FILE_SYSTEM_OUT_OF_SPACE = -ENOSPC,
  FILE_SYSTEM_NO_ENTRY_ERROR = -ENOENT
} FILESYSTEM_ERROR;

inline unsigned long ino_num_to_blk_num(ino_t ino_num) {
  return (l5sb->inode_loc / l5sb->blocksize) + ino_num;
}

/* Superblock operations */
void lab5fs_read_inode(struct inode *inode)
{
  printk(KERN_INFO "********Inside lab5fs_read_inode********\n");
  ino_t ino = inode->i_ino;
  unsigned long block_addr = ino_num_to_blk_num(ino);
  struct buffer_head *bh = NULL;
  lab5fs_ino *inode_temp = NULL;

  if(!inode)
     printk(KERN_INFO "inode is null\n");
  printk(KERN_INFO "Read inode num -> blk = %lu\n", block_addr);
  printk(KERN_INFO "blocksize = %lu\n", l5sb->blocksize);

  /* Read at given block address */
  if (!g_bdev) { printk(KERN_INFO "BLOCK DEV IS NULL!\n"); }
  bh = __bread(g_bdev, block_addr, l5sb->blocksize);
  if (!bh) { printk(KERN_INFO "BUFFER HEAD IS NULL!\n"); }
  inode_temp = (lab5fs_ino *) bh->b_data;
  
  if (!inode_temp) { printk(KERN_INFO "INODE TEMP IS NULL!\n"); }
  printk(KERN_INFO "Filling out inode info...\n");
  inode->i_mode = inode_temp->i_mode;
  inode->i_uid = inode_temp->i_uid;
  inode->i_gid = inode_temp->i_gid;
  inode->i_atime = inode_temp->i_atime; 
  inode->i_mtime = inode_temp->i_mtime;
  inode->i_ctime = inode_temp->i_ctime;
  printk(KERN_INFO "Inode temp has %d blocks\n", inode_temp->blocks);
  inode->i_blocks = inode_temp->blocks;
  inode->i_size = inode_temp->size;
  inode->i_blksize = inode->i_sb->s_blocksize;
  inode->i_nlink = 1;  // temp
  inode->i_fop = &lab5fs_file_operations;
  inode->i_op = &lab5fs_inode_operations;
  inode->i_mapping->a_ops = &lab5fs_aspace_operations;

  /* Set up defaults */
  /*inode->i_mode = S_IRUGO|S_IWUSR;
  inode->i_nlink = 1;
  inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;*/

  if (ino == ROOT_INO) {
    printk(KERN_INFO "lab5fs reading root\n");
    inode->i_mode |= S_IFDIR;
    inode->i_nlink = 2;
    inode->i_fop = &lab5fs_dir_operations;
    inode->i_size = l5sb->blocksize;
    inode->i_blocks = 1;
    inode->i_bytes = l5sb->blocksize;
  }
  // FIXME:
  //brelse(bh);
} /* read_inode */

int lab5fs_write_inode(struct inode *inode, int unused)
{
  printk(KERN_INFO "********Inside lab5fs_write_inode********\n");
  // Read inode from disk and update
  printk(KERN_INFO "PRE: Writing inode\n");
  int ino_num = inode->i_ino;
  struct buffer_head *bh = __bread(g_bdev, ino_num+3, l5sb->blocksize);
  lab5fs_ino *ondiskino = (lab5fs_ino *) bh->b_data;


  printk(KERN_INFO "Writing inode %d!\n", ino_num);
  ondiskino->i_uid = inode->i_uid;
  ondiskino->i_gid = inode->i_gid;
  ondiskino->i_mode = inode->i_mode;
  printk(KERN_INFO "inodeondiskblks %d\n", ondiskino->blocks);
  printk(KERN_INFO "inodeblks %d\n", inode->i_blocks);
  ondiskino->blocks = inode->i_blocks;
  ondiskino->size = inode->i_size;
  ondiskino->i_atime = inode->i_atime;
  ondiskino->i_ctime = inode->i_ctime;
  ondiskino->i_mtime = inode->i_mtime;
  //ondiskino->name = inode->name;  //FIXME:would be strcpy
  mark_buffer_dirty_inode(bh, inode);
  printk(KERN_INFO "Done marking as dirty\n");

  brelse(bh);
  return 0;
} /* write_inode */

/* Superblock functions */
static void lab5fs_put_super(struct super_block *sb)
{
  printk(KERN_INFO "********Inside put_super*********\n");
  //kfree(l5sb);
}

int lab5fs_fill_sb(struct super_block *sb,
                   void *data,
                   int silent)
{
  printk(KERN_INFO "********Inside lab5fs_fill_sb********\n");
  struct inode *root;

  /* Read in superblock from device */
  struct buffer_head *bh = sb_bread(sb, 0);
  l5sb = (lab5fs_sb*) bh->b_data;
  //memcpy(l5sb, bh->b_data, sizeof(lab5fs_sb));

  /* Set global block device */
  g_bdev = sb->s_bdev;

  if (!sb_set_blocksize(sb, l5sb->blocksize)){
    printk(KERN_INFO "Cannot set device to size\n");
    return FILE_SYSTEM_BLOCKSIZE_ERROR; 
  }

  if (l5sb) {
    printk(KERN_INFO "Read lab5fs superblock...\n");
    printk(KERN_INFO "Magic number of device: 0x%lX\n", l5sb->magic_num);
    printk(KERN_INFO "Blocksize of device: %lu\n", l5sb->blocksize);
  }

  /* Set sizes, etc. */
  sb->s_op = &lab5fs_ops;
  sb->s_blocksize = l5sb->blocksize;
  sb->s_blocksize_bits = l5sb->blocksize_bits;
  sb->s_maxbytes = l5sb->max_bytes;
  sb->s_magic = l5sb->magic_num;

  /* Get root / */
  root = iget(sb, ROOT_INO);
  sb->s_root = d_alloc_root(root);
  if (!sb->s_root) {
    printk(KERN_INFO "lab5fs could not allocate root\n");
    return -EINVAL;
  }

  brelse(bh);
  return 0;
}

struct super_block *lab5fs_get_sb(struct file_system_type *fs_type,
                                  int flags,
                                  const char *dev_name,
                                  void *data)
{
  printk(KERN_INFO "********Inside lab5fs_get_sb********\n");
  struct super_block *sb = NULL;
  sb = get_sb_bdev(fs_type, flags, dev_name, data, lab5fs_fill_sb);
  g_sb = sb;
  return sb;
}

/* Superblock operations struct */
static struct super_operations lab5fs_ops = {
  .read_inode = lab5fs_read_inode,
  .write_inode = lab5fs_write_inode,
  .put_super = lab5fs_put_super
};

/* Directory operations */
int lab5fs_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
  printk(KERN_INFO "********Inside lab5fs_readdir********\n");
  printk(KERN_INFO "READING FROM %d\n", filp->f_pos);
  int retval = 0, i, inode_index = 0;
  char *map = NULL;
  struct inode *in;
  struct dentry *de = filp->f_dentry;
  struct inode *dir = filp->f_dentry->d_inode;
  struct buffer_head *bh = NULL;
  lab5fs_ino *l5inode = NULL;

  /* Check bounds of directory */
  if (filp->f_pos > (l5sb->inode_blocks_total - l5sb->inode_blocks_free)) {
    printk(KERN_INFO "At end of directory\n");
    return 0;
  }

  /* Read . or .. from directory entry */
  if (filp->f_pos == 0) {
    /* . dir */
    printk(KERN_INFO "Getting .\n");
    retval = filldir(dirent, ".", 1, filp->f_pos, 19, DT_DIR);//dir->i_ino, DT_DIR);
    printk(KERN_INFO "Inode num %d...\n", dir->i_ino);
    //if (retval < 0) goto done;
    filp->f_pos++;
    goto done;
  } 
  if (filp->f_pos == 1) {
    /* .. dir */
    printk(KERN_INFO "Getting ..\n");
    retval = filldir(dirent, "..", 2, filp->f_pos,
                     20, DT_DIR);//de->d_parent->d_inode->i_ino, DT_DIR);
    printk(KERN_INFO "Inode num %d...\n", de->d_parent->d_inode->i_ino);
    //if (retval < 0) goto done;
    filp->f_pos++;
    goto done;
  }

  /* Loop through other files */
  bh = __bread(g_bdev, 1, l5sb->blocksize);
  map = (char *) bh->b_data;

  for (i = 1; i < l5sb->blocksize; i++) {
    char is_inode_used = map[i];

    if (is_inode_used) {
      inode_index++;
      printk(KERN_INFO "inode_index (used) = %d\n", inode_index);

      if (inode_index == filp->f_pos - 1) {
        printk(KERN_INFO "inode_index == fp\n");
        break;
      }
    } else {
      printk(KERN_INFO "inode_index not used @ %d\n", inode_index);
    }
  }

  brelse(bh);

  int block_num = ino_num_to_blk_num(i);
  printk(KERN_INFO "Reading from block %d\n", block_num);

  bh = __bread(g_bdev, block_num, l5sb->blocksize);
  l5inode = (lab5fs_ino *) bh->b_data;

  in = (struct inode *) kmalloc(sizeof(struct inode), GFP_KERNEL);
  in->i_ino = filp->f_pos + 2; //FIXME
  printk(KERN_INFO "Inode num %d...\n", in->i_ino);
  printk(KERN_INFO "MODE: %lu\n", l5inode->i_mode);
  in->i_mode = l5inode->i_mode;
  in->i_uid = l5inode->i_uid;
  in->i_gid = l5inode->i_gid;
  in->i_atime = l5inode->i_atime; 
  in->i_mtime = l5inode->i_mtime;
  in->i_ctime = l5inode->i_ctime;
  printk(KERN_INFO "CTIME: %lu\n", l5inode->i_ctime);
  in->i_blocks = l5inode->blocks;
  in->i_size = l5inode->size;
  in->i_blksize = l5sb->blocksize;
  in->i_nlink = 1;  // temp
  in->i_op = &lab5fs_inode_operations;
  in->i_fop = &lab5fs_dir_operations;
  //in->i_mapping->a_ops = &lab5fs_aspace_operations;
  brelse(bh);

  printk(KERN_INFO "About to do filldir\n");
  retval = filldir(dirent, l5inode->name, 256, filp->f_pos, in->i_ino, DT_REG);
  printk(KERN_INFO "Updating filepos\n");
  filp->f_pos++;

 done:
  /* Update access time */
  filp->f_version = dir->i_version;
  update_atime(dir);
  printk(KERN_INFO "Going to return %d...\n", retval);
  return 0;
} /* readdir */

int lab5fs_fsync(struct file *file, struct dentry *de, int datasync)
{
  printk(KERN_INFO "************* fsync **************\n");
  simple_sync_file(file, de, datasync);
  /*int err;
  struct inode *inode = de->d_inode;
  printk(KERN_INFO "************* fsync **************\n");
  if (!(inode->i_state & I_DIRTY)) {
    printk(KERN_INFO "Returning - inode isn't dirty at all\n");
    return err;
  }
  if (datasync && !(inode->i_state & I_DIRTY_DATASYNC)) {
    printk(KERN_INFO "Returning - inode isn't dirty enough\n");
    return err;
  }
  printk(KERN_INFO "Writing\n");
  err |= lab5fs_write_inode(inode, 1);
  return err ? -EIO : 0;*/
}

static int lab5fs_link(struct dentry *old, struct inode *dir, struct dentry *new)
{
  printk(KERN_INFO "********* lab5fs_link ***********\n");

  /* Get old inode */
  struct inode *inode = old->d_inode;
  char *name = new->d_name.name;
  int name_len = new->d_name.len;

  /* Find free bit for new inode */
  printk(KERN_INFO "Finding bit to assign inum...\n");
  struct buffer_head *bh = __bread(g_bdev, 1, l5sb->blocksize);
  char *map = (char*) bh->b_data;
  int ino_num = find_first_free_index(map);

  if (ino_num <= 0) {
    printk(KERN_INFO "Couldn't get a free bit\n");
    return -ENOSPC;
  }

  map[ino_num] = 1;
  mark_buffer_dirty(bh);
  brelse(bh);

  /* Create a new inode */
  printk(KERN_INFO "Creating new inode...\n");
  bh = __bread(g_bdev, 3+ino_num, l5sb->blocksize);
  struct lab5fs_ino *new_ino = (lab5fs_ino*) bh->b_data;
  new_ino->i_uid = inode->i_uid;
  new_ino->i_gid = inode->i_gid;
  new_ino->i_mode = inode->i_mode;
  new_ino->blocks = inode->i_blocks;
  new_ino->size = inode->i_size;
  new_ino->i_atime = CURRENT_TIME;
  new_ino->i_mtime = CURRENT_TIME;
  new_ino->i_ctime = CURRENT_TIME;
  strcpy(new_ino->name, name);
  new_ino->is_hard_link = 1;
  new_ino->block_to_link_to = (DATA_N(ino_num, l5sb->blocksize)/l5sb->blocksize) - 1;
  printk(KERN_INFO "BLock to link to: %lu\n", new_ino->block_to_link_to);
  mark_buffer_dirty(bh);
  brelse(bh);

  /* Read inode */
  /* Maybe not? */

  /* Modify free count in sb */
  bh = __bread(g_bdev, 0, l5sb->blocksize);
  lab5fs_sb *sb = (lab5fs_sb *) bh->b_data;
  sb->inode_blocks_free--;
  mark_buffer_dirty(bh);
  memcpy(l5sb, sb, sizeof(lab5fs_sb));
  brelse(bh);

  /* Update counts, etc. */
  inode->i_nlink++;
  inode->i_ctime = CURRENT_TIME;
  mark_inode_dirty(inode);
  atomic_inc(&inode->i_count);
  d_instantiate(new, inode);
  return 0;
}

static struct file_operations lab5fs_file_operations = {
  .llseek = generic_file_llseek,
  .read = generic_file_read,
  .write = generic_file_write,
  .mmap = generic_file_mmap,
  .fsync = lab5fs_fsync,//lab5fs_fsync,
};

static struct file_operations lab5fs_dir_operations = {
  .open = dcache_dir_open,
  .release = dcache_dir_close,
  .llseek = dcache_dir_lseek,
  .read = generic_read_dir,
  .readdir = lab5fs_readdir,
  .fsync = lab5fs_fsync,//lab5fs_fsync,
};

struct dentry *lab5fs_lookup(struct inode *dir, struct dentry *dentry,
                             struct nameidata *nd) 
{
  printk(KERN_INFO "********Inside lab5fs_lookup ********\n");
  struct buffer_head *bh = __bread(g_bdev, 1, l5sb->blocksize);
  char *map = (char *) bh->b_data;
  int i;
  struct inode *_inode = NULL;

  for (i = 0; i < l5sb->blocksize; i++) {
    int block = 3 + i;    
    int is_valid = (map[i] == 1);

    if (is_valid) {
      struct buffer_head *bh2 = __bread(g_bdev, block, l5sb->blocksize);
      lab5fs_ino *ino = (lab5fs_ino *) bh2->b_data;

      if (strcmp(ino->name, dentry->d_iname) == 0) {
        /* Found file */
        _inode = iget(dir->i_sb, i);

        if (!_inode) {
          return ERR_PTR(-EACCES);
        }

        d_add(dentry, _inode);
        break;
      }
    }
  }

  return NULL;
}

int find_first_free_index_data(const char *map)
{
  /* Look for free bit */
  int i;
  for (i = 1; i < l5sb->blocksize; i++) {
    char free = map[i];
    if (free == 0) {
      /* Found free bit return index */
      return i;
    }
  }
  /* Could not find free bit */
  return -1;
}

int find_first_free_index(const char *map)
{
  /* Look for free bit */
  int i;
  for (i = 0; i < l5sb->blocksize; i++) {
    char free = map[i];
    if (free == 0) {
      /* Found free bit return index */
      return i;
    }
  }
  /* Could not find free bit */
  return -1;
}

int lab5fs_unlink(struct inode *dir, struct dentry *dentry)
{
  printk(KERN_INFO "********Inside lab5fs_unlink********\n");
  struct inode *inode = dentry->d_inode;
  int index = inode->i_ino;
  lab5fs_sb *sb = NULL;

  printk(KERN_INFO "Reading inode map...\n");
  struct buffer_head *bh = __bread(g_bdev, 1, l5sb->blocksize);
  char *map = (char*) bh->b_data;

  /* Free entry in map */
  if (map[index] == 0) {
    printk(KERN_INFO "Trying to free unused block!!! ERROR!!\n");
    return FILE_SYSTEM_NO_ENTRY_ERROR;
  }
  printk(KERN_INFO "Marking inode free...\n");
  map[index] = 0;
  mark_buffer_dirty(bh);
  brelse(bh);

  /* Free data blocks if need be */
  if (inode->i_nlink == 1) {
    printk(KERN_INFO "Removing data blocks as well...\n");
    printk(KERN_INFO "DB %lu being cleared...\n", 4+index);
    bh = __bread(g_bdev, 4+index, l5sb->blocksize);
    memset(bh->b_data, 0, l5sb->blocksize);
    mark_buffer_dirty(bh);
    brelse(bh);
  }
  
  //inode->i_nlink--;
  //printk(KERN_INFO "Number of links is %d\n", inode->i_nlink);

  /* Zero out inode */
  printk(KERN_INFO "Zeroing inode on disk...\n");
  bh = __bread(g_bdev, 3+index, l5sb->blocksize);
  memset(bh->b_data, 0, l5sb->blocksize);
  mark_buffer_dirty(bh);
  brelse(bh);

  /* Update free inodes */
  printk(KERN_INFO "Updating free inode count...\n");
  bh = __bread(g_bdev, 0, l5sb->blocksize);
  sb = (lab5fs_sb*) bh->b_data;
  sb->inode_blocks_free++;
  memcpy(l5sb, sb, sizeof(lab5fs_sb));
  mark_buffer_dirty(bh);
  brelse(bh);

  return 0;
}

int lab5fs_create(struct inode *inode, struct dentry *dentry,
              int mode, struct nameidata *nd)
{
  printk(KERN_INFO "********Inside lab5fs_create********\n");
  struct inode *new_ino = NULL;
  lab5fs_sb *sb = NULL;
  lab5fs_ino ino_meta;
  int ino_num = -1;
  struct buffer_head *bh = NULL;
  struct buffer_head *bh_meta = NULL;
  char *map = NULL;

  /* Find first place open in bitmap for inode */
  printk(KERN_INFO "Finding bit to assign inum...\n");
  bh = __bread(g_bdev, 1, l5sb->blocksize);
  map = (char*) bh->b_data;
  ino_num = find_first_free_index(map);

  if (ino_num <= 0) {
    printk(KERN_INFO "Couldn't get a free bit\n");
    return -ENOSPC;
  }

  /* Create a new inode */
  printk(KERN_INFO "Creating new inode...\n");
  new_ino = new_inode(g_sb);

  if (!new_ino) {
    printk(KERN_INFO "Couldn't create new inode\n");
    return -ENOMEM;
  }

  new_ino->i_ino = ino_num;
  new_ino->i_mode = mode;
  new_ino->i_blksize = l5sb->blocksize;
  new_ino->i_sb = g_sb;
  new_ino->i_uid= current->fsuid;
  new_ino->i_gid = current->fsgid;
  new_ino->i_mtime = new_ino->i_ctime = new_ino->i_atime = CURRENT_TIME;
  new_ino->i_blkbits = 9;

  if (S_ISREG(new_ino->i_mode)) {
    new_ino->i_op = &lab5fs_inode_operations;
    new_ino->i_fop = &lab5fs_file_operations;
    new_ino->i_mapping->a_ops = &lab5fs_aspace_operations;
    new_ino->i_mode |= S_IFREG;
  } else if (S_ISDIR(new_ino->i_mode)) {
    new_ino->i_op = &lab5fs_inode_operations;
    new_ino->i_fop = &lab5fs_dir_operations;
    new_ino->i_mode |= S_IFDIR;
  }

  /* Create new inode metadata on disk */
  printk(KERN_INFO "File name is %s\n", nd->last.name);

  bh_meta = __bread(g_bdev, ino_num+3, l5sb->blocksize);
  strcpy(ino_meta.name, nd->last.name);
  ino_meta.block_to_link_to = 0;
  ino_meta.is_hard_link = 0;
  ino_meta.i_mode = mode;
  memcpy(bh_meta->b_data, &ino_meta, sizeof(lab5fs_ino));
  mark_buffer_dirty(bh_meta);
  brelse(bh_meta);

  /* Mark bit in bitmap as now-used */
  printk(KERN_INFO "Marking bitmap for inode as used\n");
  map[ino_num] = 1; //FIXME: Look at me
  mark_buffer_dirty(bh);
  brelse(bh);

  /* Mark actual inode as needing to be written to disk */
  printk(KERN_INFO "Marking new inode as need-to-be-written\n");
  printk(KERN_INFO "insert to hash\n");
  insert_inode_hash(new_ino);
  printk(KERN_INFO "mark dirty\n");
  mark_inode_dirty(new_ino);
  printk(KERN_INFO "done marking dirty...\n");

  /* Modify free count in sb */
  bh = __bread(g_bdev, 0, l5sb->blocksize);
  sb = (lab5fs_sb *) bh->b_data;
  sb->inode_blocks_free--;
  mark_buffer_dirty(bh);
  memcpy(l5sb, sb, sizeof(lab5fs_sb));
  brelse(bh);

  d_instantiate(dentry, new_ino);
  printk(KERN_INFO "after instantiate...\n");
  return 0;
} /* lab5fs_create */

int lab5fs_get_block(struct inode *ino, sector_t block_offset,
		     struct buffer_head *bh_result, int create)
{
  printk(KERN_INFO "******* get_block *********\n");
  printk(KERN_INFO "BO: %lu\n", block_offset);
  printk(KERN_INFO "DATA LOC: %lu\n", DATA_N(ino->i_ino, l5sb->blocksize)/l5sb->blocksize);

  struct buffer_head *bh = __bread(g_bdev, 3+ino->i_ino, l5sb->blocksize);
  lab5fs_ino *_ino = (lab5fs_ino*) bh->b_data;

  printk(KERN_INFO "Is hard link: %d\n", (_ino->is_hard_link));
  if (_ino->is_hard_link) {
     printk(KERN_INFO "Is hard link!!!! -- btlt: %d\n", _ino->block_to_link_to);
     map_bh(bh_result, g_sb, _ino->block_to_link_to);
     brelse(bh);
     return 0;
  }

  if (block_offset == 0) {
     map_bh(bh_result, g_sb, DATA_N(ino->i_ino, l5sb->blocksize)/l5sb->blocksize);
     return 0;
  }

  return FILE_SYSTEM_OUT_OF_SPACE;
}


/*int stamfs_get_block(struct inode *ino, long block_offset,
                     struct buffer_head *bh_result, int create)
{
  int err = 0;
  int block_num = -1;

  err = lab5fs_inode_block_offset_to_number(ino,block_offset, &block_num);
  if (err)
    goto ret_err;

  if (block_num != -1) {
     bh_result->b_dev = ino->i_dev;
     bh_result->b_blocknr = block_num;
     bh_result->b_state |= (1UL << BH_Mapped);
     goto ret;
  }

  if (create == 0) {
     goto ret;
  }

  block_num = lab5fs_alloc_block(ino->i_sb);
  if (block_num == 0) {
     err = -ENOSPC;
     goto ret_err;
  }

  err = lab5fs_inode_map_block_offset_to_number(ino,
					      block_offset,
					      block_num);
  if (err) {
     goto ret_err;
  }

  bh_result->b_dev = ino->i_dev;
  bh_result->b_blocknr = block_num;
  bh_result->b_state |= (1UL << BH_New) | (1UL << BH_Mapped);

  err = 0;
  goto ret;

  ret_err:
  if (block_num > 0)
    lab5fs_release_block(ino->i_sb, block_num);

  ret:
    return err;
}*/


static int lab5fs_writepage(struct page *page, struct writeback_control *wbc)
{
  printk(KERN_INFO "******** lab5fs_writepage **********\n");
  return block_write_full_page(page, lab5fs_get_block, wbc);
}

static int lab5fs_readpage(struct file *file, struct page *page)
{
  printk(KERN_INFO "******** lab5fs_readpage **********\n");
  return block_read_full_page(page, lab5fs_get_block);
}

static sector_t lab5fs_bmap(struct address_space *mapping, sector_t block)
{
  printk(KERN_INFO "******** lab5fs_bmap **********\n");
  return generic_block_bmap(mapping, block, lab5fs_get_block);
}

static int lab5fs_prepare_write(struct file *file, struct page *page,
                                unsigned from, unsigned to)
{
  printk(KERN_INFO "******** lab5fs_prepare_write **********\n");
  return block_prepare_write(page, from, to, lab5fs_get_block);
}

/* Declare inode operations */
static struct inode_operations lab5fs_inode_operations = {
  .link = lab5fs_link,
  .create = lab5fs_create,
  .unlink = lab5fs_unlink,
  .getattr = simple_getattr,
  .lookup = lab5fs_lookup,
};

/* Declare address space operations */
static struct address_space_operations lab5fs_aspace_operations = {
  .readpage = lab5fs_readpage,
  .writepage = lab5fs_writepage,
  .sync_page = block_sync_page,
  .bmap = lab5fs_bmap,
  .commit_write = generic_commit_write,
  .prepare_write = lab5fs_prepare_write,
};

/* Declare file system type */
static struct file_system_type lab5fs_type = {
  .name = "lab5fs",
  .fs_flags = FS_REQUIRES_DEV,
  .get_sb = lab5fs_get_sb,
  .kill_sb = kill_block_super,
  .owner = THIS_MODULE
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
