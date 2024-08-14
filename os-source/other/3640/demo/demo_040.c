/*
 * $Id: demo_040.c,v 1.5 91/08/20 17:02:59 mks Exp $
 *
 * $Log:	demo_040.c,v $
 * Revision 1.5  91/08/20  17:02:59  mks
 * Changed window title to mark it as Public Domain
 * 
 * Revision 1.4  91/07/25  07:52:45  mks
 * Fixed spelling errors
 *
 * Revision 1.3  91/07/23  22:00:37  mks
 * Cleaned up documentation and comments.
 *
 * Revision 1.2  91/07/23  21:58:01  mks
 * Added autodoc entry for release
 *
 * Revision 1.1  91/07/23  21:15:01  mks
 * Initial revision
 *
 */

/* includes */
#include <intuition/intuition.h>
#include <dos/dosextens.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <devices/timer.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/locale_protos.h>
#include <clib/timer_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/timer_pragmas.h>

#include <string.h>

#include "CPUSpeed_rev.h"

/*
******* CPUSpeed *************************************************************
*
*   NAME
*	CPUSpeed
*
*   SYNOPSIS
*	CPUSpeed - Test and compare the speed of your CPU with A2000 & A3000
*
*   FUNCTION
*	To show the performance of your CPU as compared to the CPU in
*	stock A2000 and A3000/25 machines.
*
*	This is a simple CPU performance test program.  It tests
*	both integer and floating point performance.
*
*	The tests are run in a standard system so that running the
*	tests while other tasks are running will result in lower
*	numbers.
*
*	The Integer math test tests the performance of the CPU
*	doing integer addition, subtraction, multiplication, and
*	division along with a few load and store operations.  The
*	number displayed as the result is the number of operations
*	per second.  This is an average as divide takes longer than
*	addition.  There are an equal number of the four operations
*	in the test loop.  Over 24,576,000 operations are done in
*	the test in order to produce reasonably accurate results.
*	The operations are done on a table of numbers and thus
*	produce a rather wide range of operands.
*
*	The Floating point math test tests the performance of the
*	FPU/CPU combination in doing floating point math.  The loop
*	contains basic floating point instructions (no
*	transcendental instructions) and does over 20,736,000
*	instructions (including a few load and store with
*	rounding).  There are also 5,760,000 integer load/store
*	instructions executed in an attempt to make the code less
*	of a "fixed" test.
*
*	Both of the test loops operate on a table of source values
*	that is 4096-bytes in size.  This means that the data cache
*	only helps in the burst loading of the values from memory.
*	The code is also an unrolled loop as to break the cache as
*	much as possible.
*
*	The values given on the result graph are the number of
*	operations per second.  For the floating point side, that
*	is FLOPS (floating point operations per second) and on the
*	integer side, it is IMADS (integer multiplications and
*	divisions per second).
*
*   INPUTS
*	none - Just run the program from CLI or Workbench
*
*   RESULTS
*	A visual graph of relative performance
*
*   SEE ALSO
*	None
*
*   BUGS
*	None
*
******************************************************************************
*/

/*
 * The floating point test runs the following code 128 times
 * and then loops around it 9000 times.
 *
 *	move.l		a4,a5		; Save for later...
 *	fmovem.x	(a4)+,fp0
 *	move.l		(a4)+,d0
 *	fmove.x		fp0,fp1
 *	fmove.x		fp0,fp2
 *	fmul.x		fp0,fp1
 *	fadd.x		fp2,fp2
 *	move.l		d0,(a6)		; Write to memory
 *	fmove.x		fp0,fp3
 *	fmove.x		fp0,fp4
 *	fmul.x		fp0,fp3
 *	fadd.x		fp4,fp4
 *	move.l		d0,(a6)		; Write to memory
 *	fmove.x		fp0,fp1
 *	fmove.x		fp0,fp2
 *	fmul.x		fp0,fp1
 *	fadd.x		fp2,fp2
 *	move.l		d0,(a6)		; Write to memory
 *	fmove.x		fp0,fp3
 *	fmove.x		fp0,fp4
 *	fmul.x		fp0,fp3
 *	fadd.x		fp4,fp4
 *	fmovem.x	fp0,(a5)	; Write to memory
 *
 * This makes for 9000 * 128 * 18 floating point instructions
 * and 9000 * 128 * 5 integer memory/move instructions.
 */
