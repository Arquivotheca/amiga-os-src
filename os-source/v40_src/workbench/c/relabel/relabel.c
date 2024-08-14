; /*
lc -j73 -d -rr -O -o/obj/Relabel.o -i/include -v -csf Relabel
blink /obj/Relabel.o to /bin/Relabel LIB lcr.lib sc sd nd batch
protect /bin/Relabel +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 382-8265    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*	       John Toebes     Mary Ellen Toebes  Doug Walker	Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*-----------------------------------------------------------------------*/
/* Command: Relabel							 */
/* Author:  Bruce M. Drake						 */
/* Change History:							 */
/*  Date    Person	  Action					 */
/* -------  ------------- -----------------				 */
/* 24MAR89  Bruce Drake   Initial Creation				 */
/* 15APR89    "     "     Trimmed with addition of more calls to new dos */
/* 30OCT89    "     "     Converted and tested to 1.4 Specs              */
/* 08DEC89    "     "     Completed and ready.                           */
/* 13APR91    "     "     Corrected last minute problems with buffer     */
/*			    overruns and list locking.			 */
/* Notes:								 */
/*-----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "relabel_rev.h"
#include <string.h>

#define TEMPLATE    "DRIVE/A,NAME/A" CMDREV
#define OPT_DRIVE   0
#define OPT_NAME    1
#define OPT_COUNT   2

#define NAME_SIZE   128

#define MSG_NO_COLON	   "':' not legal character in volume name\n"
#define MSG_INVALID_DEVICE "Invalid device or volume name\n"

int cmd_relabel(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   struct RDargs *rdargs;
   struct DosList *doslist;
   int returncode;
   long chrpos;
   LONG opts[OPT_COUNT];
   UBYTE drivename[NAME_SIZE];

   returncode = RETURN_FAIL;
   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)))
   {
      memset((char *)opts, 0, sizeof(opts));
      if ((rdargs = ReadArgs(TEMPLATE, opts, NULL)) == NULL)
	 PrintFault(IoErr(), NULL);
      else
      {
	 if (strchr((char *)opts[OPT_NAME], ':') )
	 {
	    PutStr(MSG_NO_COLON);
	 }
	 else
	 {
	    chrpos = strlen((char *)opts[OPT_DRIVE]);
	    ((char *)opts[OPT_DRIVE])[chrpos - 1] = '\0';

	    doslist = LockDosList( LDF_ALL | LDF_READ );
	    doslist = FindDosEntry( doslist, (char *)opts[OPT_DRIVE], LDF_ALL);
	    UnLockDosList( LDF_ALL | LDF_READ );

	    if ( !doslist )
	    {
	      PutStr(MSG_INVALID_DEVICE);
	    }
	    else
	    {
	      strcpy ( drivename, (char *)opts[OPT_DRIVE] );
	      strcat ( drivename, ":");

	      if ( !Relabel( drivename, (char *)opts[OPT_NAME] ) )
	      {
		 PrintFault(IoErr(), NULL);
	      }
	      else
	      {
		 returncode = RETURN_OK;
	      }
	    }
	 }
	 FreeArgs(rdargs);
      }
      CloseLibrary((struct Library *)DOSBase);
   }
   else
   {
     OPENFAIL;
   }
   return(returncode);
}
