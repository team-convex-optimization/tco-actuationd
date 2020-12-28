#include <stdlib.h>

#include "io.h"
#include "pca9685.h"
#include "debug.h"

#define PWM_FREQ 50        /* Hz */
#define MOTOR_CH 0         /* Motor must always be plugged into this channel. */
#define MOTOR_GPIO_CALIB 0 /* GPIO pin to use for motor calibration. */

#define PULSE_LEN_MIN_DEFUALT 100
#define PULSE_LEN_MAX_DEFUALT 200

static uint8_t motor_needs_init = 1; /* >0 if needs initialization and =1 if already initialized.*/
/* XXX: Pulse length min and max are handled internally here and nowhere else! */
static const uint16_t ch_pulse_length[PCA9685_REG_CH_NUM][2] = {
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {100, 420},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT},
};

/**!
 * @brief Initialize the motor and calibrate it to the PWM signal from PCA9685.
 * @param io_handle Pointer received from io_init.
 * @param cal_gpio Number of the GPIO pin to use for controlling calibration.
 * @return 0 on success and -1 on failure.
 */
static int motor_init(void *io_handle, uint8_t const cal_gpio)
{
    return 0;
}

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

int io_ch_control(void *io_handle,
                  uint8_t const ch,
                  float const control_frac)
{
    if (ch > PCA9685_REG_CH_NUM)
    {
        dbg_prnt_err("Channel %u does not exist.", ch);
        return -1;
    }
    if (control_frac < 0 || control_frac > 1)
    {
        dbg_prnt_err("Control fraction is not in the range 0 to 1.");
        return -1;
    }
    uint16_t const duty_cycle = ch_pulse_length[ch][0] + ((ch_pulse_length[ch][1] - ch_pulse_length[ch][0]) * control_frac);
    dbg_prnt_dbg("Channel %u set to duty cycle %u counts\n", ch, duty_cycle);
    if (ch == MOTOR_CH && motor_needs_init > 0)
    {
        if (motor_init(io_handle, MOTOR_GPIO_CALIB) != 0)
        {
            dbg_prnt_err("Failed to initialize the motor.");
            return -1;
        }
        else
        {
            dbg_prnt_inf("Motor was initialized.");
            motor_needs_init = 0;
        }
    }
    if (pca9685_reg_ch_set(io_handle, ch, duty_cycle) != ERR_OK)
    {
        dbg_prnt_err("Failed to set the new duty cycle for channel %u.", ch);
        return -1;
    }
    return 0;
}
