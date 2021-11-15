#ifndef BLOCK_GROUP_H
#define BLOCK_GROUP_H

#include <stddef.h>
#include "utils/bitvec.h"

typedef struct block_group block_group_t;
struct block_group {
    bitvec_t data_bitmap;
    bitvec_t inode_bitmap;
    size_t next_data;
    size_t next_inode;
};

#endif
