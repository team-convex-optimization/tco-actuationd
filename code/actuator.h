#ifndef _ACTUATOR_H_
#define _ACTUATOR_H_

#include <stdint.h>

/* TODO: This may need to be moved to be in a more obvious place. */
#define PCA9685_I2C_ADAPTER_ID 2

/**
 * @brief Init actuator devices.
 * @return 0 on success and -1 on failure.
 */
int actr_init(void);

/**
 * @brief Deinit actuator devices.
 * @return 0 on success and -1 on failure.
 */
int actr_deinit(void);

/**
 * @brief Control one of the channels on the PCA9685 board. "actr_init" must be called before using
 * this function.
 * @param channel Channel to control.
 * @param pulse_frac Any float in range [0,1]. E.g. For a servo: 0 = min angle, 0.5 = center, 1.0 =
 * max angle.
 * @return 0 on success and -1 on failure.
 */
int actr_ch_set(uint8_t const channel, float const pulse_frac);

#endif /* _ACTUATOR_H_ */
