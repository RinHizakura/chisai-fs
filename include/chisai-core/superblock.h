#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdint.h>
#include <sys/stat.h>
#include "chisai-core/device.h"

#define MAGIC 0x52696B6F

typedef struct superblock superblock_t;
struct superblock {
    uint32_t magic;
    unsigned int block_size;

    unsigned int blocks_cnt;
    unsigned int inodes_cnt;
    unsigned int free_blocks;
    unsigned int free_inodes;

    unsigned int groups;
    unsigned int data_block_per_groups;
};
void superblock_init(superblock_t *sb,
                     unsigned int blk_size,
                     unsigned int groups);
void superblock_save(superblock_t *sb, device_t *d);
void superblock_load(superblock_t *sb, device_t *d);
#endif
