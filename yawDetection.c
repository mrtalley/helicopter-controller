/*
 * yawDetection.c
 * Function for detection of yaw pin changes and quadrature encoding to gain value in degrees
 *  Created on: 28/05/2018
 *      Author: tjl54
 */
// Includes
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

#include "yawDetection.h"

// *** globals
static int32_t yaw_degrees;             // Yaw calculation in degrees
volatile static int32_t yaw;            // Raw, unconverted yaw value
static int32_t state_yaw = 0;           // Tracks current state of the yaw calculation
static int32_t prev_state_yaw = 0;      // Tracks previous state of yaw calculation
volatile static int16_t yawRef = 1;     // Stores whether the ref signal has been reached
volatile static int16_t g_trigger = 0;

//*************************************************************************
// ISR and Yaw Quadrature encoding
//*************************************************************************
void
handleYaw (void)
{
    int32_t PinA;
    int32_t PinB;

    // Clear the interrupt (documentation recommends doing this early)
    GPIOIntClear (GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Reads pin values
    PinA = GPIOPinRead (GPIO_PORTB_BASE, GPIO_PIN_0);
    PinB = GPIOPinRead (GPIO_PORTB_BASE, GPIO_PIN_1);

    // Yaw States
    if (!PinA) {
        if (!PinB) {
            state_yaw = 1; //hold
        }
        else if (PinB) {
            state_yaw = 2; //clockwise
        }
    }
    if (PinA) {
        if (PinB) {
            state_yaw = 3; //high
        }
        else if (!PinB) {
            state_yaw = 4; //counterclockwise
        }
    }

    // Detects direction moving and increments/decrements yaw
    if (prev_state_yaw == 1 && state_yaw == 2) {
        yaw++;
    }
    if (prev_state_yaw == 2 && state_yaw == 3) {
        yaw++;
    }
    if (prev_state_yaw == 3 && state_yaw == 4) {
        yaw++;
    }
    if (prev_state_yaw == 4 && state_yaw == 1) {
        yaw++;
    }
    if (prev_state_yaw == 4 && state_yaw == 3) {
        yaw--;
    }
    if (prev_state_yaw == 3 && state_yaw == 2) {
        yaw--;
    }
    if (prev_state_yaw == 2 && state_yaw == 1) {
        yaw--;
    }
    if (prev_state_yaw == 1 && state_yaw == 4) {
        yaw--;
    }
    prev_state_yaw = state_yaw;

    // Sets yaw_degrees
    yaw_degrees = (yaw * 360) / 448;
}

// Interrupt handler for yaw interrupt
void yawRefHandler (void)
{
    // detect interrupt
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_4);

    yawRef = 0;
    yaw = 1;

    if (!yawRef && g_trigger == 0)
    {
        yaw = 0;
        g_trigger = 1;
    }
}

//*************************************************************************
// Yaw Port & Pin Configuration
//*************************************************************************
void
initYaw(void)
{
    // Pin Setup
    // Enable the GPIOB peripheral for configuration and use
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Set up the specific port pin as medium strength current & pull-down config
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // Set data direction register as input
    GPIODirModeSet(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_DIR_MODE_IN);

    // Interrupt Setup
    // Register interrupt handler on port B
    GPIOIntRegister(GPIO_PORTB_BASE, handleYaw);

    // Configure pins to be triggered on BOTH edges
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_BOTH_EDGES);

    // Enable interrupts on the pins
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Enable interrupts on port B
    IntEnable(INT_GPIOB);
}

//*********************************************************
// Yaw Reference initialization
//*********************************************************
void initRef (void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA,
            GPIO_PIN_TYPE_STD_WPU);

    GPIODirModeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);

    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    GPIOIntRegister(GPIO_PORTC_BASE, yawRefHandler);

    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_PIN_4);
    IntEnable(INT_GPIOC);
}

// Return calculated yaw value
int getYaw(void)
{
    return yaw_degrees;
}

// Return boolean value storing state of yaw ref
int checkYawRef(void)
{
    return (yawRef);
}
