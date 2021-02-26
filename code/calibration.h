#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

/**
 * @brief Main function for calibration mode. Once called, runs an interactive TUI to help in
 * calibrating pulse lengths for each channel. 
 */
void cal_main(void);

/**
 * @brief Print out a usage message for the calibration mode.
 */
void cal_usage(void);

#endif /* _CALIBRATION_H_ */