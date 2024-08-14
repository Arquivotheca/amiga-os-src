/*
    C Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/

typedef float single;

#ifdef mc68000
# define BigEndian
#endif

#define floor	IEEEDPFloor
#define fabs	IEEEDPAbs

#define LIBM_PI	3.14159265358979323846
double acos(), sqrt(), atan(), asin(), atan2(), j0(), sin(), cos(), y0(),
       log(), j1(), y1(), jn(), yn(), erf(), erfc(), exp(), gamma(),
       sinh(), cosh(), tanh(), log10(), pow(), tan(), atof(), ldexp(),
       frexp(), modf(), ceil(), floor(), hypot(), fabs(), cabs(), fmod();
float  rldexp();


typedef struct {
    unsigned long dbl_sign : 1;
    unsigned long dbl_exp : 11;
    unsigned long dbl_frac1 : 20;
    unsigned long dbl_frac2 : 32;
} DOUBLE;
typedef struct {
    unsigned long flt_sign : 1;
    unsigned long flt_exp : 8;
    unsigned long flt_frac1 : 23;
} FLOAT;

#define KNOWN_FLOATING_POINT_FORMAT	1
#define EXTRACTEXPONENT(x)	(((DOUBLE *)&(x))->dbl_exp-1022)
#define EXTRACTFLOATEXPONENT(x)	(((FLOAT *)&(x))->flt_exp-126)
#define SETEXPONENT(x,exp)	(((DOUBLE *)&(x))->dbl_exp = ((exp)+1022))
#define SETFLOATEXPONENT(x,exp)	(((FLOAT *)&(x))->flt_exp = ((exp)+126))
#define AUGMENTEXPONENT(x, inc) (((DOUBLE *)&(x))->dbl_exp += (inc))
#define AUGMENTFLOATEXPONENT(x, inc) (((FLOAT *)&(x))->flt_exp += (inc))
#define MINIMUMEXPONENT		-1021
#define MINIMUMFLOATEXPONENT	-125
#define MAXIMUMEXPONENT		1024
#define MAXIMUMFLOATEXPONENT	128
#define HUGE			(1.7e308)
#define HUGEFLOAT		(3.4e38)
#define HALVE(x)        halve(&x)
