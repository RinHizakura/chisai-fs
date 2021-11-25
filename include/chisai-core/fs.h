#ifndef FS_H
#define FS_H

#include "chisai-core/block_group.h"
#include "chisai-core/device.h"
#include "chisai-core/dir.h"
#include "chisai-core/inode.h"
#include "chisai-core/superblock.h"

enum chisai_error {
    CHISAI_ERR_OK = 0,        // No error
    CHISAI_ERR_NOFILE = -39,  // No such file exist
};

struct chisai_dir_info {
    dir_t dir;
    unsigned int pos;
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
int fs_get_data(filesystem_t *fs,
                struct chisai_dir_info *dir,
                const char *path,
                struct chisai_info *info);
void fs_destroy(filesystem_t *fs);
#endif
