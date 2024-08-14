/* $Header: /usr/machines/heartofgold/amiga/V36/src/workbench/c/setclock/RCS/alfred.c,v 1.2 90/09/06 19:48:57 andy Exp $
 *
 * Alfred - the program that takes care of BattClock.resource and
 *	BattMem.resource. Also, the replacement for SetClock.
 *
 * $Author: andy $
 *
 * $Date: 90/09/06 19:48:57 $
 *
 * $Id: alfred.c,v 1.2 90/09/06 19:48:57 andy Exp $
 *
 * $Locker:  $
 *
 * $RCSfile: alfred.c,v $
 *
 * $Revision: 1.2 $
 *
 * $Source: /usr/machines/heartofgold/amiga/V36/src/workbench/c/setclock/RCS/alfred.c,v $
 *
 * $State: Exp $
 *
 * $Log:	alfred.c,v $
 * Revision 1.2  90/09/06  19:48:57  andy
 * revived alfred for use under 1.3 kickstart
 * 
 * Revision 1.1  90/04/12  13:37:17  andy
 * Initial revision
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
#include <stdio.h>

extern APTR OpenResource(char *name);
void usage( void );


APTR BattClockBase;


void main(int argc, char **argv)
	{
	int function = 0;
	struct timerequest timermsg;

	
	if (!(BattClockBase = OpenResource(BATTCLOCKNAME)))
		{
		fprintf(stderr, "Battery backed up clock not found.\n");
		exit(20);
		}

	if (argc != 2)
		{
		usage();
		}

	if (!stricmp("l", argv[1]) || !stricmp("load", argv[1]))
		{
		function = 1;
		}
	else
		{
		if (!stricmp("s", argv[1]) || !stricmp("save", argv[1]))
			{
			function = 2;
			}
		else
			{
			if (!stricmp("r", argv[1]) || !stricmp("reset", argv[1]))
				{
				function = 3;
				}
			}
		}

	if (OpenDevice( TIMERNAME, 0, &timermsg, 0 ))
		{
		fprintf(stderr, "OpenDevice of %s failed\n", TIMERNAME);
		exit(100);
		}
	timermsg.tr_node.io_Message.mn_Node.ln_Type = 0;
	timermsg.tr_node.io_Message.mn_Node.ln_Pri = 0;
	timermsg.tr_node.io_Message.mn_Node.ln_Name = NULL;	
	timermsg.tr_node.io_Message.mn_ReplyPort = NULL;
	timermsg.tr_time.tv_micro = 0;

	switch(function)
		{
		case 1: /* load */
			timermsg.tr_time.tv_secs = ReadBattClock();
			timermsg.tr_node.io_Command = TR_SETSYSTIME;
			DoIO(&timermsg);
			break;
		case 3: /* reset */
			ResetBattClock();
		case 2: /* save */
			timermsg.tr_node.io_Command = TR_GETSYSTIME;
			DoIO(&timermsg);
			WriteBattClock(timermsg.tr_time.tv_secs);
			break;
		default:
			CloseDevice( &timermsg );
			usage();
		}
	CloseDevice( &timermsg );
	}


void usage( void )
	{
	fprintf(stderr, "Usage:  SetClock load|save|reset\n");
	fprintf(stderr, "    load -- loads system date/time from clock chip\n");
	fprintf(stderr, "    save -- saves system date/time to clock chip\n");
	exit(10);
	}
