/* common string functions */

#include "exec/types.h"

#define TOUPPER(x) ((x) & 0x5f)

char *
strcpy(dest, source)
register char    *dest;
register char    *source;
{
    char *orig = dest;
    while (*dest++ = *source++);

    return( orig );
}

int stricmp(astr, bstr)
UBYTE *astr;
UBYTE *bstr;
{

UBYTE c;

    while ((c = TOUPPER(*astr)) && (c == TOUPPER(*bstr))) {
	astr++, bstr++;
    }
    if (!c)return(0);
    if( c < *bstr ) return( -1 );
    return( 1 );
}

char *
strcat( s, new )
register char *s, *new;
{
    char c;
    char *orig = s;
    while( *s ) s++;
    do 	{
       	c = *s++ = *new++;
    		} while( c );

    return( orig );
}

strlen( str ) 
char *str;
{
register char *pt ;

for ( pt=str ; *pt != '\0' ; pt++ );
return( pt-str );
} 

int
atoi(str)
char	*str;
{
register int value=0;

while(*str != '\0' && *str >='0' && *str <= '9' )value=value*10+ (*str++)-'0';
return(value);
}
