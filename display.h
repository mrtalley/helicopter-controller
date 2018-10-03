/*
 * display.h
 *
 *  Created on: 28/05/2018
 *      Author: tjl54
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

void initDisplay(void);

void displayFlightData(int16_t altitude, uint16_t main_duty, uint16_t tail_duty, int16_t yaw_actual);

#endif /* DISPLAY_H_ */
