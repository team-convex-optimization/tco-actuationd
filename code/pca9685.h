#ifndef _PCA9685_H_
#define _PCA9685_H_

#include <stdint.h>

/* I2C adapter connected to PCA9685 */
#define I2C_ADAPTER_PATH "/dev/i2c-1"

/* PCA9685 hardware definition */
#define PCA9685_ADDR_I2C 0x40
#define PCA9685_ADDR_I2C_RESET 0x0
#define PCA9685_REG_CH_ON 0U
#define PCA9685_REG_CH_OFF 1U
#define PCA9685_REG_CH_LOW 0U
#define PCA9685_REG_CH_HIGH 1U
#define PCA9685_REG_CH_NUM 16U
#define PCA9685_REG_PRESCALE_MIN 3
#define PCA9685_REG_PRESCALE_MAX 255 /* prescale = (PCA9685_OSC_FREQ/(4096 * desired_freq)) - 1 */
#define PCA9685_OSC_FREQ 25000000    /* 25 MHz */

/* PCA9685 register addresses */
#define PCA9685_REG_MODE1 0x00U
#define PCA9685_REG_MODE2 0x01U
#define PCA9685_REG_SUBADDR1 0x02U
#define PCA9685_REG_SUBADDR2 0x03U
#define PCA9685_REG_SUBADDR3 0x04U
#define PCA9685_REG_ALLCALLADDR 0x05U
/*
For all channels, the ON and OFF registers must never have the same value. ON count = counts before
start of logical 1 which is the delay. OFF count = counts before start of logical 0 (on PWM signal)
which is logical 1 counts - ON count. Whole signal is DELAY->ON->OFF. If logical 1 in the 4th bit of
HIGH registers, then always ON (if in HIGH ON register) or always OFF (if in HIGH OFF register)
*/
#define PCA9685_REG_CH(ch_num, on_off, low_high) (((ch_num * 4U) + 0x06U + low_high) + (2U * on_off))
#define PCA9685_REG_ALL(on_off, low_high) ((0xfaU + low_high) + (2U * on_off))
#define PCA9685_REG_PRESCALE 0xfeU
#define PCA9685_REG_TESTMODE 0xffU

/* Mode1 register settings */
/* ALLCALL | SUB3 | SUB2 | SUB1 | SLEEP | AI | EXTCLK | RESTART */
#define PCA9685_REG_MODE1_ALLCALL 0x01U /* Enable or disable response to PCA9685_REG_ALLCALLADDR */
#define PCA9685_REG_MODE1_SUB3 0x02U    /* Enable or disable response to chained PCA9685 chips */
#define PCA9685_REG_MODE1_SUB2 0x04U
#define PCA9685_REG_MODE1_SUB1 0x08U
#define PCA9685_REG_MODE1_SLEEP 0x10U   /* Switch to low power mode with oscillator turned off (1) */
#define PCA9685_REG_MODE1_AUTOINC 0x20U /* Autoincrement address in control register after each access (1) */
#define PCA9685_REG_MODE1_EXTCLK 0x40U  /* Use internal(0) or external(1) clock for PWM */
#define PCA9685_REG_MODE1_RESTART 0x80U /* Used to restart PWM channels after SLEEP (1) */

/* Mode2 register settings */
/* OUTNE(2) | OUTDRV | OCH | INVRT | X(3) */
#define PCA9685_REG_MODE2_OUTNE_0 0x01U /* If OE is high, the OUTNE bits determine desired output on all channels */
#define PCA9685_REG_MODE2_OUTNE_1 0x02U
#define PCA9685_REG_MODE2_OUTDRV 0x04U /* Use totem-pole (1) or open-drain (0)  */
#define PCA9685_REG_MODE2_OCH 0x08U    /* Change outputs on STOP (0) or on ACK (1)  */
#define PCA9685_REG_MODE2_INVRT 0x10U  /* Use inverted channel register bits to determine duty cycle (1) */

/* PCA9685 register defaults */
#define PCA9685_REG_MODE1_DEFAULT PCA9685_REG_MODE1_SLEEP | PCA9685_REG_MODE1_ALLCALL
#define PCA9685_REG_MODE2_DEFAULT PCA9685_REG_MODE2_OUTDRV
#define PCA9685_REG_PRESCALE_DEFAULT 30U /* Default PWM freq is 200Hz  */

/* Errors */
typedef enum
{
    ERR_OK = 0, /* Weird naming but its done for consistency. */
    ERR_UNK = 1,
    ERR_CRIT,
    ERR_NOMEM,
    ERR_I2C_WRITE,
    ERR_I2C_READ,
    ERR_I2C_CTRL
} error_t;

typedef struct
{
    int fd;
    uint8_t reg_mode1;
    uint8_t reg_mode2;
    uint8_t reg_prescale;
} pca9685_info_t;

/**
 * @brief Establishes connection to the PCA9685 chip over I2C and returns a void pointer to an
 * pca9685_info_t struct which contains persistent pca9685 info.
 * @param pca9685_info_ptr Location where to write the pointer to pca9685_info_t created on init.
 * @param pwm_freq The desired PWM frequency in Hz.
 * @return Status code
 */
error_t pca9685_init(void **pca9685_info_loc, uint8_t pwm_freq);

/**
 * @brief Perform a software reset of the PCA9685 chip to default settings.
 * @param pca9685_info_ptr Void pointer to the struct received on pca9685_init.
 * @return Status code
 */
error_t pca9685_reset(void *pca9685_info_ptr);

/**
 * @brief Set the value of a register on the PCA9685 chip.
 * @param pca9685_info_ptr Void pointer to the struct received on pca9685_init.
 * @param reg_addr Address of the register to write to.
 * @param data Value to set the register to.
 * @return Status code
 */
error_t pca9685_reg_set(void *pca9685_info_ptr, uint8_t const reg_addr, uint8_t const data);

/**
 * @brief Get the value of a register from the PCA9685 chip.
 * @param pca9685_info_ptr Void pointer to the struct received on pca9685_init.
 * @param reg_addr Address of the register to read from.
 * @param data Pointer where to write the read register value to. Underfined on error.
 * @return Status code
 */
error_t pca9685_reg_get(void *pca9685_info_ptr, uint8_t const reg_addr, uint8_t *data);

/**
 * @brief A simple interface function to set the duty cycle for a specified channel.
 * @param pca9685_info_ptr Void pointer to the struct received on pca9685_init.
 * @param num_ch Number of the channel to modify.
 * @param duty_cycle Duty cycle of the desired signal. 0 : 0%, >=(2^12)-1 : 100%.
 * @return Status code
 */
error_t pca9685_reg_ch_set(void *pca9685_info_ptr, uint8_t const num_ch, uint16_t const duty_cycle);

/**
 * @brief Print the contents of all registers on the PCA9685 chip.
 * @param pca9685_info_ptr Void pointer to the struct received on pca9685_init.
 * @return Status code
 */
error_t pca9685_reg_print_all(void *pca9685_info_ptr);

#endif /* _PCA9685_H_ */
