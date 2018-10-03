/**
 * @file reset.h
 *
 * @brief Soft reset module.
 */

#ifndef RESET_H_
#define RESET_H_

/**
 * Initialise the reset module. Configures the system to reset via an interrupt
 * on the required GPIO pin.
 */
void initReset(void);

#endif /* RESET_H_ */
