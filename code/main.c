#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

#include "actuator.h"
#include "pca9685.h"
#include "tco_shmem.h"
#include "tco_libd.h"

int log_level = LOG_INFO | LOG_DEBUG | LOG_ERROR;

int main(int argc, const char *argv[])
{
    if (log_init("actuatord", "./log.txt") != 0)
    {
        printf("Failed to initialize the logger\n");
        return EXIT_FAILURE;
    }

    struct tco_shmem_data_control *control_data;
    sem_t *control_data_sem;
    if (shmem_map(TCO_SHMEM_NAME_CONTROL, TCO_SHMEM_SIZE_CONTROL, TCO_SHMEM_NAME_SEM_CONTROL, O_RDONLY, (void **)&control_data, &control_data_sem) != 0)
    {
        log_error("Failed to map shared memory and associated semaphore");
        return EXIT_FAILURE;
    }

    void *actr_handle = actr_init();
    if (actr_handle == NULL)
    {
        log_error("Failed to initialize IO hardware");
        return EXIT_FAILURE;
    }

    struct tco_shmem_data_control ctrl_cpy = {0};
    while (1)
    {
        if (sem_wait(control_data_sem) == -1)
        {
            log_error("sem_wait: %s", strerror(errno));
            return EXIT_FAILURE;
        }
        /* START: Critical section */
        memcpy(&ctrl_cpy, control_data, TCO_SHMEM_SIZE_CONTROL);
        /* END: Critical section */
        if (sem_post(control_data_sem) == -1)
        {
            log_error("sem_post: %s", strerror(errno));
            return EXIT_FAILURE;
        }

        /* Update actuators */
        if (ctrl_cpy.valid > 0)
        {
            uint8_t ctrl_ch_count = sizeof(ctrl_cpy.ch) / sizeof(ctrl_cpy.ch[0]);
            for (uint8_t ch_i = 0; ch_i < ctrl_ch_count; ch_i++)
            {
                if (ctrl_cpy.ch[ch_i].active > 0)
                {
                    actr_ch_control(actr_handle, ch_i, ctrl_cpy.ch[ch_i].pulse_frac);
                }
                else
                {
                    /* TODO: Check if some devices need a signal other than 0.5 as a neutral position. */
                    actr_ch_control(actr_handle, ch_i, 0.5f);
                }
            }
        }
        usleep(100000); /* Update actuator state every ~0.1 seconds (a little more than that actually). */
    }

    return EXIT_SUCCESS;
}
