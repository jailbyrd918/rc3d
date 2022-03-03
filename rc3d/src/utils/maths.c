#include <stdbool.h>
#include <math.h>

#include <SDL.h>

#include "maths.h"


float math_angle_from_vec2
(const float x, const float y)
{
        return atan2f(y, x);
}

float math_x_from_angle
(const float angle)
{
        return cosf(angle);
}

float math_y_from_angle
(const float angle)
{
        return sinf(angle);
}


bool math_normalize_angle
(float *angle)
{
        if (!angle)
                return false;

        (*angle) = remainderf((*angle), PI * 2.f);

        if ((*angle) < 0)
                (*angle) += PI * 2.f;

        return true;
}

float math_distance_between_points
(const float x0, const float y0, const float x1, const float y1)
{
        return (sqrtf(powf(x1 - x0, 2.f) + powf(y1 - y0, 2.f)));
}
