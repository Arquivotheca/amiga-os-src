/****** socket/ffs ******************************************
*
*   NAME
*		ffs -- find the first bit set
*
*   SYNOPSIS
*		return = ffs(i)
*
*		int ffs (int); 
*
*   FUNCTION
*		returns the index of the first bit set in i.
*		Bits are numbered starting with 1 from the right.
*
*	INPUTS
*		i	integer
*
*   RESULT
*		1-32 will be returned if successful.  
*		-1 is returned if the input is zero.
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
** Scarfed from GNU project.  Rewrite in assembly language at some
** point.
*/


int
ffs (i)
     	register unsigned long i;
{
  	register int value = 1;

  	if (!i){
		return -1;
	}

  	if (!(i & 0xFFFF)){
    		value += 16, i >>= 16;
	}

  	if (!(i & 0xFF)){
    		value += 8, i >>= 8;
	}

  	if (!(i & 0xF)){
    		value += 4, i >>= 4;
	}

  	if (!(i & 0x3)){
    		value += 2, i >>= 2;
	}

  	if (!(i & 1)){
    		value++;
	}

  	return value;
}

