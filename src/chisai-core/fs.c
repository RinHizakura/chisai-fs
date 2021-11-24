#include "chisai-core/fs.h"
#include <fcntl.h>
#include <linux/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "chisai-core/config.h"
#include "chisai-core/dir.h"
#include "chisai-core/inode.h"
#include "utils/assert_.h"
#include "utils/log.h"

static unsigned long BLKGRP_SIZE = 0;

static inline size_t fs_inode_to_offset(filesystem_t *fs,
                                        chisai_size_t inode_idx)
{
    // find the offset of inode in block device
    /* remember that we have the same numbers of data block and inode for each
     * group */
    unsigned int grp_idx = (inode_idx - 1) / fs->sb.data_block_per_groups;
    unsigned int bitvec_idx = (inode_idx - 1) % fs->sb.data_block_per_groups;
    unsigned int blk_size = fs->sb.block_size;

    size_t offset = blk_size +              /* super block */
                    grp_idx * BLKGRP_SIZE + /* previous block group */
                    2 * blk_size +          /* inode bitmap and data bitmap */
                    INODE_SIZE * bitvec_idx;
    return offset;
}

static inline size_t fs_data_to_offset(filesystem_t *fs, chisai_size_t data_idx)
{
    // find the offset of data block in block device
    unsigned int grp_idx = (data_idx - 1) / fs->sb.data_block_per_groups;
    unsigned int bitvec_idx = (data_idx - 1) % fs->sb.data_block_per_groups;
    unsigned int blk_size = fs->sb.block_size;

    size_t offset = blk_size +              /* super block */
                    grp_idx * BLKGRP_SIZE + /* previous block group */
                    2 * blk_size +          /* inode bitmap and data bitmap */
                    INODE_SIZE * fs->sb.data_block_per_groups + /* inodes */
                    blk_size * bitvec_idx;
    return offset;
}

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
    size_t offset = fs_inode_to_offset(fs, inode_idx);
    inode_save(inode, &fs->d, offset);
}


static void fs_save_dir(filesystem_t *fs, dir_t *dir, chisai_size_t data_idx)
{
    size_t offset = fs_data_to_offset(fs, data_idx);
    dir_save(dir, &fs->d, offset);
}

static void fs_create_root(filesystem_t *fs)
{
    // create an inode instance
    inode_t root_inode;
    inode_init(&root_inode);
    inode_set_mode(&root_inode,
                   S_IFDIR | (S_IRWXU | S_IRWXG | S_IRWXO));  // 0777
    inode_set_nlink(&root_inode, 2);  // the two links include . and ..

    // create a directory instance
    dir_t root_dir;
    dir_init(&root_dir);

    // allocate a data block for root, and store the directory information
    chisai_size_t data_idx = fs_data_alloc(fs);
    assert_eq(data_idx, 1);
    fs_save_dir(fs, &root_dir, data_idx);

    // before we allocate inode, we preserved inode number 1 for bad block
    chisai_size_t inode_idx = fs_inode_alloc(fs);
    assert_eq(inode_idx, BADBLK_INODE);

    // FIXME: update inode for the modified time
    /* allocate an inode for root, add the allocated data block to the inode,
     * and store the inode information */
    inode_idx = fs_inode_alloc(fs);
    assert_eq(inode_idx, ROOT_INODE);
    inode_add_block(&root_inode, data_idx);
    fs_save_inode(fs, &root_inode, inode_idx);

    info("FS_ROOT_CREATE DONE\n");
}

static bool fs_find_inode(filesystem_t *fs,
                          chisai_size_t inode_idx,
                          inode_t *inode)
{
    if (!blkgrps_inode_exist(fs->blk_grps, inode_idx))
        return false;

    /* FIXME: some repeat calculation could happen between
     * blkgrp_inode_exist and fs_inode_to_offset */
    size_t offset = fs_inode_to_offset(fs, inode_idx);
    inode_load(inode, &fs->d, offset);
    return true;
}

static chisai_size_t fs_path_to_inode(filesystem_t *fs,
                                      const char *path,
                                      inode_t *inode)
{
    // TODO: support to find inode which doesn't belong to root
    return fs_find_inode(fs, ROOT_INODE, inode) ? ROOT_INODE : 0;
}

void fs_init(filesystem_t *fs, device_t *d)
{
    unsigned int blk_size;
    memcpy(&fs->d, d, sizeof(device_t));

    // load back the superblock
    superblock_load(&fs->sb, d);
    blk_size = fs->sb.block_size;

    assert_eq(BLKGRP_SIZE, 0);
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

    assert_le(sizeof(inode_t), INODE_SIZE);
    assert_le(sizeof(dir_t), blk_size);

    info("FS_INIT DONE\n");
}

int fs_get_metadata(filesystem_t *fs,
                    const char *path,
                    struct chisai_info *info)
{
    inode_t inode;
    chisai_size_t idx = fs_path_to_inode(fs, path, &inode);
    if (idx == NO_INODE)
        return CHISAI_ERR_NOFILE;

    memset(info, 0, sizeof(struct chisai_info));

    info->ino = idx;
    info->mode = inode.mode;
    info->nlink = inode.nlink;
    info->atim = inode.atim;
    info->mtim = inode.mtim;
    info->ctim = inode.ctim;

    info->size = inode.size;
    info->blkcnt = inode.blkcnt;
    info->uid = inode.uid;
    info->gid = inode.gid;

    return CHISAI_ERR_OK;
}

int fs_get_data(filesystem_t *fs,
                struct chisai_dir_info *dir,
                const char *path,
                struct chisai_info *info)
{
    memset(info, 0, sizeof(struct chisai_info));
    // info of current working directory(.)
    if (dir->pos == 0) {
        info->mode = S_IFDIR | (S_IRWXU | S_IRWXG | S_IRWXO);
        strcpy(info->name, ".");
        dir->pos += 1;
        return 1;  // return 1 means we still have next file to read
    }
    // info of parent directory(..)
    else if (dir->pos == 1) {
        info->mode = S_IFDIR | (S_IRWXU | S_IRWXG | S_IRWXO);
        strcpy(info->name, "..");
        dir->pos += 1;
        return 1;
    }
    return CHISAI_ERR_OK;
}

void fs_destroy(filesystem_t *fs)
{
    blkgrps_destroy(fs->blk_grps);
    free(fs->blk_grps);
    close(fs->device_fd);
}
