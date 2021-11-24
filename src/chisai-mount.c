#include <errno.h>
#include <fcntl.h>
#include <fuse/fuse.h>
#include <stdio.h>
#include <string.h>
#include "chisai-core/device.h"
#include "chisai-core/fs.h"
#include "chisai-deviceop.h"
#include "utils/log.h"

static filesystem_t fs;

static void chisai_fuse_tostat(struct stat *s, struct chisai_info *info)
{
    memset(s, 0, sizeof(struct stat));

    s->st_ino = info->ino;
    s->st_mode = info->mode;
    s->st_nlink = info->nlink;
    s->st_atime = info->atim;
    s->st_mtime = info->mtim;
    s->st_ctime = info->ctim;

    s->st_size = info->size;
    s->st_blocks = info->blkcnt;
    s->st_uid = info->uid;
    s->st_gid = info->gid;
}

void chisai_mount(const char *device_path)
{
    int fd = open(device_path, O_RDWR);
    if (fd < 0)
        die("Failed to open the block device\n");

    // we need to config the device operation
    device_t device = (device_t){
        .context = (void *) (uintptr_t) fd,
        .read = chisai_device_read,
        .write = chisai_device_write,
    };
    fs_init(&fs, &device);
}

void *chisai_fuse_init(struct fuse_conn_info *conn)
{
    // TODO: update timestamp for superblock
    return NULL;
}

void chisai_fuse_destroy(void *p)
{
    // TODO: unmount the file system properly
    printf("### Try to destroy\n");
    fs_destroy(&fs);
}

int chisai_fuse_statfs(const char *path, struct statvfs *s)
{
    printf("### Try to statfs\n");
    memset(s, 0, sizeof(struct statvfs));
    // TODO: set statvfs
    return -EPERM;
}

int chisai_fuse_getattr(const char *path, struct stat *s)
{
    printf("### Try to getattr of %s\n", path);

    if (s == NULL || path == NULL)
        return -EINVAL;

    struct chisai_info info;
    int err = fs_get_metadata(&fs, path, &info);
    if (err) {
        return err;
    }

    chisai_fuse_tostat(s, &info);

    return 0;
}

int chisai_fuse_access(const char *path, int mask)
{
    // TODO
    printf("### (1) Try to access %s\n", path);
    return -EPERM;
}

int chisai_fuse_mkdir(const char *path, mode_t mode)
{
    // TODO
    printf("### Try to mkdir\n");
    return -EPERM;
}

int chisai_fuse_unlink(const char *path)
{
    // TODO
    printf("### Try to unlink\n");
    return -EPERM;
}

int chisai_fuse_opendir(const char *path, struct fuse_file_info *fi)
{
    // TODO
    printf("### (3) Try to opendir\n");
    return -EPERM;
}

int chisai_fuse_releasedir(const char *path, struct fuse_file_info *fi)
{
    // TODO
    printf("### Try to releasedir\n");
    return -EPERM;
}

int chisai_fuse_readdir(const char *path,
                        void *buf,
                        fuse_fill_dir_t filler,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    // TODO
    printf("### Try to readdir\n");
    return -EPERM;
}

int chisai_fuse_rename(const char *from, const char *to)
{
    // TODO
    printf("### Try to rename\n");
    return -EPERM;
}

int chisai_fuse_open(const char *path, struct fuse_file_info *fi)
{
    // TODO
    printf("Try to open\n");
    return -EPERM;
}

int chisai_fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    // TODO
    printf("### Try to create\n");
    return -EPERM;
}

int chisai_fuse_truncate(const char *path, off_t size)
{
    // TODO
    printf("### Try to truncate\n");
    return -EPERM;
}

int chisai_fuse_release(const char *path, struct fuse_file_info *fi)
{
    // TODO
    printf("### Try to release\n");
    return -EPERM;
}

int chisai_fuse_fgetattr(const char *path,
                         struct stat *s,
                         struct fuse_file_info *fi)
{
    // TODO
    printf("Try to fgetattr\n");
    return -EPERM;
}

int chisai_fuse_read(const char *path,
                     char *buf,
                     size_t size,
                     off_t off,
                     struct fuse_file_info *fi)
{
    // TODO
    printf("### Try to read\n");
    return -EPERM;
}

int chisai_fuse_write(const char *path,
                      const char *buf,
                      size_t size,
                      off_t off,
                      struct fuse_file_info *fi)
{
    // TODO
    printf("### Try to write\n");
    return -EPERM;
}

int chisai_fuse_fsync(const char *path,
                      int isdatasync,
                      struct fuse_file_info *fi)
{
    // TODO
    printf("### Try to fsync\n");
    return -EPERM;
}

int chisai_fuse_flush(const char *path, struct fuse_file_info *fi)
{
    // TODO
    printf("### Try to flush\n");
    return -EPERM;
}

// unsupported functions
int chisai_fuse_link(const char *from, const char *to)
{
    // not supported, fail
    printf("### Try to link\n");
    return -EPERM;
}

int chisai_fuse_mknod(const char *path, mode_t mode, dev_t dev)
{
    // not supported, fail
    printf("### Try to mknod\n");
    return -EPERM;
}

int chisai_fuse_chmod(const char *path, mode_t mode)
{
    // not supported, always succeed
    printf("### Try to chmod\n");
    return 0;
}

int chisai_fuse_chown(const char *path, uid_t uid, gid_t gid)
{
    // not supported, fail
    printf("### Try to chown\n");
    return -EPERM;
}

int chisai_fuse_utimens(const char *path, const struct timespec ts[2])
{
    // not supported, always succeed
    printf("### Try to utimens\n");
    return 0;
}

struct fuse_operations chisai_fuse_opts = {
    .init = chisai_fuse_init,
    .destroy = chisai_fuse_destroy,
    .statfs = chisai_fuse_statfs,

    .getattr = chisai_fuse_getattr,
    .access = chisai_fuse_access,

    .mkdir = chisai_fuse_mkdir,
    .rmdir = chisai_fuse_unlink,
    .opendir = chisai_fuse_opendir,
    .releasedir = chisai_fuse_releasedir,
    .readdir = chisai_fuse_readdir,

    .rename = chisai_fuse_rename,
    .unlink = chisai_fuse_unlink,

    .open = chisai_fuse_open,
    .create = chisai_fuse_create,
    .truncate = chisai_fuse_truncate,
    .release = chisai_fuse_release,
    .fgetattr = chisai_fuse_fgetattr,
    .read = chisai_fuse_read,
    .write = chisai_fuse_write,
    .fsync = chisai_fuse_fsync,
    .flush = chisai_fuse_flush,

    .link = chisai_fuse_link,
    .symlink = chisai_fuse_link,
    .mknod = chisai_fuse_mknod,
    .chmod = chisai_fuse_chmod,
    .chown = chisai_fuse_chown,
    .utimens = chisai_fuse_utimens,
};
