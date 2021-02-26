#include <stdlib.h>
#include <stdint.h>

#include <curses.h>
#include <string.h>

#include "calibration.h"
#include "pca9685.h"
#include "actuator.h"

#define CH_NUM 16U
#define CH_WIN_WIDTH 13
#define CH_WIN_HEIGHT 5

enum ch_val_t
{
    CH_VAL_WIN = 0,
    CH_VAL_MIN,
    CH_VAL_MAX,
    CH_VAL_NUM
};

enum mode_t
{
    MODE_VISUAL = 0,
    MODE_EDIT
};

typedef struct
{
    WINDOW *ch_win[CH_NUM];
    uint16_t ch_info[CH_NUM][2];
} cal_info_t;

static int term_width, term_height;
static uint8_t ch_selected = 0; /* 0 to (CH_NUM - 1)*/
static enum ch_val_t ch_val_selected = CH_VAL_WIN;
static enum mode_t mode = MODE_VISUAL;
static uint8_t incr_step = 1; /* Step size for the edits */

#define border_simple(win) wborder(win, '|', '|', '-', '-', '+', '+', '+', '+')

static int32_t clamp_delta(int32_t val, const int32_t min, const int32_t max, const int32_t delta)
{
    if (val + delta > max)
    {
        val = max;
    }
    else if (val + delta < min)
    {
        val = min;
    }
    else
    {
        val += delta;
    }

    return val;
}

static void draw_ch_win(cal_info_t *cal_info)
{
    WINDOW *win; /* macro for clarity */
    for (size_t win_i = 0, win_y = 1, win_x = 0; win_i < CH_NUM; win_i++)
    {
        win = cal_info->ch_win[win_i];
        if (win != NULL)
        {
            delwin(win);
        }
        /* TODO: Move windows to the right position instead of recreating them. */
        win = newwin(CH_WIN_HEIGHT, CH_WIN_WIDTH, win_y, win_x);
        border_simple(win);

        win_x += CH_WIN_WIDTH - 1;
        if (win_x + CH_WIN_WIDTH - 1 >= term_width)
        {
            win_x = 0;
            win_y += CH_WIN_HEIGHT - 1;
        }
        wrefresh(win);
        cal_info->ch_win[win_i] = win;
    }

    for (size_t win_i = 0; win_i < CH_NUM; win_i++)
    {
        win = cal_info->ch_win[win_i];
        if (win_i == ch_selected)
        {
            wattron(win, A_STANDOUT);
        }

        mvwprintw(win, 1, 2, "Ch %u", win_i + 1, mode);
        wattrset(win, 0);
        if (win_i == ch_selected && ch_val_selected == CH_VAL_MIN)
        {
            wattron(win, A_STANDOUT);
            if (mode == MODE_EDIT)
            {
                wattron(win, COLOR_PAIR(2));
            }
        }
        mvwprintw(win, 2, 2, "Min: %u", cal_info->ch_info[win_i][0]);
        wattrset(win, 0);

        if (win_i == ch_selected && ch_val_selected == CH_VAL_MAX)
        {
            wattron(win, A_STANDOUT);
            if (mode == MODE_EDIT)
            {
                wattron(win, COLOR_PAIR(2));
            }
        }
        mvwprintw(win, 3, 2, "Max: %u", cal_info->ch_info[win_i][1]);
        wattrset(win, 0);

        wrefresh(win);
    }
}

static void draw_status(WINDOW *win_status)
{
    wresize(win_status, 1, term_width);
    werase(win_status);
    wbkgd(win_status, COLOR_PAIR(1));
    wattron(win_status, A_BOLD);

    char *mode_text;
    switch (mode)
    {
    case MODE_EDIT:
        mode_text = "edit";
        break;
    case MODE_VISUAL:
        mode_text = "visual";
        break;
    }

    mvwprintw(win_status, 0, 0, "MODE: %s\tIncrement step %u\tSelected %u\tSelected val %u", mode_text, incr_step, ch_selected + 1, ch_val_selected);
    wattroff(win_status, 0);
    wrefresh(win_status);
}

