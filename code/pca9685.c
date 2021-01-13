#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#include "tco_libd.h"

#include "pca9685.h"

/* TODO: The reset sequence can be moved out into a function of its own then can also restart from
arbitrary state by saving the old state first, resetting, then restoing old state. This can then be
used to set the prescale value. */
error_t pca9685_init(void **pca9685_info_loc, uint8_t pwm_freq)
{
    pca9685_info_t *pca9685_info = malloc(sizeof(pca9685_info_t));
    if (pca9685_info == NULL)
    {
        log_error("Could not allocate memory for pca9685_info_t");
        return ERR_NOMEM;
    }

    pca9685_info->fd = open(I2C_ADAPTER_PATH, O_RDWR);
    if (pca9685_info->fd < 0)
    {
        free(pca9685_info); /* Avoid memory leak. */
        log_error("Failed to open the i2c bus connected to PCA9685. Check that the connection to the board is reliable");
        log_error("open: %s", strerror(errno));
        return ERR_CRIT;
    }

    /* Software reset chip before configuring it. */
    if (pca9685_reset(pca9685_info) != ERR_OK)
    {
        free(pca9685_info); /* Avoid memory leak. */
        log_error("Failed to reset the PCA9685 chip to default values hence register state is unknown");
        return ERR_CRIT;
    }
    usleep(10); /* Reset time is 4.2 microseconds. */
    /* Chip should be in sleep mode here so it's safe to set the prescale value. */
    uint8_t prescale_val = (PCA9685_OSC_FREQ / (4096 * pwm_freq)) - 1;
    if (pca9685_reg_set(pca9685_info, PCA9685_REG_PRESCALE, prescale_val) != ERR_OK)
    {
        free(pca9685_info); /* Avoid memory leak. */
        log_error("Failed to set the prescale value");
        return ERR_CRIT;
    }

    /* Config the chip using mode registers */
    if (pca9685_reg_set(pca9685_info, PCA9685_REG_MODE1, 0U) != ERR_OK)
    {
        free(pca9685_info); /* Avoid memory leak. */
        log_error("Failed to wake up PCA9685 from SLEEP mode");
        return ERR_I2C_WRITE;
    }
    usleep(1000); /* Need to wait at least 500 microseconds before writing to the RESTART bit. */
    if (pca9685_reg_set(pca9685_info, PCA9685_REG_MODE1, PCA9685_REG_MODE1_RESTART | PCA9685_REG_MODE1_ALLCALL) != ERR_OK ||
        pca9685_reg_set(pca9685_info, PCA9685_REG_MODE2, PCA9685_REG_MODE2_OUTDRV) != ERR_OK)
    {
        free(pca9685_info); /* Avoid memory leak. */
        log_error("Failed to set mode registers");
        return ERR_I2C_WRITE;
    }

    *pca9685_info_loc = pca9685_info;
    return ERR_OK;
}

error_t pca9685_reset(void *pca9685_info_ptr)
{
    pca9685_info_t *pca9685_info = (pca9685_info_t *)pca9685_info_ptr;

    if (ioctl(pca9685_info->fd, I2C_SLAVE, PCA9685_ADDR_I2C_RESET) < 0)
    {
        log_error("Failed to send the general call address for software reset");
        log_error("ioctl: %s", strerror(errno));
        return ERR_I2C_CTRL;
    }
    /* 0x06 is special and the exact value expected by the chip after receiving a reset address. */
    if (i2c_smbus_write_byte(pca9685_info->fd, 0x06U) != 0)
    {
        log_error("Failed to send SWRST data byte");
        log_error("i2c_smbus_write_byte: %s", strerror(errno));
        return ERR_I2C_WRITE;
    }
    return ERR_OK;
}

error_t pca9685_reg_set(void *pca9685_info_ptr, uint8_t const reg_addr, uint8_t const data)
{
    pca9685_info_t *pca9685_info = (pca9685_info_t *)pca9685_info_ptr;

    if (ioctl(pca9685_info->fd, I2C_SLAVE, PCA9685_ADDR_I2C) < 0)
    {
        log_error("Failed to send the slave address");
        log_error("ioctl: %s", strerror(errno));
        return ERR_I2C_CTRL;
    }
    if (i2c_smbus_write_byte_data(pca9685_info->fd, reg_addr, data) != 0)
    {
        log_error("Failed to send register address and a new register value");
        log_error("i2c_smbus_write_byte_data: %s", strerror(errno));
        return ERR_I2C_WRITE;
    }
    return ERR_OK;
}

