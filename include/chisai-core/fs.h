#ifndef FS_H
#define FS_H

#include "chisai-core/block_group.h"
#include "chisai-core/inode.h"
#include "chisai-core/superblock.h"

typedef struct filesystem filesystem_t;
struct filesystem {
    int device_fd;
    superblock_t sb;
    block_group_t *blk_grps;
};

void fs_init(filesystem_t *fs, int fd);

#endif
