;/* rebooto.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 rebooto.c
Blink FROM LIB:c.o,rebooto.o,coldreboot.o TO rebooto LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

/* rebooto.c
** LC -b1 -cfist -v -y rebooto.c
**
** you need two object files for this:
**    rebooto.o        ;compile from this code
**    coldreboot.o     ;assemble from coldreboot.a
** link them with:
**    FROM LIB:c.o+"rebooto.o"+"coldreboot.o"
**    TO "rebooto"
**    LIB LIB:lcnb.lib LIB:amiga.lib
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <devices/timer.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(VOID) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(VOID) { return(0); }  /* really */
#endif

#define MINARGS 3

UBYTE *vers = "\0$VER: rebooto 37.2";
UBYTE *ourname = "RebootoTask";

/*-------------------------------------------------------------------
** Prototype for the reboot entry point.
*/
VOID ColdReboot(VOID);

/*-------------------------------------------------------------------
**
*/
VOID waittenth(struct timerequest *timeIO)
{
timeIO->tr_node.io_Command = TR_ADDREQUEST;
timeIO->tr_time.tv_secs  = 0;
timeIO->tr_time.tv_micro = 100000;
DoIO((struct IORequest *)timeIO);
}

/*-------------------------------------------------------------------
**
*/
LONG getval(UBYTE *s)
{
LONG value, count;

if ((s[1]|0x20) == 'x')
	count = stch_i(&s[2],&value);
else
	count = stcd_i(&s[0],&value);
return(value);
}

/*-------------------------------------------------------------------
** To check if it is a keyup/down, look at bit 8.  The remaining bits
**  are the hardware scan code.
*/
UBYTE CheckKey(VOID)
{
		UBYTE far	*cia	= (UBYTE *)0xbfec01; /* note the "far" kludge */
static	UBYTE		 last	= 0;
		UBYTE		 retval;

retval = *cia;

if (retval != last)
	last = retval;
else
	retval = 0;

/* since 'C' is totally bad about not providing things like ROR.B 
** you get really bad looking stuff like the following:
*/
retval = ((retval & 1) << 7) + ((retval>>1)&0x7f);
return((UBYTE)(~retval));
}

/*-------------------------------------------------------------------
** Usage: rebooto seconds 0x<RawCode>	OR   rebooto OFF
**    0 for seconds means never time out
**    0 for RawCode means no key will reboot
*/
VOID main(int argc, char **argv)
{
struct Task *task;
UBYTE *oldname = NULL;
ULONG seconds, countdown;
UBYTE rawcode, rawpress;

struct timerequest	*timeIO;
struct MsgPort		*msgPort;

/* If keyword OFF is passed, just kill other guy and exit */
if((argc == 2)&&(!(stricmp(argv[1],"off"))))
	{
	if(task = FindTask(ourname))   Signal(task,SIGBREAKF_CTRL_C);
	else printf("No rebooto currently running\n");
	exit(RETURN_OK);
	}
else if((argc<MINARGS)||(argv[argc-1][0] == '?'))
	{
	printf("Usage: rebooto seconds 0x<RawCode>  OR  rebooto OFF\n");
	printf("0 seconds means never time out, 0 RawCode means no key will reboot\n");
	exit(RETURN_OK);
	}
else
	{
	task = FindTask(NULL);
	oldname = task->tc_Node.ln_Name;
	task->tc_Node.ln_Name = ourname;

	seconds = getval(argv[1]);
	rawcode = getval(argv[2]);

	countdown = seconds ? seconds * 10 : 1;

	if (NULL != (msgPort = CreatePort(0,0)))
		{
		if (NULL != (timeIO = (struct timerequest *)
				CreateExtIO(msgPort,sizeof(struct timerequest))))
			{
			if (0 == OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)timeIO,0))
				{
				while(countdown)
					{
					if(SetSignal(0,0) & SIGBREAKF_CTRL_C)
						break;
					waittenth(timeIO);
					if (rawcode == (rawpress = CheckKey()))
						countdown = 0;
					else if (seconds)
						countdown--;
					}
				CloseDevice((struct IORequest *)timeIO);
				}
			DeleteExtIO((struct IORequest *)timeIO);
			}
		DeletePort(msgPort);
		}
	task->tc_Node.ln_Name = oldname;
	}
/* It's kind of stupid to clean everything up, as we're just going
** to reboot the machine...
** (that is Ken's comment - I think I did clean up...)
*/

/* New version - we only reboot if killed by key or countdown */
if(!countdown)	ColdReboot();
else exit(RETURN_OK);
}
