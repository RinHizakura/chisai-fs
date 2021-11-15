#include "chisai-core/block_group.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "utils/log.h"

void blkgrp_load(block_group_t *blk_grps,
                 int fd,
                 unsigned int blk_size,
                 unsigned int groups)
{
    unsigned long blk_group_size =
        (2 * blk_size) + (blk_size * BYTE_BITS) * (INODE_SIZE + blk_size);

    for (unsigned int i = 0; i < groups; i++) {
        // Remember that one block is reserved for superblock
        lseek(fd, blk_size + blk_group_size * i, SEEK_SET);

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

        // TEST01: check if the data bitmap is formatted as we want
        if (i == 0) {
            assert(bitvec_get(&(blk_grps[i].data_bitmap), 0) == 1);
            assert(bitvec_get(&(blk_grps[i].data_bitmap), 1) == 0);
            assert(bitvec_get(&(blk_grps[i].data_bitmap), 2) == 0);
            assert(bitvec_get(&(blk_grps[i].data_bitmap), 3) == 0);
            assert(bitvec_get(&(blk_grps[i].data_bitmap), 4) == 1);
            assert(bitvec_get(&(blk_grps[i].data_bitmap), 5) == 0);
            assert(bitvec_get(&(blk_grps[i].data_bitmap), 6) == 1);
            assert(bitvec_get(&(blk_grps[i].data_bitmap), 7) == 0);
        }
    }
}

bool blkgrp_inode_exist(block_group_t *blk_grps,
                        unsigned int grp_idx,
                        unsigned int inode_idx)
{
    if (inode_idx == 0)
        return false;

    return bitvec_get(&(blk_grps[grp_idx].inode_bitmap), inode_idx - 1);
}
