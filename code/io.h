#ifndef _IO_H_
#define _IO_H_

#include <stdint.h>

#define PWM_FREQ 50 /* Hz */

/**!
 * @brief Init IO devices. The returned pointer allows other functions to refer to these initialized
 * devices without persistent internal state.
 * @return 0 on success and -1 on failure.
 */
void *io_init(void);

/**!
 * @brief Turn a servo to a given position.
 * @param io_handle Pointer received from io_init.
 * @param pulse_len_min Minimum pulse length (0 degree position).
 * @param pulse_len_max Maximum pulse length (whole rotation position).
 * @param ch Channel where the servo is connected to the PCA9685 chip.
 * @param turn_frac 0 is min angle, 1 is max angle.
 * @return 0 on success and -1 on failure.
 */
int io_servo_steer(void *io_handle,
                   uint16_t const pulse_len_min,
                   uint16_t const pulse_len_max,
                   uint8_t const ch,
                   float const turn_frac);

/**!
 * @brief Initialize the motor and calibrate it to the PWM signal from PCA9685.
 * @param io_handle Pointer received from io_init.
 * @param pulse_len_min Minimum pulse length (max throttle backward).
 * @param pulse_len_max Maximum pulse length (max throttle forward).
 * @param ch Channel where the motor is connected to the PCA9684 chip.
 * @param throttle Positive means forward, negative means backward. 0 is stationary.
 * @param cal_gpio Number of the GPIO pin to use for controlling calibration.
 * @return 0 on success and -1 on failure.
 */
int io_motor_init(void *io_handle,
                  uint16_t const pulse_len_min,
                  uint16_t const pulse_len_max,
                  uint8_t const ch,
                  uint8_t const cal_gpio);

/**!
 * @brief Set the motor speed to a given value.
 * @param io_handle Pointer received from io_init.
 * @param pulse_len_min Minimum pulse length (0 degree position).
 * @param pulse_len_max Maximum pulse length (whole rotation position).
 * @param ch Channel where the motor is connected to the PCA9684 chip.
 * @param throttle_frac 0 = max reverse, 0.5 = zero throttle, 1 = max throttle. 
 * @return 0 on success and -1 on failure.
 */
int io_motor_drive(void *io_handle,
                   uint16_t const pulse_len_min,
                   uint16_t const pulse_len_max,
                   uint8_t const ch,
                   float const throttle_frac);

#endif /* _IO_H_ */
