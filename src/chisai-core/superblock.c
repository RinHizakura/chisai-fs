#include "chisai-core/superblock.h"
#include "chisai-core/config.h"
#include "utils/assert_.h"
#include "utils/log.h"

void superblock_init(superblock_t *sb,
                     unsigned int blk_size,
                     unsigned int groups)
{
    assert_le(sizeof(superblock_t), blk_size);

    *sb = (superblock_t){
        .magic = MAGIC,
        .block_size = blk_size,
        .blocks_cnt = (blk_size * BYTE_BITS) * groups,
        .inodes_cnt = (blk_size * BYTE_BITS) * groups,
        .free_blocks = (blk_size * BYTE_BITS) * groups,
        .free_inodes = (blk_size * BYTE_BITS) * groups,
        .groups = groups,
        .data_block_per_groups = (blk_size * BYTE_BITS),
    };
}

void superblock_save(superblock_t *sb, device_t *d)
{
    device_data_save(d, 0, sb, sizeof(superblock_t));
}

void superblock_load(superblock_t *sb, device_t *d)
{
    device_data_load(d, 0, sb, sizeof(superblock_t));
}
