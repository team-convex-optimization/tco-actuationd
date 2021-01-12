#ifndef _CAL_H_
#define _CAL_H_

/**
 * @brief Main function for calibration mode. Once called, runs an interactive TUI to help in
 * calibrating pulse lengths for each channel. 
 * @param pca9685_handle Pointer received from pca99685_init.
 */
void cal_main(void *pca9685_handle);

/**
 * @brief Print out a usage message for the calibration mode.
 */
void cal_usage(void);

#endif /* _CAL_H_ */