/*
 * reset.c
 *
 *  Created on: 28/05/2018
 *      Author: tjl54
 */


#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

#include "reset.h"

// Interrupt handler for the reset button
static void ResetHandler(void) {
    GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_6);
    SysCtlReset();
}

// Initialize the reset button and the reset interrupt
void initReset(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_STRENGTH_2MA,
            GPIO_PIN_TYPE_STD_WPU);
    GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_DIR_MODE_IN);
    GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_FALLING_EDGE);
    GPIOIntRegister(GPIO_PORTA_BASE, ResetHandler);
    GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_6);
    IntEnable(INT_GPIOA);
}
