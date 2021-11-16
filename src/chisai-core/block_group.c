#include "chisai-core/block_group.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "utils/log.h"

/* FIXME: We save some values related to the block size
 * to perform block-group-related operations efficiently.
 * Since these values have already been calculated elsewhere
 * in our filesystem. We could preserve the values in a global
 * region instead of making another copies. */
static unsigned int BLK_SIZE = 0;
static unsigned int BLK_INODE_NUM = 0;
static unsigned long BLKGRP_SIZE = 0;

void blkgrp_load(block_group_t *blk_grps,
                 int fd,
                 unsigned int blk_size,
                 unsigned int groups)
{
    // We should only call blkgrp_load once in normal situation
    assert((BLK_SIZE == 0) && (BLK_INODE_NUM == 0) && (BLKGRP_SIZE == 0));

    BLK_SIZE = blk_size;
    BLK_INODE_NUM = blk_size * BYTE_BITS;
    BLKGRP_SIZE =
        (2 * blk_size) + (blk_size * BYTE_BITS) * (INODE_SIZE + blk_size);

    for (unsigned int i = 0; i < groups; i++) {
        // Remember that one block is reserved for superblock
        lseek(fd, blk_size + BLKGRP_SIZE * i, SEEK_SET);

        // TODO: the allocated memory should be reclaimed elsewhere
        // 1. read the data bitmap
        uint8_t *databuf = malloc(blk_size);
        ssize_t ret = read(fd, databuf, blk_size);
        if (ret < 0)
            die("Failed to read the block device\n");
        bitvec_init(&blk_grps[i].data_bitmap, databuf);

        // 2. read the inode bitmap
        uint8_t *inodebuf = malloc(blk_size);
        ret = read(fd, inodebuf, blk_size);
        if (ret < 0)
            die("Failed to read the block device\n");
        bitvec_init(&blk_grps[i].inode_bitmap, inodebuf);

        // FIXME: 3. find the index of next availible data block / inode
        blk_grps[i].next_data = 0;
        blk_grps[i].next_inode = 0;
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
