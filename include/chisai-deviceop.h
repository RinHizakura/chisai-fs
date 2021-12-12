#ifndef CHISAI_DEVICEOP_H
#define CHISAI_DEVICEOP_H

ssize_t chisai_device_read(const struct device *d,
                           size_t offset,
                           void *buffer,
                           size_t size);
ssize_t chisai_device_write(const struct device *d,
                            size_t offset,
                            const void *buffer,
                            size_t size);
void *chisai_device_malloc(size_t size);
void chisai_device_free(void *ptr);
#endif
