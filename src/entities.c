#include "entities.h"
#include <math.h>

int rect_overlap(Rect a, Rect b)
{
    return (a.x < b.x + b.w) && (a.x + a.w > b.x) && (a.y < b.y + b.h) && (a.y + a.h > b.y);
}

float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float lerpf(float a, float b, float t)
{
    return a + (b - a) * t;
}

float dist2f(float ax, float ay, float bx, float by)
{
    float dx = bx - ax;
    float dy = by - ay;
    return sqrtf(dx * dx + dy * dy);
}