#ifndef FS_H
#define FS_H

#include "chisai-core/block_group.h"
#include "chisai-core/device.h"
#include "chisai-core/superblock.h"

typedef struct filesystem filesystem_t;
struct filesystem {
    int device_fd;
    superblock_t sb;
    block_group_t *blk_grps;
    device_t d;
};

void fs_init(filesystem_t *fs, device_t *d);
void fs_destroy(filesystem_t *fs);
#endif
