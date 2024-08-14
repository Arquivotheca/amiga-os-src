;/* ext_sushi.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 ext_sushi.c
Blink FROM LIB:c.o,ext_sushi.o TO ext_sushi LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit

ext_sushi.c - An example of an external display or watcher program
		which is signalled by sushi.
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */

extern struct Library *SysBase;
extern struct Library *DOSBase;
#include "pragmas/exec_pragmas.h"
#include "pragmas/dos_pragmas.h"
#endif


#define MINARGS 1

UBYTE *vers = "\0$VER: ext_sushi 37.2";
UBYTE *Copyright = 
  "ext_sushi v37.2\nCopyright (c) 1992 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: ext_sushi";


void bye(UBYTE *s, int e);
void cleanup(void);


UBYTE sushiname[] = "sushi_CAS_port";

/* Definitions of WTP and AppStruct structures */

struct AppStruct {
   /* These are for READ-ONLY use by external application */
   UBYTE	*subuf;		/* Sushi's circular output buffer */
   ULONG	subufsize;	/* Size of buffer in bytes (a power of 2K) */
   ULONG	subufi;		/* Where Sushi will place next char received */
   ULONG	subufli;	/* Sushi's local index of what it has processed */
   ULONG	wrapcnt;	/* How may times circular buffer has wrapped */ 
   ULONG	wrapmask;	/* Hex mask Sushi uses when wrapping subufi */

   /* External app writes to these if not in use; must clear them before exiting */
   struct Task	*extsigtask;
   LONG		extsignum;
   ULONG	extsignal;

   /* Microseconds that Sushi waits between checking for Enforcer hits.
    * Default is 100000 (1/10 sec).  If you find it necessary to touch this,
    * BE REASONABLE - and PUT BACK THE DEFAULT before you exit !!!!
    * Note that Sushi itself is Signalled for all other debugging output
    * any time a CR or LF is placed in buffer.  Sushi will only signal
    * the external task if there is actually something in the buffer.
    */
   ULONG	sumicros;

   /* private stuff here - subject to moving since any additional
    * public things will be added right here.
    */
   };


/* Sushi's public named port - contains pointer to AppStruct above */
struct WTP {
   struct MsgPort wtPort;
   struct AppStruct *appstruct;
   /* private stuff here - subject to moving since any additional
    * public things will be added right here.
    */
   };

struct  WTP	  *wtp = NULL;
struct  AppStruct *aps = NULL;
LONG	sushisignum  = -1;
BOOL	GotSushi     = FALSE;


void main(int argc, char **argv)
    {
    struct  Task 	*mytask;
    UBYTE   		*buf;	
    ULONG   sushisignal, signals, bufi, bufli; 
    BOOL    Done;

    if(((argc)&&(argc<MINARGS))||(argv[argc-1][0]=='?'))
	{
	printf("%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}

    bufli = 0;		/* this application's last index into sushi buffer */
    mytask = FindTask(NULL);
    GotSushi = FALSE;

    Forbid();
    if (wtp = FindPort(sushiname))
    	{
	aps = wtp->appstruct;
    	if(! aps->extsigtask)	/* You MUST not install if someone else has */
	    {
    	    if((sushisignum = AllocSignal(-1)) != -1)
	    	{
	    	sushisignal 	= 1L << sushisignum;

	    	aps->extsigtask	= mytask;
	    	aps->extsignum 	= sushisignum;
	    	aps->extsignal	= sushisignal;
		buf 		= aps->subuf;
	    	GotSushi 	= TRUE;
	    	}
	    }
    	}
    Permit();

    if(!GotSushi)
	bye("Can't allocate sushi - not found or in use\n", RETURN_FAIL);

    Done = FALSE;

    while(!Done)
    	{
    	signals = Wait( SIGBREAKF_CTRL_C | sushisignal );

    	/* Make sure you turn off your compiler's CTRL-C handling
     	 * if you use CTRL-C as your exit signal.
     	 */
    	if( signals & SIGBREAKF_CTRL_C)
	    {
	    Done = TRUE;
	    }

    	if(signals & sushisignal)
	    {
	    /* There's some new info in the sushi buffer.
	     * Here we will just print it out.
	     * You could instead copy it elsewhere for fancy display, etc.
	     */
	    bufi = aps->subufi;	/* grab where sushi has filled to */
	    
	    if(bufi > bufli)		/* buffer has not wrapped since last */
		{
		Write(Output(),&buf[bufli],bufi - bufli);
		}
	    else if(bufli > bufi)	/* buffer has wrapped since last */		
		{
		/* Write end of buffer info, then new info at start of buffer */
		Write(Output(),&buf[bufli],aps->subufsize - bufli);
		Write(Output(),buf,bufi);
		}
	    /* Update variable that saves where YOU are up to */
	    bufli = bufi;
	    }

#if 0
	/* FYI - Other things you can tell Sushi to do */

	/* Tell Sushi to write its buffer as file t:sushi.out.
	 * Sushi writes its buffer in chronological order -
	 * i.e., if the buffer has wrapped, it writes older end of buffer,
	 * then writes data from start of buffer to current position.
	 */
	Signal(aps->sutask, SIGBREAKF_CTRL_F);

	/* Tell Sushi to empty (clear) its buffer (i.e. reset its indexes).
	 * If you do this, you should probably reset your bufli to 0.
	 */
	Signal(aps->sutask, SIGBREAKF_CTRL_E);

	/* Tell Sushi to exit if it can.
	 * It can not exit if there is an aps->extsigtask.
	 */
	Signal(aps->sutask, SIGBREAKF_CTRL_C);

#endif

    	}
    bye("",RETURN_OK);	/* Will do Sushi and other cleanup */
    }


void bye(UBYTE *s, int e)
    {
    cleanup();
    exit(e);
    }


void cleanup()
    {
    /* Required cleanup for external sushi program before exiting ! */
    if(GotSushi && (aps != NULL))
	{
	/* Required Sushi cleanup before you exit !!!! */
	Forbid();
	aps->extsignum  = 0L;
	aps->extsignal  = 0L;
	aps->extsigtask = NULL;
	Permit();
	}
    if(sushisignum != -1)	FreeSignal(sushisignum);
    }

