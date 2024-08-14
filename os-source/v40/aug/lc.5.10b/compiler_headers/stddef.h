#ifndef _STDDEFH
#define _STDDEFH 1

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

typedef long int ptrdiff_t;
typedef unsigned char wchar_t;

#define offsetof(type,memb) (size_t)&((( type *)0L)->memb)


/**
*
* Define NULL if it's not already defined
*
*/
#ifndef NULL
#define	NULL	0L
#endif


#endif

