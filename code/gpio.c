#include "tco_libd.h"

#include "gpio.h"

struct gpiod_line *gpio_line_init(enum gpio_dir dir, int pin)
{
    struct gpiod_chip *chip = NULL;
    struct gpiod_line *line = NULL;

    chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
    if (!chip)
    {
        log_error("Opening GPIO chip by name failed");
        goto clean_up;
    }

    line = gpiod_chip_get_line(chip, pin);
    if (!line)
    {
        log_error("Getting chip line (GPIO_ID : %d) failed", pin);
        goto clean_up;
    }

    switch (dir)
    {
    case GPIO_IN:
        if (gpiod_line_request_input(line, "gpio_state") < 0)
        {
            log_error("Request GPIO line as input failed");
            goto clean_up;
        }
        break;
    case GPIO_OUT:
        if (gpiod_line_request_output(line, "Consumer", 0) < 0)
        {
            log_error("Request GPIO line as output failed");
            goto clean_up;
        }
        break;
    default:
        log_error("Unknown gpio_dir, got : %d", dir);
        goto clean_up;
    }

    return line;

clean_up:
    if (chip != NULL)
    {
        gpiod_chip_close(chip);
    }
    if (line != NULL)
    {
        gpiod_line_release(line);
    }
    return NULL;
}

int gpio_line_write(struct gpiod_line *line, enum gpio_value value)
{
    int ret = gpiod_line_set_value(line, value);
    if (ret < 0)
    {
        log_error("Failed to write to GPIO pin.");
    }
    return ret;
}

int gpio_line_read(struct gpiod_line *line)
{
    int ret = gpiod_line_get_value(line);
    if (ret < 0)
    {
        log_error("Failed to read from GPIO pin.");
    }
    return ret;
}

void gpio_line_close(struct gpiod_line *line)
{
    gpiod_line_release(line);
}

void gpio_chip_close(struct gpiod_chip *chip)
{
    gpiod_chip_close(chip);
}
