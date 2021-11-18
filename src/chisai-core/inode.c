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

void inode_set_mode(inode_t *inode, mode_t mode)
{
    inode->mode = mode;
}

void inode_set_nlink(inode_t *inode, nlink_t nlink)
{
    inode->nlink = nlink;  // . and ..
}

void inode_add_block(inode_t *inode, size_t data_idx)
{
    // FIXME: enable to append more direct block
    inode->direct_blks[0] = data_idx;
}
