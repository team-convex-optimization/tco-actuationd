#ifndef _ULTRASOUND_H_
#define _ULTRASOUND_H_

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "tco_libd.h"

#include "gpio.h"

#define TIME_T_TO_SECONDS 12960000

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
