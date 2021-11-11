#include "chisai-core/fs.h"
#include <unistd.h>
#include "utils.h"

void fs_init(filesystem_t *fs, int fd)
{
    fs->device_fd = fd;
    // load back the superblock
    ssize_t ret = read(fd, &fs->sb, sizeof(superblock_t));
    if (ret < 0)
        die("Failed to read the block device\n");

    // check the magic number in superblock
    if (fs->sb.magic != MAGIC)
        die("Magic number=%x in superblock is not expected=%x!\n", fs->sb.magic,
            MAGIC);

    info("FS_INIT DONE\n");
}
