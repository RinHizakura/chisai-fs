#include "chisai-core/inode.h"
#include "utils/log.h"

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
                0,
            },
        .indirect_blk = 0,
        .double_indirect_blk = 0,
        .uid = 0xa,     // dummy
        .gid = 0xa,     // dummy
        .checksum = 0,  // dummy
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

void inode_add_block(inode_t *inode, chisai_size_t data_idx)
{
    // FIXME: enable to append more direct block
    inode->direct_blks[0] = data_idx;
}

void inode_save(inode_t *inode, device_t *d, size_t offset)
{
    ssize_t ret = d->write(d, offset, inode, sizeof(inode_t));
    if (ret < 0)
        die("Failed to write the block device\n");
}

void inode_load(inode_t *inode, device_t *d, size_t offset)
{
    ssize_t ret = d->read(d, offset, inode, sizeof(inode_t));
    if (ret < 0)
        die("Failed to write the block device\n");
}
