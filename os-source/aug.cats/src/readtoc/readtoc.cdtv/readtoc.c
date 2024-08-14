/********************************************************************
*
*   NAME
*	ReadTOC
*
*   SYNOPSIS
*	Read the Table of Contents from a CD.
*
*   DESCRIPTION
*	This version uses cdtv.device to read the table of contents
*	from a CD.
*
*   FILES
*	None.
*
*   NOTES
*	Verison 1.1 adds a better laid-out table with track length
*	as well as starting LSN.
*
*/


/*  DEFINES  ********************************************************
*/


/*  INCLUDES  *******************************************************
*/
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <cdtv/cdtv.h>


/*  PROTOS  **********************************************************
*/
void shutdown(int rc, char *retmsg);


/*  GLOBALS  *********************************************************
*/
char
	versiontag[] = "\0$VER:  ReadTOC 1.1 (4-MAY-93)";

int
	startLSN, lengthLSN, disclength, m, s, f;

struct MsgPort 
	*reply;

struct IOStdReq
	*cdio;

static struct CDTOC
	TOCArray[ 100 ];

long
	i,err;


/*  MAIN  ************************************************************
*********************************************************************/
main()
{
	/*  Create reply port.
	*/
	if (!(reply = CreatePort(NULL,NULL)))
		shutdown(10, "Cannot allocate reply port.");

	/*  Create IO structure.
	*/
	if (!(cdio = (struct IOStdReq *) CreateStdIO(reply)))
    		shutdown(10, "Cannot allocate CD I/O structure.");

	/*  Open the device.  Give error value if error occurs.
	*/
	if (err = OpenDevice( CDTV_NAME, 0, (struct IORequest *) cdio, 0 )  )
		shutdown(err, "Cannot open cd device.");

	/*  Prepare I/O request.
	*/
	cdio->io_Command = CDTV_TOCLSN;
	cdio->io_Offset  = 0;	
	cdio->io_Length  = 100;
	cdio->io_Data    = (APTR) TOCArray;

	/*  Do I/O Request.  Give error value if error occurs.
	*/
	if ( err = DoIO( (struct IORequest *) cdio ))
		shutdown(err, "I/O request failed.");


	/*  Calculate length of disc.
	*/
	disclength = TOCArray[0].Position.LSN;

	/*  Display results.
	*/
	/*  Print headers for TOC table.
	*/
	printf("TRACK       START      LENGTH IN\n");
	printf("NUMBER       LSN        SECTORS\n");
	printf("------     -------     ---------\n");

	/*  Cycle through each track, giving starting LSN and length.  Note
	*   that the length of the last track must be calculated differently
	*   since there is no "next track" to substract LSNs.
	*/
	for (i=1; i <= cdio->io_Actual; i++)
	{
		startLSN  = TOCArray[i].Position.LSN;

		if (i < cdio->io_Actual)
			lengthLSN = TOCArray[i+1].Position.LSN - startLSN;
		else
			lengthLSN = disclength - startLSN;

		printf("   %3d     %7d       %7d\n", i, startLSN, lengthLSN);
	}

	shutdown(0, NULL);
}


/********************************************************************
*
*   NAME
*       shutdown  --  clean up system and exit program
*
*   FUNCTION
*       Close libraries, filehandles and deallocate structures and
*       memory.  Exit while giving the appropriate return code and
*       a message string if one was passed to the function and the
*       return code was greater than zero.
*
*   INPUT
*       rc          standard return code
*       retmsg      string to be sent to stderr
*
*   RESULT
*       None.  Sends a string to stderr.
*
*   NOTES
*
*/

void shutdown(int rc, char *retmsg)
{
	if (cdio)
	{
		CloseDevice( (struct IORequest *) cdio );
		DeleteStdIO((struct IOStdReq *)cdio);
	}

	if (reply)
		DeletePort(reply);

    if (rc)
        fprintf(stderr, "Error Code %i\n%s\n", rc, retmsg);
    exit(rc);
}