error_t pca9685_reg_get(void *pca9685_info_ptr, uint8_t const reg_addr, uint8_t *data)
{
    pca9685_info_t *pca9685_info = (pca9685_info_t *)pca9685_info_ptr;

    if (ioctl(pca9685_info->fd, I2C_SLAVE, PCA9685_ADDR_I2C) < 0)
    {
        log_error("Failed to send the slave address");
        log_error("ioctl: %s", strerror(errno));
        return ERR_I2C_CTRL;
    }
    int const data_tmp = i2c_smbus_read_byte_data(pca9685_info->fd, reg_addr);
    if (data_tmp < 0)
    {
        log_error("Failed to send register address and read the register value");
        log_error("i2c_smbus_read_byte: %s", strerror(errno));
        return ERR_I2C_READ;
    }
    *data = data_tmp;
    return ERR_OK;
}

error_t pca9685_reg_ch_set(void *pca9685_info_ptr, uint8_t const num_ch, uint16_t const duty_cycle)
{
    /*
    1. No need for delay so ON_HIGH and ON_LOW for the channel are zero'd out.
    2. Write duty_cycle as is since there are 4095 counts in a PWM signal where 4th bit in HIGH
       means 'always on' so its safe to write the entire 16 bit parameter.
    */
    uint8_t const zero = 0;
    /*
    Keep the 4 MSBs 0 as they are reserved anyways and 4th bit is used for the always-off bit. This
    leaves 12 bits i.e. a range of 0 to 4095.
    */
    uint8_t const packet_duty_cycle[2] = {(duty_cycle & 0x0f00) >> 8, duty_cycle & 0x00ff};
    if (pca9685_reg_set(pca9685_info_ptr, PCA9685_REG_CH(num_ch, PCA9685_REG_CH_ON, PCA9685_REG_CH_LOW), zero) != ERR_OK ||
        pca9685_reg_set(pca9685_info_ptr, PCA9685_REG_CH(num_ch, PCA9685_REG_CH_ON, PCA9685_REG_CH_HIGH), zero) != ERR_OK ||
        pca9685_reg_set(pca9685_info_ptr, PCA9685_REG_CH(num_ch, PCA9685_REG_CH_OFF, PCA9685_REG_CH_LOW), packet_duty_cycle[1]) != ERR_OK ||
        pca9685_reg_set(pca9685_info_ptr, PCA9685_REG_CH(num_ch, PCA9685_REG_CH_OFF, PCA9685_REG_CH_HIGH), packet_duty_cycle[0]) != ERR_OK)
    {
        log_error("Failed to write duty cycle to the channel registers");
        return ERR_I2C_WRITE;
    }
    return ERR_OK;
}

error_t pca9685_reg_print_all(void *pca9685_info_ptr)
{
    uint8_t mode1, mode2, prescale;
    uint8_t ch[PCA9685_REG_CH_NUM][4];
    if (pca9685_reg_get(pca9685_info_ptr, PCA9685_REG_MODE1, &mode1) != ERR_OK ||
        pca9685_reg_get(pca9685_info_ptr, PCA9685_REG_MODE2, &mode2) != ERR_OK ||
        pca9685_reg_get(pca9685_info_ptr, PCA9685_REG_PRESCALE, &prescale) != ERR_OK)
    {
        log_error("Failed to read from mode and prescale registers");
        return ERR_I2C_READ;
    }

    for (uint8_t ch_i = 0; ch_i < PCA9685_REG_CH_NUM; ch_i++)
    {
        if (pca9685_reg_get(pca9685_info_ptr, PCA9685_REG_CH(ch_i, PCA9685_REG_CH_ON, PCA9685_REG_CH_HIGH), &ch[ch_i][0]) != ERR_OK ||
            pca9685_reg_get(pca9685_info_ptr, PCA9685_REG_CH(ch_i, PCA9685_REG_CH_ON, PCA9685_REG_CH_LOW), &ch[ch_i][1]) != ERR_OK ||
            pca9685_reg_get(pca9685_info_ptr, PCA9685_REG_CH(ch_i, PCA9685_REG_CH_OFF, PCA9685_REG_CH_HIGH), &ch[ch_i][2]) != ERR_OK ||
            pca9685_reg_get(pca9685_info_ptr, PCA9685_REG_CH(ch_i, PCA9685_REG_CH_OFF, PCA9685_REG_CH_LOW), &ch[ch_i][3]) != ERR_OK)
        {
            log_error("Failed to read from the ON and/or OFF channel registers");
            return ERR_I2C_READ;
        }
    }

    log_info("====PCA9685====\n"
             "Mode1     0x%x\n"
             "Mode2     0x%x\n"
             "Prescale  %u",
             mode1, mode2, prescale);
    for (uint8_t ch_i = 0; ch_i < PCA9685_REG_CH_NUM; ch_i++)
    {
        log_info("==Ch%u==\n"
                 "ON   %04u  (0x%02x_%02x)\n"
                 "OFF  %04u  (0x%02x_%02x)",
                 ch_i, (uint16_t)((ch[ch_i][0] << 8) | ch[ch_i][1]), ch[ch_i][0], ch[ch_i][1],
                 (uint16_t)((ch[ch_i][2] << 8) | ch[ch_i][3]), ch[ch_i][2], ch[ch_i][3]);
    }

    return ERR_OK;
}
