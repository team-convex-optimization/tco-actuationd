#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "pca9685.h"

const int debug = 1;
const int verbose = 1;

int main(int argc, const char *argv[])
{
    void *pca9685_info;
    if (pca9685_init(&pca9685_info, 50U) != ERR_OK)
    {
        return EXIT_FAILURE;
    }
    sleep(1);
    printf("0 degrees\n");
    /* 0.4ms pulse length. */
    if (pca9685_reg_ch_set(pca9685_info, 0U, 82U) != ERR_OK)
    {
        return EXIT_FAILURE;
    }
    sleep(1);
    printf("90 degrees\n");
    /* 1.4ms pulse length. */
    if (pca9685_reg_ch_set(pca9685_info, 0U, 287U) != ERR_OK)
    {
        return EXIT_FAILURE;
    }
    sleep(1);
    printf("180 degrees\n");
    /* 2.4ms pulse length. */
    if (pca9685_reg_ch_set(pca9685_info, 0U, 492U) != ERR_OK)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
