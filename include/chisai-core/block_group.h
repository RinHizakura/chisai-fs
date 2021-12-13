#ifndef BLOCK_GROUP_H
#define BLOCK_GROUP_H

#include <stdbool.h>
#include <stddef.h>
#include "chisai-core/config.h"
#include "chisai-core/device.h"
#include "utils/bitvec.h"

typedef struct block_group block_group_t;
struct block_group {
    bitvec_t data_bitmap;
    bitvec_t inode_bitmap;
    chisai_size_t next_data;
    chisai_size_t next_inode;
};

void blkgrps_load(block_group_t *blk_grps,
                  device_t *d,
                  unsigned int blk_size,
                  unsigned int groups);
bool blkgrps_inode_exist(block_group_t *blk_grps, chisai_size_t inode_idx);
bool blkgrps_data_exist(block_group_t *blk_grps, chisai_size_t data_idx);
chisai_size_t blkgrps_inode_alloc(block_group_t *blk_grps);
chisai_size_t blkgrps_data_alloc(block_group_t *blk_grps);
void blkgrps_inode_release(block_group_t *blk_grps, chisai_size_t inode_idx);
void blkgrps_data_release(block_group_t *blk_grps, chisai_size_t data_idx);
void blkgrps_destroy(block_group_t *blk_grps);

#endif
