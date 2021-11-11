#include "chisai-core/superblock.h"
#include "config.h"

#define MAGIC 0x52696B6F

void superblock_init(superblock_t *sb,
                     unsigned int blk_size,
                     unsigned int groups)
{
    *sb = (superblock_t){
        .magic = MAGIC,
        .block_size = blk_size,
        .created_at = 1,       // dummy
        .modified_at = 2,      // dummy
        .last_mounted_at = 3,  // dummy
        .blocks_cnt = (blk_size * BYTE_BITS) * groups,
        .inodes_cnt = (blk_size * BYTE_BITS) * groups,
        .free_blocks = (blk_size * BYTE_BITS) * groups,
        .free_inodes = (blk_size * BYTE_BITS) * groups,
        .groups = groups,
        .data_block_per_groups = (blk_size * BYTE_BITS),
        .uid = 0xa,     // dummy
        .gid = 0xa,     // dummy
        .checksum = 0,  // dummy
    };
}
