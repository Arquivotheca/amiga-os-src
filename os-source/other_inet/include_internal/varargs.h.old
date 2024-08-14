/*
** Our own version of varargs.  Manx should have provided this....
*/

#ifndef VARARGS_H
#define VARARGS_H

typedef char *va_list;

#define va_dcl		int	va_alist;
#define va_start(list)	(list) = (char *)&va_alist;
#define va_arg(list, type)	((type *)(list += sizeof(type)))[-1]
#define va_end(list)

#endif
