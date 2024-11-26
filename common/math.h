#pragma once
#ifndef E_MATH_H
#define E_MATH_H

float float_remap(float value, float in_min,  float in_max, float out_min, float out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#endif // E_MATH_H
