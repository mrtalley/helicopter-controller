/**********************************************************
 *
 * pwmGen.c - Example code which generates a single PWM
 *    output on J4-05 (M0PWM7) with duty cycle fixed and
 *    the frequency controlled by UP and DOWN buttons in
 *    the range 50 Hz to 400 Hz.
 * 2017: Modified for Tiva and using straightforward, polled
 *    button debouncing implemented in 'buttons4' module.
 *
 * P.J. Bones   UCECE
 * Last modified:  7.2.2018
 **********************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h" //Needed for pin configure
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "buttons4.h"

/**********************************************************
 * Generates a single PWM signal on Tiva board pin J4-05 =
 * PC5 (M0PWM7).  This is the same PWM output as the
 * helicopter main rotor.
 **********************************************************/

/**********************************************************
 * Constants
 **********************************************************/
// Systick configuration
#define SYSTICK_RATE_HZ    100

// PWM configuration
#define PWM_START_RATE_HZ   250
#define PWM_RATE_STEP_HZ    50
#define PWM_RATE_MIN_HZ     50
#define PWM_RATE_MAX_HZ     400
#define PWM_FIXED_DUTY      67
#define PWM_DIVIDER_CODE    SYSCTL_PWMDIV_4
#define PWM_DIVIDER         4
#define PWM_TAIL_CONST_HZ   200

// Duty Cycle Configuration
#define DUTY_CYCLE_MAX              95
#define DUTY_CYCLE_MIN              5
#define DUTY_CYCLE_START            25
#define DUTY_CYCLE_CHANGE_AMOUNT    5
#define DUTY_CYCLE_TAIL_CONSTANT    10

//  PWM Hardware Details M0PWM7 (gen 3)
//  ---Main Rotor PWM: PC5, J4-05
#define PWM_MAIN_BASE           PWM0_BASE
#define PWM_MAIN_GEN            PWM_GEN_3
#define PWM_MAIN_OUTNUM         PWM_OUT_7
#define PWM_MAIN_OUTBIT         PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM     SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO    SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE      GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG    GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN       GPIO_PIN_5

// PWM Hardware Details M1PWM5
// --Tail Rotor PWM: PF1, J3-10
#define PWM_TAIL_BASE           PWM1_BASE
#define PWM_TAIL_GEN            PWM_GEN_2
#define PWM_TAIL_OUTNUM         PWM_OUT_5
#define PWM_TAIL_OUTBIT         PWM_OUT_5_BIT
#define PWM_TAIL_PERIPH_PWM     SYSCTL_PERIPH_PWM1
#define PWM_TAIL_PERIPH_GPIO    SYSCTL_PERIPH_GPIOF
#define PWM_TAIL_GPIO_BASE      GPIO_PORTF_BASE
#define PWM_TAIL_GPIO_CONFIG    GPIO_PF1_M1PWM5
#define PWM_TAIL_GPIO_PIN       GPIO_PIN_1

/*******************************************
 *      Local prototypes
 *******************************************/
void SysTickIntHandler (void);
void initClocks (void);
void initSysTick (void);
void initialiseMainPWM (void);
void initialiseTailPWM (void);
void setMainPWM (uint32_t u32Freq, uint32_t u32Duty);
void setTailPWM (uint32_t u32Freq, uint32_t u32Duty);



/***********************************************************
 * ISR for the SysTick interrupt (used for button debouncing).
 ***********************************************************/
void
SysTickIntHandler (void)
{
	//
	// Poll the buttons
	updateButtons();
	//
	// It is not necessary to clear the SysTick interrupt.
}

/***********************************************************
 * Initialisation functions: clock, SysTick, PWM
 ***********************************************************
 * Clock
 ***********************************************************/
void
initClocks (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    // Set the PWM clock rate (using the prescaler)
    SysCtlPWMClockSet(PWM_DIVIDER_CODE);
}

void
initDisplay (void)
{
  // intialise the Orbit OLED display
    OLEDInitialise ();
}

/*************************************************************
 * SysTick interrupt
 ************************************************************/
void
initSysTick (void)
{
    //
    // Set up the period for the SysTick timer.  The SysTick
    // timer period is set as a function of the system clock.
    SysTickPeriodSet (SysCtlClockGet() / SYSTICK_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister (SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable ();
    SysTickEnable ();
}

/*********************************************************
 * initialisePWM
 * M0PWM7 (J4-05, PC5) is used for the main rotor motor
 *********************************************************/
void
initialiseMainPWM (void)
{
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);

    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN);

    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    // Set the initial PWM parameters
    setMainPWM (PWM_START_RATE_HZ, DUTY_CYCLE_START);

    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, false);
}

void
initialiseTailPWM (void)
{
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_GPIO);

    GPIOPinConfigure(PWM_TAIL_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_TAIL_GPIO_BASE, PWM_TAIL_GPIO_PIN);

    PWMGenConfigure(PWM_TAIL_BASE, PWM_TAIL_GEN,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    // Set the initial PWM parameters
    setTailPWM (PWM_TAIL_CONST_HZ, DUTY_CYCLE_TAIL_CONSTANT);

    PWMGenEnable(PWM_TAIL_BASE, PWM_TAIL_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, false);
}