void __asm F_SpeedTest(void);

#define	F_NUM_OPS	20736000

/*
 * Results on other machines
 */
#define	A2000_FTEST	0
#define	A3000_FTEST	616000

/*
 * The integer test runs the following code 128 times
 * and then loops around it 6000 times.
 *
 *	move.l		(a4)+,d0
 *	move.l		(a4)+,d1
 *	move.l		(a4)+,d2
 *	move.l		(a4)+,d3
 *	divu.w		#123,d0
 *	add.l		d2,d2
 *	mulu.w		d1,d0
 *	divu.w		#456,d3
 *	add.l		d2,d2
 *	mulu.w		d1,d3
 *	move.l		d0,(a6)
 *	divu.w		#123,d0
 *	add.l		d2,d2
 *	mulu.w		d1,d0
 *	divu.w		#456,d3
 *	add.l		d2,d2
 *	mulu.w		d1,d3
 *	move.l		d0,(a6)
 *	divu.w		#123,d0
 *	add.l		d2,d2
 *	mulu.w		d1,d0
 *	divu.w		#456,d3
 *	add.l		d2,d2
 *	mulu.w		d1,d3
 *	move.l		d0,(a6)
 *	divu.w		#123,d0
 *	add.l		d2,d2
 *	mulu.w		d1,d0
 *	divu.w		#456,d3
 *	add.l		d2,d2
 *	mulu.w		d1,d3
 *	move.l		d0,(a6)
 *
 * This makes for 6000 * 128 * 8 integer divisions
 *                6000 * 128 * 8 integer multiplications
 *                6000 * 128 * 8 integer additions
 *                6000 * 128 * 8 integer memory accesses
 *
 * or, 6000 * 128 * 32 integer operations.
 */
void __asm I_SpeedTest(void);

#define	I_NUM_OPS	24576000

/*
 * Results on other machines
 */
#define	A2000_ITEST	217000
#define	A3000_ITEST	1728000

/*
 * Some constants for the boxes...
 */
WORD	BoxPos[2]={11,180};
char	*BoxName[2]={"Integer math","Floating point math"};
char	TestString[]="Testing";
char	VerTag[]=VERSTAG;

#define	BOX_TOP		11
#define	BOX_WIDTH	146
#define	BOX_HEIGHT	105

char	win_title[]="CPUSpeed  (Public Domain)";

/* The font we will use */
struct	TextAttr	textattr = {"topaz.font",8,FS_NORMAL,FPF_ROMFONT|FPF_DESIGNED};

/*
 * We define this here as we will need it.  We can't have it here as
 * the first routine *must* be demo_040
 */
ULONG MaxDivide(ULONG x,ULONG y,ULONG digits);
void DrawTesting(struct Window *window,struct Library *GfxBase,char *Testing,short num,short colour);
void DrawResult(struct RastPort *rp,struct Library *GfxBase,short top,short left,ULONG A2000, ULONG A3000, ULONG result);

/*
 * This is the main function.  We let the system just call
 * us without C startup code...
 */
