#include "utils/bitvec.h"

int bitvec_get(bitvec_t bitvec, unsigned int index)
{
    unsigned int split = index >> 3;
    unsigned int pos = index & 0x7;
    // TODO: The endian should be concerned, but we just assume little endian
    // here
    return (bitvec[split] >> pos) & 1;
}

void bitvec_set(bitvec_t bitvec, unsigned int index)
{
    unsigned int split = index >> 3;
    unsigned int pos = index & 0x7;
    // TODO: The endian should be concerned, but we just assume little endian
    // here
    bitvec[split] |= (1 << pos);
}
