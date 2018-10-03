/*
 * altADC.c
 *
 *  Created on: 28/05/2018
 *      Author: tjl54
 */

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"

#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "circBufT.h"

#include "altADC.h"
//

#define BUF_SIZE            10

static circBuf_t g_inBuffer;
static int16_t g_altitude;
static uint16_t g_buffer_values;
//*****************************************************************************
//
// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
//
//*****************************************************************************
void ADCIntHandler(void)
{

    uint32_t ulValue;

    //
    // Get the single sample from ADC0.  ADC_BASE is defined in
    // inc/hw_memmap.h
    ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);
    //
    // Place it in the circular buffer (advancing write index)
    writeCircBuf(&g_inBuffer, ulValue);

    //
    // Clean up, clearing the interrupt
    ADCIntClear(ADC0_BASE, 3);
}

// Initialize the ADC module
void initADC(void)
{
    //
    // The ADC0 peripheral must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // on the ADC sequences and steps, refer to the LM3S1968 datasheet.
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE | ADC_CTL_END);

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    //
    // Register the interrupt handler
    ADCIntRegister(ADC0_BASE, 3, ADCIntHandler);

    //
    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);

    // Initialize circular buffer to be used with ADC
    initCircBuf(&g_inBuffer, BUF_SIZE);
}

// Average the data in the circular buffer to calculate altitude
void updateAlt(void)
{
    int32_t sum;
    static int16_t landedValue = 0;
    static int16_t maxValue = 0;
    int16_t meanValue = 0;
    uint16_t i;
    static uint8_t landed_set = 0;


    // Background task: calculate the (approximate) mean of the values in the
        // circular buffer and display it, together with the sample number.
        sum = 0;
        for (i = 0; i < BUF_SIZE; i++) {
            sum = sum + readCircBuf(&g_inBuffer);
        }

        // Calculate and display the rounded mean of the buffer contents
        meanValue = (2 * sum + BUF_SIZE) / 2 / BUF_SIZE;
        g_altitude = (meanValue - landedValue) * 100 / (maxValue - landedValue);
        g_buffer_values++;

        if (landed_set == 0 && g_buffer_values >= 20)
        {
            landed_set = 1;
            landedValue = meanValue;
            maxValue = landedValue - 993;
        }
}

// Trigget the ADC to run when the SysTick interrupt runs
void triggerADC(void)
{
    ADCProcessorTrigger(ADC0_BASE, 3);
}

// Get the stored altitude value from the altitude module
int getAlt(void)
{
    return g_altitude;
}
