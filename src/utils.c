#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void die(const char *format, ...)
{
    va_list vargs;
    va_start(vargs, format);
    vfprintf(stderr, format, vargs);
    va_end(vargs);
    exit(123);
}
