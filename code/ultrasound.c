#include "ultrasound.h"

ultrasound_sensor *us_init(int gpio_trig, int gpio_echo)
{
    ultrasound_sensor *us = (ultrasound_sensor *)malloc(sizeof(ultrasound_sensor));

    struct gpiod_line *trig = gpio_line_init(OUT, gpio_trig);
    struct gpiod_line *echo = gpio_line_init(IN, gpio_echo);
    if (trig == NULL || echo == NULL)
    {
        log_error("Could not initialize ultrasonic sensor");
        return NULL;
    }
    log_info("Ultrasound sensor has been correctly instantiated");
    return us;
}

double get_distance(ultrasound_sensor *us)
{
    time_t start, end;

    //Send pulse
    gpio_line_write(us->trig, 1); //TDO Make macro for HIGH and LOW 
    usleep(10000);
    gpio_line_write(us->trig, 0);

    //Recieve pulse
    while(gpio_line_read(us->echo) == 0) {}
    start = time(NULL);
    while(gpio_line_read(us->echo) == 1) {}
    end = time(NULL);

    //Calculate the length of the pulse in seconds
    float rtt = ((end - start) / TIME_T_TO_SECONDS);
    double distance = rtt * 17150; //(1/2)speed of sound in cm/s.
    return distance;
}

void us_cleanup(ultrasound_sensor *us) 
{
    gpio_line_close(us->echo);
    gpio_line_close(us->trig);
    free(us);
    log_info("Ultrasound sensor has been cleaned up");
}

void us_test(int gpio_trig, int gpio_echo, int num_pings)
{
    ultrasound_sensor *us = us_init(gpio_trig, gpio_echo);
    assert(us != NULL);
    printf("Testing ping\n");
    for (int i = 0; i < num_pings; i++) {
        double dist = get_distance(us);
        printf("\rPing %d has distance of %f cm(s)", i, dist);
    }
    printf("\nTest complete. cleaning...\n");
    us_cleanup(us);
}

void us_usage() {
    printf("Ultrasound sensor test mode:\n"
           "Run the program with './<program> -u <trigger_pin> <echo_pin> <num_pings>' where:\n"
           "trigger_pin: The GPIO pin where trig is connected to.\n"
           "echo_pin: The GPIO pin where echo is connected to.\n"
           "num_pings: The number of pings you want to test.\n"
           "EXAMPLE: './program 16 18 1000'\n");
}
