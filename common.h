/*----------------------------------
updated by wonderly321 on 4/11/19.
-----------------------------------*/// Created by huangyingfan on 5/22/18.
//

#ifndef SIFT_COMMON_H
#define SIFT_COMMON_H

#include <stdarg.h>
#include <math.h>

// *** Optimization options ***
#define USE_FAST_FUNC					1

// *** Dump functions to get intermediate results ***
#define DUMP_OCTAVE_IMAGE				0
#define DUMP_GAUSSIAN_PYRAMID_IMAGE		0
#define DUMP_DOG_IMAGE					0

// Print out matched keypoint pairs in match_keypoints() function.
#define	PRINT_MATCH_KEYPOINTS			1


// *** Macro definition ***
// Macro definition
#define PI				3.141592653589793f
#define _2PI			6.283185307179586f
#define PI_4			0.785398163397448f
#define PI_3_4			 2.356194490192345f
#define SQRT2			1.414213562373095f


// *** Helper functions ***
inline float my_log2(float n)
{
    // Visual C++ does not have log2...
    return (float) ((log(n))/0.69314718055994530941723212145818);
}

// *** Fast math functions ***
// Fast Atan2() function
#define EPSILON_F	1.19209290E-07F
inline float fast_atan2_f (float y, float x)
{
    float angle, r ;
    float const c3 = 0.1821F ;
    float const c1 = 0.9675F ;
    float abs_y    = fabsf (y) + EPSILON_F ;

    if (x >= 0) {
        r = (x - abs_y) / (x + abs_y) ;
        angle = PI_4; //
    } else {
        r = (x + abs_y) / (abs_y - x) ;
        angle = PI_3_4; //
    }
    angle += (c3*r*r - c1) * r ;

    return (y < 0) ? _2PI - angle : angle;
}

// Fast Sqrt() function
inline float fast_resqrt_f (float x)
{
    /* 32-bit version */
    union {
        float x ;
        int  i ;
    } u ;

    float xhalf = (float) 0.5 * x ;

    /* convert floating point value in RAW integer */
    u.x = x ;

    /* gives initial guess y0 */
    u.i = 0x5f3759df - (u.i >> 1);

    /* two Newton steps */
    u.x = u.x * ( (float) 1.5  - xhalf*u.x*u.x) ;
    u.x = u.x * ( (float) 1.5  - xhalf*u.x*u.x) ;
    return u.x ;
}

inline float fast_sqrt_f (float x)
{
    return (x < 1e-8) ? 0 : x * fast_resqrt_f (x) ;
}

#endif //SIFT_COMMON_H