void cal_main()
{
    pca9685_handle_t handle = {0};
    if (i2c_port_open(PCA9685_I2C_ADAPTER_ID, &(handle.fd)) != ERR_OK)
    {
        log_error("Failed to open I2C adapter connected to PCA9685");
        exit(EXIT_FAILURE);
    }
    if (pca9685_init(&handle) != ERR_OK)
    {
        log_error("Failed to initialize PCA9685");
        exit(EXIT_FAILURE);
    }

    /* Init ncurses */
    initscr();
    cbreak();             /* Remove input delays */
    noecho();             /* Don't echo keyboard input */
    keypad(stdscr, TRUE); /* For special key support (e.g. arrows) */
    getmaxyx(stdscr, term_height, term_width);
    refresh();
    start_color(); /* Enable color support */
    init_pair(1, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);

    /* Draw windows per channel and keep track of calibration data */
    cal_info_t cal_info = {};

    /* Status bar window */
    WINDOW *win_status = newwin(1, term_width, 0, 0);

    /* Update loop */
    int ch = 0;
    do
    {
        int old_term_width = term_width, old_term_height = term_height;
        getmaxyx(stdscr, term_height, term_width);
        if (old_term_width != term_width || old_term_height != term_height)
        {
            erase();
        }

        if (mode == MODE_EDIT)
        {
            uint16_t *val_edit = &cal_info.ch_info[ch_selected][ch_val_selected - 1];
            switch (ch)
            {
            case 'e':
                mode = MODE_VISUAL;
                break;
            case KEY_LEFT:
            case KEY_DOWN:
                *val_edit = clamp_delta(*val_edit, 0, (1 << 12), -incr_step);
                break;
            case KEY_RIGHT:
            case KEY_UP:
                *val_edit = clamp_delta(*val_edit, 0, (1 << 12), incr_step);
                break;
            case '.':
                incr_step = clamp_delta(incr_step, 0, INT32_MAX, 1);
                break;
            case ',':
                incr_step = clamp_delta(incr_step, 0, INT32_MAX, -1);
                break;
            case ' ':
                if (*val_edit == 0)
                {
                    *val_edit = (1 << 12);
                }
                else
                {
                    *val_edit = 0;
                }
                break;
            }
            if (pca9685_ch_raw_set(&handle, ch_selected, *val_edit) != ERR_OK)
            {
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            switch (ch)
            {
            case KEY_LEFT:
                ch_selected = clamp_delta(ch_selected, 0, CH_NUM - 1, -1);
                break;
            case KEY_RIGHT:
                ch_selected = clamp_delta(ch_selected, 0, CH_NUM - 1, 1);
                break;
            case KEY_UP:
                ch_val_selected = clamp_delta(ch_val_selected, 0, CH_VAL_NUM - 1, -1);
                break;
            case KEY_DOWN:
                ch_val_selected = clamp_delta(ch_val_selected, 0, CH_VAL_NUM - 1, 1);
                break;
            case 'e':
                if (mode == MODE_VISUAL && ch_val_selected > CH_VAL_WIN)
                {
                    mode = MODE_EDIT;
                }
                break;
            }
        }

        draw_ch_win(&cal_info);
        draw_status(win_status);
    } while ((ch = getch()) != 'q');

    /* Deinit ncurses resources and exit */
    delwin(win_status);
    for (uint8_t win_i = 0; win_i < CH_NUM; win_i++)
    {
        delwin(cal_info.ch_win[win_i]);
    }
    endwin();
    exit(EXIT_SUCCESS);
}

void cal_usage(void)
{
    printf("Calibration mode:\n"
           "'e': To enter/leave edit mode.\n"
           "',': In edit mode this decreases the increment step (-1) for changing values.\n"
           "'.': In edit mode this increases the increment step (+1) for changing values.\n"
           "'q': Quit the calibration app.\n"
           "'Up' and 'Right' arrow keys increment the selected value in edit mode.\n"
           "'Down' and 'Left' arrow keys decrement the selected value in edit mode.\n"
           "'Left' and 'Right' arrow keys can be used to select the channel in visual mode.\n"
           "'Space' bar in edit mode will zero-out the edited value when its current value is >0 and otherwise will set it to max i.e. 4096.\n");
}
