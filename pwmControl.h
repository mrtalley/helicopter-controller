#ifndef PWMCONTROL_H
#define PWMCONTROL_H

#include <stdint.h>

void initialiseMainPWM (void);

void initialiseTailPWM (void);

void setMainPWM (uint16_t ui16Duty);

void setTailPWM (uint16_t ui16Duty);

void setMainPWMOutput(uint16_t main_output);

void setTailPWMOutput(uint16_t tail_output);

#endif
