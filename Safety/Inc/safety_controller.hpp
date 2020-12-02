#ifndef SAFETY_CONTROLLER_H
#define SAFETY_CONTROLLER_H
#include "stm32f0xx_hal_iwdg.h"
#include "PPM.hpp"

/*
*  Call this function after PPM and PWM manager have been inited.
*/
void safety_controller_init();
void safety_run(PPMChannel &ppm);
bool isSafetyManual();
void setPWMChannel(PWMManager &manager, int channel, int percentage);
int getPPM(PPMChannel &ppm, int channel);

#endif