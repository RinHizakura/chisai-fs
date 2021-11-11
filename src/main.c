#include <stdio.h>
#include "chisai-api.h"
#include "utils.h"
int main(int argc, char *argv[])
{
    if (argc != 2)
        die("usage: %s <loop device>\n", argv[0]);

    chisai_format(argv[1]);
    printf("Format Complete!\n");
    return 0;
}
