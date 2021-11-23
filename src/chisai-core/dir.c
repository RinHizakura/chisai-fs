#include "chisai-core/dir.h"
#include <string.h>
#include "utils/log.h"

void dir_init(dir_t *dir)
{
    memset(dir, 0, sizeof(dir_t));
}

void dir_save(dir_t *dir, device_t *d, size_t offset)
{
    ssize_t ret = d->write(d, offset, dir, sizeof(dir_t));
    if (ret < 0)
        die("Failed to write the block device\n");
}

void dir_load(dir_t *dir, device_t *d, size_t offset)
{
    ssize_t ret = d->read(d, offset, dir, sizeof(dir_t));
    if (ret < 0)
        die("Failed to write the block device\n");
}
