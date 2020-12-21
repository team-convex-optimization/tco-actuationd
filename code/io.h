#ifndef _IO_H_
#define _IO_H_

#include <stdint.h>

#define PWM_FREQ 50

/**!
 * @brief Init IO devices. The returned pointer allows other functions to refer to these initialized
 * devices without persistent internal state.
 * @return 0 on success and -1 on failure.
 */
void *io_init(void);

/**!
 * @brief Turn a servo to a given position.
 * @param io_handle Pointer received from io_init.
 * @param servo_min Minimum pulse length (0 degree position).
 * @param servo_max Maximum pulse length (whole rotation position).
 * @param ch Channel where the servo is connected to the PCA9685 chip.
 * @param turn_frac 0 is min angle, 1 is max angle.
 * @return 0 on success and -1 on failure.
 */
int io_servo_steer(void *io_handle, uint16_t const servo_min, uint16_t const servo_max, uint8_t const ch, float const turn_frac);

/**!
 * @brief Set the motor speed to a given value.
 * @param io_handle Pointer received from io_init.
 * @param ch Channel where the motor is connected to the PCA9684 chip.
 * @param speed Positive means forward, negative means backward. 0 is stationary.
 * @return 0 on success and -1 on failure.
 */
int io_motor_drive(void *io_handle, uint8_t const ch, int16_t const speed);

#endif /* _IO_H_ */
