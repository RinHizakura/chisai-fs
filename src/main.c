#include <fuse/fuse.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "chisai-format.h"
#include "chisai-mount.h"
#include "utils/log.h"


static const char *device = NULL;
static bool format = false;

enum lfs_fuse_keys {
    KEY_FORMAT,
};

static struct fuse_opt lfs_fuse_opts[] = {FUSE_OPT_KEY("--format", KEY_FORMAT),
                                          FUSE_OPT_END};

static int lfs_fuse_opt_proc(void *data,
                             const char *arg,
                             int key,
                             struct fuse_args *args)
{
    // option parsing
    switch (key) {
    case FUSE_OPT_KEY_NONOPT:
        if (!device) {
            device = strdup(arg);
            return 0;
        }
        break;
    case KEY_FORMAT:
        format = true;
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    fuse_opt_parse(&args, NULL, lfs_fuse_opts, lfs_fuse_opt_proc);

    if (!device)
        die("Missing device parameter\n");

    if (format) {
        chisai_format(device);
        info("FORMAT DONE\n");
        return 0;
    }

    chisai_mount(device);
    fuse_opt_add_arg(&args, "-s");

    int err = fuse_main(args.argc, args.argv, &chisai_fuse_opts, NULL);
    if (err)
        die("Error happens in fuse_main!\n");

    return 0;
}
