#include "utils/bitvec.h"

void bitvec_init(bitvec_t *bitvec, uint8_t *inner)
{
    bitvec->inner = inner;
}

int bitvec_get(bitvec_t *bitvec, unsigned int index)
{
    uint8_t *inner = bitvec->inner;

    unsigned int split = index >> 3;
    unsigned int pos = index & 0x7;
    // TODO: The endian should be concerned, but we just assume little endian
    // here
    return (inner[split] >> pos) & 1;
}

void bitvec_set(bitvec_t *bitvec, unsigned int index)
{
    uint8_t *inner = bitvec->inner;

    unsigned int split = index >> 3;
    unsigned int pos = index & 0x7;
    // TODO: The endian should be concerned, but we just assume little endian
    // here
    inner[split] |= (1 << pos);
}
