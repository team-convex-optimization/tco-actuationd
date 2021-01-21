#ifndef _ULTRASOUND_H_
#define _ULTRASOUND_H_

#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "tco_libd.h"

#include "gpio.h"

#define NANO_SEC_TO_SEC 1000000000.0
#define SPEED_OF_SOUND_CM_HALF 17150 /* (1/2)speed of sound in cm/s. */

typedef struct {
    struct gpiod_line *trig;
    struct gpiod_line *echo;
} sensor_ultrasound;

/**
 * @brief will initialize an ultrasound sensor with given GPIO ports
 * @param gpio_trig GPIO pin with the ultrasound trigger
 * @param gpio_echo GPIO pin with the ultrasound echo
 * @return NULL of failure, a void * to the sensor_ultrasound struct
 */
void *us_init(int gpio_trig, int gpio_echo);

/**
 * @brief Returns the distance from a given ultrasound sensor
 * @param us a pointer to an ultrasound sensor
 * @return the distance in cm's
 */
double us_get_distance(sensor_ultrasound *us);

/**
 * @brief Clears the sensor_ultrasound memory and GPIO pins
 * @param us a pointer to an ultrasound sensor
 */
void us_cleanup(sensor_ultrasound *us);

/**
 * @brief Test an ultrasound sensor
 * @param gpio_trig GPIO pin with the ultrasound trigger
 * @param gpio_echo GPIO pin with the ultrasound echo
 * @param num_pings The number of "echos" to recieve from the sensor
 */
void us_test(int gpio_trig, int gpio_echo, int num_pings);

/**
 * @brief Print out a usage message for the ultrasound test mode.
 */
void us_usage(void);

#endif /* _US_H */
