#ifndef CHISAI_API
#define CHISAI_API

void chisai_format(const char *device_path);
void chisai_mount(const char *device_path);

extern struct fuse_operations chisai_fuse_opts;
#endif
