/*
        DESCRIPTION:
                Handles mathematics related utility functions
*/

#ifndef MATHS_H
#define MATHS_H


// >> since the provided 'M_PI' is of type < double >, converts it to type of < float >
#define PI                              ((float)M_PI)

// >> converts given angle in degree to radian
#define DEG_TO_RAD(degree)              ((degree * PI) / 180)

// >> converts given angle in radian to degree
#define RAD_TO_DEG(radian)              ((radian * 180) / PI)

// >> returns the greater value of both
#define MAX(val1, val2)                 ((val1 > val2) ? val1 : val2)

// >> returns the smaller value of both
#define MIN(val1, val2)                 ((val1 < val2) ? val1 : val2)

// >> restricts the given value to between min value and max value
#define CLAMP(val, min_val, max_val)    (MIN(MAX(val, min_val), max_val))


// >> returns angle (in radian) from a 2d unit vector with given x and y
float math_angle_from_vec2
(const float x, const float y);

// >> returns x value of 2d unit vector from an angle
float math_x_from_angle
(const float angle);

// >> returns y value of 2d unit vector from an angle
float math_y_from_angle
(const float angle);


// >> restrict given angle to between 0 and 2 PI
bool math_normalize_angle
(float *angle);


// >> get distance between 2 given points (x, y)
float math_distance_between_points
(const float x0, const float y0, const float x1, const float y1);


#endif
