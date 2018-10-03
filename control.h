/*
 * control.h
 *
 *  Created on: May 22, 2018
 *      Author: maxtalley
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include <stdint.h>

// *************************
// alt_pid:
// *************************
uint16_t alt_pid(int16_t current_alt, int16_t desired_alt, float dt);

// *************************
// yaw_pid:
// *************************
uint16_t yaw_pid(int32_t current_yaw, int32_t desired_yaw, float dt);

#endif /* CONTROL_H_ */
