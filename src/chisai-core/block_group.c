#include "chisai-core/block_group.h"
#include <assert.h>
#include "chisai-core/config.h"
#include "utils/log.h"

static inline void blkgrp_update_next_inode(block_group_t *blk_grp)
{
    /* NOTE: ffs(x) will return one plus the index of the least significant
     * 1-bit of x,
     * so we don't need to add one here for the 1-based inode index */
    blk_grp->next_inode = bitvec_find_first_set(&blk_grp->inode_bitmap);
}

static inline void blkgrp_update_next_data(block_group_t *blk_grp)
{
    /* NOTE: ffs(x) will return one plus the index of the least significant
     * 1-bit of x,
     * so we don't need to add one here for the 1-based inode index */
    blk_grp->next_data = bitvec_find_first_set(&blk_grp->data_bitmap);
}

static inline chisai_size_t blkgrp_inode_alloc(block_group_t *blk_grp)
{
    chisai_size_t idx = blk_grp->next_inode;
    bitvec_set(&blk_grp->inode_bitmap, idx - 1);
    blkgrp_update_next_inode(blk_grp);
    return idx;
}

static inline chisai_size_t blkgrp_data_alloc(block_group_t *blk_grp)
{
    chisai_size_t idx = blk_grp->next_data;
    bitvec_set(&blk_grp->data_bitmap, idx - 1);
    blkgrp_update_next_data(blk_grp);
    return idx;
}

static inline chisai_size_t blkgrp_free_inode_num(block_group_t *blk_grp)
{
    return bitvec_count_zeros(&blk_grp->inode_bitmap);
}

static inline chisai_size_t blkgrp_free_data_num(block_group_t *blk_grp)
{
    return bitvec_count_zeros(&blk_grp->data_bitmap);
}

void blkgrps_load(block_group_t *blk_grps,
                  device_t *d,
                  unsigned int blk_size,
                  unsigned int groups)
{
    blk_grps->groups = groups;
    blk_grps->blk_inode_num = blk_size * BYTE_BITS;
    blk_grps->blkgrp_size =
        (2 * blk_size) + (blk_size * BYTE_BITS) * (INODE_SIZE + blk_size);

    for (unsigned int i = 0; i < groups; i++) {
        // Remember that one block is reserved for superblock
        size_t off = blk_size + blk_grps->blkgrp_size * i;

        // 1. read the data bitmap
        bitvec_init(&blk_grps[i].data_bitmap, blk_size);
        ssize_t ret = d->read(d, off, blk_grps[i].data_bitmap.inner, blk_size);
        if (ret < 0)
            die("Failed to read the block device\n");

        // 2. read the inode bitmap
        bitvec_init(&blk_grps[i].inode_bitmap, blk_size);
        ret = d->read(d, off + blk_size, blk_grps[i].inode_bitmap.inner,
                      blk_size);
        if (ret < 0)
            die("Failed to read the block device\n");

        // 3. update the next index for inode and block allocation
        blkgrp_update_next_data(&blk_grps[i]);
        blkgrp_update_next_inode(&blk_grps[i]);
    }
}

bool blkgrps_inode_exist(block_group_t *blk_grps, chisai_size_t inode_idx)
{
    if (inode_idx == 0)
        return false;

    unsigned int grp_idx = (inode_idx - 1) / blk_grps->blk_inode_num;
    chisai_size_t bitvec_idx = (inode_idx - 1) % blk_grps->blk_inode_num;

    return bitvec_get(&(blk_grps[grp_idx].inode_bitmap), bitvec_idx);
}

bool blkgrps_data_exist(block_group_t *blk_grps, chisai_size_t data_idx)
{
    if (data_idx == 0)
        return false;

    /* remember that we have the same numbers of data block and inode for each
     * group */
    unsigned int grp_idx = (data_idx - 1) / blk_grps->blk_inode_num;
    chisai_size_t bitvec_idx = (data_idx - 1) % blk_grps->blk_inode_num;

    return bitvec_get(&(blk_grps[grp_idx].data_bitmap), bitvec_idx);
}

chisai_size_t blkgrps_inode_alloc(block_group_t *blk_grps)
{
    for (unsigned int i = 0; i < blk_grps->groups; i++) {
        if (blkgrp_free_inode_num(&blk_grps[i]) == 0)
            continue;

        chisai_size_t idx = blkgrp_inode_alloc(&blk_grps[i]);

        return i * blk_grps->blk_inode_num + idx;
    }
    return 0;
}

chisai_size_t blkgrps_data_alloc(block_group_t *blk_grps)
{
    for (unsigned int i = 0; i < blk_grps->groups; i++) {
        if (blkgrp_free_data_num(&blk_grps[i]) == 0)
            continue;

        chisai_size_t idx = blkgrp_data_alloc(&blk_grps[i]);

        return i * blk_grps->blk_inode_num + idx;
    }
    return 0;
}

void blkgrps_inode_release(block_group_t *blk_grps, chisai_size_t inode_idx)
{
    if (inode_idx == 0)
        return;

    unsigned int grp_idx = (inode_idx - 1) / blk_grps->blk_inode_num;
    chisai_size_t bitvec_idx = (inode_idx - 1) % blk_grps->blk_inode_num;

    return bitvec_reset(&(blk_grps[grp_idx].inode_bitmap), bitvec_idx);
}

void blkgrps_data_release(block_group_t *blk_grps, chisai_size_t data_idx)
{
    if (data_idx == 0)
        return;

    unsigned int grp_idx = (data_idx - 1) / blk_grps->blk_inode_num;
    chisai_size_t bitvec_idx = (data_idx - 1) % blk_grps->blk_inode_num;

    return bitvec_reset(&(blk_grps[grp_idx].data_bitmap), bitvec_idx);
}

void blkgrps_destroy(block_group_t *blk_grps)
{
    // TODO: sync block group data back to the device
    for (unsigned int i = 0; i < blk_grps->groups; i++) {
        bitvec_destroy(&blk_grps[i].inode_bitmap);
        bitvec_destroy(&blk_grps[i].data_bitmap);
    }
}
