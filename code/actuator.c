#include <stdlib.h>

#include "actuator.h"
#include "pca9685.h"
#include "tco_libd.h"

#define PWM_FREQ 50        /* Hz */
#define MOTOR_CH 0         /* Motor must always be plugged into this channel. */
#define MOTOR_GPIO_CALIB 0 /* GPIO pin to use for motor calibration. */

#define PULSE_LEN_MIN_DEFUALT 100
#define PULSE_LEN_MAX_DEFUALT 200

static uint8_t motor_needs_init = 1; /* >0 if needs initialization and =1 if already initialized.*/
/* XXX: Pulse length min and max are handled internally here and nowhere else! */
static uint16_t const ch_pulse_length[PCA9685_REG_CH_NUM][2] = {
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
 * @param actr_handle Pointer received from actr_init.
 * @param cal_gpio Number of the GPIO pin to use for controlling calibration.
 * @return 0 on success and -1 on failure.
 */
static int motor_init(void *actr_handle, uint8_t const cal_gpio)
{
    return 0;
}

void *actr_init(void)
{
    void *actr_handle;
    if (pca9685_init(&actr_handle, PWM_FREQ) != ERR_OK)
    {
        log_error("Failed to initialize PCA9685");
        return NULL;
    }
    return actr_handle;
}

int actr_ch_control(void *actr_handle,
                    uint8_t const ch,
                    float const pulse_frac)
{
    if (ch > PCA9685_REG_CH_NUM)
    {
        log_error("Channel %u does not exist", ch);
        return -1;
    }
    if (pulse_frac < 0 || pulse_frac > 1)
    {
        log_error("Control fraction is not in the range 0 to 1");
        return -1;
    }
    uint16_t const duty_cycle = ch_pulse_length[ch][0] + ((ch_pulse_length[ch][1] - ch_pulse_length[ch][0]) * pulse_frac);
    log_debug("Channel %u set to duty cycle %u", ch, duty_cycle);
    if (ch == MOTOR_CH && motor_needs_init > 0)
    {
        if (motor_init(actr_handle, MOTOR_GPIO_CALIB) != 0)
        {
            log_error("Failed to initialize the motor");
            return -1;
        }
        else
        {
            log_info("Motor was initialized");
            motor_needs_init = 0;
        }
    }
    if (pca9685_reg_ch_set(actr_handle, ch, duty_cycle) != ERR_OK)
    {
        log_error("Failed to set the new duty cycle for channel %u", ch);
        return -1;
    }
    return 0;
}