long demo_040(void)
{
struct	ExecBase	*SysBase = (*((struct ExecBase **) 4));
struct	Process		*process;
struct	WBStartup	*wbMsg = NULL;
	LONG		failureLevel = RETURN_FAIL;
	LONG		failureCode = 0;

	process = (struct Process *)SysBase->ThisTask;
	if (!(process->pr_CLI))
	{
		WaitPort(&process->pr_MsgPort);
		wbMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
	}

	if (SysBase->LibNode.lib_Version < 37)
	{
		failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
	}
	else
	{
	struct	Library		*IntuitionBase;
	struct	Library		*GfxBase;
	struct	Library		*GadToolsBase;
	struct 	timerequest	*tr;

		IntuitionBase=OpenLibrary("intuition.library",37);
		GfxBase=OpenLibrary("graphics.library",37);
		GadToolsBase=OpenLibrary("gadtools.library",37);
		tr=CreateIORequest(&process->pr_MsgPort,sizeof(struct timerequest));

		if (IntuitionBase && GfxBase && GadToolsBase && tr)
		{
			if (!OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)tr,NULL))
			{
			struct	Library		*TimerBase=tr->tr_node.io_Device;
			struct	Screen		*screen;
			struct	Window		*window=NULL;
			struct	TextFont	*font;

				font=OpenFont(&textattr);

				if (font) if (screen=LockPubScreen(NULL))
				{
				struct TagItem openwin[] =
					{
					{WA_PubScreen,0},
					{WA_Title, 0},
					{WA_Flags, WFLG_ACTIVATE
							| WFLG_SMART_REFRESH
							| WFLG_CLOSEGADGET
							| WFLG_DRAGBAR
							| WFLG_DEPTHGADGET
							| WFLG_NOCAREREFRESH},
					{WA_IDCMP, IDCMP_CLOSEWINDOW},
					{WA_InnerWidth,339},
					{WA_InnerHeight,BOX_HEIGHT+BOX_TOP+12},
					{TAG_END,0}
					};

					openwin[0].ti_Data=(ULONG)screen;
					openwin[1].ti_Data=(ULONG)win_title;
					window=OpenWindowTagList(NULL,openwin);
					UnlockPubScreen(NULL,screen);
				}

				if (window)	/* Did we open? */
				{
				struct	RastPort	*rp=window->RPort;
					void		*vi;

					SetFont(rp,font);
					SetAPen(rp,1);
					SetBPen(rp,0);
					SetDrMd(rp,JAM2);

					if (vi=GetVisualInfoA(window->WScreen,NULL))
					{
					struct	timeval		StartTime;
						ULONG		FTest;	/* Result of the Floating point test */
						ULONG		ITest;	/* Result of the integer test */
						short		loop;

						for (loop=0;loop<2;loop++)
						{
						struct	TagItem	drawbox[] = { {GT_VisualInfo,0},{GTBB_Recessed,TRUE},{TAG_END,0} };

							drawbox[0].ti_Data=(ULONG)vi;
							DrawBevelBoxA(rp,window->BorderLeft + BoxPos[loop],window->BorderTop + BOX_TOP,BOX_WIDTH,BOX_HEIGHT,drawbox);

							Move(rp,window->BorderLeft+BoxPos[loop],window->BorderTop+BOX_TOP-3);
							Text(rp,BoxName[loop],strlen(BoxName[loop]));

							Move(rp,window->BorderLeft+BoxPos[loop]+4,window->BorderTop+BOX_TOP+BOX_HEIGHT+8);
							Text(rp,"A2000",5);

							Move(rp,window->BorderLeft+BoxPos[loop]+53,window->BorderTop+BOX_TOP+BOX_HEIGHT+8);
							Text(rp,"A3000",5);

							Move(rp,window->BorderLeft+BoxPos[loop]+118,window->BorderTop+BOX_TOP+BOX_HEIGHT+8);
							Text(rp,"*",1);
						}

						/*******************************/

						DrawTesting(window,GfxBase,TestString,0,1);

						/* Test integer */
						tr->tr_node.io_Command=TR_GETSYSTIME;
						tr->tr_node.io_Flags=IOF_QUICK;
						DoIO((struct IORequest *)tr);
						StartTime=tr->tr_time;

						I_SpeedTest();

						tr->tr_node.io_Command=TR_GETSYSTIME;
						tr->tr_node.io_Flags=IOF_QUICK;
						DoIO((struct IORequest *)tr);
						SubTime(&(tr->tr_time),&StartTime);
						ITest=(tr->tr_time.tv_secs * 1000000) + tr->tr_time.tv_micro;
						ITest=MaxDivide(I_NUM_OPS,ITest,6);

						DrawResult(rp,GfxBase,window->BorderTop+BOX_TOP,window->BorderLeft+BoxPos[0],A2000_ITEST,A3000_ITEST,ITest);

						/*******************************/

						DrawTesting(window,GfxBase,TestString,1,1);

						if (SysBase->AttnFlags & AFF_68881)
						{
							/* Test floating point */
							tr->tr_node.io_Command=TR_GETSYSTIME;
							tr->tr_node.io_Flags=IOF_QUICK;
							DoIO((struct IORequest *)tr);
							StartTime=tr->tr_time;

							F_SpeedTest();

							tr->tr_node.io_Command=TR_GETSYSTIME;
							tr->tr_node.io_Flags=IOF_QUICK;
							DoIO((struct IORequest *)tr);
							SubTime(&(tr->tr_time),&StartTime);
							FTest=(tr->tr_time.tv_secs * 1000000) + tr->tr_time.tv_micro;
							FTest=MaxDivide(F_NUM_OPS,FTest,6);
						}
						else FTest=0;

						DrawResult(rp,GfxBase,window->BorderTop+BOX_TOP,window->BorderLeft+BoxPos[1],A2000_FTEST,A3000_FTEST,FTest);

						/*******************************/

						FreeVisualInfo(vi);

						/* Wait for the one message I can get */
						WaitPort(window->UserPort);
					}
					CloseWindow(window);
				}

				if (font) CloseFont(font);

				CloseDevice(tr);
			}
		}

		/* DeleteIORequest works with NULL */
		DeleteIORequest(tr);

		/* CloseLibrary works with NULL */
		CloseLibrary(GadToolsBase);
		CloseLibrary(GfxBase);
		CloseLibrary(IntuitionBase);
	}

	/* Now, exit correctly... */
	if (wbMsg)
	{
		Forbid();
		ReplyMsg(wbMsg);
	}
	process->pr_Result2 = failureCode;  /* can't use SetIoErr() cause DOS ain't open! */
	return(failureLevel);
}

