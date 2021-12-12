#include "chisai-core/inode.h"
#include "utils/log.h"

void inode_init(inode_t *inode)
{
    *inode = (inode_t){
        .mode = 0,
        .nlink = 1,
        .blkcnt = 0,
        .size = 0,
        .direct_blks =
            {
                0,
            },
        .indirect_blk = 0,
        .double_indirect_blk = 0,
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

void inode_set_size(inode_t *inode, off_t size)
{
    inode->size = inode->size > size ? inode->size : size;
    inode->blkcnt = inode->size / 512 + 1;
}

void inode_save(inode_t *inode, device_t *d, size_t offset)
{
    device_data_save(d, offset, inode, sizeof(inode_t));
}

void inode_load(inode_t *inode, device_t *d, size_t offset)
{
    device_data_load(d, offset, inode, sizeof(inode_t));
}
