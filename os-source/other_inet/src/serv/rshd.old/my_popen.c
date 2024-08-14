/* -----------------------------------------------------------------------
 * MY_POPEN.C for RSHD  (Manx 36 only)
 *
 * $Locker:  $
 *
 * $Id: my_popen.c,v 1.3 90/11/26 15:52:03 bj Exp $
 *
 * $Revision: 1.3 $
 *
 * $Header: HOG:Other/inet/src/serv/rshd/RCS/my_popen.c,v 1.3 90/11/26 15:52:03 bj Exp $
 *
 * $Log:	my_popen.c,v $
 * Revision 1.3  90/11/26  15:52:03  bj
 * *** empty log message ***
 * 
 * Revision 1.2  90/11/20  17:22:44  bj
 * Added RCS Header.
 * 
 *
 *------------------------------------------------------------------------
 */

 
/*
** popen()/pclose() for Amiga.  Relies on the existance of PIPE: device.
*/

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <exec/types.h>
#include <exec/tasks.h>
#include <libraries/dosextens.h>

static char template[] = "PIPE:popnXXXXXXXX" ;

int my_popen( program,pname)
char *program; 
register char *pname;
{
	struct FileHandle *fh ;

	*pname = '\0';

	Forbid() ;

	strcpy(pname, template) ;
	mktemp(pname) ;

	if((fh = (struct FileHandle *)Open(pname,MODE_NEWFILE)) == NULL ){
			Permit();
			perror(pname);
			return 0;
	}

	Permit() ;

	Execute(program, 0L, fh ) ; 

	Close(fh);
	return( 1 ) ;
}

