#include "chisai-core/fs.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils/log.h"

void fs_init(filesystem_t *fs, int fd)
{
    fs->device_fd = fd;
    // load back the superblock
    superblock_load(&fs->sb, fd);

    // load back the block group metadata
    // TODO: the allocated memory should be reclaimed elsewhere
    fs->blk_grps = malloc(sizeof(block_group_t) * fs->sb.groups);
    block_group_load(fs->blk_grps, fd, fs->sb.block_size, fs->sb.groups);

    // check the magic number in superblock
    if (fs->sb.magic != MAGIC)
        die("Magic number=%x in superblock is not expected=%x!\n", fs->sb.magic,
            MAGIC);

    info("FS_INIT DONE\n");
}
