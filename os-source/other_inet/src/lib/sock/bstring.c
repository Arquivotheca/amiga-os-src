/****** socket/bstring ******************************************
*
*   NAME
*		bstring -- bit and byte string operations
*
*   SYNOPSIS
*		bcopy (src , dest, num)
*		bmov  (src, dest, num)
*		return = bcmp  (b1, b2, num)
*		bset (src, num, c)
*		bzero (src, num)
*
*		void bcopy (void *, void *, int);
*		void bmov (void *, void *, int);
*		int bcmp (void *, void *, int);
*		void bset (void *, int, char);
*		void bzero(void *, int); 
*
*   FUNCTION
*		bcopy() and bmov() copy 'num' bytes from 'src' to 'dest'.  
*		The two functions are identical. 
*
*		bcmp() compares the two strings 'b1' and 'b2' for 'num' bytes.
*
*		bset() sets 'num' bytes to 'c' starting at location 'src'.
*		bzero() is a special case of bset() where 'c'=0.
*
*   RESULT
*		For bcmp(), 0 is returned if the two strings are identical,
*		non-zero otherwise. 
*
*   NOTES
*		bcmp() and bcopy() are similar to memcpy() and memmove() except
*		the parameters are in reverse order.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/


