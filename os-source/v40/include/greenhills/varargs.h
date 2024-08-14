typedef char *va_list;
#define va_dcl		int va_alist;
#define va_start(ap)	(ap) = (char *)(&va_alist)
#define va_arg(ap,mode)	*((mode *)(ap))++
#define va_end(ap)
