#include "chisai-core/inode.h"

void inode_init(inode_t *inode)
{
    *inode = (inode_t){
        .mode = 0,
        .nlink = 1,
        .blkcnt = 0,
        .size = 0,
        .atim = 0,  // dummy
        .mtim = 0,  // dummy
        .ctim = 0,  // dummy
        .direct_blks =
            {
                NO_INODE,
            },
        .indirect_blk = NO_INODE,
        .double_indirect_blk = NO_INODE,
        .uid = 0xa,  // dummy
        .gid = 0xa,  // dummy
        .checksum = 0,
    };
}
