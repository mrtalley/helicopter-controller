/*
 * display.c
 *
 *  Created on: 28/05/2018
 *      Author: tjl54
 */

#include <stdint.h>
#include <stdbool.h>

#include "stdio.h"
#include "stdlib.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"

#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"

#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"

#include "display.h"

void initDisplay(void)
{
    // intialise the Orbit OLED display
    OLEDInitialise();
}

//*************************************************************************
// Displays altitude as percentage and yaw in degrees
//*************************************************************************
void displayFlightData(int16_t altitude, uint16_t main_duty, uint16_t tail_duty, int16_t yaw_actual)
{
    char string[17]; // Display fits 16 characters wide.

    usnprintf(string, sizeof(string), "Altitude: %4d%%", altitude);
    OLEDStringDraw(string, 0, 0);

    usnprintf(string, sizeof(string), "YAW %4d deg", yaw_actual);
    OLEDStringDraw(string, 0, 1);

    usnprintf(string, sizeof(string), "Main DC: %d%%", main_duty);
    OLEDStringDraw(string, 0, 2);

    usnprintf(string, sizeof(string), "Tail DC: %d%%", tail_duty);
    OLEDStringDraw(string, 0, 3);
}
