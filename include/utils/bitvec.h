#ifndef BITVEC_H
#define BITVEC_H

#include <stdint.h>

typedef struct bitvec bitvec_t;
struct bitvec {
    uint8_t *inner;
};

void bitvec_init(bitvec_t *bitvec, uint8_t *inner);
int bitvec_get(bitvec_t *bitvec, unsigned int index);
void bitvec_set(bitvec_t *bitvec, unsigned int index);

#endif
