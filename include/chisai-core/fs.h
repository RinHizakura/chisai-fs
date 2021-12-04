#ifndef FS_H
#define FS_H

#include "chisai-core/block_group.h"
#include "chisai-core/device.h"
#include "chisai-core/dir.h"
#include "chisai-core/inode.h"
#include "chisai-core/superblock.h"

enum chisai_error {
    CHISAI_ERR_OK = 0,        // No error
    CHISAI_ERR_ENOENT = -2,   // No such file or directory exist
    CHISAI_ERR_ENOMEM = -12,  // Out of memory
    CHISAI_ERR_EEXIST = -17,  // File exists
    CHISAI_ERR_EINVAL = -22,  // Invalid argument
    CHISAI_ERR_EFBIG = -27,   // File too large
    CHISAI_ERR_CORRUPT = -84  // Corrupt
};

struct chisai_dir_info {
    dir_t dir;
    unsigned int pos;
};

/* This is similar to `struct chisai_info`
 * because using inode is enoungh to represent file for chisai-fs. */
struct chisai_file_info {
    chisai_size_t idx;
    inode_t inode;
};

struct chisai_info {
    chisai_size_t idx;
    inode_t inode;
    char name[CHISAI_FILE_LEN];
};

typedef struct filesystem filesystem_t;
struct filesystem {
    int device_fd;
    superblock_t sb;
    block_group_t *blk_grps;
    device_t d;
};

void fs_init(filesystem_t *fs, device_t *d);
int fs_get_metadata(filesystem_t *fs,
                    const char *path,
                    struct chisai_info *info);
int fs_get_dir(filesystem_t *fs, const char *path, struct chisai_dir_info *dir);
int fs_get_data(filesystem_t *fs,
                const char *path,
                struct chisai_dir_info *dir,
                struct chisai_info *info);
int fs_mkdir(filesystem_t *fs, const char *path, mode_t mode);
int fs_create_file(filesystem_t *fs,
                   mode_t mode,
                   struct chisai_file_info *file);
void fs_destroy(filesystem_t *fs);
#endif
