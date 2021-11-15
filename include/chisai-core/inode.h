#ifndef INODE_H
#define INODE_H

/* The design of inode number can be referenced to:
 * - https://ext4.wiki.kernel.org/index.php/Ext4_Disk_Layout#Special_inodes */

enum inode {
    NO_INODE = 0,
    BADBLK_INODE = 1,
    ROOT_INODE = 2,
};

#endif
