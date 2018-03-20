#ifndef PWMCONTROL_H
#define PWMCONTROL_H

#include <stdint.h>

void initSysTick (void);

void initialiseMainPWM (void);

void initialiseTailPWM (void);

void setMainPWM (uint32_t ui32Freq, uint32_t ui32Duty);

void setTailPWM (uint32_t ui32Freq, uint32_t ui32Duty);

#endif
