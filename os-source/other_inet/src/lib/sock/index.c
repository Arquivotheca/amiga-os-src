/****** socket/index ******************************************
*
*   NAME
*		index -- find a character in a string
*
*   SYNOPSIS
*		ptr = index( s, c )
*
*		char *index ( char *, char ); 
*
*   FUNCTION
*		Returns a pointer to the first occurrence of the character c
*		in string s.
*
*	INPUTS
*		s	input string
*
*		c	character to find
*
*   RESULT
*		ptr		pointer to first character c in string s
*
*		Will return NULL if c is not found.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/*
 * index() for lattice, and BSD compatible one for Manx
 */


char *
index(s, c)
	register char *s, c;
{
	while(*s){
		if(*s == c){
			break;
		}
		s++;
	}

	if(*s == c){
		return s;
	}
	return (char *)0;
}
