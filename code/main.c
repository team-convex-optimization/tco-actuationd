#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

int debug = 0;
int verbose = 0;

#include "debug.h"
#include "io.h"
#include "pca9685.h"

#define SERVO_MG995_PULSE_LEN_MIN 100U /* counts */
#define SERVO_MG995_PULSE_LEN_MAX 420U /* counts */
#define CH_STEERING 0
#define CH_MOTOR 1

int main(int argc, const char *argv[])
{
    void *io_handle = io_init();
    if (io_handle == NULL)
    {
        return -1;
    }

    while (1)
    {
        for (float turn_frac = 0.0f; turn_frac < 1.0f; turn_frac += 0.01f)
        {
            if (io_servo_steer(io_handle, SERVO_MG995_PULSE_LEN_MIN, SERVO_MG995_PULSE_LEN_MAX, CH_STEERING, turn_frac) != 0)
            {
                // dbg_prnt_err("Failed to steer servo to %f turn fraction\n", turn_frac);
                return -1;
            }
            usleep(10000);
        }
        sleep(1);
        for (float turn_frac = 1.0f; turn_frac >= 0.0f; turn_frac -= 0.01f)
        {
            if (io_servo_steer(io_handle, SERVO_MG995_PULSE_LEN_MIN, SERVO_MG995_PULSE_LEN_MAX, CH_STEERING, turn_frac) != 0)
            {
                // dbg_prnt_err("Failed to steer servo to %f turn fraction\n", turn_frac);
                return -1;
            }
            usleep(10000);
        }
        sleep(1);
    }

    return EXIT_SUCCESS;
}
