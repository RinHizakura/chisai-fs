#include "utils/bitvec.h"
#include <stdlib.h>

void bitvec_init(bitvec_t *bitvec, size_t size)
{
    bitvec->size = size;
    bitvec->inner = malloc(size);
}

int bitvec_get(bitvec_t *bitvec, size_t index)
{
    uint8_t *inner = bitvec->inner;

    size_t split = index >> 3;
    size_t pos = index & 0x7;
    // TODO: The endian should be concerned, but we just assume little endian
    // here
    return (inner[split] >> pos) & 1;
}

void bitvec_set(bitvec_t *bitvec, size_t index)
{
    uint8_t *inner = bitvec->inner;

    size_t split = index >> 3;
    size_t pos = index & 0x7;
    // TODO: The endian should be concerned, but we just assume little endian
    // here
    inner[split] |= (1 << pos);
}

size_t bitvec_count_zeros(bitvec_t *bitvec)
{
    // TODO: implement it more efficiently
    size_t sum = 0;
    for (int i = 0; i < bitvec->size; i++) {
        sum += (8 - __builtin_popcount(bitvec->inner[i]));
    }
    return sum;
}

size_t bitvec_find_first_set(bitvec_t *bitvec)
{
    // TODO: implement it more efficiently
    size_t ffs = 0;
    for (int i = 0; i < bitvec->size; i++) {
        size_t tmp = __builtin_ffs(~bitvec->inner[i]);

        if (tmp > 0)
            return ffs + tmp;

        ffs += 8;
    }
    return 0;
}

void bitvec_destroy(bitvec_t *bitvec)
{
    free(bitvec->inner);
}
