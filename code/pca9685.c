#include <unistd.h>

#include "pca9685.h"

/*
For all channels, the ON and OFF registers must never have the same value. ON count = counts before
start of logical 1 which is the delay. OFF count = counts before start of logical 0 (on PWM signal)
which is logical 1 counts - ON count. Whole signal is DELAY->ON->OFF. If logical 1 in the 4th bit of
HIGH registers, then always ON (if in HIGH ON register) or always OFF (if in HIGH OFF register).
*/
#define PCA9685_REG_CH(ch_num, on_off, low_high) (((ch_num * 4U) + 0x06U + low_high) + (2U * on_off))
#define PCA9685_REG_ALL(on_off, low_high) ((0xfaU + low_high) + (2U * on_off))

#define PWM_FREQ 50 /* Hz */
#define PRESCALE_VAL (PCA9685_OSC_FREQ / (4096 * PWM_FREQ)) - 1

#define PULSE_LEN_MIN_DEFUALT 100
#define PULSE_LEN_MAX_DEFUALT 200
#define PULSE_LEN_INVERT_DEFUALT 0

/* PCA9685 register defaults. */
#define PCA9685_REG_MODE1_DEFAULT PCA9685_REG_MODE1_SLEEP | PCA9685_REG_MODE1_ALLCALL
#define PCA9685_REG_MODE2_DEFAULT PCA9685_REG_MODE2_OUTDRV
#define PCA9685_REG_PRESCALE_DEFAULT 30U /* Default PWM freq is 200Hz  */

/* XXX: Pulse length min and max are handled internally here and nowhere else! */
uint16_t static const CH_PULSE_LENGTH[PCA9685_REG_CH_NUM][3] = {
    {210, 440, 0},
    {190, 410, 1},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
    {PULSE_LEN_MIN_DEFUALT, PULSE_LEN_MAX_DEFUALT, PULSE_LEN_INVERT_DEFUALT},
};

error_t pca9685_init(pca9685_handle_t *const handle)
{
    if (pca9685_reset(handle) != ERR_OK)
    {
        log_error("Failed to reset the PCA9685 chip");
        return ERR_CRIT;
    }

    /* Data for all subsequent I2C writes. */
    uint8_t data[4] = {PRESCALE_VAL, 0U, PCA9685_REG_MODE1_RESTART | PCA9685_REG_MODE1_ALLCALL, PCA9685_REG_MODE2_OUTDRV};

    /* Chip should be in sleep mode here so it's safe to set the prescale value. */
    if (i2c_cmd_write(handle->fd, PCA9685_ADDR, PCA9685_REG_PRESCALE, &(data[0]), 1) != ERR_OK)
    {
        log_error("Failed to set the prescale value");
        return ERR_CRIT;
    }
    handle->prescale = data[0];

    /* Config the chip using mode registers. */
    if (i2c_cmd_write(handle->fd, PCA9685_ADDR, PCA9685_REG_MODE1, &data[1], 1) != ERR_OK)
    {
        log_error("Failed to wake up PCA9685 from SLEEP mode");
        return ERR_I2C_WRITE;
    }

    usleep(1000); /* Need to wait at least 500 microseconds before writing to the RESTART bit. */
    if (i2c_cmd_write(handle->fd, PCA9685_ADDR, PCA9685_REG_MODE1, &data[2], 1) != ERR_OK ||
        i2c_cmd_write(handle->fd, PCA9685_ADDR, PCA9685_REG_MODE2, &data[3], 1) != ERR_OK)
    {
        log_error("Failed to set mode registers");
        return ERR_I2C_WRITE;
    }

    return ERR_OK;
}

error_t pca9685_reset(pca9685_handle_t *const handle)
{
    /* 0x06 is special and the exact value expected by the chip after receiving a reset address. */
    if (i2c_cmd_write(handle->fd, PCA9685_RESET_ADDR, 0x06U, NULL, 0) != 0)
    {
        log_error("Failed to send SWRST data byte");
        return ERR_I2C_WRITE;
    }
    usleep(10); /* Reset time is 4.2 microseconds. */
    /* Chip should be in sleep more right now. */
    return ERR_OK;
}

int pca9685_ch_frac_set(pca9685_handle_t *const handle, uint8_t const channel, float const pulse_frac)
{
    /*
    1. No need for delay so ON_HIGH and ON_LOW for the channel are zero'd out.
    2. Write duty_cycle as is since there are 4095 counts in a PWM signal where 4th bit in HIGH
       means 'always on' so its safe to write the entire 16 bit parameter.

    Keep the 4 MSBs 0 as they are reserved anyways and 4th bit is used for the always-off bit. This
    leaves 12 bits i.e. a range of 0 to 4095.
    */
    if (channel > PCA9685_REG_CH_NUM)
    {
        log_error("Channel %u does not exist", channel);
        return -1;
    }
    if (pulse_frac < 0 || pulse_frac > 1)
    {
        log_error("Pulse fraction is not in the range 0 to 1");
        return -1;
    }
    float real_pulse_frac = 0.0f;
    if (CH_PULSE_LENGTH[channel][2])
    {
        real_pulse_frac = 1.0 - pulse_frac;
    }
    else
    {
        real_pulse_frac = pulse_frac;
    }
    uint16_t const duty_cycle = CH_PULSE_LENGTH[channel][0] + ((CH_PULSE_LENGTH[channel][1] - CH_PULSE_LENGTH[channel][0]) * real_pulse_frac);
    if (pca9685_ch_raw_set(handle, channel, duty_cycle) != 0)
    {
        return -1;
    }
    return 0;
}

int pca9685_ch_raw_set(pca9685_handle_t *const handle, uint8_t const channel, uint16_t const duty_cycle)
{
    uint8_t data[4] = {0, 0, duty_cycle & 0x00ff, (duty_cycle & 0x0f00) >> 8};
    if (i2c_cmd_write(handle->fd, PCA9685_ADDR, PCA9685_REG_CH(channel, PCA9685_REG_CH_ON, PCA9685_REG_CH_LOW), &data[0], 1) != ERR_OK ||
        i2c_cmd_write(handle->fd, PCA9685_ADDR, PCA9685_REG_CH(channel, PCA9685_REG_CH_ON, PCA9685_REG_CH_HIGH), &data[1], 1) != ERR_OK ||
        i2c_cmd_write(handle->fd, PCA9685_ADDR, PCA9685_REG_CH(channel, PCA9685_REG_CH_OFF, PCA9685_REG_CH_LOW), &data[3], 1) != ERR_OK ||
        i2c_cmd_write(handle->fd, PCA9685_ADDR, PCA9685_REG_CH(channel, PCA9685_REG_CH_OFF, PCA9685_REG_CH_HIGH), &data[4], 1) != ERR_OK)
    {
        log_error("Failed to write duty cycle to the channel registers");
        return -1;
    }
    return 0;
}
