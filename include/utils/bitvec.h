#ifndef BITVEC_H
#define BITVEC_H

#include <stdint.h>

typedef uint8_t *bitvec_t;

int bitvec_get(bitvec_t bitvec, unsigned int index);
void bitvec_set(bitvec_t bitvec, unsigned int index);

#endif
