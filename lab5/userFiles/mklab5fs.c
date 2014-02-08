#include <linux/fs.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "/usr/src/kernels/linux-2.6.9lab5/fs/lab5fs/lab5fs.h"

lab5fs_ino create_inode(const char *fname, int dir, int sblock, int eblock)
{
    struct passwd *user_pw;
    struct timespec ctime;
    lab5fs_ino i;
    user_pw = getpwnam((const char *) getlogin());

    if (!user_pw) {
        printf("ERROR: Could not get user information\n"); 
        exit(1);
    }

    ctime.tv_sec = time(NULL);
    ctime.tv_nsec = 0;

    i.i_uid  = user_pw->pw_uid;
    i.i_gid  = user_pw->pw_gid;
    if (dir == 1) {
        i.i_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    } else {
        i.i_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    }
    i.i_atime = ctime;
    i.i_mtime = ctime;
    i.i_ctime = ctime;
    i.i_sblock = sblock;
    i.i_eblock = eblock;
    i.blocks = (sblock == 0) ? 0 : 1;
    i.is_hard_link     = 0;
    i.block_to_link_to = 0;
    strcpy(i.name, fname);

    return i;
}

int main(int argc, char *argv[]) {
    char *c = "This is a test\0";
    int len = strlen(c);
    char *fn = NULL;
    FILE *file = NULL;
    lab5fs_sb sb;
    lab5fs_ino root_inode, f1, f2, f3;

    /* Determine output location (file) */
    if (argc != 2) {
        printf("ERROR: ./mklab5fs <filename>\n");
        exit(1);
    }

    fn = argv[1];
    file = fopen(fn, "r+");

    /* Write superblock */
    sb.magic_num = LAB5FS_SB_MAGIC_NUM;
    sb.blocksize = 512;
    sb.blocksize_bits = 9;
    sb.max_bytes = sb.blocksize;
    sb.inode_bitmap_loc = BLOCK_N(1, sb.blocksize);
    sb.data_bitmap_loc = BLOCK_N(2, sb.blocksize);
    sb.inode_loc = BLOCK_N(3, sb.blocksize);
    sb.data_loc = BLOCK_N(129, sb.blocksize);
    sb.root_inode_loc = sb.inode_loc;
    sb.blocks_total = 2441;
    sb.inode_blocks_total = 1250;
    sb.data_blocks_total  = sb.blocks_total - sb.inode_blocks_total;
    sb.data_blocks_free   = sb.data_blocks_total;
    sb.inode_blocks_free  = sb.inode_blocks_total - 1;
    sb.last_data_block = 0;
    fwrite(&sb, 1, sizeof(lab5fs_sb), file);

    /* Inode bitmap */
    fseek(file, BLOCK_N(1, sb.blocksize), SEEK_SET);
    fprintf(file, "%c", 1);

    /* Data bitmap */
    //fseek(file, BLOCK_N(2, sb.blocksize), SEEK_SET);
    //fprintf(file, "%c%c%c%c", 0, 1, 1, 1);

    /* Root inode */
    root_inode = create_inode("/", 1, 0, 0);
    fseek(file, INODE_N(0, sb.blocksize), SEEK_SET);
    fwrite(&root_inode, 1, sizeof(lab5fs_ino), file);

    /* Testing files */
    /*f1 = create_inode("foo.txt", 0, 1, 1);
    fseek(file, INODE_N(1, sb.blocksize), SEEK_SET);
    fwrite(&f1, 1, sizeof(lab5fs_ino), file);

    fseek(file, BLOCK_N(130, sb.blocksize), SEEK_SET);
    fwrite(c, 1, len, file);

    f2 = create_inode("bar.txt", 0, 2, 2);
    fseek(file, INODE_N(2, sb.blocksize), SEEK_SET);
    fwrite(&f2, 1, sizeof(lab5fs_ino), file);

    f3 = create_inode("biz.txt", 0, 3, 3);
    fseek(file, INODE_N(4, sb.blocksize), SEEK_SET);
    fwrite(&f3, 1, sizeof(lab5fs_ino), file);*/

    /* Close out file */
    //fseek(file, 10240, SEEK_SET);
    //fprintf(file, "%c", 0);
    fclose(file);
    return 0;
}
