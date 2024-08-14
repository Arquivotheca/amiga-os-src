#include "ed.h"

/*#define TOUPPER(c)      ((c)>='a'&&(c)<='z'?(c)-'a'+'A':(c))*/
/*#define HIGHER(x,y)     ((x)>(y)?(x):(y))*/

#define TOUPPER(x) ((x) & 0x5f)


#if	AMIGA

UBYTE *Lstrcpy(dest, source, length)
UBYTE    *dest;
UBYTE    *source;
int length;
{
int i;

    UBYTE *orig = dest;
    for(i=0; i< length; i++) {
	if(!(*dest++ = *source++))break;
    }
    return(orig);
}

int atoi( s )
UBYTE *s;
{
    int num = 0;
    int neg = 0;

    while (*s && (*s == ' '))s++; /* eat leading spaces */

    if( *s == '+' ) s++;
    else if( *s == '-' ) {
        neg = 1;
        s++;
    }

    while( *s >= '0' && *s <= '9' ) {
        num = num * 10 + *s++ - '0';
    }
/*
    while( *s >= '0' && *s <= '9' ) {
        num = num * 10 + *s++ - '0';
    }
*/
    if( neg ) return( - num );
    return( num );
}

#if 0
UBYTE *strcpy(dest, source)
UBYTE    *dest;
UBYTE    *source;
{
    UBYTE *orig = dest;
    while (*dest++ = *source++);

    return( orig );
}


int strcmp( a, b )
UBYTE *a, *b;
{
    while( *a++ == *b ) {
        if( ! *b++ ) return( 0 );
    }

    if( *--a < *b ) return( -1 );
    return( 1 );
}

UBYTE *strcat( s, new )
UBYTE *s, *new;
{
    UBYTE c;
    UBYTE *orig = s;
    while( *s ) s++;
    do 	{
       	c = *s++ = *new++;
    		} while( c );

    return( orig );
}

int strlen( str ) 
UBYTE *str;
{
UBYTE *pt ;

for ( pt=str ; *pt != '\0' ; pt++ );
return( pt-str );
} 

int stricmp( a, b )
UBYTE *a, *b;
{
    while( TOUPPER(*a++) == TOUPPER(*b) ) {
        if( ! *b++ ) return( 0 );
    }
    if( ! *b ) return( 0 ); /* partial match */
    if( *--a < *b ) return( -1 );
    return( 1 );
}

#endif

#endif
