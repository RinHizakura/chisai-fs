#include <fuse/fuse.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "chisai-core/superblock.h"
#include "config.h"
#include "utils.h"

void chisai_format(const char *device_path)
{
    int fd = open(device_path, O_RDWR);
    if (fd < 0)
        die("Failed to open the block device\n");

    // get sector size
    unsigned int blk_size;
    int err = ioctl(fd, BLKSSZGET, &blk_size);
    if (err)
        die("Failed to retrieve the block size\n");

    // get size in sectors
    unsigned long blk_cnt;
    err = ioctl(fd, BLKGETSIZE, &blk_cnt);
    if (err)
        die("Failed to retrieve the counts of block\n");

    unsigned long disk_size = blk_cnt * blk_size;
    /* For each block group:
     * - We have one block for inode bitmap
     * - We have one block for data bitmap
     * - For each bits in inode bitmap, it can represent a inode.
     * - For each bits in data bitmap, it can represent a data block. */
    unsigned long blk_group_size =
        (2 * blk_size) + (blk_size * BYTE_BITS) * (INODE_SIZE + blk_size);
    // at least a superblock and a block group should be fit
    if (disk_size < blk_size + blk_group_size)
        die("Device size=%lx is too small! At least %lx is required\n",
            disk_size, blk_group_size);
    // FIXME: We need to consider the waste of disk space for the simple
    // implementation.
    unsigned int groups = (disk_size - blk_size) / blk_group_size;

    superblock_t sb;
    superblock_init(&sb, blk_size, groups);

    ssize_t ret = write(fd, &sb, sizeof(superblock_t));
    if (ret < 0)
        die("Failed to write the block device\n");

    close(fd);
}
