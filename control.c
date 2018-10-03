#include <stdio.h>
#include <stdint.h>

#include "control.h"

static float I_alt = 0;
static float error_previous_alt = 0;
static float I_yaw = 0;
static float error_previous_yaw = 0;

uint16_t
alt_pid(int16_t current_alt, int16_t desired_alt, float dt)
{
    uint16_t control_alt;
    float Kp_alt = 1.5;
    float Ki_alt = 0.0015;
    float Kd_alt = 0;

    float error_alt = desired_alt - current_alt;
    float P_alt = Kp_alt * error_alt;

    float dI_alt = Ki_alt * error_alt * dt;

    float D_alt = (Kd_alt / dt) * (error_alt - error_previous_alt);

    control_alt = P_alt + (dI_alt + i_error_alt) + D_alt;

    error_previous_alt = error_alt;

    if (control_alt > 98) {
        control_alt = 98;
    }

    else if (control_alt < 2) {
        control_alt = 2;
    }

    else {
        I_alt += dI_alt;
    }

    return control_alt;
}



uint16_t
yaw_pid(int32_t current_yaw, int32_t desired_yaw, float dt)
{
    uint16_t control_yaw; //value that is returned to the duty cycle
    float Kp_yaw = 0.3;
    float Ki_yaw = .0015;
    float Kd_yaw = 0;

    float error_yaw = desired_yaw - current_yaw;
    float P_yaw = Kp_yaw * error_yaw;

    float dI_yaw = Ki_yaw * error_yaw * dt;

    float D_yaw = (Kd_yaw / dt) * (error_yaw - error_previous_yaw);

    control_yaw = P_yaw + (dI_yaw + i_error_yaw) + D_yaw;

    error_previous_yaw = error_yaw;

    if (control_yaw > 98) {
            control_yaw = 98;
    }

    else if (control_yaw < 2) {
            control_yaw = 2;
    }

    else {
        I_yaw += dI_yaw;
    }

    return control_yaw;
}
