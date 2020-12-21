#include <stdlib.h>

#include "io.h"
#include "pca9685.h"
#include "debug.h"

void *io_init(void)
{
    void *io_handle;
    if (pca9685_init(&io_handle, PWM_FREQ) != ERR_OK)
    {
        dbg_prnt_err("Failed to initialize PCA9685.");
        return NULL;
    }
    return io_handle;
}

int io_servo_steer(void *io_handle, uint16_t const servo_min, uint16_t const servo_max, uint8_t const ch, float const turn_frac)
{
    if (turn_frac < 0 || turn_frac > 1)
    {
        dbg_prnt_err("Turn fraction is not in the range 0 to 1.");
        return -1;
    }
    uint16_t const duty_cycle = servo_min + ((servo_max - servo_min) * turn_frac);
    dbg_prnt_inf("Channel %u set to duty cycle %u counts\n", ch, duty_cycle);
    if (pca9685_reg_ch_set(io_handle, ch, duty_cycle) != ERR_OK)
    {
        dbg_prnt_err("Failed to set the new duty cycle for channel %u.", ch);
        return -1;
    }
    return 0;
}

int io_motor_drive(void *io_handle, uint8_t const ch, int16_t const speed)
{
    return -1;
}
