#ifndef LOG_H
#define LOG_H

#include <errno.h>
#include <string.h>

void __die(const char *format, ...);
void __info(const char *format, ...);

#define die(format, ...) \
    __die("\nERRNO: %s\n" format, strerror(errno) __VA_OPT__(, ) __VA_ARGS__)
#define info(format, ...) __info(format __VA_OPT__(, ) __VA_ARGS__)
#endif
