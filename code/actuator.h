#ifndef _ACTUATOR_H_
#define _ACTUATOR_H_

#include <stdint.h>

/**!
 * @brief Init actuator devices. The returned pointer allows other functions to refer to these initialized
 * devices without persistent internal state.
 * @return 0 on success and -1 on failure.
 */
void *actr_init(void);

/**!
 * @brief Control one of the channels on the PCA9685 board.
 * @param actr_handle Pointer received from io_init.
 * @param ch Channel to control.
 * @param pulse_frac Any float in range [0,1]. E.g. For a servo: 0 = min angle, 0.5 = center, 1.0 =
 * max angle.
 * @return 0 on success and -1 on failure.
 */
int actr_ch_control(void *actr_handle,
                    uint8_t const ch,
                    float const pulse_frac);

#endif /* _ACTUATOR_H_ */
