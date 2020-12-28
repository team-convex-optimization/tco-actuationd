#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

int debug = 0;
int verbose = 1;

#include "debug.h"
#include "io.h"
#include "pca9685.h"
#include "tco_shmem.h"

int main(int argc, const char *argv[])
{
    /* Map shared memory object for control data into memory and open its associated semaphore. */
    int shmem_fd = shm_open(TCO_SHMEM_NAME_CONTROL, O_RDONLY, 0666);
    if (shmem_fd == -1)
    {
        perror("shm_open");
        printf("Failed to open the shared memory object.\n");
        return EXIT_FAILURE;
    }
    struct tco_shmem_data_control *control_data = (struct tco_shmem_data_control *)mmap(0, TCO_SHMEM_SIZE_CONTROL, PROT_READ, MAP_SHARED, shmem_fd, 0);
    if (control_data == MAP_FAILED)
    {
        perror("mmap");
        printf("Failed to map the shared memory object into memory.\n");
        return EXIT_FAILURE;
    }
    if (close(shmem_fd) == -1) /* No longer needed. */
    {
        perror("close"); /* Not a critical error. */
    }
    sem_t *control_data_sem = sem_open(TCO_SHMEM_NAME_SEM_CONTROL, 0);
    if (control_data_sem == SEM_FAILED)
    {
        perror("sem_open");
        printf("Failed to open the semaphore associated with the shared memory object '%s'.\n", TCO_SHMEM_NAME_CONTROL);
        return EXIT_FAILURE;
    }
    /* === */

    void *io_handle = io_init();
    if (io_handle == NULL)
    {
        printf("Failed to initialize IO hardware.\n");
        return EXIT_FAILURE;
    }

    struct tco_shmem_data_control ctrl_cpy = {0};
    while (1)
    {
        if (sem_wait(control_data_sem) == -1)
        {
            perror("sem_wait");
            return EXIT_FAILURE;
        }
        /* START: Critical section */
        memcpy(&ctrl_cpy, control_data, TCO_SHMEM_SIZE_CONTROL);
        /* END: Critical section */
        if (sem_post(control_data_sem) == -1)
        {
            perror("sem_post");
            return EXIT_FAILURE;
        }

        /* Update IO */
        if (ctrl_cpy.valid > 0)
        {
            uint8_t ctrl_ch_count = sizeof(ctrl_cpy.ch) / sizeof(ctrl_cpy.ch[0]);
            for (uint8_t ch_i = 0; ch_i < ctrl_ch_count; ch_i++)
            {
                if (ctrl_cpy.ch[ch_i].active > 0)
                {
                    io_ch_control(io_handle, ch_i, ctrl_cpy.ch[ch_i].pulse_frac);
                }
                else
                {
                    /* TODO: Check if some devices need a signal other than 0.5 as a neutral position. */
                    io_ch_control(io_handle, ch_i, 0.5f);
                }
            }
        }
        usleep(100000); /* Update IO state every ~0.1 seconds (a little more than that actually). */
    }

    return EXIT_SUCCESS;
}
