#include "chisai-core/fs.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils/log.h"

static inline bool fs_root_exist(filesystem_t *fs)
{
    // the first block group contains root
    if (blkgrp_inode_exist(fs->blk_grps, 0, ROOT_INODE))
        return true;
    return false;
}

static void fs_create_root(filesystem_t *fs)
{
    // TODO: create root directory for the first time mounting filesystem
}

void fs_init(filesystem_t *fs, int fd)
{
    fs->device_fd = fd;
    // load back the superblock
    superblock_load(&fs->sb, fd);

    // load back the block group metadata
    // TODO: the allocated memory should be reclaimed elsewhere
    fs->blk_grps = malloc(sizeof(block_group_t) * fs->sb.groups);
    blkgrp_load(fs->blk_grps, fd, fs->sb.block_size, fs->sb.groups);

    // check the magic number in superblock
    if (fs->sb.magic != MAGIC)
        die("Magic number=%x in superblock is not expected=%x!\n", fs->sb.magic,
            MAGIC);

    // create root directory
    if (!fs_root_exist(fs))
        fs_create_root(fs);

    info("FS_INIT DONE\n");
}