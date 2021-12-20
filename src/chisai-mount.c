#include <errno.h>
#include <fcntl.h>
#include <fuse/fuse.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "chisai-core/device.h"
#include "chisai-core/fs.h"
#include "chisai-deviceop.h"
#include "utils/log.h"

/* The value of uid and gid are not important. We just want
 * it as a non-zero value(which is for root) */
#define MAGIC_UID 0x00616d69
#define MAGIC_GID 0x0052696e

static filesystem_t fs;

static void chisai_fuse_tostat(struct stat *s, struct chisai_file_info *info)
{
    memset(s, 0, sizeof(struct stat));

    s->st_ino = info->idx;
    s->st_mode = info->inode.mode;
    s->st_nlink = info->inode.nlink;
    s->st_size = info->inode.size;
    s->st_blocks = info->inode.blkcnt;

    s->st_uid = MAGIC_UID;
    s->st_gid = MAGIC_GID;
}

void chisai_mount(const char *device_path)
{
    int fd = open(device_path, O_RDWR);
    if (fd < 0)
        die("Failed to open the block device\n");

    // we need to config the device operation
    fs.d = (device_t){
        .context = (void *) (uintptr_t) fd,
        .read = chisai_device_read,
        .write = chisai_device_write,
        .malloc = chisai_device_malloc,
        .free = chisai_device_free,
    };
    fs_init(&fs);
}

void *chisai_fuse_init(__attribute__((unused)) struct fuse_conn_info *conn)
{
    // TODO: update timestamp for superblock
    info("### Try to init\n");
    return NULL;
}

void chisai_fuse_destroy(__attribute__((unused)) void *p)
{
    info("### Try to destroy\n");
    fs_destroy(&fs);
    close(fs.device_fd);
}

int chisai_fuse_statfs(__attribute__((unused)) const char *path,
                       struct statvfs *s)
{
    info("### Try to statfs\n");
    memset(s, 0, sizeof(struct statvfs));
    return -EPERM;
}

int chisai_fuse_getattr(const char *path, struct stat *s)
{
    info("### Try to getattr of %s\n", path);
    if (s == NULL || path == NULL)
        return CHISAI_ERR_EINVAL;

    struct chisai_file_info info;
    int err = fs_get_metadata(&fs, path, &info);
    if (err) {
        return err;
    }

    chisai_fuse_tostat(s, &info);
    return 0;
}

int chisai_fuse_access(const char *path, __attribute__((unused)) int mask)
{
    // FIXME: check the permissions of access target
    info("### Try to access %s\n", path);
    if (path == NULL)
        return CHISAI_ERR_EINVAL;

    struct chisai_file_info info;
    return fs_get_metadata(&fs, path, &info);
}

int chisai_fuse_mkdir(const char *path, mode_t mode)
{
    info("### Try to mkdir\n");
    return fs_mkdir(&fs, path, mode);
}

int chisai_fuse_unlink(const char *path)
{
    info("### Try to unlink\n");
    return fs_remove_file(&fs, path);
}

int chisai_fuse_opendir(const char *path, struct fuse_file_info *fi)
{
    info("### Try to opendir %s\n", path);
    if (path == NULL || fi == NULL)
        return CHISAI_ERR_EINVAL;

    struct chisai_dir_info *dir = malloc(sizeof(struct chisai_dir_info));
    int ret = fs_get_dir(&fs, path, dir);
    if (ret) {
        free(dir);
        return ret;
    }
    fi->fh = (uintptr_t) dir;

    return 0;
}

int chisai_fuse_releasedir(const char *path, struct fuse_file_info *fi)
{
    info("### Try to releasedir\n");
    if (path == NULL || fi == NULL)
        return CHISAI_ERR_EINVAL;

    struct chisai_dir_info *dir = (struct chisai_dir_info *) fi->fh;
    free(dir);
    return -EPERM;
}

int chisai_fuse_readdir(const char *path,
                        void *buf,
                        fuse_fill_dir_t filler,
                        __attribute__((unused)) off_t offset,
                        struct fuse_file_info *fi)
{
    /* FIXME: We may have to consider the offset of directory */

    info("### Try to readdir %s\n", path);
    if (path == NULL || buf == NULL || fi == NULL)
        return CHISAI_ERR_EINVAL;

    struct chisai_dir_info *dir = (struct chisai_dir_info *) fi->fh;
    struct chisai_file_info info;
    char name[CHISAI_FILE_LEN];
    struct stat s;

