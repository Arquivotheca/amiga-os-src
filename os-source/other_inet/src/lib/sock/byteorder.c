/****** socket/byteorder *************************************************
*
*   NAME
*		byteorder -- macros to convert between host and network byte order
*
*   SYNOPSIS
*		#include <sys/types.h>
*		#include <netinet/in.h>
*
*		netlong   = htonl ( hostlong ); 
*		netshort  = htons ( hostshort);
*		hostlong  = ntohl ( netlong );
*		hostshort = ntohs ( netshort);
*
*   FUNCTION
*		These routines convert between host byte order to network byte
*		order.  This is necessary because not all CPUs store words and
*		longs in the same manner.  Some CPUs, called "Big Endian" store
*		words as high byte followed by low byte.  Others, called "Little
*		Endian" store a word as low byte followed by high byte.
*
*		Big endian machines include IBM mainframes and Motorola 68000
*		family machines.  Little endian machines include DEC VAXen and
*		Intel 80x86 family CPUs.
*
*		Network order is big endian.  Therefore, on big endian machines,
*		these functions are null macros.
*
*   EXAMPLE
*
*   NOTES
*		These macros are provided for portability.
*		They are null macros on the Amiga.
*
*		#define htons(x)	(x)
*		#define ntohs(x)	(x)
*		#define htonl(x)	(x)
*		#define ntohl(x)	(x)
*
*   SEE ALSO
*
******************************************************************************
*
*/
