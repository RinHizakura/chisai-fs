#ifndef BITVEC_H
#define BITVEC_H

#include <stddef.h>
#include <stdint.h>

typedef struct bitvec bitvec_t;
struct bitvec {
    size_t size;
    uint8_t *inner;
};

void bitvec_init(bitvec_t *bitvec, size_t size);
int bitvec_get(bitvec_t *bitvec, size_t index);
void bitvec_set(bitvec_t *bitvec, size_t index);
size_t bitvec_count_zeros(bitvec_t *bitvec);
size_t bitvec_find_first_set(bitvec_t *bitvec);
void bitvec_destroy(bitvec_t *bitvec);

#endif
