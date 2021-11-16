#ifndef INODE_H
#define INODE_H

/* The design of inode number can be referenced to:
 * - https://ext4.wiki.kernel.org/index.php/Ext4_Disk_Layout#Special_inodes */
#include <stdint.h>
#include <sys/stat.h>

enum inode_numlist {
    NO_INODE = 0,
    BADBLK_INODE = 1,
    ROOT_INODE = 2,
};

/* https://man7.org/linux/man-pages/man2/lstat.2.html */
typedef struct inode inode_t;
struct inode {
    mode_t mode;     /* File type and mode */
    nlink_t nlink;   /* Number of hard links */
    blkcnt_t blkcnt; /* Number of 512B blocks allocated */
    off_t size;      /* Total size, in bytes */

    unsigned long atim; /* Time of last access */
    unsigned long mtim; /* Time of last modification */
    unsigned long ctim; /* Time of last status change */

    /* At more 12 direct blocks are availible */
    unsigned int direct_blks[12];
    int indirect_blk;
    unsigned int double_indirect_blk;

    uid_t uid; /* User ID of owner */
    gid_t gid; /* Group ID of owner */
    unsigned int checksum;
};

void inode_init(inode_t *inode);
#endif
