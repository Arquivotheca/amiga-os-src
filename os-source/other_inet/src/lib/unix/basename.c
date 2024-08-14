/****** socket/basename ******************************************
*
*   NAME
*		basename -- get the filename from the pathname
*
*   SYNOPSIS
*		name = basename( pathname )
*
*		char *basename (char *); 
*
*   FUNCTION
*		Returns pointer to basename in the input string.
*
*	INPUTS
*		pathname	string containing pathname of file.
*
*   RESULT
*		name		pointer to basename in string
*
*   EXAMPLE
*		name = basename("dh0:graphics/pics/car.iff");
*		name points to "car.iff"
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

char *basename(char *p)
{
	register char *op;

	op = p;
	while(*p){
		if(*p == '/' || *p == ':'){
			op = ++p;
		} else {
			p++;
		}
	}

	return op;
}

