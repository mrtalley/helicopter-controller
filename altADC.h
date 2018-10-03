/*
 * altADC.h
 *
 *  Created on: 29/05/2018
 *      Author: tjl54
 */

#ifndef ALTADC_H_
#define ALTADC_H_

// Initialize the ADC module
void initADC(void);

// Average the data in the circular buffer to calculate altitude
void updateAlt(void);

// Trigget the ADC to run when the SysTick interrupt runs
void triggerADC(void);

// Get the stored altitude value from the altitude module
int getAlt(void);


#endif /* ALTADC_H_ */
