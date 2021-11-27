#include <stdlib.h>
#include <unistd.h>

#include "tco_libd.h"

#include "actuator.h"
#include "pca9685.h"

static pca9685_handle_t pca9685_handle = {0};
static uint8_t motor_init_done = 0;         /* 0 if needs initialization and >0 if initialized.*/
static const uint8_t MOTOR_CH = 0;          /* Motor must always be plugged into this channel */
static const uint8_t MOTOR_GPIO_CALIB = 24; /* For motor calibration */
static const uint8_t MOTOR_GPIO_POWER = 23; /* For turning ESC on and off */

/**!
 * @brief Initialize the motor and calibrate it to the PWM signal from PCA9685.
 * @param actr_handle Pointer received from actr_init.
 * @param cal_gpio Number of the GPIO pin to use for controlling calibration.
 * @return 0 on success and -1 on failure.
 */
static int motor_init()
{
    /* NOTE: No need to calibrate the turnigy 80 motor/esc */
    motor_init_done = 1;
    return 0;
}

int actr_init()
{
    if (i2c_port_open(PCA9685_I2C_ADAPTER_ID, &(pca9685_handle.fd)) != ERR_OK)
    {
        log_error("Failed to open I2C adapter with ID %u", PCA9685_I2C_ADAPTER_ID);
        return -1;
    }
    if (pca9685_init(&pca9685_handle) != ERR_OK)
    {
        log_error("Failed to initialize PCA9685");
        return -1;
    }
    if (motor_init() != 0)
    {
        log_error("Failed to initialize the motor");
    }
    else
    {
        log_info("Motor was initialized");
        /* If trully initialized, 'motor_init_done' will be set accordingly. */
    }
    return 0;
}

int actr_deinit(void)
{
    if (pca9685_reset(&pca9685_handle) != ERR_OK)
    {
        log_error("Failed to deinitialize the PCA9685 board");
        return -1;
    }
    return 0;
}

int actr_ch_set(uint8_t const channel, float const pulse_frac)
{
    if (channel == MOTOR_CH && motor_init_done == 0)
    {
        /* Not critical, can still set the duty cycle but likely without any effect. */
        log_error("Motor needs to be initialized to control it");
    }
    if (pca9685_ch_frac_set(&pca9685_handle, channel, pulse_frac) != ERR_OK)
    {
        log_error("Failed to set a new pulse fraction on channel %u", channel);
        return -1;
    }
    return 0;
}
