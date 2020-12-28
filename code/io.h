#ifndef _IO_H_
#define _IO_H_

#include <stdint.h>

/**!
 * @brief Init IO devices. The returned pointer allows other functions to refer to these initialized
 * devices without persistent internal state.
 * @return 0 on success and -1 on failure.
 */
void *io_init(void);

/**!
 * @brief Control one of the channels on the PCA9685 board.
 * @param io_handle Pointer received from io_init.
 * @param ch Channel to control.
 * @param control_frac Any float in range [0,1].
 * @return 0 on success and -1 on failure.
 */
int io_ch_control(void *io_handle,
                  uint8_t const ch,
                  float const control_frac);

#endif /* _IO_H_ */
