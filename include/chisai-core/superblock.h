#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdint.h>

typedef struct superblock superblock_t;
struct superblock {
    uint32_t magic;
    unsigned int block_size;

    unsigned long created_at;
    unsigned long modified_at;
    unsigned long last_mounted_at;

    unsigned int blocks_cnt;
    unsigned int inodes_cnt;
    unsigned int free_blocks;
    unsigned int free_inodes;

    unsigned int groups;
    unsigned int data_block_per_groups;
    unsigned int uid;
    unsigned int gid;
    unsigned int checksum;
};
void superblock_init(superblock_t *sb,
                     unsigned int blk_size,
                     unsigned int groups);

#endif
