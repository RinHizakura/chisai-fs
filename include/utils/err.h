#ifndef ERR_H
#define ERR_H

#define err_propagate(f, cond) \
    int ret = (f);             \
    if (ret != 0)              \
        return ret;

#endif
