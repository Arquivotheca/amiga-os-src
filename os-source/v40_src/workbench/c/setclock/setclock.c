/* $Header: /usr/machines/ghostwheel/commodore/amiga/V36/src/workbench/c/setclock/RCS/setclock.c,v 1.9 90/09/06 19:48:40 andy Exp $
 *
 * Alfred - the program that takes care of BattClock.resource and
 *	BattMem.resource. Also, the replacement for SetClock.
 *
 * $Author: andy $
 *
 * $Date: 90/09/06 19:48:40 $
 *
 * $Id: setclock.c,v 1.9 90/09/06 19:48:40 andy Exp $
 *
 * $Locker:  $
 *
 * $RCSfile: setclock.c,v $
 *
 * $Revision: 1.9 $
 *
 * $Source: /usr/machines/ghostwheel/commodore/amiga/V36/src/workbench/c/setclock/RCS/setclock.c,v $
 *
 * $State: Exp $
 *
 * $Log:	setclock.c,v $
 * Revision 1.9  90/09/06  19:48:40  andy
 * release 2.0 version
 * 
 * Revision 1.8  90/05/30  13:08:24  andy
 * fixed error return
 * 
 * Revision 1.7  90/05/29  22:59:06  andy
 * removed incorrect initialization of message
 * 
 * Revision 1.6  90/04/12  17:49:08  andy
 * returns RETURN_OK if no error, rather than a 20
 * 
 * Revision 1.2  89/12/10  19:06:44  rsbx
 * Change in error message.
 * 
 * Revision 1.1  89/10/27  22:40:41  rsbx
 * Initial revision
 * 
 *
 */
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <exec/devices.h>
#include <devices/timer.h>
#include <resources/battclock.h>
#include <dos/dos.h>

#include "pragmas/battclock_pragmas.h"

#include "string.h"
#include "internal/commands.h"
#include "setclock_rev.h"

extern struct Library *OpenLibrary();
extern void aprintf();



#define TEMPLATE    "LOAD/S,SAVE/S,RESET/S" CMDREV

#define OPT_LOAD    	0
#define OPT_SAVE        1
#define OPT_RESET       2

#define OPT_COUNT	3



long cmd_setclock(void)
{
    struct Library *SysBase = (*((struct Library **) 4));
    struct Resource *BattClockBase;
    struct DosLibrary *DOSBase;
    long rc=RETURN_FAIL;
    struct timerequest *timermsg=0;
    long opts[OPT_COUNT],td= -1;
    struct RDargs *rdargs;

    if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {
    	memset((char *)opts, 0, sizeof(opts));
      	rdargs = ReadArgs(TEMPLATE, opts, NULL);
        if (rdargs == NULL) {
   	    PrintFault(IoErr(), NULL);
	    goto err;
	}

	if (!(BattClockBase = OpenResource(BATTCLOCKNAME))) {
	    aprintf("Can't find battery-backed up clock\n");
	    goto err;
	}
	timermsg=AllocMem(sizeof(struct timerequest),MEMF_PUBLIC|MEMF_CLEAR);
 	if(!timermsg)goto err;

	if (td=(long)OpenDevice( TIMERNAME, 0, timermsg, 0 )) {
	    aprintf("Could not open timer.device\n");
    	    goto err;
	}
	rc=RETURN_OK;

	timermsg->tr_node.io_Message.mn_Node.ln_Type = 0;
	timermsg->tr_node.io_Message.mn_Node.ln_Pri = 0;
	timermsg->tr_node.io_Message.mn_Node.ln_Name = NULL;	
	timermsg->tr_node.io_Message.mn_ReplyPort = NULL;
	timermsg->tr_time.tv_micro = 0;

	if(opts[OPT_LOAD]) {
		timermsg->tr_time.tv_secs = ReadBattClock();
		timermsg->tr_node.io_Command = TR_SETSYSTIME;
		DoIO(timermsg);
	}
	else if(opts[OPT_RESET])ResetBattClock();
	else if(opts[OPT_SAVE]) {
    		timermsg->tr_node.io_Command = TR_GETSYSTIME;
		DoIO(timermsg);
		WriteBattClock(timermsg->tr_time.tv_secs);
	}

err:    if(rdargs)FreeArgs(rdargs);
	if(!td)CloseDevice( timermsg );
	if(timermsg)FreeMem(timermsg,sizeof(struct timerequest));
        CloseLibrary((struct Library *)DOSBase);
	}
	else {OPENFAIL}

    return(rc);
}

void aprintf( fmt, args )
char *fmt, *args;
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct Library *DOSBase;
   if(DOSBase=OpenLibrary(DOSLIB,DOSVER)) {
       VFPrintf( Output(), fmt, (LONG *)&args );
       CloseLibrary(DOSBase);
   }
}
