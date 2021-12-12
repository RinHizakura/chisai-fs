#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "chisai-core/device.h"

ssize_t chisai_device_read(const struct device *d,
                           size_t offset,
                           void *buffer,
                           size_t size)
{
    int fd = (uintptr_t) d->context;
    lseek(fd, offset, SEEK_SET);
    return read(fd, buffer, size);
}

ssize_t chisai_device_write(const struct device *d,
                            size_t offset,
                            const void *buffer,
                            size_t size)
{
    int fd = (uintptr_t) d->context;
    lseek(fd, offset, SEEK_SET);
    return write(fd, buffer, size);
}

void *chisai_device_malloc(size_t size)
{
    return malloc(size);
}

void chisai_device_free(void *ptr)
{
    free(ptr);
}
