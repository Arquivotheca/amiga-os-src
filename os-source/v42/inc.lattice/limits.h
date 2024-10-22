/**
*
* The following symbols are specified in the ANSI C standard as limit
* values for various non-float characteristics.
*
**/

#define CHAR_BIT 8		/* bits per char 			*/
#define CHAR_MAX 127		/* max value for char 			*/
#define CHAR_MIN -128		/* min value for char 			*/
#define SCHAR_MAX 127		/* max value for signed char 		*/
#define SCHAR_MIN -128		/* min value for signed char 		*/
#define UCHAR_MAX 255		/* max value for unsigned char 		*/
#define SHRT_MAX 32767		/* max value for short int 		*/
#define SHRT_MIN -32768 	/* min value for short int 		*/
#define USHRT_MAX 65535		/* max value for unsigned short int 	*/
#ifdef SHORTINT
#define INT_MAX 32767		/* max value for short int 		*/
#define INT_MIN -32768 	/* min value for short int 		*/
#define UINT_MAX 65535		/* max value for unsigned short int 	*/
#else
#define INT_MAX 2147483647	/* max value for int 			*/
#define INT_MIN -2147483648 	/* min value for int 			*/
#define UINT_MAX 4294967295 	/* max value for unsigned int 		*/
#endif
#define LONG_MAX 2147483647	/* max value for long int 		*/
#define LONG_MIN -2147483648	/* min value for long int 		*/
#define ULONG_MAX 4294967295	/* max value for unsigned long int 	*/
