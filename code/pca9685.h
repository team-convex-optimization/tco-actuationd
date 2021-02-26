#ifndef _PCA9685_H_
#define _PCA9685_H_

#include <stdint.h>
#include "tco_libd.h"

#define PCA9685_ADDR 0x40
#define PCA9685_RESET_ADDR 0x0
#define PCA9685_REG_CH_NUM 16U
#define PCA9685_REG_PRESCALE_MIN 3
#define PCA9685_REG_PRESCALE_MAX 255
#define PCA9685_OSC_FREQ 25000000 /* 25 MHz. */

/* PCA9685 hardware definition. */
typedef enum pca9685_reg_off_t
{
    PCA9685_REG_CH_ON = 0U,
    PCA9685_REG_CH_OFF = 1U,
    PCA9685_REG_CH_LOW = 0U,
    PCA9685_REG_CH_HIGH = 1U,
} pca9685_reg_off_t;

/* PCA9685 register addresses. */
typedef enum pca9685_reg_t
{
    PCA9685_REG_MODE1 = 0x00U,
    PCA9685_REG_MODE2 = 0x01U,
    PCA9685_REG_SUBADDR1 = 0x02U,
    PCA9685_REG_SUBADDR2 = 0x03U,
    PCA9685_REG_SUBADDR3 = 0x04U,
    PCA9685_REG_ALLCALLADDR = 0x05U
} pca9685_reg_t;

/*
For all channels, the ON and OFF registers must never have the same value. ON count = counts before
start of logical 1 which is the delay. OFF count = counts before start of logical 0 (on PWM signal)
which is logical 1 counts - ON count. Whole signal is DELAY->ON->OFF. If logical 1 in the 4th bit of
HIGH registers, then always ON (if in HIGH ON register) or always OFF (if in HIGH OFF register).
*/
#define PCA9685_REG_CH(ch_num, on_off, low_high) (((ch_num * 4U) + 0x06U + low_high) + (2U * on_off))
#define PCA9685_REG_ALL(on_off, low_high) ((0xfaU + low_high) + (2U * on_off))
#define PCA9685_REG_PRESCALE 0xfeU /* prescale = (PCA9685_OSC_FREQ/(4096 * desired_freq)) - 1. */
#define PCA9685_REG_TESTMODE 0xffU

/* Mode1 register settings. */
/* ALLCALL | SUB3 | SUB2 | SUB1 | SLEEP | AI | EXTCLK | RESTART. */
typedef enum pca9685_reg_mode1_t
{
    PCA9685_REG_MODE1_ALLCALL = 0x01U, /* Enable or disable response to PCA9685_REG_ALLCALLADDR. */
    PCA9685_REG_MODE1_SUB3 = 0x02U,    /* Enable or disable response to chained PCA9685 chips. */
    PCA9685_REG_MODE1_SUB2 = 0x04U,
    PCA9685_REG_MODE1_SUB1 = 0x08U,
    PCA9685_REG_MODE1_SLEEP = 0x10U,   /* Switch to low power mode with oscillator turned off (1). */
    PCA9685_REG_MODE1_AUTOINC = 0x20U, /* Autoincrement address in control register after each access (1). */
    PCA9685_REG_MODE1_EXTCLK = 0x40U,  /* Use internal(0) or external(1) clock for PWM. */
    PCA9685_REG_MODE1_RESTART = 0x80U  /* Used to restart PWM channels after SLEEP (1). */
} pca9685_reg_mode1_t;

/* Mode2 register settings. */
/* OUTNE(2) | OUTDRV | OCH | INVRT | X(3). */
typedef enum pca9685_reg_mode2_t
{
    PCA9685_REG_MODE2_OUTNE_0 = 0x01U, /* If OE is high, the OUTNE bits determine desired output on all channels. */
    PCA9685_REG_MODE2_OUTNE_1 = 0x02U,
    PCA9685_REG_MODE2_OUTDRV = 0x04U, /* Use totem-pole (1) or open-drain (0). */
    PCA9685_REG_MODE2_OCH = 0x08U,    /* Change outputs on STOP (0) or on ACK (1). */
    PCA9685_REG_MODE2_INVRT = 0x10U   /* Use inverted channel register bits to determine duty cycle (1). */
} pca9685_reg_mode2_t;

/* PCA9685 register defaults. */
#define PCA9685_REG_MODE1_DEFAULT PCA9685_REG_MODE1_SLEEP | PCA9685_REG_MODE1_ALLCALL
#define PCA9685_REG_MODE2_DEFAULT PCA9685_REG_MODE2_OUTDRV
#define PCA9685_REG_PRESCALE_DEFAULT 30U /* Default PWM freq is 200Hz  */

/**
 * @brief Establishes connection to the PCA9685 chip and configures it to desired values.
 * @param i2c_port_fd FD for the opened I2C adapter connected to PCA9685.
 * @return Status code.
 */
i2c_error_t pca9685_init(int const i2c_port_fd);

/**
 * @brief Resets the chip and put it into sleep mode.
 * @param i2c_port_fd FD for the opened I2C adapter connected to PCA9685.
 * @return Status code.
 */
i2c_error_t pca9685_reset(int const i2c_port_fd);

/**
 * @brief A simple interface function to set the duty cycle for a specified channel by specifying a pulse fraction.
 * @param i2c_port_fd FD for the opened I2C adapter connected to PCA9685.
 * @param num_ch Number of the channel to modify.
 * @param pulse_frac Pulse fraction of the desired signal. 0:0%, 1:100.
 * @return 0 on success, -1 on failure.
 */
int pca9685_ch_frac_set(int const i2c_port_fd, uint8_t const num_ch, float const pulse_frac);

/**
 * @brief A simple interface function to set the duty cycle for a specified channel.
 * @param i2c_port_fd FD for the opened I2C adapter connected to PCA9685.
 * @param num_ch Number of the channel to modify.
 * @param duty_cycle Duty cycle of the desired signal. 0:0%, >=(2^12)-1:100%.
 * @return 0 on success, -1 on failure.
 */
int pca9685_ch_raw_set(int const i2c_port_fd, uint8_t const channel, uint16_t const duty_cycle);

#endif /* _PCA9685_H_ */
