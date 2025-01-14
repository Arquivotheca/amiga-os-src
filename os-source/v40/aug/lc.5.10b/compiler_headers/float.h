/**
*
* The following symbols are specified in the ANSI C standard for the
* floating point number system.
*
**/
#define FLT_RADIX 2		/* radix of exponent 			*/
#define FLT_ROUNDS 0		/* rounding mode during translation	*/
				/*   0 => chop				*/
				/*   1 => round				*/
				/*   2 => indeterminate			*/
#define FLT_GUARD 0		/* guard digits during multiplication 	*/
				/*   0 => No				*/
				/*   1 => Yes				*/
#define FLT_NORMALIZE 1		/* normalization required		*/
				/*   0 => No				*/
				/*   1 => Yes				*/
#define DBL_MAX_EXP 308		/* max decimal exponent for double	*/
#define FLT_MAX_EXP 37 		/* max decimal exponent for float 	*/
#define DBL_MIN_EXP -307	/* min decimal exponent for double	*/
#define FLT_MIN_EXP -38		/* min decimal exponent for float	*/
#define DBL_DIG 16		/* max decimal digits for double	*/
#define FLT_DIG 7		/* max decimal digits for float		*/

#ifndef HUGE_VAL
#define HUGE_VAL 1.797693E+308	/* huge double value			*/
#endif

#define DBL_MAX  1.797693E+308	/* huge double value			*/
#define FLT_MAX  3.402823e+38 	/* huge float value			*/
