;/* joymouse.c - Execute me to compile me with SAS C 6.x
SC joymouse.c data=near nominc strmer streq nostkchk saveds ign=73
Slink FROM LIB:c.o,joymouse.o TO joymouse LIBRARY LIB:SC.lib,LIB:Amiga.lib ND
quit
*/

/* 39.2 - fix so qualifier always has IEQUALIFIER_RELATIVEMOUSE
 * 40.1 - add CREATEKEYS option, SAS 6-ify
 * 40.2 - x/y factor now default 4 and are command line options have effect 
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/ports.h>

#include <libraries/lowlevel.h>

#include <devices/inputevent.h>
#include <devices/input.h>
#include <devices/gameport.h>
#include <devices/timer.h>

#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/alib_protos.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "joymouse.h"

void sendrawmouse(struct IOStdReq *inreq, struct InputEvent *ie,
		  struct timerequest *treq,
		  WORD mx, WORD my, UWORD code, UWORD qual);


#ifdef __SASC
void __regargs __chkabort(void) {}      /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}                  /* Disable LATTICE CTRL-C checking */
#endif
#endif

#define NEED_SAFEDOIO

#define MINARGS 1

UBYTE *vers = "\0$VER: joymouse 40.2";
UBYTE *Copyright = 
  "joymouse v40.2\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage =
"Usage: joymouse [XMOVE=n YMOVE=n DELAY=nframes ACCEL CREATEKEYS] OR [OFF]\n"
"Defaults: XMOVE=4 YMOVE=4 DELAY=1 and no CREATEKEYS or ACCEL\n"
"Re CREATEKEYS see lowlevel.doc SystemControl() and libraries/lowlevel.h\n"
"For CD32 game controller mouse emulation. Requires lowlevel.library\n";

/**********    debug macros     ***********/
#define MYDEBUG  0
void kprintf(UBYTE *fmt,...);
void dprintf(UBYTE *fmt,...);
#define DEBTIME 0
#define bug kprintf
#if MYDEBUG
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME);
#define D2(x) ;
#else
#define D(x) ;
#define D2(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/

/* purposely short to prevent extra frame wait (should be 16667) */
#define MICROS_PER_FRAME 16000

struct Library 	*GfxBase = NULL;
struct Library 	*LowLevelBase = NULL;
struct Task 	*joymousetask = NULL;
struct Task 	*maintask = NULL;
struct MsgPort 	*joymouseport = NULL;
UBYTE *oldmainname = NULL;

#define DEF_XFACTOR 4
#define DEF_YFACTOR 4
#define DEF_DELAY 1
WORD  xfactor=DEF_XFACTOR, yfactor=DEF_YFACTOR, afactor=0;
ULONG delay=DEF_DELAY, micros, acounter=0;

BOOL FromWb, Accel;

/* :ts=4
*
*	Based On joy.c
*
*	by William A. Ware							B207
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright (C) 1991, Silent Software, Incorporated.						*
*   All Rights Reserved.													*
****************************************************************************/

char timername[] 	= "timer.device";
char inputname[]    	= "input.device";
char JoyMousePortName[] = JOYMOUSEPORTNAME;
char maintaskname[]	= "Joymouse_Process";


/****** SafeWaitIO *************************************
*
*   NAME
*	SafeWaitIO -- Safer I/O completion mechanism.
*
*   SYNOPSIS
*	error = SafeWaitIO (ior);
*	 D0		     A0
*
*	LONG			error;
*	struct IORequest	*ior;
*
*   FUNCTION
*	This routine is identical to WaitIO(), except that it prevents
*	additional calls to WaitIO() on IORequests that have already
*	completed.  To elaborate, the sequence:
*
*		SendIO (ior);
*		WaitIO (ior);
*		WaitIO (ior);	*  WaitIO() on already-completed IOR.  *
*
*	is unsafe, since WaitIO() unconditionally calls Remove() on the
*	IORequest.  SafeWaitIO() detects and avoids this condition.
*
*	SafeWaitIO() and WaitIO() CANNOT be intermixed if you wish to reap
*	the benefits of SafeWaitIO().
*
*   INPUTS
*	ior	- pointer to an IORequest structure
*
*   RESULT
*	error	- copy of the IORequest.io_Error field
*
*   EXAMPLE
*	SendIO (ior);
*	SafeWaitIO (ior);
*	SafeWaitIO (ior);
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	SafeDoIO()
*
*****************************************************************************
*/
LONG __asm SafeWaitIO(	register __a0 struct IORequest *ioreq)
{
	register LONG	retval;

	Forbid();
	if (ioreq->io_Message.mn_Node.ln_Succ) {
		/*
		 * The I/O packet has not been checked back in.  WaitIO() on
		 * it and flag it as well and truly completed.
		 */
		retval = WaitIO(ioreq);
		ioreq->io_Message.mn_Node.ln_Succ = NULL;
	} 
	else {
		/*
		 * The I/O packet is already in; use existing error code.
		 */
		retval = ioreq->io_Error;
	}
	Permit();
	return (retval);
}



