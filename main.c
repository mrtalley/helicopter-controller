//*****************************************************************************
// Final Helicopter Controller
// Author:  Maxwell Talley & Tom Layburn
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "stdio.h"
#include "stdlib.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"

#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"

#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"

#include "circBufT.h"
#include "buttons4.h"
#include "control.h"
#include "pwmControl.h"
#include "uart.h"
#include "display.h"
#include "yawDetection.h"
#include "reset.h"
#include "altADC.h"

//*****************************************************************************
// Constants
//*****************************************************************************
#define BUF_SIZE            20
#define SYSTICK_RATE_HZ     100
#define SLOWTICK_RATE_HZ    4

// Enumerations
enum modeNum {LANDED = 0, ORIENTING, FLYING, LANDING};

//*****************************************************************************
// Global variables
//*****************************************************************************
static uint32_t g_ulSampCnt;            // Counter for the interrupts
volatile uint8_t slowTick = false;

//*****************************************************************************
//
// The interrupt handler for the for SysTick interrupt.
//
//*****************************************************************************
void SysTickIntHandler(void)
{
    //
    // Initiate a conversion
    //
    triggerADC();
    g_ulSampCnt++;

    static uint8_t tickCount = 0;
    const uint8_t ticksPerSlow = SYSTICK_RATE_HZ / SLOWTICK_RATE_HZ;

    if(++tickCount >= ticksPerSlow)
    {
        tickCount = 0;
        slowTick = true;
    }
}

//*****************************************************************************
// Initialization functions for the clock (incl. SysTick), ADC, display
//*****************************************************************************
void initClock(void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SYSTICK_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}

void main(void) {
    char mode_names[4][10] = {"landed", "orienting", "flying", "landing"};

    int32_t actual_yaw;              // Raw, unconverted yaw value
    int32_t desired_yaw = 0;

    int16_t actual_alt;
    int16_t desired_alt = 0;

    uint16_t main_duty = 0;
    uint16_t tail_duty = 0;

    uint8_t switchCurState = 0, switchPrevState = 0;
    uint8_t programStart = 1;
    uint8_t mode = LANDED;
    uint8_t yawRef = 1;

    // Initialize each of the modules
    initClock();
    initADC();
    initButtons();  // Initialises 4 pushbuttons (UP, DOWN, LEFT, RIGHT)
    initYaw();      // Initialize port & pins used for yaw calculation
    initDisplay();
    initUART();
    initReset();
    initRef();
    initialiseMainPWM();
    initialiseTailPWM();

    // Enable interrupts to the processor.
    IntMasterEnable();

    while(1) {

        updateAlt();

        // Get the current state of the SW1 switch
        switchCurState = checkSwitch();

        // Heli State Machine
        switch(mode) {
            case LANDED:

                // Initial state -- heli needs to orient
                if(switchCurState && (switchCurState != switchPrevState) && programStart)
                {
                    mode = ORIENTING;
                    switchPrevState = switchCurState;
                    programStart = 0;

                    // Turn on motor output
                    setMainPWMOutput(true);
                    setTailPWMOutput(true);
                }

                // Heli already oriented, switch state to FLYING
                else if(switchCurState && (switchCurState != switchPrevState))
                {
                    mode = FLYING;
                    switchPrevState = switchCurState;
                }

                break;


            case ORIENTING:

                // Check if we've reached the reference signal
                yawRef = checkYawRef();

                if (!yawRef) {
                    desired_alt = 0;
                    desired_yaw = 0;
                    actual_yaw = 0;

                    main_duty = 0;
                    tail_duty = 0;
                    mode = FLYING;
                }

                else {
                    // Set main and tail duty to find ref yaw
                    main_duty = 5;
                    tail_duty = 10;
                }

                break;

            case FLYING:

                // switch down = go to landing
                if(!switchCurState && (switchCurState != switchPrevState))
                {
                    mode = LANDING;
                    switchPrevState = switchCurState;
                }

                // *******************************************
                // *** BUTTON CONTROL ***
                // *******************************************
                updateButtons();

                // Rise 10%
                if(checkButton(UP) == PUSHED && desired_alt < 100 && mode == FLYING) {
                    desired_alt += 10;
                }

                // Lower 10%
                if(checkButton(DOWN) == PUSHED && desired_alt > 0 && mode == FLYING) {
                    desired_alt -= 10;
                }

                // Rotate 15 degrees ccw
                if(checkButton(LEFT) == PUSHED && mode == FLYING) {
                    desired_yaw += 15;
                }

                // Rotate 15 degrees cw
                if(checkButton(RIGHT) == PUSHED && mode == FLYING) {
                    desired_yaw -= 15;
                }

                break;


            case LANDING:

                // Set desired values to 0
                // Orient, then lower to ground
                desired_alt = 0;
                desired_yaw = 0;

                // set mode to landed when heli is landed
                if(mode == LANDING && actual_alt == 0
                    && actual_yaw <= 5 && actual_yaw >= -5) {

                    mode = LANDED;

                    // Heli is landed, turn motors off
                    setMainPWMOutput(false);
                    setTailPWMOutput(false);
                }

                break;
        }

        // Get actual yaw and altitude measurements
        actual_yaw = getYaw();
        actual_alt = getAlt();

        // Pulse Width Sets
        // Set main motor
        setMainPWM(main_duty);

        // Set tail motor
        setTailPWM(tail_duty);

        // PID control
        main_duty = alt_pid(actual_alt, desired_alt, .005);
        tail_duty = yaw_pid(actual_yaw, desired_yaw, .005);

        // Set a delay on display/UART output
        if(slowTick) {
            // Send UART output
            formatUARTOutput(main_duty, tail_duty, actual_alt, desired_alt,
                actual_yaw, desired_yaw, mode_names[mode]);

            // Display flight data on OLED (alt, yaw, main dc, tail dc, yaw)
            displayFlightData(actual_alt, main_duty, tail_duty, actual_yaw);
        }
    }
}
