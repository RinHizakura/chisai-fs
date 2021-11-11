#ifndef CHISAI_MOUNT
#define CHISAI_MOUNT

void chisai_mount(const char *device_path);
extern struct fuse_operations chisai_fuse_opts;

#endif