/****** FreeIORequest **********************************
*
*   NAME
*	FreeIORequest -- Free an I/O session started with AllocIORequest().
*
*   SYNOPSIS
*	FreeIORequest (ior);
*			A0
*
*	struct IORequest	*ior;
*
*   FUNCTION
*	This routine frees all resources procured with AllocIORequest().
*
*	Any outstanding IO is aborted and allowed to complete.  The device
*	associated with the IORequest is closed, the associated replyport is
*	deleted, and the IORequest itself is then freed.
*
*   INPUTS
*	ior	- pointer to IORequest structure
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	AllocIORequest()
*
*****************************************************************************
*/
VOID __asm FreeIORequest( register __a0 void *ioreq  )
{
	register struct IORequest *r;

	if (!(r = (struct IORequest *)ioreq)) return;
	if (r->io_Device)
	{
		if (r->io_Message.mn_Node.ln_Succ) AbortIO(r);
		SafeWaitIO(r);
		CloseDevice( r );
	}
	if (r->io_Message.mn_ReplyPort)
		DeletePort( r->io_Message.mn_ReplyPort );
	DeleteExtIO( r );
}

/****** AllocIORequest *********************************
*
*   NAME
*	AllocIORequest -- Start a session with an Exec device.
*
*   SYNOPSIS
*	ior = AllocIORequest (devname, unit, flags, size);
*	D0			A0      D0     D1    D2
*
*	struct IORequest	*ior;
*	char			*devname;
*	ULONG			unit, flags, size;
*
*   FUNCTION
*	This routine does all the mish-mash necessary to start a session
*	with an Exec-style I/O device.
*
*	A nameless replyport is created, an IORequest structure is created,
*	the named device is opened with the specified unit number and
*	flags, and a pointer to the IORequest is returned.  If any of these
*	operations fail, NULL is returned and nothing is allocated.
*
*   INPUTS
*	devname	- pointer to name of device
*	unit	- device unit number, as used by OpenDevice()
*	flags	- device flags, as used by OpenDevice()
*	size	- size of desired IORequest structure, in bytes
*
*   RESULT
*	ior	- pointer to allocated IORequest structure, or NULL
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	FreeIORequest()
*
*****************************************************************************
*/
VOID * __asm AllocIORequest(	register __a0 char *devname, 
				register __d0 ULONG unitNumber, 
				register __d1 ULONG flags, 
				register __d2 ULONG size)
{
	register struct IORequest *r;
	register struct MsgPort *m;
	
	if (!(m = (struct MsgPort *)CreatePort(0L,0L)))	return(NULL);
	if (!(r = (struct IORequest *)CreateExtIO( m,size)))
	{
		DeletePort(m);
		return(NULL);
	}
	if (OpenDevice( devname, unitNumber, r, flags )) 
	{
		r->io_Device = NULL;
		FreeIORequest( r );
		return(NULL);
	}
	return( (VOID *)r );
}


#ifdef NEED_SAFEDOIO
/****** SafeDoIO ***************************************
*
*   NAME
*	SafeDoIO -- Safer I/O operation mechanism.
*
*   SYNOPSIS
*	error = SafeDoIO (ior);
*	 D0		   A0
*
*	LONG			error;
*	struct IORequest	*ior;
*
*   FUNCTION
*	Identical in operation to DoIO().  Exists for the same reason as
*	SafeWaitIO().  In particular:
*
*		DoIO (ior);
*		WaitIO (ior);
*
*	is unsafe.
*	
*	SafeDoIO() and DoIO() cannot be intermixed.  See the SafeWaitIO()
*	docs for more details.
*
*   INPUTS
*	ior	- pointer to an IORequest structure
*
*   RESULT
*	error	- copy of IORequest.io_Error field
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	SafeWaitIO()
*
*****************************************************************************
*/
LONG __asm SafeDoIO (  register __a0 struct IORequest *ioreq )

