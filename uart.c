/*
 * uart.c
 *
 *  Created on: May 28, 2018
 *      Author: maxtalley
 */

#include <stdint.h>
#include <stdbool.h>

#include "stdio.h"

#include "inc/hw_memmap.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "utils/ustdlib.h"

//---USB Serial comms: UART0, Rx:PA0 , Tx:PA1
#define BAUD_RATE               9600
#define UART_USB_BASE           UART0_BASE
#define UART_USB_PERIPH_UART    SYSCTL_PERIPH_UART0
#define UART_USB_PERIPH_GPIO    SYSCTL_PERIPH_GPIOA
#define UART_USB_GPIO_BASE      GPIO_PORTA_BASE
#define UART_USB_GPIO_PIN_RX    GPIO_PIN_0
#define UART_USB_GPIO_PIN_TX    GPIO_PIN_1
#define UART_USB_GPIO_PINS      UART_USB_GPIO_PIN_RX | UART_USB_GPIO_PIN_TX

// Initialize UART output
void initUART()
{
    // Enable GPIO port A which is used for UART0 pins.
    SysCtlPeripheralEnable(UART_USB_PERIPH_UART);
    SysCtlPeripheralEnable(UART_USB_PERIPH_GPIO);

    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART(UART_USB_GPIO_BASE, UART_USB_GPIO_PINS);

    // REMOVING UART_CONFIG_WLEN_8 from last parameter
    UARTConfigSetExpClk(UART_USB_BASE, SysCtlClockGet(), BAUD_RATE,
            UART_CONFIG_WLEN_8 |
            UART_CONFIG_STOP_ONE |
            UART_CONFIG_PAR_NONE);

    UARTFIFOEnable(UART_USB_BASE);
    UARTEnable(UART_USB_BASE);
}

//**********************************************************************
// Transmit a string via UART0
//**********************************************************************
void
UARTSend (char *pucBuffer)
{
    // Loop while there are more characters to send.
    while(*pucBuffer)
    {
        // Write the next character to the UART Tx FIFO.
        UARTCharPut(UART_USB_BASE, *pucBuffer);
        pucBuffer++;
    }
}

// Format and send UART data (main & tail duty cycles, current altitude
    // and yaw, desired altitude and yaw, and the current mode)
void formatUARTOutput(uint16_t main_duty, uint16_t tail_duty, int16_t altitude,
    int16_t desired_alt, int16_t yaw, int16_t desired_yaw, char* mode_name)
{
    char statusStr[30];

    sprintf(statusStr, "******\n\r");
    UARTSend(statusStr);

    sprintf(statusStr, "Main: %d, Tail: %d\n\r", main_duty, tail_duty);
    UARTSend(statusStr);

    sprintf(statusStr, "Alt: %d [%d]\n\r", altitude, desired_alt);
    UARTSend(statusStr);

    sprintf(statusStr, "Yaw: %d [%d]\n\r", yaw, desired_yaw);
    UARTSend(statusStr);

    sprintf(statusStr, "Mode: %s\n\r", mode_name);
    UARTSend(statusStr);
}
