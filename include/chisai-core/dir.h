#ifndef DIR_H
#define DIR_H

#include <stdbool.h>
#include "chisai-core/config.h"
#include "chisai-core/device.h"

/* FIXME: The size of dir_t on chisai-fs should be restricted to single block
 * size (now we assume a block should always be larger than 512 bytes).
 * The bad design limits the numbers of file in each directory and also limits
 * the name of files. The only advantage of this is the low complexity
 * of filesystem. */
struct dir_node {
    char name[CHISAI_FILE_LEN];
    chisai_size_t idx;
};

typedef struct dir dir_t;
struct dir {
    struct dir_node node[CHISAI_FILE_PER_DIR];
    unsigned int size;
    unsigned int checksum;
};

void dir_init(dir_t *dir);
void dir_save(dir_t *dir, device_t *d, size_t offset);
void dir_load(dir_t *dir, device_t *d, size_t offset);
chisai_size_t dir_file_inode_idx(dir_t *dir, const char *file_path);
bool dir_insert(dir_t *dir, const char *file_path, chisai_size_t inode_idx);
bool dir_rename_file(dir_t *dir, const char *from, const char *to);
chisai_size_t dir_remove(dir_t *dir, const char *file_path);
#endif