{
	register LONG	retval;

	retval = DoIO (ioreq);
	ioreq->io_Message.mn_Node.ln_Succ = NULL;	/*  It's back.  */
	return (retval);
}
#endif



VOID __saveds JoyMouse( void )
{
    	WORD mx, my;
    	UWORD code, qual;
    	ULONG U, D, R, L, lmb=0, rmb=0, dirs=0, lastdirs, lastlmb, lastrmb;
    	ULONG state, ctype;
        struct timerequest              *timer = NULL;
	struct MsgPort 			*inport;
	struct IOStdReq			*in_req = NULL;
	struct InputEvent 		event;
	UWORD	notrep = 1;		/* We have not signaled calling
					 * task yet 
					 */
	struct Message			*msg = NULL;
#if MYDEBUG
	UBYTE ctr=0;
#endif

	/*--------- Startup Code ----------------*/

	D(bug("JoyMouse task entry\n"));
	
	if (!(inport = (struct MsgPort *)CreatePort(JoyMousePortName,0L)))
		goto xit;

	if (!(in_req = AllocIORequest( inputname, 0, 0, 
		sizeof(struct IOStdReq)))) goto xit;
        if (!(timer = AllocIORequest( timername, UNIT_VBLANK, 0,
                sizeof( *timer )))) goto xit;

	notrep = 0;	/* It is now false we have replied */

	while(!(msg = GetMsg( inport )))
	    {
    	    timer->tr_node.io_Command = TR_ADDREQUEST;
    	    timer->tr_time.tv_secs  = 0;
    	    timer->tr_time.tv_micro = micros;
    	    DoIO((struct IOStdReq *)timer);

	    state = ReadJoyPort(1);
#if MYDEBUG
	    if(!(++ctr % 32)) D2(bug("state=%08lx\n",state));
#endif
	    ctype = state & JP_TYPE_MASK;
	    if((ctype != JP_TYPE_GAMECTLR )&&(ctype != JP_TYPE_JOYSTK ))
		continue;

            lastlmb = lmb;
            lastrmb = rmb;
	    lastdirs = dirs;

	    dirs = state & JP_DIRECTION_MASK;
	    lmb = state & JPF_BUTTON_RED;
	    rmb = state & JPF_BUTTON_BLUE;

	    if((Accel)&&((!dirs)||(dirs != lastdirs)))
		{
		acounter = 0;  /* reset acceleration variables */
		afactor = 0;
		}
	
	    if((!dirs)&&(lmb == lastlmb)&&(rmb == lastrmb))	continue;

	    if(Accel)
		{
	    	if (acounter<64) 	acounter++;	/* will trigger acceleration */
		if (acounter == 12)  	afactor = 1;
		else if(acounter == 18) afactor = 2;
		else if(acounter == 24) afactor = 3;
		else if(acounter == 30) afactor = 4;
		}

            mx = my = 0;
            code = qual = 0;
	    if(U = dirs & JPF_JOY_UP)		my -= (yfactor + afactor);
;	    if(D = dirs & JPF_JOY_DOWN)		my += (yfactor + afactor);
	    if(R = dirs & JPF_JOY_RIGHT)	mx += (xfactor + afactor);
	    if(L = dirs & JPF_JOY_LEFT)		mx -= (xfactor + afactor);

            qual =  (lmb)  ? IEQUALIFIER_LEFTBUTTON : 0;
	    qual |= (rmb)  ? IEQUALIFIER_RBUTTON : 0;

	    qual |= IEQUALIFIER_RELATIVEMOUSE;

            if(lmb != lastlmb)
               	{
               	code = lmb ? IECODE_LBUTTON :
                             IECODE_LBUTTON|IECODE_UP_PREFIX;
		D(bug(" Lbutton=%s ",lmb & IECODE_UP_PREFIX ? "UP" : "DOWN"));
               	mx = 0;
               	my = 0;
               	}
            else if(rmb != lastrmb)
               	{
               	code = rmb ? IECODE_RBUTTON :
                             IECODE_RBUTTON|IECODE_UP_PREFIX;
		D(bug(" Rbutton=%s ",lmb & IECODE_UP_PREFIX ? "UP" : "DOWN"));
               	mx = 0;
               	my = 0;
               	}
            else
               	{
               	code = IECODE_NOBUTTON;
		D(bug("."));
		}
	    sendrawmouse(in_req, &event, timer, mx, my, code, qual);
	    }
xit:
	Forbid();
	if (timer)  FreeIORequest(timer);
	if (in_req) FreeIORequest(in_req);
	if (msg)  	ReplyMsg(msg);
	if (inport) 
	    {
	    while( msg = GetMsg(inport) ) ReplyMsg(msg);
	    DeletePort(inport);
	    }

	Permit();
}


