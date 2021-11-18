#include "chisai-core/fs.h"
#include <fcntl.h>
#include <linux/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "chisai-core/config.h"
#include "utils/assert_.h"
#include "utils/log.h"

static unsigned long BLKGRP_SIZE = 0;

static inline bool fs_root_exist(filesystem_t *fs)
{
    if (blkgrps_inode_exist(fs->blk_grps, ROOT_INODE))
        return true;
    return false;
}

static chisai_size_t fs_inode_alloc(filesystem_t *fs)
{
    chisai_ssize_t inode_idx = blkgrps_inode_alloc(fs->blk_grps);
    if (inode_idx <= 0)
        die("Failed to allocate free inode, return %d\n", inode_idx);
    fs->sb.free_inodes--;

    return inode_idx;
}

static chisai_size_t fs_data_alloc(filesystem_t *fs)
{
    chisai_ssize_t data_idx = blkgrps_data_alloc(fs->blk_grps);
    if (data_idx <= 0)
        die("Failed to allocate free data, return %d\n", data_idx);
    fs->sb.free_blocks--;

    return data_idx;
}

static void fs_save_inode(filesystem_t *fs,
                          inode_t *inode,
                          chisai_size_t inode_idx)
{
    // find the offset of inode in block device
    unsigned int grp_idx = (inode_idx - 1) / fs->sb.data_block_per_groups;
    unsigned int bitvec_idx = (inode_idx - 1) % fs->sb.data_block_per_groups;
    unsigned int blk_size = fs->sb.block_size;

    size_t offset = blk_size +              /* super block */
                    grp_idx * BLKGRP_SIZE + /* previous block group */
                    2 * blk_size +          /* inode bitmap and data bitmap */
                    INODE_SIZE * bitvec_idx;

    inode_save(inode, &fs->d, offset);
}

static void fs_create_root(filesystem_t *fs)
{
    // TODO: create root directory for the first time mounting filesystem

    inode_t root_inode;
    inode_set_mode(&root_inode, S_IFDIR | 0777);
    inode_set_nlink(&root_inode, 2);  // the two links include . and ..

    // before we allocate inode, we preserved inode number 1 for bad block
    chisai_size_t inode_idx = fs_inode_alloc(fs);
    assert_eq(inode_idx, BADBLK_INODE);

    inode_idx = fs_inode_alloc(fs);
    assert_eq(inode_idx, ROOT_INODE);

    chisai_size_t data_idx = fs_data_alloc(fs);
    assert_eq(data_idx, 1);
    inode_add_block(&root_inode, data_idx);

    fs_save_inode(fs, &root_inode, inode_idx);

    info("FS_ROOT_CREATE DONE\n");
}

void fs_init(filesystem_t *fs, device_t *d)
{
    assert_le(sizeof(inode_t), INODE_SIZE);
    memcpy(&fs->d, d, sizeof(device_t));
    // load back the superblock
    superblock_load(&fs->sb, d);

    assert_eq(BLKGRP_SIZE, 0);
    unsigned int blk_size = fs->sb.block_size;
    BLKGRP_SIZE =
        (2 * blk_size) + (blk_size * BYTE_BITS) * (INODE_SIZE + blk_size);

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
