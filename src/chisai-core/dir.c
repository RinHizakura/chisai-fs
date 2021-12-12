#include "chisai-core/dir.h"
#include "utils/log.h"

void dir_init(dir_t *dir)
{
    memset(dir, 0, sizeof(dir_t));
}

void dir_save(dir_t *dir, device_t *d, size_t offset)
{
    device_data_save(d, offset, dir, sizeof(dir_t));
}

void dir_load(dir_t *dir, device_t *d, size_t offset)
{
    device_data_load(d, offset, dir, sizeof(dir_t));
}
