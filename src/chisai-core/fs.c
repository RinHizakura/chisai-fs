#include "chisai-core/fs.h"
#include <fcntl.h>
#include <linux/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils/assert_.h"
#include "utils/log.h"

static inline bool fs_root_exist(filesystem_t *fs)
{
    if (blkgrps_inode_exist(fs->blk_grps, ROOT_INODE))
        return true;
    return false;
}

static unsigned int fs_inode_alloc(filesystem_t *fs)
{
    int inode_idx = blkgrps_inode_alloc(fs->blk_grps);
    if (inode_idx <= 0)
        die("Failed to allocate free inode, return %d\n", inode_idx);
    fs->sb.free_inodes--;

    return inode_idx;
}

static void fs_create_root(filesystem_t *fs)
{
    // TODO: create root directory for the first time mounting filesystem
    inode_t root_inode;
    inode_init(&root_inode);
    root_inode.mode = S_IFDIR | 0777;
    root_inode.nlink = 2;  // . and ..

    // before we allocate inode, we preserved inode number 1 for bad block
    unsigned int inode_idx = fs_inode_alloc(fs);
    assert_eq(inode_idx, BADBLK_INODE);

    inode_idx = fs_inode_alloc(fs);
    assert_eq(inode_idx, ROOT_INODE);

    // fs_data_block_alloc();

    // inode_save(inode, inode_idx);
    info("FS_ROOT_CREATE DONE\n");
}

void fs_init(filesystem_t *fs, device_t *d)
{
    // load back the superblock
    superblock_load(&fs->sb, d);

    // load back the block group metadata
    fs->blk_grps = malloc(sizeof(block_group_t) * fs->sb.groups);
    blkgrps_load(fs->blk_grps, d, fs->sb.block_size, fs->sb.groups);

    // check the magic number in superblock
    if (fs->sb.magic != MAGIC)
        die("Magic number=%x in superblock is not expected=%x!\n", fs->sb.magic,
            MAGIC);

    // create root directory
    if (!fs_root_exist(fs))
        fs_create_root(fs);

    info("FS_INIT DONE\n");
}

void fs_destroy(filesystem_t *fs)
{
    blkgrps_destroy(fs->blk_grps);
    free(fs->blk_grps);
    close(fs->device_fd);
}
