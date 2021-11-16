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
    // TODO: implement it correctly
    return 0;
}

size_t bitvec_find_first_set(bitvec_t *bitvec)
{
    // TODO: implement it correctly
    return 0;
}

void bitvec_destroy(bitvec_t *bitvec)
{
    free(bitvec->inner);
}