/*
 * It knows that the Y value is fixed point by n-digits...
 */
ULONG MaxDivide(ULONG x,ULONG y,ULONG digits)
{
ULONG	result;
ULONG	num=0;	/* Number of 10 units adjusted for so far */

	while ((x<399999999) && (num<digits))
	{
		x*=10;
		num++;
	}

	while (num<digits)
	{
		num++;
		if (num==digits) y+=5;	/* Last one, so round it... */
		y=y/10;
	}

	if (y) result=x/y;
	else result=-1;	/* MAX INT if y=0 */

	return(result);
}

void DrawTesting(struct Window *window,struct Library *GfxBase,char *Testing,short num,short colour)
{
	SetAPen(window->RPort,colour);
	Move(window->RPort,window->BorderLeft+BoxPos[num]+BOX_WIDTH/2-(4*strlen(Testing)),window->BorderTop+BOX_TOP+BOX_HEIGHT/2);
	Text(window->RPort,Testing,strlen(Testing));
}

void DrawResult(struct RastPort *rp,struct Library *GfxBase,short top,short left,ULONG A2000, ULONG A3000, ULONG result)
{
ULONG	res[3];
ULONG	draw;
ULONG	max;
ULONG	tmp;
short	loop;

	max=A2000;
	if (A3000>max) max=A3000;
	if (result>max) max=result;

	res[0]=A2000;
	res[1]=A3000;
	res[2]=result;

	SetAPen(rp,0);
	RectFill(rp,left+2,top+1,left+BOX_WIDTH-3,top+BOX_HEIGHT-2);

	SetAPen(rp,3);
	for (loop=0;loop<3;loop++)
	{
		draw=((BOX_HEIGHT-5) * res[loop]) / max;

		RectFill(rp,	left+(loop*49)+3,
				top+BOX_HEIGHT-3-draw,
				left+(loop*49)+43,
				top+BOX_HEIGHT-3);

		if (tmp=res[loop])
		{
		char	num[12];
		char	unit;

			unit='\0';
			if (tmp>9999)
			{
				tmp=(tmp+500)/1000;
				unit='K';
				if (tmp>9999)
				{
					tmp=(tmp+500)/1000;
					unit='M';
				}
			}

			tmp=stcul_d(num,tmp);
			num[tmp]=unit;
			num[tmp+1]='\0';

			if (draw<9) draw+=10;

			SetDrMd(rp,COMPLEMENT);
			Move(rp,left+(loop*49)+44-(8*strlen(num)),top+BOX_HEIGHT+4-draw);
			Text(rp,num,strlen(num));
			SetDrMd(rp,JAM2);
		}
		else
		{
			Move(rp,left+(loop*49)+11,top+BOX_HEIGHT-6);
			Text(rp,"n/a",3);
		}

	}
}
