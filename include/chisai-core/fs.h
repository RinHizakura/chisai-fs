#ifndef FS_H
#define FS_H

#include "chisai-core/block_group.h"
#include "chisai-core/device.h"
#include "chisai-core/superblock.h"

enum chisai_error {
    CHISAI_ERR_NOFILE = -39,  // No such file exist
};

struct chisai_info {
    chisai_size_t ino;
    mode_t mode;        /* File type and mode */
    nlink_t nlink;      /* Number of hard links */
    blkcnt_t blkcnt;    /* Number of 512B blocks allocated */
    off_t size;         /* Total size, in bytes */
    unsigned long atim; /* Time of last access */
    unsigned long mtim; /* Time of last modification */
    unsigned long ctim; /* Time of last status change */
    uid_t uid;          /* User ID of owner */
    gid_t gid;          /* Group ID of owner */
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
void fs_destroy(filesystem_t *fs);
#endif
