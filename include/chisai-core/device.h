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
                     void *buffer,
                     size_t size);
};

#endif
