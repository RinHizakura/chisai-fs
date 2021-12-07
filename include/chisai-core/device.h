#ifndef DEVICE_H
#define DEVICE_H

#include <stddef.h>
#include <sys/types.h>

typedef struct device device_t;
struct device {
    void *context;

    ssize_t (*read)(const struct device *d,
                    size_t offset,
                    void *buffer,
                    size_t size);
    ssize_t (*write)(const struct device *d,
                     size_t offset,
                     const void *buffer,
                     size_t size);
};

#define device_data_save(d, offset, buf, size)          \
    do {                                                \
        ssize_t ret = (d)->write(d, offset, buf, size); \
        if (ret < 0)                                    \
            die("Failed to write the block device\n");  \
    } while (0)


#endif
