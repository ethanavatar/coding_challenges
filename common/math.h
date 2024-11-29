#pragma once
#ifndef E_MATH_H
#define E_MATH_H

struct Vector2_Int { int x, y; };

float lerp(float a, float b, float t)         { return (1 - t) * a + b * t; }
float inverse_lerp(float a, float b, float v) { return (v - a) / (b - a); }
float remap(float a, float b, float a1, float b1, float v) { return lerp(a1, b1, inverse_lerp(a, b, v)); }

#endif // E_MATH_H
