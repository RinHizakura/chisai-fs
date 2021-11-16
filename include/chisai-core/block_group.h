#ifndef BLOCK_GROUP_H
#define BLOCK_GROUP_H

#include <stdbool.h>
#include <stddef.h>
#include "utils/bitvec.h"

typedef struct block_group block_group_t;
struct block_group {
    bitvec_t data_bitmap;
    bitvec_t inode_bitmap;
    size_t next_data;
    size_t next_inode;
};

void blkgrp_load(block_group_t *blk_grps,
                 int fd,
                 unsigned int blk_size,
                 unsigned int groups);
bool blkgrp_inode_exist(block_group_t *blk_grps, unsigned int inode_idx);

#endif