/********************************************************
 * Function to set the freq, duty cycle of M0PWM7
 ********************************************************/
void
setMainPWM (uint32_t ui32Freq, uint32_t ui32Duty)
{
    // Calculate the PWM period corresponding to the freq.
    uint32_t ui32Period =
        SysCtlClockGet() / PWM_DIVIDER / ui32Freq;

    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, ui32Period);
    PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM,
        ui32Period * ui32Duty / 100);
}

void
setTailPWM (uint32_t ui32Freq, uint32_t ui32Duty)
{
    // Calculate the PWM period corresponding to the freq.
    uint32_t ui32Period =
        SysCtlClockGet() / PWM_DIVIDER / ui32Freq;

    PWMGenPeriodSet(PWM_TAIL_BASE, PWM_TAIL_GEN, ui32Period);
    PWMPulseWidthSet(PWM_TAIL_BASE, PWM_TAIL_OUTNUM,
        ui32Period * ui32Duty / 100);
}

void
displayData (char *butStr, char *stateStr, uint32_t data, uint8_t charLine)
{
    char string[17]; // Display fits 16 characters wide.

    OLEDStringDraw ("                ", 0, charLine);
    usnprintf (string, sizeof(string), "%s - %2d %s", butStr, data, stateStr);
    OLEDStringDraw (string, 0, charLine);
}

int
main (void)
{
    uint32_t ui32MainFreq = PWM_START_RATE_HZ;
    uint32_t ui32MainDutyCycle = DUTY_CYCLE_START;

    uint32_t ui32TailFreq = PWM_TAIL_CONST_HZ;
    uint32_t ui32TailDutyCycle = DUTY_CYCLE_TAIL_CONSTANT;

    initClocks ();

    // As a precaution, make sure that the peripherals used are reset
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_PWM);  // Main Rotor PWM
    SysCtlPeripheralReset (PWM_TAIL_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (PWM_TAIL_PERIPH_PWM);  // Tail Rotor PWM
    SysCtlPeripheralReset (UP_BUT_PERIPH);        // UP button GPIO
    SysCtlPeripheralReset (DOWN_BUT_PERIPH);      // DOWN button GPIO

    initButtons ();  // Initialises 4 pushbuttons (UP, DOWN, LEFT, RIGHT)
    initialiseMainPWM ();
    initialiseTailPWM ();
    initSysTick ();
    initDisplay();

    // Initialisation is complete, so turn on the output
    // Turn on PWM_MAIN
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);

    // Turn on PWM_TAIL
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, true);

    //
    // Enable interrupts to the processor.
    IntMasterEnable ();

    displayData("M-FREQ", "HZ", ui32MainFreq, 0);
    displayData("M-DUTCYC", "%", ui32MainDutyCycle, 1);

    displayData("T-FREQ", "HZ", ui32TailFreq, 2);
    displayData("T-DUTCYC", "%", ui32TailDutyCycle, 3);

    //
    // Loop forever, controlling the PWM frequency and
    // maintaining the the PWM duty cycle.
    while (1)
    {
        // Background task: Check for button pushes and control
        // the PWM frequency within a fixed range.
        if ((checkButton (UP) == PUSHED) && (ui32MainFreq < PWM_RATE_MAX_HZ))
        {
    	        ui32MainFreq += PWM_RATE_STEP_HZ;
    	        setMainPWM(ui32MainFreq, ui32MainDutyCycle);
    	        displayData("M-FREQ", "HZ", ui32MainFreq, 0);
        }

        if ((checkButton (DOWN) == PUSHED) && (ui32MainFreq > PWM_RATE_MIN_HZ))
        {
            ui32MainFreq -= PWM_RATE_STEP_HZ;
            setMainPWM(ui32MainFreq, ui32MainDutyCycle);
            displayData("M-FREQ", "HZ", ui32MainFreq, 0);
        }

        //right -- maximum of 95%
        if((checkButton(RIGHT) == PUSHED) && (ui32MainDutyCycle < DUTY_CYCLE_MAX))
        {
            //increase duty cycle 5%
            ui32MainDutyCycle += DUTY_CYCLE_CHANGE_AMOUNT;
            setMainPWM(ui32MainFreq, ui32MainDutyCycle);
            displayData("M-DUTCYC", "%", ui32MainDutyCycle, 1);

        }

        //left -- minimum of 5%
        if((checkButton(LEFT) == PUSHED) && (ui32MainDutyCycle > DUTY_CYCLE_MIN))
        {
            //decrease duty cycle 5%
            ui32MainDutyCycle -= DUTY_CYCLE_CHANGE_AMOUNT;
            setMainPWM(ui32MainFreq, ui32MainDutyCycle);
            displayData("M-DUTCYC", "%", ui32MainDutyCycle, 1);
        }
    }
}
