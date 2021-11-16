#include "chisai-core/block_group.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "chisai-core/config.h"
#include "utils/log.h"

/* FIXME: We save some values related to the block size
 * to perform block-group-related operations efficiently.
 * Since these values have already been calculated elsewhere
 * in our filesystem. We could preserve the values in a global
 * region instead of making another copies. */
static unsigned int BLK_SIZE = 0;
static unsigned int GROUPS = 0;
static unsigned int BLK_INODE_NUM = 0;
static unsigned long BLKGRP_SIZE = 0;

void blkgrp_load(block_group_t *blk_grps,
                 device_t *d,
                 unsigned int blk_size,
                 unsigned int groups)
{
    // We should only call blkgrp_load once in normal situation
    assert((BLK_SIZE == 0) && (GROUPS == 0) && (BLK_INODE_NUM == 0) &&
           (BLKGRP_SIZE == 0));

    BLK_SIZE = blk_size;
    GROUPS = groups;
    BLK_INODE_NUM = blk_size * BYTE_BITS;
    BLKGRP_SIZE =
        (2 * blk_size) + (blk_size * BYTE_BITS) * (INODE_SIZE + blk_size);

    for (unsigned int i = 0; i < groups; i++) {
        // Remember that one block is reserved for superblock
        size_t off = blk_size + BLKGRP_SIZE * i;

        // TODO: the allocated memory should be reclaimed elsewhere
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

        // FIXME: 3. find the index of next availible data block / inode
        blk_grps[i].next_data =
            bitvec_find_first_set(&blk_grps[i].inode_bitmap) + 1;
        blk_grps[i].next_inode =
            bitvec_find_first_set(&blk_grps[i].inode_bitmap) + 1;
    }
}

bool blkgrp_inode_exist(block_group_t *blk_grps, unsigned int inode_idx)
{
    if (inode_idx == 0)
        return false;

    unsigned int grp_idx = (inode_idx - 1) / BLK_INODE_NUM;
    unsigned int bitvec_idx = (inode_idx - 1) % BLK_INODE_NUM;

    return bitvec_get(&(blk_grps[grp_idx].inode_bitmap), bitvec_idx);
}

int blkgrp_inode_alloc(block_group_t *blk_grps)
{
    for (unsigned int i = 0; i < GROUPS; i++) {
        if (bitvec_count_zeros(&blk_grps[i].inode_bitmap) == 0)
            continue;

        size_t next_inode = blk_grps[i].next_inode;
        bitvec_set(&blk_grps[i].inode_bitmap, next_inode);
        blk_grps[i].next_inode =
            bitvec_find_first_set(&blk_grps[i].inode_bitmap) + 1;

        return i * BLK_INODE_NUM + next_inode;
    }
    return -1;
}

void blkgrp_destroy(block_group_t *blk_grps)
{
    // TODO: sync block group data back to the device
    for (unsigned int i = 0; i < GROUPS; i++) {
        bitvec_destroy(&blk_grps[i].inode_bitmap);
        bitvec_destroy(&blk_grps[i].data_bitmap);
    }
}
