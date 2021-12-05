#ifndef INODE_H
#define INODE_H

/* The design of inode number can be referenced to:
 * - https://ext4.wiki.kernel.org/index.php/Ext4_Disk_Layout#Special_inodes */
#include <stddef.h>
#include <sys/stat.h>
#include "chisai-core/config.h"
#include "chisai-core/device.h"

/* At more 12 direct blocks are availible */
#define DIRECT_BLKS_NUM 12

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

    chisai_size_t direct_blks[DIRECT_BLKS_NUM];
    chisai_size_t indirect_blk;
    chisai_size_t double_indirect_blk;

    uid_t uid; /* User ID of owner */
    gid_t gid; /* Group ID of owner */
    unsigned int checksum;
};

void inode_init(inode_t *inode);
void inode_set_mode(inode_t *node, mode_t mode);
void inode_set_nlink(inode_t *node, nlink_t nlink);
void inode_add_block(inode_t *inode, chisai_size_t data_idx);
void inode_save(inode_t *inode, device_t *d, size_t offset);
void inode_load(inode_t *inode, device_t *d, size_t offset);

#endif
