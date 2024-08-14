/****** socket/getdtablesize ******************************************
*
*   NAME
*		getdtablesize -- get descriptor table size
*
*   SYNOPSIS
*		numfds = getdtablesize()
*
*		int getdtablesize(); 
*
*   FUNCTION
*		Returns the size of the descriptor table. 
*
*	INPUTS
*
*   RESULT
*		numfds	number of descriptors;
*
*   EXAMPLE
*
*   NOTES
*		In Manx, this is fixed at MAXSTREAM in stdio.h.
*		
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

#ifdef LATTICE
extern int _nufbs;
#define NUMFD _nufbs
#else
#include <fcntl.h>
#define NUMFD _numdev
#endif

int
getdtablesize()
{
	return NUMFD;
}

