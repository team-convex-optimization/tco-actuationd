#ifndef _GPIO_H_
#define _GPIO_H_

#include <gpiod.h>

#define GPIO_CHIP_NAME "gpiochip0"

enum gpio_dir
{
    IN = 0,
    OUT,
};

/**
 * @brief Initaites communication and ensures access to the GPIO kernel interface from userspace.
 * @param dir Direction of GPIO comm.
 * @param pin GPIO pin to init.
 * @return Pointer to gpiod_line.
*/
struct gpiod_line *gpio_line_init(enum gpio_dir dir, int pin);

/**
 * @brief Allows to write a value to a pin
 * @param line The gpiod_line of the pin. 
 * @param value What to write.
 * @return 0 on success, else on failure.
*/
int gpio_line_write(struct gpiod_line *line, unsigned int value);

/**
 * @brief Allows to read the value of a pin
 * @param line the gpiod_line to the pin. Must first be initialized by init_gpio_line
 * @return read value on success, else on failure
*/
int gpio_line_read(struct gpiod_line *line);

/**
 * @brief Close the gpio_chip
*/
void gpio_chip_close(struct gpiod_chip *chip);

/**
 * @brief Close a gpio_line (pin)
*/
void gpio_line_close(struct gpiod_line *line);

#endif /* _GPIO_H_ */
