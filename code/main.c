#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

int debug = 1;
int verbose = 1;

#include "debug.h"
#include "io.h"
#include "pca9685.h"

int main(int argc, const char *argv[])
{
    void *io_handle = io_init();
    if (io_handle == NULL)
    {
        return EXIT_FAILURE;
    }

    while (1)
    {
    }

    return EXIT_SUCCESS;
}
