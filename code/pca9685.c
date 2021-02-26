#include <unistd.h>

#include "pca9685.h"

#define PWM_FREQ 50 /* Hz */
#define PRESCALE_VAL (PCA9685_OSC_FREQ / (4096 * PWM_FREQ)) - 1

#define PULSE_LEN_MIN_DEFUALT 100
#define PULSE_LEN_MAX_DEFUALT 200
#define PULSE_LEN_INVERT_DEFUALT 0

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

i2c_error_t pca9685_init(int const i2c_port_fd)
{
    if (pca9685_reset(i2c_port_fd) != ERR_OK)
    {
        log_error("Failed to reset the PCA9685 chip");
        return ERR_CRIT;
    }
    /* Chip should be in sleep mode here so it's safe to set the prescale value. */
    if (i2c_cmd_write(i2c_port_fd, PCA9685_ADDR, PCA9685_REG_PRESCALE, PRESCALE_VAL, 1) != ERR_OK)
    {
        log_error("Failed to set the prescale value");
        return ERR_CRIT;
    }
    /* Config the chip using mode registers. */
    if (i2c_cmd_write(i2c_port_fd, PCA9685_ADDR, PCA9685_REG_MODE1, 0U, 1) != ERR_OK)
    {
        log_error("Failed to wake up PCA9685 from SLEEP mode");
        return ERR_I2C_WRITE;
    }

    usleep(1000); /* Need to wait at least 500 microseconds before writing to the RESTART bit. */
    if (i2c_cmd_write(i2c_port_fd, PCA9685_ADDR, PCA9685_REG_MODE1, PCA9685_REG_MODE1_RESTART | PCA9685_REG_MODE1_ALLCALL, 1) != ERR_OK ||
        i2c_cmd_write(i2c_port_fd, PCA9685_ADDR, PCA9685_REG_MODE2, PCA9685_REG_MODE2_OUTDRV, 1) != ERR_OK)
    {
        log_error("Failed to set mode registers");
        return ERR_I2C_WRITE;
    }

    return ERR_OK;
}

i2c_error_t pca9685_reset(int const i2c_port_fd)
{
    /* 0x06 is special and the exact value expected by the chip after receiving a reset address. */
    if (i2c_cmd_write(i2c_port_fd, PCA9685_RESET_ADDR, 0x06U, 0, 0) != 0)
    {
        log_error("Failed to send SWRST data byte");
        return ERR_I2C_WRITE;
    }
    usleep(10); /* Reset time is 4.2 microseconds. */
    /* Chip should be in sleep more right now. */
    return ERR_OK;
}

int pca9685_ch_frac_set(int const i2c_port_fd, uint8_t const channel, float const pulse_frac)
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
    if (pca9685_ch_raw_set(i2c_port_fd, channel, duty_cycle) != 0)
    {
        return -1;
    }
    return 0;
}

int pca9685_ch_raw_set(int const i2c_port_fd, uint8_t const channel, uint16_t const duty_cycle)
{
    uint8_t const packet_duty_cycle[2] = {(duty_cycle & 0x0f00) >> 8, duty_cycle & 0x00ff};
    if (i2c_cmd_write(i2c_port_fd, PCA9685_ADDR, PCA9685_REG_CH(channel, PCA9685_REG_CH_ON, PCA9685_REG_CH_LOW), 0, 1) != ERR_OK ||
        i2c_cmd_write(i2c_port_fd, PCA9685_ADDR, PCA9685_REG_CH(channel, PCA9685_REG_CH_ON, PCA9685_REG_CH_HIGH), 0, 1) != ERR_OK ||
        i2c_cmd_write(i2c_port_fd, PCA9685_ADDR, PCA9685_REG_CH(channel, PCA9685_REG_CH_OFF, PCA9685_REG_CH_LOW), packet_duty_cycle[1], 1) != ERR_OK ||
        i2c_cmd_write(i2c_port_fd, PCA9685_ADDR, PCA9685_REG_CH(channel, PCA9685_REG_CH_OFF, PCA9685_REG_CH_HIGH), packet_duty_cycle[0], 1) != ERR_OK)
    {
        log_error("Failed to write duty cycle to the channel registers");
        return -1;
    }
    return 0;
}
