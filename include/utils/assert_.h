#ifndef UTILS_ASSERT_H
#define UTILS_ASSERT_H

#include <assert.h>
#include <stdio.h>

#ifndef NDEBUG
#define assert_eq(a, b)                                                      \
    for (; !(a == b); assert(a == b)) {                                      \
        fprintf(                                                             \
            stderr,                                                          \
            "Assertion Failed: The left value is %lx but the left value is " \
            "%lx\n",                                                         \
            (long) a, (long) b);                                             \
    }
#else
#define assert_eq(a, b)
#endif

#endif /* UTILS_ASSERT_H */