    while (true) {
        int err = fs_get_data(&fs, dir, &info, name);
        if (err != 1) {
            return err;
        }

        chisai_fuse_tostat(&s, &info);
        filler(buf, name, &s, 0);
    }
}

int chisai_fuse_rename(__attribute__((unused)) const char *from,
                       __attribute__((unused)) const char *to)
{
    // TODO
    info("### Try to rename\n");
    return -EPERM;
}

int chisai_fuse_open(const char *path, struct fuse_file_info *fi)
{
    info("### Try to open %s\n", path);
    struct chisai_file_info *file = malloc(sizeof(struct chisai_file_info));
    int ret = fs_get_metadata(&fs, path, file);
    if (ret != 0) {
        free(file);
        return ret;
    }
    fi->fh = (uintptr_t) file;
    return 0;
}

int chisai_fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    info("### Try to create %s\n", path);

    struct chisai_file_info *file = malloc(sizeof(struct chisai_file_info));
    int ret = fs_create_file(&fs, path, mode, file);
    fi->fh = (uintptr_t) file;
    return ret;
}

int chisai_fuse_truncate(const char *path, off_t size)
{
    // TODO
    info("### Try to truncate\n");

    struct chisai_file_info file;
    int ret = fs_get_metadata(&fs, path, &file);
    if (ret != 0)
        return ret;

    return fs_truncate_file(&fs, &file, size);
}

int chisai_fuse_release(const char *path, struct fuse_file_info *fi)
{
    info("### Try to release\n");
    if (path == NULL || fi == NULL)
        return CHISAI_ERR_EINVAL;

    struct chisai_file_info *file = (struct chisai_file_info *) fi->fh;
    free(file);
    return 0;
}

int chisai_fuse_fgetattr(const char *path,
                         struct stat *s,
                         struct fuse_file_info *fi)
{
    info("### Try to fgetattr\n");
    if (path == NULL || s == NULL || fi == NULL)
        return CHISAI_ERR_EINVAL;

    /* For fgetattr, we can ignore the path of file because we already obtain
     * a structure which describing it */
    struct chisai_file_info *file = (struct chisai_file_info *) fi->fh;
    chisai_fuse_tostat(s, file);
    return 0;
}

int chisai_fuse_read(__attribute__((unused)) const char *path,
                     char *buf,
                     size_t size,
                     off_t off,
                     struct fuse_file_info *fi)
{
    info("### Try to read\n");
    struct chisai_file_info *file = (struct chisai_file_info *) fi->fh;
    return fs_read_file(&fs, file, buf, size, off);
}

int chisai_fuse_write(__attribute__((unused)) const char *path,
                      const char *buf,
                      size_t size,
                      off_t off,
                      struct fuse_file_info *fi)
{
    info("### Try to write\n");
    struct chisai_file_info *file = (struct chisai_file_info *) fi->fh;
    return fs_write_file(&fs, file, buf, size, off);
}

int chisai_fuse_fsync(__attribute__((unused)) const char *path,
                      __attribute__((unused)) int isdatasync,
                      __attribute__((unused)) struct fuse_file_info *fi)
{
    /* since we don't buffer any write on memory, this will
     * do nothing and always succeed */
    info("### Try to fsync\n");
    return 0;
}

int chisai_fuse_flush(__attribute__((unused)) const char *path,
                      __attribute__((unused)) struct fuse_file_info *fi)
{
    /* since we don't buffer any write on memory, this will
     * do nothing and always succeed */
    info("### Try to flush\n");
    return 0;
}

int chisai_fuse_link(__attribute__((unused)) const char *from,
                     __attribute__((unused)) const char *to)
{
    // not supported, fail
    info("### Try to link\n");
    return -EPERM;
}

int chisai_fuse_mknod(__attribute__((unused)) const char *path,
                      __attribute__((unused)) mode_t mode,
                      __attribute__((unused)) dev_t dev)
{
    // not supported, fail
    info("### Try to mknod\n");
    return -EPERM;
}

int chisai_fuse_chmod(__attribute__((unused)) const char *path,
                      __attribute__((unused)) mode_t mode)
{
    // not supported, always succeed
    info("### Try to chmod\n");
    return 0;
}

int chisai_fuse_chown(__attribute__((unused)) const char *path,
                      __attribute__((unused)) uid_t uid,
                      __attribute__((unused)) gid_t gid)
{
    // not supported, fail
    info("### Try to chown\n");
    return -EPERM;
}

int chisai_fuse_utimens(__attribute__((unused)) const char *path,
                        __attribute__((unused)) const struct timespec ts[2])
{
    // not supported, always succeed
    info("### Try to utimens\n");
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