void sendrawmouse(struct IOStdReq *inreq, struct InputEvent *ie,
		  struct timerequest *treq,
		  WORD mx, WORD my, UWORD code, UWORD qual)
    {
    treq->tr_node.io_Command = TR_GETSYSTIME;
    DoIO((struct IOStdReq *)treq);

    inreq->io_Message.mn_Node.ln_Type = NT_MESSAGE;
    inreq->io_Command = IND_WRITEEVENT;
    inreq->io_Length = sizeof(struct InputEvent);
    inreq->io_Data   = (APTR)ie;
    inreq->io_Flags = 0;
  
    ie->ie_Class = IECLASS_RAWMOUSE;
    ie->ie_SubClass = NULL;
    ie->ie_Code  = code;
    ie->ie_Qualifier  = qual;
    ie->ie_TimeStamp.tv_secs  = treq->tr_time.tv_secs;
    ie->ie_TimeStamp.tv_micro = treq->tr_time.tv_micro;
    ie->ie_X     = mx;
    ie->ie_Y     = my;
    ie->ie_NextEvent = NULL;

    inreq->io_Flags = IOF_QUICK;
    SafeDoIO((struct IOStdReq *)inreq);
    }




VOID MemSet( UBYTE *where, UBYTE value, ULONG bytes )
    {
    ULONG k;

    for(k=0; k<bytes; k++)	where[k] = value;
    }

/****** InstallJoyMouse ********************************
*
*   NAME
*	InstallJoyMouse -- Install joystick event rerouting task.
*
*   SYNOPSIS
*	success = InstallJoyMouse ();
*	  D0
*
*	LONG	success;
*
*   FUNCTION
*	The CDTV remote control has this little harmless-looking button
*	labelled "JOY/MOUSE."  This enables the IR remote to appear as
*	either a mouse in gameport 0 or as a joystick in gameport 1.
*
*	This routine installs a task that sniffs gameport 1 and sends any
*	events it finds there down the input food chain.  The events are
*	identical to ordinary mouse events.  This enables applications
*	expecting a mouse to work no matter what mode the IR remote is in.
*
*	This call does NOT nest.
*
*   INPUTS
*
*   RESULT
*	success	- non-zero if task successfully installed or already there;
*		  zero otherwise
*
*   EXAMPLE
*
*   NOTES
*	The only difference between joystick and mouse events is very
*	subtle; the ie_SubCode field of the InputEvent is set to the
*	gameport unit number from which the event originated.
*
*   BUGS
*
*   SEE ALSO
*	RemoveJoyMouse()
*
*****************************************************************************
*/
BOOL __asm InstallJoyMouse( void )
{
BOOL ans = TRUE;

	D(bug("InstallJoyMouse\n"));
	Forbid();
	if (FindPort( JoyMousePortName )) 
		{ Permit(); goto xit; }
	Permit();

	ans = (joymousetask =
		CreateTask( JoyMousePortName, 2,(APTR)JoyMouse, 2000 )) ?
		TRUE : FALSE;
	D(bug("InstallJoyMouse task=$%lx\n",joymousetask));
xit:
	return(ans);
}

/****** RemoveJoyMouse *********************************
*
*   NAME
*	RemoveJoyMouse -- Remove joystick event rerouter.
*
*   SYNOPSIS
*	RemoveJoyMouse ();
*
*   FUNCTION
*	This routine kills off the joystick rerouting task started with
*	InstallJoyMouse().  All resources it allocated are freed.  A
*	corresponding call to InstallJoyMouse() need not have been made.
*
*	This call does NOT nest; it happens immediately.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	InstallJoyMouse()
*
*****************************************************************************
*/
VOID __asm RemoveJoyMouse( void )

