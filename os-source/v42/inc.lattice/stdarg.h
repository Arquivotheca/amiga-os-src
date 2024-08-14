#ifndef _STDARGH
#define _STDARGH 1

#ifndef _VA_LIST
#define _VA_LIST 1
typedef char *va_list;
#endif

#define va_start(a,b) a=(char *)(&b+1)
#define va_arg(a,b) *((b *)( (a+=sizeof(b))-sizeof(b) ))
#define va_end(a) /* end of varg a */

/**
*
* Define NULL if it's not already defined
*
*/
#ifndef NULL
#define	NULL	(void *)0
#endif

#endif
