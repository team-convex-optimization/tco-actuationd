#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>

#include "actuator.h"
#include "pca9685.h"
#include "calibration.h"
#include "ultrasound.h"

#include "tco_shmem.h"
#include "tco_libd.h"

#define ULTRASOUND_TRIGGER 27
#define ULTRASOUND_ECHO 22
#define MIN_DRIVE_CLEARANCE 50 /* Minimum clearance the US sensor must read to continue motor power */
#define MOTOR_CHANNEL 0 /* The channel the motor is on in the pca9685 board */

int log_level = LOG_INFO | LOG_DEBUG | LOG_ERROR;

int main(int argc, const char *argv[])
{
    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0))
    {
        cal_usage();
        printf("\n=================\n\n");
        us_usage();
        return EXIT_SUCCESS;
    }

    if (log_init("actuationd", "./log.txt") != 0)
    {
        printf("Failed to initialize the logger\n");
        return EXIT_FAILURE;
    }

    if (argc > 1 && ((strcmp(argv[1], "--ultra") == 0 || strcmp(argv[1], "-u") == 0)))
    {
        if (argc != 5) 
        {
            printf("Incorrect usage. Use '-h' for more info.\n");
            return EXIT_FAILURE;
        }
        us_test(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
        return EXIT_SUCCESS;
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

    if (argc == 2 && (strcmp(argv[1], "--calibrate") == 0 || strcmp(argv[1], "-c") == 0))
    {
        cal_main(actr_handle);
        return EXIT_SUCCESS;
    }

    /* Initialize the ultrasound */
    sensor_ultrasound *us = us_init(ULTRASOUND_TRIGGER, ULTRASOUND_ECHO);

    struct tco_shmem_data_control ctrl_cpy = {0};
    while (1)
    {
         /* Check US_distance to ensure it is safe */
        double clearance = us_get_distance(us);
        if (clearance < MIN_DRIVE_CLEARANCE)
        {
            actr_ch_control(actr_handle, MOTOR_CHANNEL, 0.5f);
            goto end_loop;
        }

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
        end_loop:
        usleep(10000); /* Update actuator state every ~0.01 seconds (a little more than that actually). */
    }

    return EXIT_SUCCESS;
}