{
	struct Message msg;
	struct MsgPort *port,*rp;

	D(bug("RemoveJoyMouse\n"));
	
	Forbid();
	if (port = FindPort( JoyMousePortName )) 
	{
		MemSet( (UBYTE *)&msg, 0, sizeof(struct Message));
		if (rp = (struct MsgPort *)CreatePort( 0L,0L )) {
			msg.mn_ReplyPort = rp;
			msg.mn_Node.ln_Type = NT_MESSAGE;
			PutMsg( port,&msg );
			Permit(); 
			WaitPort( rp );
			DeletePort(rp);
		}
		else Permit();
	}
	else Permit();
}



void cleanup( void )
    {
    if(LowLevelBase)	CloseLibrary(LowLevelBase);
    if(GfxBase)		CloseLibrary(GfxBase);
    if((maintask)&&(oldmainname)) maintask->tc_Node.ln_Name = oldmainname;
    }

void bye(UBYTE *s, int e)
    {
    if(!FromWb)	if(*s)	printf("%s\n",s);
    cleanup();
    exit(e);	
    }


void main(int argc, char **argv)
    {
    struct Task *task;
    ULONG state, tag[4];
    BOOL createkeys, installed;
    int k;

    FromWb = argc ? FALSE : TRUE;
    createkeys=installed=FALSE;


    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	exit(RETURN_OK);
	}

    if((argc==2)&&(!(stricmp(argv[1],"OFF"))))
	{
	Forbid();
	if(task = FindTask(maintaskname))	Signal(task,SIGBREAKF_CTRL_C);
	Permit();
	bye("",RETURN_OK);
	}
    else if(task = FindTask(maintaskname))
	{
	bye("joymouse already running",RETURN_WARN);
	}

    for(k=1; k<argc; k++)
        {
	if(!(strnicmp(argv[k],"XMOVE=",6)))		xfactor=atoi(&argv[k][6]);
	else if(!(strnicmp(argv[k],"YMOVE=",6)))	yfactor=atoi(&argv[k][6]);
	else if(!(strnicmp(argv[k],"DELAY=",6)))	delay=atoi(&argv[k][6]);
	else if(!(strnicmp(argv[k],"CREATEKEYS",10)))	createkeys=TRUE;
	else if(!(strnicmp(argv[k],"ACCELERATE",10)))	Accel=TRUE;
	else if(!(strnicmp(argv[k],"ACCEL",5)))		Accel=TRUE;
	else printf("joymouse: unknown option %s\n",argv[k]);
	}
    if(!xfactor) xfactor=DEF_XFACTOR;
    if(!yfactor) yfactor=DEF_YFACTOR;
    if(!delay)	 delay=DEF_DELAY;

    micros = delay * MICROS_PER_FRAME;

    D(bug("xmove=%ld ymove=%ld delay=%ld micros=%ld createkeys=%ld\n",
		xfactor,yfactor,delay,micros,createkeys));

    if(!(LowLevelBase = OpenLibrary("lowlevel.library",40)))
	{
	bye("Can't open lowlevel.library v40+",RETURN_WARN);
	}	
    if(!(GfxBase = OpenLibrary("graphics.library",0)))
	{
	bye("Can't open graphics.library",RETURN_WARN);
	}	

    state = ReadJoyPort(1);

    Forbid();
    maintask = FindTask(NULL);
    oldmainname = maintask->tc_Node.ln_Name;
    maintask->tc_Node.ln_Name = maintaskname;
    Permit();


    installed=InstallJoyMouse();
    if(createkeys)
	{
    	tag[0] = SCON_AddCreateKeys;
    	tag[1] = 1;
	tag[2] = TAG_DONE;
	SystemControlA((struct TagItem *)tag);
	}

    Wait(SIGBREAKF_CTRL_C);

    if(createkeys)
	{
    	tag[0] = SCON_RemCreateKeys;
    	tag[1] = 1;
	tag[2] = TAG_DONE;
	SystemControlA((struct TagItem *)tag);
	}

    if(installed)  RemoveJoyMouse();


    bye("",RETURN_OK);
    }
