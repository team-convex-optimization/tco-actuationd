#ifndef _ULTRASOUND_H_
#define _ULTRASOUND_H_

#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "tco_libd.h"

#include "gpio.h"

#define NANO_SEC_TO_SEC 1000000000.0

typedef struct {
    struct gpiod_line *trig;
    struct gpiod_line *echo;
} ultrasound_sensor;

ultrasound_sensor *us_init(int gpio_trig, int gpio_echo);

double get_distance(ultrasound_sensor *us);

void us_cleanup(ultrasound_sensor *us);

void us_test(int gpio_trig, int gpio_echo, int num_pings);

/**
 * @brief Print out a usage message for the ultrasound test mode.
 */
void us_usage(void);

#endif /* _US_H */
