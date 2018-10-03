/*
 * uart.h
 *
 *  Created on: May 28, 2018
 *      Author: maxtalley
 */

#ifndef UART_H_
#define UART_H_

void initUART();

void UARTSend(char* pucBuffer);

void formatUARTOutput(uint16_t main_duty, uint16_t tail_duty, int16_t altitude, int16_t desired_alt, int16_t yaw, int16_t desired_yaw, char* mode_name);


#endif /* UART_H_ */
