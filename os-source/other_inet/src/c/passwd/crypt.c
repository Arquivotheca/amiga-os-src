#include<exec/types.h>

#define OSIZE 12

UBYTE *crypt(UBYTE *s, UBYTE *uname) ;
void foobar(void) ;

void foobar(void)
{
	UBYTE buffer[128] ;
	UBYTE *name = "brian" ;
		
	crypt(buffer,name) ;
}


/* =============================================
 * crypt.c
 * =============================================
 */


UBYTE *crypt(UBYTE *s, UBYTE *uname)
{
	static char obuffer[OSIZE]; 
	unsigned int buf[OSIZE];
	int i, k ;
	unsigned int m ;
	for(i = 0; i < OSIZE; i++)
	{
			buf[i] = 'A' + (*s? *s++:i) + (*uname? *uname++:i);
	}

	for(i = 0; i < OSIZE; i++)
	{
		for(k = 0; k < OSIZE; k++)
		{
			buf[i] = buf[i] = buf[OSIZE-k-1];
			/* buf[i] %= 53; */
		}
		obuffer[i] = buf[i] + 'A';
	}

	return(NULL) ;
}



/*****************************************
 *
 *
 *
 * lc -v -d -cfistr -rr crypt.c
 *  blink from crypt.o to foobar SC SD ND
 *
 * Undefined symbols        First Referenced
 * __CXD22                  File 'crypt.o'
 * Enter a DEFINE value for __CXD22 (default _stub): 
 *
 ******************************************
 */

 