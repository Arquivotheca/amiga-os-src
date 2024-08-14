/*
 *                          DiskSpeed v4.2
 *                          ScsiSpeed v4.2
 *                                by
 *                           Michael Sinz
 *
 *            Copyright (c) 1989-1992 by MKSoft Development
 *
 *			MKSoft Development
 *			163 Appledore Drive
 *			Downingtown, PA 19335
 *
 * Yes, this is yet another disk speed testing program, but with a few
 * differences.  It was designed to give the most accurate results of the
 * true disk performance in the system.  For this reason many of
 * DiskSpeed's results may look either lower or higher than current disk
 * performance tests.
 *
 ******************************************************************************
 *									      *
 *	Reading legal mush can turn your brain into guacamole!		      *
 *									      *
 *		So here is some of that legal mush:			      *
 *									      *
 * Permission is hereby granted to distribute this program's source	      *
 * executable, and documentation for non-commercial purposes, so long as the  *
 * copyright notices are not removed from the sources, executable or	      *
 * documentation.  This program may not be distributed for a profit without   *
 * the express written consent of the author Michael Sinz.		      *
 *									      *
 * This program is not in the public domain.				      *
 *									      *
 * Fred Fish is expressly granted permission to distribute this program's     *
 * source and executable as part of the "Fred Fish freely redistributable     *
 * Amiga software library."						      *
 *									      *
 * Permission is expressly granted for this program and it's source to be     *
 * distributed as part of the Amicus Amiga software disks, and the	      *
 * First Amiga User Group's Hot Mix disks.				      *
 *									      *
 ******************************************************************************
 *
 * Main code and testing sections...
 */

#include	<exec/types.h>
#include	<exec/execbase.h>
#include	<exec/memory.h>
#include	<exec/lists.h>
#include	<exec/nodes.h>
#include	<exec/devices.h>
#include	<exec/io.h>
#include	<exec/ports.h>
#include	<devices/timer.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>
#include	<devices/trackdisk.h>
#include	<intuition/intuition.h>
#include	<intuition/screens.h>
#include	<workbench/workbench.h>
#include	<workbench/startup.h>

#include	<clib/exec_protos.h>
#include	<clib/dos_protos.h>
#include	<clib/timer_protos.h>
#include	<clib/intuition_protos.h>
#include	<clib/icon_protos.h>

#include	<pragmas/exec_pragmas.h>
#include	<pragmas/dos_pragmas.h>
#include	<pragmas/timer_pragmas.h>
#include	<pragmas/intuition_pragmas.h>
#include	<pragmas/icon_pragmas.h>

#include	<string.h>
#include	<stdio.h>

#include	"renderinfo.h"
#include	"mks_list.h"
#include	"makeboxes.h"

#ifdef	SCSI_SPEED

#include	"scsispeed_rev.h"

#else	/* SCSI_SPEED */

#include	"diskspeed_rev.h"

#endif	/* SCSI_SPEED */

/*
 * First, the timer stuff...
 */
struct MyTimer
{
struct	timerequest	*tr;
struct	timeval		time;
struct	MsgPort		*port;
	BOOL		Open;
};

/*
 * The "TextLine" structure used to display text in the information
 * "window" in DiskSpeed.
 */
struct	TextLine
{
struct	Node	tl_Node;
	ULONG	tl_Size;	/* Size of the allocation */
	char	tl_Text[1];	/* For the NULL... */
};

/*
 * My global structure...
 */
struct DiskSpeed
{
struct	Window		*Window;	/* The DiskSpeed window... */
struct	RenderInfo	*ri;		/* MKSoft Render Info */

#ifdef	SCSI_SPEED

struct	IOExtTD		DiskIO;		/* Disk IO request */

#endif	/* SCSI_SPEED */

	BPTR	Output;		/* The output file handle! */
struct	Process	*Me;		/* Pointer to my process... */
struct	MyTimer	*timer;		/* Pointer to a timer structure */

#ifndef	SCSI_SPEED

struct	FileInfoBlock	*fib;	/* Pointer to a FileInfoBlock */

#endif	/* !SCSI_SPEED */

/* */
struct	MinList	TextList;	/* The list the results test is linked to... */
struct	DisplayList	*List;	/* The "List" gadget */
/* */
	ULONG	Min_Time;	/* Minimum time in seconds for a test */
	ULONG	Base_CPU;	/* Base CPU available... */
	ULONG	CPU_Total;	/* Sum of CPU availability */
	ULONG	CPU_Count;	/* Count of CPU availability */

/* Testing parameters */
	UBYTE	HighDMA;	/* Set to TRUE for high video DMA... */
	UBYTE	Test_DIR;	/* Set to test directory stuff */
	UBYTE	Test_SEEK;	/* Set to test SEEK/READ */
	UBYTE	pad;

	ULONG	Align_Types;	/* Set bits of alignment types... */
	ULONG	Mem_TYPES;	/* Set memory type flags to test... */
	ULONG	Test_Size[4];	/* The four test sizes... */

/* Now for the gadgets */
struct	Gadget	DeviceGadget;
struct	Gadget	CommentsGadget;
struct	Gadget	StartTestGadget;
struct	Gadget	SaveResultsGadget;
struct	Gadget	StopTestGadget;

struct	StringInfo	DeviceInfo;
struct	StringInfo	CommentsInfo;

struct	Border	StringBorder[4];
struct	Border	ActionBorder[4];
struct	Border	DetailBorder[2];

struct	IntuiText	DeviceText;
struct	IntuiText	CommentsText;
struct	IntuiText	StartTest;
struct	IntuiText	SaveResults;
struct	IntuiText	StopTest;

	SHORT	StringVectors[5*2*4];
	SHORT	ActionVectors[5*2*2];

/* */
	char	Device[256];	/* Device name under test... */
	char	Comments[256];	/* Comments string gadget... */
	char	Undo[256];	/* Our one UNDO buffer... */

/* */
	char	CPU_Type[6];	/* 680?0 in this string (plus NULL) */
	char	Exec_Ver[14];	/* Version of Exec */

/* */
	char	tmp1[256];	/* Some temp buffer space... */
};

#define	DEVICE_GADGET	1
#define	COMMENT_GADGET	2
#define	TEST_GADGET	3
#define	SAVE_GADGET	4
#define	STOP_GADGET	5

extern	struct	Library	*SysBase;
extern	struct	Library	*DOSBase;
extern	struct	Library	*IntuitionBase;
extern	struct	Library	*GfxBase;
extern	struct	Library	*IconBase;
	struct	Library	*LayersBase;

/* Some prototypes not given... BTW - This is mainly needed for 1.3 compatibility... ARG!!! */
void *CreateExtIO( struct MsgPort *msg, long size );
struct MsgPort *CreatePort( UBYTE *name, long pri );
void DeleteExtIO( struct IORequest *io );
void DeletePort( struct MsgPort *io );
void DeleteTask( struct Task *task );
void NewList( struct List *list );

#ifdef	SCSI_SPEED

char	BYTES_READ[]=	"Read from SCSI:";
char	COPYRIGHT[]=	"MKSoft ScsiSpeed 4.2  Copyright © 1989-92 MKSoft Development" VERSTAG;
char	RESULTS_FILE[]=	"ScsiSpeed.Results";

#else	/* SCSI_SPEED */

char	FILE_CREATE[]=	"File Create:   ";
char	FILE_OPEN[]=	"File Open:     ";
char	FILE_SCAN[]=	"Directory Scan:";
char	FILE_DELETE[]=	"File Delete:   ";
char	SEEK_READ[]=	"Seek/Read:     ";

char	BYTES_CREATE[]=	"Create file:   ";
char	BYTES_WRITE[]=	"Write to file: ";
char	BYTES_READ[]=	"Read from file:";

char	SEEK_UNITS[]=	"seeks/sec";
char	FILE_UNITS[]=	"files/sec";

char	FILE_STRING[]=	"%04lx DiskSpeed Test File ";

char	TEST_DIR[]=	" DiskSpeed Test Directory ";

char	COPYRIGHT[]=	"MKSoft DiskSpeed 4.2  Copyright © 1989-92 MKSoft Development" VERSTAG;
char	RESULTS_FILE[]=	"DiskSpeed.Results";

#endif	/* SCSI_SPEED */

char	BYTE_UNITS[]=	"bytes/sec";

char	START_TEST[]=	"Start Test";
char	SAVE_RESULTS[]=	"Save Results";
char	STOP_TEST[]=	"Stop Test";

static char fontnam[11]="topaz.font";
struct TextAttr TOPAZ80={fontnam,8,0,FPF_ROMFONT};

/*
 * This is the minimum time for test that can be extended/shorted automatically
 * This number should not be set too low otherwise the test results will be
 * inaccurate due to timer granularity.  (in seconds)
 */
#ifdef	SCSI_SPEED

#define	MIN_TEST_TIME	20

#else	/* SCSI_SPEED */

#define	MIN_TEST_TIME	8
#define	NUM_FILES	200

#endif	/* SCSI_SPEED */

/*
 * This section of code is used to test CPU availability.  Due to the nature of
 * the code, the actual test code for the task is in assembly...
 */
extern	ULONG	__far	CPU_Use_Base;
extern	ULONG	__far	CPU_State_Flag;
extern	ULONG	__far	CPU_Count_Low;
extern	ULONG	__far	CPU_Count_High;

struct Task *Init_CPU_Available(void);
void Free_CPU_Available(void);
void CPU_Calibrate(void);


/* The Wait pointer I use... */
USHORT __chip WaitPointer[36] =
{
	0x0000, 0x0000, 0x0400, 0x07C0, 0x0000, 0x07C0, 0x0100, 0x0380, 0x0000,
	0x07E0, 0x07C0, 0x1FF8, 0x1FF0, 0x3FEC, 0x3FF8, 0x7FDE, 0x3FF8, 0x7FBE,
	0x7FFC, 0xFF7F, 0x7EFC, 0xFFFF, 0x7FFC, 0xFFFF, 0x3FF8, 0x7FFE, 0x3FF8,
	0x7FFE, 0x1FF0, 0x3FFC, 0x07C0, 0x1FF8, 0x0000, 0x07E0, 0x0000, 0x0000
};

/*
 * These two defines set up and clear the WaitPointer...
 */
#define	SetWait(x)	SetPointer(x,WaitPointer,16L,16L,-6L,0L)
#define	ClearWait(x)	ClearPointer(x)

/*
 * This routine returns the amount of time in the timer...
 * The number returned is in Seconds...
 */
ULONG Read_Timer(struct MyTimer *mt)
{
struct	Library	*TimerBase=(struct Library *)(mt->tr->tr_node.io_Device);

	/* Get the current time... */
	mt->tr->tr_node.io_Command=TR_GETSYSTIME;
	mt->tr->tr_node.io_Flags=IOF_QUICK;
	DoIO((struct IORequest *)(mt->tr));

	/* Subtract last timer result and store as the timer result */
	SubTime(&(mt->tr->tr_time),&(mt->time));
	return(mt->tr->tr_time.tv_secs);
}

/*
 * Start the timer...
 */
void Start_Timer(struct MyTimer *mt)
{
	/* Get the current time... */
	mt->tr->tr_node.io_Command=TR_GETSYSTIME;
	mt->tr->tr_node.io_Flags=IOF_QUICK;
	DoIO((struct IORequest *)(mt->tr));

	/* Store current time as the timer result */
	mt->time=mt->tr->tr_time;

	/*
	 * This here is a nasty trick...  Since the timer device
	 * has a low resolution, we wait until we get to the exact
	 * cross-over from one TICK to the next.  We know that the
	 * tick value is larger than 10 so if the difference
	 * between two calls to the timer is > 10 then it must
	 * have been a TICK that just went through.  This is
	 * not "friendly" code but since we are testing the system
	 * and it is not "application" code, it is not a problem.
	 */
	while ((mt->tr->tr_time.tv_micro-mt->time.tv_micro) < 10)
	{
		/* Store current time as the timer result */
		mt->time=mt->tr->tr_time;

		/* Get the current time... */
		mt->tr->tr_node.io_Command=TR_GETSYSTIME;
		mt->tr->tr_node.io_Flags=IOF_QUICK;
		DoIO((struct IORequest *)(mt->tr));
	}

	/* Store current time as the timer result */
	mt->time=mt->tr->tr_time;
}

/*
 * Stop the timer...
 */
void Stop_Timer(struct MyTimer *mt)
{
struct	Library	*TimerBase=(struct Library *)(mt->tr->tr_node.io_Device);

	/* Get the current time... */
	mt->tr->tr_node.io_Command=TR_GETSYSTIME;
	mt->tr->tr_node.io_Flags=IOF_QUICK;
	DoIO((struct IORequest *)(mt->tr));

	/* Subtract last timer result and store as the timer result */
	SubTime(&(mt->tr->tr_time),&(mt->time));
	mt->time=mt->tr->tr_time;
}

/*
 * Free a MyTimer structure as best as possible.  Do all of the error checks
 * here since this will also be called for partial timer initializations.
 */
void Free_Timer(struct MyTimer *mt)
{
	if (mt)
	{
		if (mt->port)
		{
			if (mt->tr)
			{
				if (mt->Open)
				{
					CloseDevice((struct IORequest *)(mt->tr));
				}
				DeleteExtIO((struct IORequest *)(mt->tr));
			}
			DeletePort(mt->port);
		}
		FreeMem(mt,sizeof(struct MyTimer));
	}
}

/*
 * Initialize a MyTimer structure.  It will return NULL if it did not work.
 */
struct MyTimer *Init_Timer(void)
{
struct	MyTimer	*mt;

	if (mt=AllocMem(sizeof(struct MyTimer),MEMF_PUBLIC|MEMF_CLEAR))
	{
		mt->Open=FALSE;
		if (mt->port=CreatePort(NULL,0L))
		{
			if (mt->tr=CreateExtIO(mt->port,sizeof(struct timerequest)))
			{
				if (!OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)(mt->tr),0L))
				{
					mt->Open=TRUE;
				}
			}
		}
		if (!(mt->Open))
		{
			Free_Timer(mt);
			mt=NULL;
		}
	}
	return(mt);
}

/*
 * Now, for the routines that will pull in the lines and display them as
 * needed in the display...
 */
void AddDisplayLine(struct DiskSpeed *global,char *line)
{
	ULONG		size=strlen(line);
struct	TextLine	*tline;

	if (global->Window)
	{
		if (tline=AllocMem(size+=sizeof(struct TextLine),MEMF_PUBLIC|MEMF_CLEAR))
		{
			tline->tl_Size=size;
			tline->tl_Node.ln_Name=tline->tl_Text;
			strcpy(tline->tl_Text,line);

			AddTail((struct List *)&(global->TextList),&(tline->tl_Node));

			/* Now display it... */
			FreshList(global->Window,global->List,&(global->TextList));
		}
	}
	else if (global->Output)
	{
		Write(global->Output,line,size);
		Write(global->Output,"\n",1);
	}
}

void FreeDisplayList(struct DiskSpeed *global)
{
struct	TextLine	*tline;

	while (tline=(struct TextLine *)RemHead((struct List *)&(global->TextList)))
	{
		FreeMem(tline,tline->tl_Size);
	}

	/* Update the display */
	if (global->Window) FreshList(global->Window,global->List,NULL);
}

/*...*/

BOOL Check_Quit(struct DiskSpeed *global)
{
BOOL	worked=TRUE;
struct	IntuiMessage	*msg;

	if (global->Window)
	{
		while (msg=(struct IntuiMessage *)GetMsg(global->Window->UserPort))
		{
			if (msg->Class==CLOSEWINDOW) worked=FALSE;
			else if (msg->Class==GADGETUP) if (msg->IAddress==&(global->StopTestGadget)) worked=FALSE;
			Check_ListGadget(global->Window,msg);
			ReplyMsg((struct Message *)msg);
		}
	}

	if (SetSignal(0,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) worked=FALSE;

	if (!worked) AddDisplayLine(global,"*** Interrupted by user ***");

	return(worked);
}

/*...*/

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
		if (num==digits) y+=5;	/* Round it if last digit... */
		y=y/10;
	}

	if (y) result=x/y;
	else result=-1;	/* MAX INT if y=0 */

	return(result);
}

/*
 * Build a string and add it to the display of results.
 * This routine knows how to take the timer results and the CPU
 * results and format them into a string with the given header
 * and the given unit of measure.
 */
VOID Display_Result(struct DiskSpeed *global,char *Header,ULONG number,char *Units)
{
char	*p=global->tmp1;
char	format[48];
ULONG	clicks;	/* To figure out the number of clicks/second */
ULONG	time;
ULONG	tmp_time;

	/* First, make sure (as best as possible) that the CPU values are right */
	CPU_State_Flag=TRUE;
	Delay(1);	/* Let it run into the TRUE CPU_State_Flag */

	/* 1,000,000 micro = 1 second... */
	time=(global->timer->time.tv_secs * 1000000) + global->timer->time.tv_micro;
	/* time is now in micro seconds... */

	number=MaxDivide(number,time,6);

	strcpy(format,"%s %9ld %s");
	if (!number)
	{
		strcpy(format,"%s       < %ld %s");
		number=1;
	}

	if (global->Base_CPU)
	{
		tmp_time=time;	/* For below... */

		while (CPU_Count_High)
		{
			/* Adjust the time and the CPU count as needed */
			tmp_time=tmp_time >> 1;
			CPU_Count_Low=CPU_Count_Low >> 1;
			if (CPU_Count_High & 1) CPU_Count_Low += 0x80000000;
			CPU_Count_High=CPU_Count_High >> 1;
		}

		clicks=MaxDivide(CPU_Count_Low,tmp_time,6);
		clicks=(MaxDivide(clicks,global->Base_CPU,3)+5)/10;
		global->CPU_Total+=clicks;
		global->CPU_Count++;

		strcat(format,"  |  CPU Available: %ld%%");
	}

	sprintf(p,format,Header,number,Units,clicks);

	AddDisplayLine(global,p);
}

VOID Display_Error(struct DiskSpeed *global,char *test)
{
	sprintf(global->tmp1,"Error:  %s test failed.",test);
	AddDisplayLine(global,global->tmp1);
}

#ifdef	SCSI_SPEED

BOOL SpeedTest(struct DiskSpeed *global,ULONG size,ULONG offset,ULONG mem_type)
{
BOOL	worked=TRUE;
char	*buffer;
char	*mem;		/* What we really allocated */
char	*type;
char	*type2;
ULONG	count;

	AddDisplayLine(global,"");

	type="FAST";
	if (mem_type & MEMF_CHIP) type="CHIP";

	type2="LONG";
	if (offset & 2) type2="WORD";
	if (offset & 1) type2="BYTE";

	/* Round to block sizes */
	size=(size+511) & (~511);

	if (mem=AllocMem(size+offset,mem_type|MEMF_PUBLIC))
	{
		/* Set up memory... */
		buffer=&(mem[offset]);

		sprintf(global->tmp1,"Testing with a %ld byte, MEMF_%s, %s-aligned buffer.",size,type,type2);
		AddDisplayLine(global,global->tmp1);

		count=0;

		Start_Timer(global->timer);
		Init_CPU_Available();		/* Start counting free CPU cycles... */
		while ((worked &= Check_Quit(global)) && (Read_Timer(global->timer) < global->Min_Time))
		{
			global->DiskIO.iotd_Req.io_Command=CMD_READ;
			global->DiskIO.iotd_Req.io_Flags=NULL;
			global->DiskIO.iotd_Req.io_Length=size;
			global->DiskIO.iotd_Req.io_Data=buffer;
			global->DiskIO.iotd_Req.io_Offset=count;

			DoIO(&(global->DiskIO));
			count+=global->DiskIO.iotd_Req.io_Actual;

			if (global->DiskIO.iotd_Req.io_Error)
			{
				worked=FALSE;
				Display_Error(global,"Device Reported Error on Read");
			}
		}
		Stop_Timer(global->timer);

		if (worked) Display_Result(global,BYTES_READ,count,BYTE_UNITS);

		Free_CPU_Available();
		FreeMem(mem,size+offset);
	}
	else
	{
		sprintf(global->tmp1,"Skipping %ld byte MEMF_%s test due to lack of memory.",size,type);
		AddDisplayLine(global,global->tmp1);
	}

	return(worked);
}

#else	/* SCSI_SPEED */

/*
 * In order to keep the file create test fair, it must always do
 * the same number of files.  The way filing systems work, many times
 * the get slower as the number of files in a directory grow
 */
BOOL CreateFileTest(struct DiskSpeed *global)
{
BPTR	file;
ULONG	count;
BOOL	worked=TRUE;
char	*p=global->tmp1;	/* For speed reasons */

	Start_Timer(global->timer);
	Init_CPU_Available();		/* Start counting free CPU cycles... */

	for (count=0;(count<NUM_FILES) && (worked &= Check_Quit(global));count++)
	{
		sprintf(p,FILE_STRING,count);
		if (file=Open(p,MODE_NEWFILE)) Close(file);
		else
		{
			Display_Error(global,"File Create");
			worked=FALSE;
		}
	}

	Stop_Timer(global->timer);

	if (worked) Display_Result(global,FILE_CREATE,NUM_FILES,FILE_UNITS);

	Free_CPU_Available();

	return(worked);
}

BOOL OpenFileTest(struct DiskSpeed *global)
{
BPTR	file;
ULONG	count=0;
BOOL	worked=TRUE;
char	*p=global->tmp1;

	Start_Timer(global->timer);
	Init_CPU_Available();		/* Start counting free CPU cycles... */
	while ((worked &= Check_Quit(global)) && (Read_Timer(global->timer) < global->Min_Time))
	{
		sprintf(p,FILE_STRING,(ULONG)(count % NUM_FILES));
		count++;
		if (file=Open(p,MODE_OLDFILE)) Close(file);
		else
		{
			Display_Error(global,"File Open");
			worked=FALSE;
		}
	}
	Stop_Timer(global->timer);

	if (worked) Display_Result(global,FILE_OPEN,count,FILE_UNITS);

	Free_CPU_Available();
	return(worked);
}

BOOL ScanDirectoryTest(struct DiskSpeed *global)
{
BPTR	lock=NULL;
ULONG	count=0;
BOOL	worked=TRUE;

	Start_Timer(global->timer);
	Init_CPU_Available();		/* Start counting free CPU cycles... */
	while ((worked &= Check_Quit(global)) && (Read_Timer(global->timer) < global->Min_Time))
	{
		if (lock)
		{
			if (!ExNext(lock,global->fib)) lock=NULL;
			else count++;
		}
		else
		{
			CurrentDir(lock=CurrentDir(NULL));
			if (Examine(lock,global->fib)) count++;
			else
			{
				Display_Error(global,"Directory Scan");
				worked=FALSE;
			}
		}
	}
	Stop_Timer(global->timer);

	if (worked) Display_Result(global,FILE_SCAN,count,FILE_UNITS);

	Free_CPU_Available();
	return(worked);
}

/*
 * In order to keep the file delete test fair, it must always do
 * the same number of files.  The way filing systems work, many times
 * the get slower as the number of files in a directory grow
 */
BOOL DeleteFileTest(struct DiskSpeed *global)
{
ULONG	count;
BOOL	worked=TRUE;
char	*p=global->tmp1;

	Start_Timer(global->timer);
	Init_CPU_Available();		/* Start counting free CPU cycles... */

	for (count=0;(count<NUM_FILES) && (worked &= Check_Quit(global));count++)
	{
		sprintf(p,FILE_STRING,count);
		if (!DeleteFile(p))
		{
			Display_Error(global,"File Delete");
			worked=FALSE;
		}
	}

	Stop_Timer(global->timer);

	if (worked) Display_Result(global,FILE_DELETE,NUM_FILES,FILE_UNITS);

	Free_CPU_Available();

	return(worked);
}

BOOL SeekReadTest(struct DiskSpeed *global)
{
BPTR	file;
ULONG	size;
ULONG	count;
LONG	pos=0;
LONG	buffer[16];
BOOL	worked=FALSE;
void	*buf;

	/* First we build a file by writing the ROM to disk... */
	if (file=Open(FILE_STRING,MODE_NEWFILE))
	{
		size=0x40000;	/* Start by asking for 256K */
		while (size && (!(buf=AllocMem(size,MEMF_PUBLIC)))) size=size>>1;

		if (buf)
		{
			worked=TRUE;
			/* Write a 256K file... */
			count=0x40000/size;
			while ((count>0) && (worked&=Check_Quit(global)))
			{
				count--;
				if (size!=Write(file,buf,size))
				{
					worked=FALSE;
					Display_Error(global,"Seek/Read");
				}
			}
			FreeMem(buf,size);
		}
		else Display_Error(global,"Seek/Read");

		Start_Timer(global->timer);
		Init_CPU_Available();		/* Start counting free CPU cycles... */
		while ((worked &= Check_Quit(global)) && (Read_Timer(global->timer) < global->Min_Time))
		{
			Seek(file,pos,OFFSET_BEGINING);
			Read(file,buffer,64);
			count++;

			Seek(file,-(pos+64),OFFSET_END);
			Read(file,buffer,64);
			count++;

			Seek(file,-(pos+(size/3)),OFFSET_CURRENT);
			Read(file,buffer,64);
			count++;

			/* Come up with another position... */
			pos=(pos+(size/11)) % (size/3);
		}
		Stop_Timer(global->timer);

		if (worked) Display_Result(global,SEEK_READ,count,SEEK_UNITS);

		Free_CPU_Available();

		Close(file);
		DeleteFile(FILE_STRING);
	}
	else Display_Error(global,"Seek/Read");

	return(worked);
}

BOOL SpeedTest(struct DiskSpeed *global,ULONG size,ULONG offset,ULONG mem_type)
{
BOOL	worked=TRUE;
char	*buffer;
char	*mem;		/* What we really allocated */
char	*type;
char	*type2;
ULONG	loop;
ULONG	count;
LONG	times;
BPTR	file=NULL;

	AddDisplayLine(global,"");

	type="FAST";
	if (mem_type & MEMF_CHIP) type="CHIP";

	type2="LONG";
	if (offset & 2) type2="WORD";
	if (offset & 1) type2="BYTE";

	if (mem=AllocMem(size+offset,mem_type|MEMF_PUBLIC))
	{
		/* Set up memory... */
		buffer=&(mem[offset]);

		for (loop=0;loop<size;loop++) buffer[loop]=(UBYTE)loop;

		sprintf(global->tmp1,"Testing with a %ld byte, MEMF_%s, %s-aligned buffer.",size,type,type2);
		AddDisplayLine(global,global->tmp1);

		count=0;
		times=0;

		Start_Timer(global->timer);
		Init_CPU_Available();		/* Start counting free CPU cycles... */
		while ((worked &= Check_Quit(global)) && (Read_Timer(global->timer) < global->Min_Time))
		{
			if (times<1)
			{
				if (file) Close(file);
				DeleteFile(FILE_STRING);
				if (!(file=Open(FILE_STRING,MODE_NEWFILE)))
				{
					Display_Error(global,"Create File");
					worked=FALSE;
				}
				times=0x40000/size;	/* Try to make file at least 256K size */
			}
			if (file)
			{
				if (size!=Write(file,buffer,size))
				{
					Display_Error(global,"Create File");
					worked=FALSE;
				}
				else count+=size;
				times--;
			}
		}
		Stop_Timer(global->timer);

		if (worked) Display_Result(global,BYTES_CREATE,count,BYTE_UNITS);

		/* Fill out the file... */
		if (file) while ((worked &= Check_Quit(global)) && (times>0))
		{
			Write(file,buffer,size);
			times--;
		}

		if (file) Close(file);
		file=NULL;

		if (worked) if (!(file=Open(FILE_STRING,MODE_OLDFILE)))
		{
			Display_Error(global,"Write File");
			worked=FALSE;
		}

		count=0;
		times=0;

		Start_Timer(global->timer);
		Init_CPU_Available();		/* Start counting free CPU cycles... */
		while ((worked &= Check_Quit(global)) && (Read_Timer(global->timer) < global->Min_Time))
		{
			if (times<1)
			{
				Seek(file,0,OFFSET_BEGINNING);
				times=0x40000/size;	/* Try to make file at least 256K size */
			}
			if (size!=Write(file,buffer,size))
			{
				Display_Error(global,"Write File");
				worked=FALSE;
			}
			else count+=size;
			times--;
		}
		Stop_Timer(global->timer);

		if (worked) Display_Result(global,BYTES_WRITE,count,BYTE_UNITS);

		if (file) Close(file);
		file=NULL;

		if (worked) if (!(file=Open(FILE_STRING,MODE_OLDFILE)))
		{
			Display_Error(global,"Read File");
			worked=FALSE;
		}

		count=0;
		times=0;

		Start_Timer(global->timer);
		Init_CPU_Available();		/* Start counting free CPU cycles... */
		while ((worked &= Check_Quit(global)) && (Read_Timer(global->timer) < global->Min_Time))
		{
			if (times<1)
			{
				Seek(file,0,OFFSET_BEGINNING);
				times=0x40000/size;	/* Try to make file at least 256K size */
			}
			if (size!=Read(file,buffer,size))
			{
				Display_Error(global,"Read File");
				worked=FALSE;
			}
			else count+=size;
			times--;
		}
		Stop_Timer(global->timer);

		if (worked)
		{
			for (loop=0;loop<size;loop++) worked &= (buffer[loop]==(UBYTE)loop);
			if (!worked) AddDisplayLine(global,"*** Data Error ***  Buffer did not read correctly.");
		}

		if (worked) Display_Result(global,BYTES_READ,count,BYTE_UNITS);

		if (file) Close(file);

		Free_CPU_Available();
		FreeMem(mem,size+offset);
		DeleteFile(FILE_STRING);
	}
	else
	{
		sprintf(global->tmp1,"Skipping %ld byte MEMF_%s test due to lack of memory.",size,type);
		AddDisplayLine(global,global->tmp1);
	}

	return(worked);
}

/*
 * Clean up (remove) all of the files in the current directory...
 */
void CleanUpFiles(struct DiskSpeed *global)
{
BPTR	lock;

	CurrentDir(lock=CurrentDir(NULL));	/* Get current directory lock */

	while (lock)
	{
		if (Examine(lock,global->fib))
		{
			if (ExNext(lock,global->fib)) DeleteFile(global->fib->fib_FileName);
			else lock=NULL;
		}
		else lock=NULL;
	}
}

#endif	/* SCSI_SPEED */

void DoTests(struct DiskSpeed *global)
{
char	*p=global->tmp1;
BOOL	working;
ULONG	memtype;
ULONG	offset;
short	size;

#ifndef	SCSI_SPEED
char	*fstring;
LONG	buffers;
#endif	/* !SCSI_SPEED */

	/*
	 * Ok, so now we are ready to run...  Display the
	 * test conditions...
	 */
	strcpy(p,"CPU: ");
	strcat(p,global->CPU_Type);
	strcat(p,"  AmigaOS Version: ");
	strcat(p,global->Exec_Ver);
	if (global->HighDMA) strcat(p,"  High");
	else strcat(p,"  Normal");
	strcat(p," Video DMA");
	AddDisplayLine(global,p);

	/*
	 * Now, if we are in 2.0 OS, we can also find out the number of buffers
	 * (maybe) on that device.  This is important.
	 */
#ifdef	SCSI_SPEED

	sprintf(p,"Device: %s",global->Device);

#else	/* SCSI_SPEED */

	fstring="Device: %s    Buffers: <information unavailable>";
	if (DOSBase->lib_Version > 36L)
	{
		/*
		 * Ok, so we can now try to get a reading of the buffers
		 * for the place we are about to test...
		 *
		 * Note:  Since we are in the "CURRENTDIR" of the test disk,
		 * we are using "" as the "device" to which to call AddBuffers()
		 */
		if ((buffers=AddBuffers("",0)) > 0) fstring="Device:  %s    Buffers: %ld";
	}

	sprintf(p,fstring,global->Device,buffers);

#endif	/* SCSI_SPEED */

	AddDisplayLine(global,p);

	if (global->Comments[0])
	{
		strcpy(p,"Comments: ");
		strcat(p,global->Comments);
		AddDisplayLine(global,p);
	}

	AddDisplayLine(global,"");

	if (CPU_Use_Base) Delay(60);	/* Make sure filesystem has flushed... */

	Init_CPU_Available();
	if (CPU_Use_Base) Delay(75);	/* Get a quick reading (~1.5 seconds) */
	Free_CPU_Available();

	if (CPU_Use_Base)
	{
		/*
		 * Now, generate a countdown value that is aprox 3 times 1.5 second...
		 */
		CPU_Use_Base=(CPU_Count_Low * 3) + 1;
		CPU_Count_Low=CPU_Use_Base;
		Forbid();
		Start_Timer(global->timer);
		CPU_Calibrate();
		Stop_Timer(global->timer);
		Permit();

		/*
		 * If it looks like we did not get a good reading,
		 * set up CPU_Use_Base to 0 in order to turn off
		 * CPU readings...
		 */
		if (global->timer->time.tv_secs<4)
		{
			AddDisplayLine(global,"CPU Calibration shows that CPU availability tests");
			AddDisplayLine(global,"would be inaccurate in the current system state.");
			CPU_Use_Base=0;
		}
		else CPU_Use_Base=MaxDivide(CPU_Use_Base,(global->timer->time.tv_secs * 1000000) + global->timer->time.tv_micro,6);
	}

	global->Base_CPU=CPU_Use_Base;

	if (CPU_Use_Base) sprintf(p,"CPU Speed Rating: %ld",(((CPU_Use_Base/500)+1) >> 1 ));
	else strcpy(p,"No CPU Speed Rating -- CPU % not available.");
	AddDisplayLine(global,p);
	AddDisplayLine(global,"");

	global->CPU_Total=0L;
	global->CPU_Count=0L;

	working=Check_Quit(global);

#ifndef	SCSI_SPEED

	if (working) if (global->Test_DIR)
	{
		AddDisplayLine(global,"Testing directory manipulation speed.");
		if (working) working=CreateFileTest(global);
		if (working) working=OpenFileTest(global);
		if (working) working=ScanDirectoryTest(global);
		if (working) working=DeleteFileTest(global);
	}

	if (working) if (global->Test_SEEK)
	{
		AddDisplayLine(global,"");
		if (working) working=SeekReadTest(global);
	}

#endif	/* !SCSI_SPEED */

	/* Now for some of the more complex tests */
	/* result=SpeedTest(global,Buffer,offset,mem_type); */

	memtype=MEMF_FAST;
	while (memtype)
	{
		if (memtype & global->Mem_TYPES) for (offset=4;offset>0;offset=offset >> 1) if (offset & global->Align_Types)
		{
			for (size=0;size<4;size++) if (global->Test_Size[size])
			{
				if (working) working=SpeedTest(global,global->Test_Size[size],offset&3,memtype);
			}
		}

		if (memtype & MEMF_CHIP) memtype=0;
		else memtype=MEMF_CHIP;
	}

#ifndef	SCSI_SPEED

	CleanUpFiles(global);

#endif	/* !SCSI_SPEED */

	if ((working) && (global->CPU_Count))
	{
		AddDisplayLine(global,"");
		global->CPU_Total=(((global->CPU_Total << 1) / global->CPU_Count)+1) >> 1;
		global->CPU_Count=(((global->Base_CPU * global->CPU_Total) / 50000)+1) >> 1;
		sprintf(p,"Average CPU Available: %ld%%  |  CPU Availability index: %ld",global->CPU_Total,global->CPU_Count);
		AddDisplayLine(global,p);
	}
}

#ifdef	SCSI_SPEED

void StartTest(struct DiskSpeed *global)
{
APTR	oldwindow;
char	*p;
char	*unit=NULL;
ULONG	scsi_unit;

	oldwindow=global->Me->pr_WindowPtr;
	global->Me->pr_WindowPtr=(APTR)(-1L);

	FreeDisplayList(global);

	AddDisplayLine(global,COPYRIGHT);
	AddDisplayLine(global,"------------------------------------------------------------");

	p=global->Device;
	while (*p)
	{
		if (*p==':') unit=p;
		p++;
	}

	p=unit;

	if (unit)
	{
		*unit='\0';
		unit++;
		scsi_unit=0;
		while ((unit) && (*unit))
		{
			if ((*unit < '0') || (*unit >'9')) unit=NULL;
			else
			{
				scsi_unit*=10;
				scsi_unit+=*unit-'0';
				unit++;
			}
		}
	}

	if (unit)
	{
		memset(&(global->DiskIO),0,sizeof(struct IOExtTD));
		if (global->DiskIO.iotd_Req.io_Message.mn_ReplyPort=CreatePort(NULL,0))
		{
			if (!OpenDevice(global->Device,scsi_unit,&(global->DiskIO),NULL))
			{
				unit=NULL;
				*p=':';
				DoTests(global);
				CloseDevice(&(global->DiskIO));
			}
			DeletePort(global->DiskIO.iotd_Req.io_Message.mn_ReplyPort);
		}
		if (unit) AddDisplayLine(global,"Error:  Could not open device specified");
	}
	else AddDisplayLine(global,"Error:  Bad device specification.");

	if (p) *p=':';

	global->Me->pr_WindowPtr=oldwindow;
}

#else	/* SCSI_SPEED */

void StartTest(struct DiskSpeed *global)
{
BPTR	lock;
BPTR	newlock;
APTR	oldwindow;

	oldwindow=global->Me->pr_WindowPtr;
	global->Me->pr_WindowPtr=(APTR)(-1L);

	FreeDisplayList(global);

	AddDisplayLine(global,COPYRIGHT);
	AddDisplayLine(global,"------------------------------------------------------------");

	if (lock=Lock(global->Device,ACCESS_READ))
	{
		lock=CurrentDir(lock);
		if (newlock=CreateDir(TEST_DIR))
		{
			UnLock(newlock);
			if (newlock=Lock(TEST_DIR,ACCESS_READ))
			{
				newlock=CurrentDir(newlock);

				/*
				 * Now do all of the tests...
				 */
				DoTests(global);

				newlock=CurrentDir(newlock);
				UnLock(newlock);
			}
			else AddDisplayLine(global,"Error:  Could not access test directory.");
			DeleteFile(TEST_DIR);
		}
		else AddDisplayLine(global,"Error:  Could not create test directory.");
		lock=CurrentDir(lock);
		UnLock(lock);
	}
	else AddDisplayLine(global,"Error:  Could not get a lock on test device.");

	global->Me->pr_WindowPtr=oldwindow;
}

#endif	/* SCSI_SPEED */

VOID SetVersionStrings(struct DiskSpeed *global)
{
UWORD	flags=((struct ExecBase *)(SysBase))->AttnFlags;
char	*p;

	strcpy(global->CPU_Type,"68000");
	p=&(global->CPU_Type[3]);
	if (flags & AFF_68010) *p='1';
	if (flags & AFF_68020) *p='2';
	if (flags & AFF_68030) *p='3';
	if (flags & AFF_68040) *p='4';

	sprintf(global->Exec_Ver,"%ld.%ld",(ULONG)(SysBase->lib_Version),(ULONG)(((struct ExecBase *)SysBase)->SoftVer));
}

/*
 * A simple string check that also works with '=' at the end of the string
 */
char *Check_String(char *arg,char *match)
{
char	*p;
char	*next=NULL;

	p=arg;
	while (*p)
	{
		if (*p=='=')
		{
			*p='\0';
			next=p;
		}
		else p++;
	}

	if (stricmp(arg,match))
	{
		if (next) *next='=';
		next=NULL;
	}
	else
	{
		if (next) next++;
		else next=p;
	}

	return(next);
}

/*
 * This routine closes down the GUI
 */
void Close_GUI(struct DiskSpeed *global)
{
	if (global->Window)
	{
		CloseWindow(global->Window);
		global->Window=NULL;
	}
	if (global->List)
	{
		FreeListGadget(global->List);
		global->List=NULL;
	}
	if (global->ri)
	{
		CleanUp_RenderInfo(global->ri);
		global->ri=NULL;
	}
}

/*
 * This routine is used to open the GUI...
 */
BOOL Open_GUI(struct DiskSpeed *global)
{
	BOOL		worked=FALSE;
	UWORD		internalwidth;
	UWORD		internalheight;
struct	Gadget		*gad=NULL;
struct	RenderInfo	*ri_temp;
struct	NewWindow	nw;

	if (!global->Window)
	{
		/* Now, open/set up the GUI... */
		nw.LeftEdge=0;
		nw.TopEdge=0;
		nw.Width=540;
		nw.Height=169;
		nw.DetailPen=-1;
		nw.BlockPen=-1;
		nw.IDCMPFlags=GADGETDOWN|GADGETUP|CLOSEWINDOW|ACTIVEWINDOW|MOUSEBUTTONS;
		nw.Flags=SMART_REFRESH|ACTIVATE|NOCAREREFRESH|RMBTRAP|WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG;
		nw.FirstGadget=NULL;
		nw.CheckMark=NULL;
		nw.Title=COPYRIGHT;
		nw.Type=WBENCHSCREEN;

		/*
		 * Take a quick guess at window title bar size
		 */
		if (ri_temp=Get_RenderInfo(NULL))
		{
			nw.Height+=ri_temp->WindowTitle;
			CleanUp_RenderInfo(ri_temp);
		}

		if (global->Window=OpenWindow(&nw)) if ((global->Window->Height-global->Window->BorderTop-global->Window->BorderBottom) > 120)
		{
			SetWindowTitles(global->Window,COPYRIGHT,COPYRIGHT);
			if (global->ri=Get_RenderInfo(global->Window->WScreen))
			{
				internalwidth=global->Window->Width-global->Window->BorderLeft-global->Window->BorderRight;
				internalheight=global->Window->Height-global->Window->BorderTop-global->Window->BorderBottom;

				global->List=InitListGadget(&TOPAZ80,&gad,1,
								global->ri->Highlight,
								global->ri->Shadow,
								global->Window->BorderLeft+4,
								global->Window->BorderTop+33,
								internalwidth-8,
								internalheight-49);

				/*
				 * Set up borders for the string gadgets
				 */
				global->StringBorder[0].LeftEdge=-5;
				global->StringBorder[0].TopEdge=-3;
				global->StringBorder[0].FrontPen=global->ri->Highlight;
				global->StringBorder[0].DrawMode=JAM1;
				global->StringBorder[0].Count=5;
				global->StringBorder[0].XY=&(global->StringVectors[0*5*2]);
				global->StringBorder[0].NextBorder=&(global->StringBorder[1]);

				global->StringBorder[1]=global->StringBorder[0];
				global->StringBorder[1].FrontPen=global->ri->Shadow;
				global->StringBorder[1].XY=&(global->StringVectors[1*5*2]);
				global->StringBorder[1].NextBorder=&(global->StringBorder[2]);

				global->StringBorder[2].LeftEdge=-3;
				global->StringBorder[2].TopEdge=-2;
				global->StringBorder[2].FrontPen=global->ri->Shadow;
				global->StringBorder[2].DrawMode=JAM1;
				global->StringBorder[2].Count=5;
				global->StringBorder[2].XY=&(global->StringVectors[2*5*2]);
				global->StringBorder[2].NextBorder=&(global->StringBorder[3]);

				global->StringBorder[3]=global->StringBorder[2];
				global->StringBorder[3].FrontPen=global->ri->Highlight;
				global->StringBorder[3].XY=&(global->StringVectors[3*5*2]);
				global->StringBorder[3].NextBorder=NULL;

				FillTopLeft_Border(&(global->StringBorder[0]),internalwidth-81,14);
				FillBottomRight_Border(&(global->StringBorder[1]),internalwidth-81,14);
				FillTopLeft_Border(&(global->StringBorder[2]),internalwidth-85,12);
				FillBottomRight_Border(&(global->StringBorder[3]),internalwidth-85,12);

				/*
				 * Now add the few other gadgets to the window...
				 */
				global->DeviceGadget.NextGadget=gad;		gad=&(global->DeviceGadget);
				gad->LeftEdge=82+global->Window->BorderLeft;
				gad->TopEdge=5+global->Window->BorderTop;
				gad->Width=internalwidth-91;
				gad->Height=8;
				gad->Flags=GADGHCOMP;
				gad->Activation=RELVERIFY;
				gad->GadgetType=STRGADGET;
				gad->GadgetRender=(APTR)&(global->StringBorder[0]);
				gad->GadgetText=&(global->DeviceText);
				gad->SpecialInfo=(APTR)&(global->DeviceInfo);
				gad->GadgetID=DEVICE_GADGET;

				global->CommentsGadget=global->DeviceGadget;
				global->CommentsGadget.NextGadget=gad;		gad=&(global->CommentsGadget);
				gad->TopEdge+=15;
				gad->GadgetText=&(global->CommentsText);
				gad->SpecialInfo=(APTR)&(global->CommentsInfo);
				gad->GadgetID=COMMENT_GADGET;

				global->DeviceInfo.Buffer=global->Device;
				global->DeviceInfo.UndoBuffer=global->Undo;
				global->DeviceInfo.MaxChars=250;
				global->CommentsInfo=global->DeviceInfo;
				global->CommentsInfo.Buffer=global->Comments;

				global->DeviceText.FrontPen=1;
				global->DeviceText.BackPen=0;
				global->DeviceText.DrawMode=JAM2;
				global->DeviceText.LeftEdge=-64;	/* 8 x 8 */
				global->DeviceText.TopEdge=0;
				global->DeviceText.ITextFont=&TOPAZ80;
				global->DeviceText.IText="Device:";

				global->CommentsText=global->DeviceText;
				global->CommentsText.LeftEdge=-80;	/* 10 x 8 */
				global->CommentsText.IText="Comments:";

				/*
				 * Set up borders for the action gadgets (One set for all gadgets...)
				 */
				global->ActionBorder[0].FrontPen=global->ri->Highlight;
				global->ActionBorder[0].DrawMode=JAM1;
				global->ActionBorder[0].Count=5;
				global->ActionBorder[0].XY=&(global->ActionVectors[0*5*2]);
				global->ActionBorder[0].NextBorder=&(global->ActionBorder[1]);

				global->ActionBorder[1].FrontPen=global->ri->Shadow;
				global->ActionBorder[1].DrawMode=JAM1;
				global->ActionBorder[1].Count=5;
				global->ActionBorder[1].XY=&(global->ActionVectors[1*5*2]);
				global->ActionBorder[1].NextBorder=NULL;

				global->ActionBorder[2]=global->ActionBorder[0];
				global->ActionBorder[2].FrontPen=global->ri->Shadow;
				global->ActionBorder[2].NextBorder=&(global->ActionBorder[3]);

				global->ActionBorder[3]=global->ActionBorder[1];
				global->ActionBorder[3].FrontPen=global->ri->Highlight;

				FillTopLeft_Border(&(global->ActionBorder[0]),108,12);
				FillBottomRight_Border(&(global->ActionBorder[1]),108,12);

				/*
				 * Now for the two action gadgets at the bottom...
				 */
				global->StartTestGadget.NextGadget=gad;		gad=&(global->StartTestGadget);
				gad->LeftEdge=global->Window->BorderLeft+4;
				gad->TopEdge=global->Window->Height-global->Window->BorderBottom-14;
				gad->Width=108;
				gad->Height=12;
				gad->Flags=((global->ri->Shadow==global->ri->Highlight) ? GADGHCOMP : GADGHIMAGE);
				gad->Activation=RELVERIFY;
				gad->GadgetType=BOOLGADGET;
				gad->GadgetRender=(APTR)&(global->ActionBorder[0]);
				gad->SelectRender=(APTR)&(global->ActionBorder[2]);
				gad->GadgetText=&(global->StartTest);
				gad->GadgetID=TEST_GADGET;

				global->StartTest.FrontPen=1;
				global->StartTest.DrawMode=JAM1;
				global->StartTest.ITextFont=&TOPAZ80;
				global->StartTest.IText=START_TEST;
				global->StartTest.TopEdge=2;
				global->StartTest.LeftEdge=14;

				global->SaveResultsGadget=global->StartTestGadget;
				global->SaveResultsGadget.NextGadget=gad;	gad=&(global->SaveResultsGadget);
				gad->LeftEdge=global->Window->Width-global->Window->BorderRight-112;
				gad->GadgetText=&(global->SaveResults);
				gad->GadgetID=SAVE_GADGET;

				global->SaveResults=global->StartTest;
				global->SaveResults.IText=SAVE_RESULTS;
				global->SaveResults.LeftEdge=6;

				global->StopTestGadget=global->StartTestGadget;
				global->StopTestGadget.NextGadget=gad;	gad=&(global->StopTestGadget);
				gad->LeftEdge=(global->Window->Width-108)/2;
				gad->GadgetText=&(global->StopTest);
				gad->GadgetID=STOP_GADGET;

				global->StopTest=global->StartTest;
				global->StopTest.IText=STOP_TEST;
				global->StopTest.LeftEdge=18;

				if (global->List)
				{
					AddGList(global->Window,gad,-1,-1,NULL);
					RefreshGList(gad,global->Window,NULL,-1);
					worked=TRUE;
				}
			}
		}

		if (!worked) Close_GUI(global);
	}

	return(global->Window!=NULL);
}

/*
 * This routine will append to the end of a file the results currently in memory...
 */
void Save_Results(struct DiskSpeed *global)
{
	BPTR		fh;
struct	Node		*node;

	if (fh=Open(RESULTS_FILE,MODE_OLDFILE))
	{
		Seek(fh,0,OFFSET_END);
		Write(fh,"\n\n\n",2);
	}
	else fh=Open(RESULTS_FILE,MODE_NEWFILE);

	if (fh)
	{
		node=(struct Node *)(global->TextList.mlh_Head);
		while (node->ln_Succ)
		{
			Write(fh,node->ln_Name,strlen(node->ln_Name));
			Write(fh,"\n",1);
			node=node->ln_Succ;
		}

		Close(fh);
	}
}

/***********************************************************/
/**                                                       **/
/**  This is the config requester code for DiskSpeed 4.2  **/
/**                                                       **/
/***********************************************************/

/*
 * Ok, now for some silly vectors that will be needed to make the 1.3 versions
 * of some gadgets that are very simple under 2.0
 */
SHORT	CheckVec[18]={9,5, 12,8, 18,2, 19,2, 17,2, 11,8, 8,5, 7,5, 10,8};
SHORT	CheckBox1[10]={24,0, 1,0, 1,9, 0,10, 0,0};
SHORT	CheckBox2[10]={1,10, 24,10, 24,1, 25,0, 25,10};

struct	Border	Check_Box2={0,0,0,0,JAM2,5,CheckBox2,NULL};
struct	Border	Check_Box1={0,0,0,0,JAM2,5,CheckBox1,&Check_Box2};
struct	Border	CheckOn={0,0,1,0,JAM2,9,CheckVec,&Check_Box1};
struct	Border	CheckOff={0,0,0,0,JAM2,9,CheckVec,&Check_Box1};

UBYTE	*BufNames[]={	"Test Buffer 1",
			"Test Buffer 2",
			"Test Buffer 3",
			"Test Buffer 4"	};

UBYTE	*CheckGads[]={	"DIR",
			"SEEK",
			"NOCPU",
			"LONG",
			"WORD",
			"BYTE",
			"FAST",
			"CHIP"	};

SHORT	CheckPos[]={	8,4,
			8,15,
			8,26,
			8,123,
			8,134,
			8,145,
			87,123,
			87,134	};

struct	ConfigRequest
{
struct	Requester	req;		/* The requester */

struct	Border		reqBorders[4];	/* Border structures for the requester */
struct	Border		StringBorder[4];
struct	Border		BoolBorder[4];

struct	Gadget		CheckGads[8];	/* The 8 check gadgets */
struct	Gadget		OkGadget;
struct	Gadget		MinTimeGadget;
struct	Gadget		BufGads[4];

struct	StringInfo	MinTimeString;
struct	StringInfo	BufString[4];

struct	IntuiText	reqText[2];	/* Text structures for the requester */
struct	IntuiText	CheckText[8];	/* Text for the 8 check gadgets */
struct	IntuiText	OkText;
struct	IntuiText	MinTimeText;
struct	IntuiText	BufText[4];

	SHORT		reqVecs[5*2*4];	/* The vectors for the req border */
	SHORT		StringVectors[5*2*4];
	SHORT		BoolVecs[5*2*2];

	UBYTE		MinTimeBuf[12];
	UBYTE		Buf[12*4];
	UBYTE		Undo[12];
};

#define	REQ_HEIGHT	165
#define	REQ_WIDTH	225

#define	OK_GADGET	99

/*
 * This routine will display and handle the config window
 */
void Do_Config(struct DiskSpeed *global)
{
struct	ConfigRequest	*config;
struct	Gadget		*gad=NULL;
struct	IntuiMessage	*msg;
	short		loop;
	BOOL		error=TRUE;
	short		temp;

	Check_Box1.FrontPen=global->ri->Highlight;
	Check_Box2.FrontPen=global->ri->Shadow;

	if (config=AllocMem(sizeof(struct ConfigRequest),MEMF_PUBLIC|MEMF_CLEAR))
	{
		InitRequester(&(config->req));

		config->req.LeftEdge=global->Window->BorderLeft+2;
		config->req.TopEdge=global->Window->BorderTop+1;
		config->req.Width=global->Window->Width-global->Window->BorderLeft-global->Window->BorderRight-4;
		config->req.Height=global->Window->Height-global->Window->BorderTop-global->Window->BorderBottom-2;
		config->req.ReqBorder=config->reqBorders;
		config->req.ReqText=config->reqText;

		/*
		 * Do the OK gadget
		 */
		config->OkGadget.NextGadget=gad;
		gad=&(config->OkGadget);

		gad->LeftEdge=135;
		gad->TopEdge=148;
		gad->Width=82;
		gad->Height=13;
		gad->Flags=GFLG_GADGHIMAGE;
		gad->Activation=GACT_RELVERIFY;
		gad->GadgetType=GTYP_REQGADGET|GTYP_BOOLGADGET;
		gad->GadgetRender=(APTR)(&(config->BoolBorder[0]));
		gad->SelectRender=(APTR)(&(config->BoolBorder[2]));
		gad->GadgetText=&(config->OkText);
		gad->GadgetID=OK_GADGET;

		config->OkText.FrontPen=1;
		config->OkText.DrawMode=JAM2;
		config->OkText.LeftEdge=33;
		config->OkText.TopEdge=3;
		config->OkText.ITextFont=&TOPAZ80;
		config->OkText.IText="OK";

		/*
		 * Set up the requester text structures
		 */
		config->reqText[0]=config->OkText;
		config->reqText[0].LeftEdge=9;
		config->reqText[0].TopEdge=42;
		config->reqText[0].IText="Read/Write Buffer Sizes";
		config->reqText[0].NextText=&(config->reqText[1]);

		config->reqText[1]=config->reqText[0];
		config->reqText[1].TopEdge=111;
		config->reqText[1].IText="Read/Write Buffer Types";
		config->reqText[1].NextText=NULL;

		/*
		 * Now, build the border structures for the requester
		 */
		config->reqBorders[0].FrontPen=global->ri->Shadow;
		config->reqBorders[0].DrawMode=JAM2;
		config->reqBorders[0].Count=5;
		config->reqBorders[0].XY=&(config->reqVecs[5*2*0]);
		config->reqBorders[0].NextBorder=&(config->reqBorders[1]);

		config->reqBorders[1]=config->reqBorders[0];
		config->reqBorders[1].FrontPen=global->ri->Highlight;
		config->reqBorders[1].XY=&(config->reqVecs[5*2*1]);
		config->reqBorders[1].NextBorder=&(config->reqBorders[2]);

		config->reqBorders[2]=config->reqBorders[1];
		config->reqBorders[2].TopEdge=1;
		config->reqBorders[2].LeftEdge=2;
		config->reqBorders[2].XY=&(config->reqVecs[5*2*2]);
		config->reqBorders[2].NextBorder=&(config->reqBorders[3]);

		config->reqBorders[3]=config->reqBorders[0];
		config->reqBorders[3].TopEdge=1;
		config->reqBorders[3].LeftEdge=2;
		config->reqBorders[3].XY=&(config->reqVecs[5*2*3]);
		config->reqBorders[3].NextBorder=NULL;

		FillTopLeft_Border(&(config->reqBorders[0]),REQ_WIDTH,REQ_HEIGHT);
		FillBottomRight_Border(&(config->reqBorders[1]),REQ_WIDTH,REQ_HEIGHT);
		FillTopLeft_Border(&(config->reqBorders[2]),REQ_WIDTH-4,REQ_HEIGHT-2);
		FillBottomRight_Border(&(config->reqBorders[3]),REQ_WIDTH-4,REQ_HEIGHT-2);

		/*
		 * Now, set up the string gadget border structure (it is the same for all)
		 */
		config->StringBorder[0]=config->reqBorders[1];
		config->StringBorder[0].LeftEdge=-5;
		config->StringBorder[0].TopEdge=-3;
		config->StringBorder[0].XY=&(config->StringVectors[0*5*2]);
		config->StringBorder[0].NextBorder=&(config->StringBorder[1]);

		config->StringBorder[1]=config->StringBorder[0];
		config->StringBorder[1].FrontPen=global->ri->Shadow;
		config->StringBorder[1].XY=&(config->StringVectors[1*5*2]);
		config->StringBorder[1].NextBorder=&(config->StringBorder[2]);

		config->StringBorder[2]=config->StringBorder[1];
		config->StringBorder[2].LeftEdge=-3;
		config->StringBorder[2].TopEdge=-2;
		config->StringBorder[2].XY=&(config->StringVectors[2*5*2]);
		config->StringBorder[2].NextBorder=&(config->StringBorder[3]);

		config->StringBorder[3]=config->StringBorder[2];
		config->StringBorder[3].FrontPen=global->ri->Highlight;
		config->StringBorder[3].XY=&(config->StringVectors[3*5*2]);
		config->StringBorder[3].NextBorder=NULL;

		FillTopLeft_Border(&(config->StringBorder[0]),86,13);
		FillBottomRight_Border(&(config->StringBorder[1]),86,13);
		FillTopLeft_Border(&(config->StringBorder[2]),82,11);
		FillBottomRight_Border(&(config->StringBorder[3]),82,11);

		/*
		 * Now set up the rel-verify gadget border structure
		 * (Also the same everywhere)
		 */
		config->BoolBorder[0]=config->StringBorder[0];
		config->BoolBorder[0].LeftEdge=0;
		config->BoolBorder[0].TopEdge=0;
		config->BoolBorder[0].XY=&(config->BoolVecs[0*5*2]);
		config->BoolBorder[0].NextBorder=&(config->BoolBorder[1]);

		config->BoolBorder[1]=config->BoolBorder[0];
		config->BoolBorder[1].FrontPen=global->ri->Shadow;
		config->BoolBorder[1].XY=&(config->BoolVecs[1*5*2]);
		config->BoolBorder[1].NextBorder=NULL;

		config->BoolBorder[2]=config->BoolBorder[0];
		config->BoolBorder[2].FrontPen=global->ri->Shadow;
		config->BoolBorder[2].NextBorder=&(config->BoolBorder[3]);

		config->BoolBorder[3]=config->BoolBorder[1];
		config->BoolBorder[3].FrontPen=global->ri->Highlight;

		FillTopLeft_Border(&(config->BoolBorder[0]),82,13);
		FillBottomRight_Border(&(config->BoolBorder[1]),82,13);

		/*
		 * Do the MinTime gadget
		 */
		config->MinTimeGadget.NextGadget=gad;
		gad=&(config->MinTimeGadget);

		gad->LeftEdge=128;
		gad->TopEdge=20;
		gad->Width=76;
		gad->Height=8;
		gad->Flags=GFLG_GADGHCOMP;
		gad->Activation=GACT_RELVERIFY|GACT_LONGINT;
		gad->GadgetType=GTYP_STRGADGET;
		gad->GadgetRender=(APTR)(&(config->StringBorder[0]));
		gad->GadgetText=&(config->MinTimeText);
		gad->SpecialInfo=(APTR)(&(config->MinTimeString));

		config->MinTimeString.Buffer=config->MinTimeBuf;
		config->MinTimeString.UndoBuffer=config->Undo;
		config->MinTimeString.MaxChars=11;
		config->MinTimeString.LongInt=global->Min_Time;
		sprintf(config->MinTimeBuf,"%ld",config->MinTimeString.LongInt);

		config->MinTimeText=config->OkText;
		config->MinTimeText.TopEdge=-12;
		config->MinTimeText.LeftEdge=12;
		config->MinTimeText.IText="MINTIME";

		/*
		 * Now for the 4 buffer gadgets and the Default gadgets for them
		 */
		for (loop=0;loop<4;loop++)
		{
			config->BufGads[loop]=config->MinTimeGadget;
			config->BufGads[loop].NextGadget=gad;
			gad=&(config->BufGads[loop]);

			gad->LeftEdge=136;
			gad->TopEdge=56+(loop*13);
			gad->GadgetText=&(config->BufText[loop]);
			gad->SpecialInfo=(APTR)(&(config->BufString[loop]));

			config->BufString[loop]=config->MinTimeString;
			config->BufString[loop].Buffer=&(config->Buf[loop*12]);
			config->BufString[loop].LongInt=global->Test_Size[loop];
			sprintf(config->BufString[loop].Buffer,"%ld",config->BufString[loop].LongInt);

			config->BufText[loop]=config->OkText;
			config->BufText[loop].TopEdge=0;
			config->BufText[loop].LeftEdge=-127;
			config->BufText[loop].IText=BufNames[loop];
		}

		/*
		 * Now set up the checkmark gadgets...
		 */
		for (loop=0;loop<8;loop++)
		{
			config->CheckGads[loop].NextGadget=gad;
			gad=&(config->CheckGads[loop]);

			gad->LeftEdge=CheckPos[(loop*2)];
			gad->TopEdge=CheckPos[(loop*2)+1];
			gad->Width=26;
			gad->Height=11;
			gad->Flags=GFLG_GADGHIMAGE;
			gad->Activation=GACT_TOGGLESELECT;
			gad->GadgetType=GTYP_REQGADGET|GTYP_BOOLGADGET;
			gad->GadgetRender=(APTR)(&CheckOff);
			gad->SelectRender=(APTR)(&CheckOn);
			gad->GadgetText=&(config->CheckText[loop]);

			/*
			 * Now, set the initial selected state for this gadget
			 */
			switch (loop)
			{
#ifdef	SCSI_SPEED

			case	0:
			case	1:	temp=FALSE; gad->Flags|=GFLG_DISABLED;	break;
			case	2:	temp=(CPU_Use_Base==FALSE);		break;
			case	3:	temp=TRUE; gad->Flags|=GFLG_DISABLED;	break;
			case	4:
			case	5:	temp=FALSE; gad->Flags|=GFLG_DISABLED;	break;

#else	/* SCSI_SPEED */

			case	0:	temp=(global->Test_DIR==TRUE);		break;
			case	1:	temp=(global->Test_SEEK==TRUE);		break;
			case	2:	temp=(CPU_Use_Base==FALSE);		break;
			case	3:	temp=(global->Align_Types & 4);		break;
			case	4:	temp=(global->Align_Types & 2);		break;
			case	5:	temp=(global->Align_Types & 1);		break;

#endif	/* SCSI_SPEED */

			case	6:	temp=(global->Mem_TYPES & MEMF_FAST);	break;
			case	7:	temp=(global->Mem_TYPES & MEMF_CHIP);	break;
			}
			if (temp) gad->Flags|=GFLG_SELECTED;

			config->CheckText[loop]=config->OkText;
			config->CheckText[loop].LeftEdge=30;
			config->CheckText[loop].TopEdge=2;
			config->CheckText[loop].IText=CheckGads[loop];
		}

		config->req.ReqGadget=gad;

		if (Request(&(config->req),global->Window))
		{
			error=FALSE;
			temp=TRUE;
			while(temp)
			{
				WaitPort(global->Window->UserPort);
				while (msg=(struct IntuiMessage *)GetMsg(global->Window->UserPort))
				{
					if (msg->Class==GADGETUP)
					{
						if (OK_GADGET==((struct Gadget *)(msg->IAddress))->GadgetID) temp=FALSE;
					}
					ReplyMsg((struct Message *)msg);
				}
			}

			EndRequest(&(config->req),global->Window);

			/*
			 * Ok, now get the information back out...
			 */
			global->Min_Time=config->MinTimeString.LongInt;

			for (loop=0;loop<4;loop++)
			{
				global->Test_Size[loop]=config->BufString[loop].LongInt;
			}

			for (loop=0;loop<8;loop++)
			{
				temp=0;
				if (config->CheckGads[loop].Flags & GFLG_SELECTED) temp=1;
				switch (loop)
				{
				case	0:	global->Test_DIR=temp;		break;
				case	1:	global->Test_SEEK=temp;		break;
				case	2:	CPU_Use_Base=(!temp);		break;
				case	3:	global->Align_Types &= (~4);
						global->Align_Types |= temp<<2;	break;
				case	4:	global->Align_Types &= (~2);
						global->Align_Types |= temp<<1;	break;
				case	5:	global->Align_Types &= (~1);
						global->Align_Types |= temp;	break;
				case	6:	global->Mem_TYPES &= (~MEMF_FAST);
						global->Mem_TYPES |= (temp * MEMF_FAST);
						break;
				case	7:	global->Mem_TYPES &= (~MEMF_CHIP);
						global->Mem_TYPES |= (temp * MEMF_CHIP);
						break;
				}
			}
		}

		FreeMem(config,sizeof(struct ConfigRequest));
	}

	if (error) AddDisplayLine(global,"Could not open configuration requester.");
}

/**********************************************************/
/**                                                      **/
/**  End of the config requester code for DiskSpeed 4.2  **/
/**                                                      **/
/**********************************************************/

/*
 * This routine is used to control the GUI
 */
void Do_GUI(struct DiskSpeed *global)
{
	BOOL		done=FALSE;
struct	IntuiMessage	*msg;

	while (!done)
	{
		WaitPort(global->Window->UserPort);
		while (msg=(struct IntuiMessage *)GetMsg(global->Window->UserPort))
		{
			if (!Check_ListGadget(global->Window,msg))
			{
				if (msg->Class==CLOSEWINDOW) done=TRUE;
				else if ((msg->Class==MOUSEBUTTONS) && (msg->Code==MENUDOWN)) Do_Config(global);
				else if (msg->Class==GADGETUP)
				{
					switch(((struct Gadget *)(msg->IAddress))->GadgetID)
					{
					case DEVICE_GADGET:	ActivateGadget(&(global->CommentsGadget),global->Window,NULL);
								break;
					case COMMENT_GADGET:	ActivateGadget(&(global->DeviceGadget),global->Window,NULL);
								break;
					case TEST_GADGET:	SetWait(global->Window);
								StartTest(global);
								ClearWait(global->Window);
								break;
					case SAVE_GADGET:	SetWait(global->Window);
								Save_Results(global);
								ClearWait(global->Window);
								break;
					}
				}
				else if (msg->Class==ACTIVEWINDOW)
				{
					ActivateGadget(&(global->DeviceGadget),global->Window,NULL);
				}
			}
			ReplyMsg((struct Message *)msg);
		}
	}
	/* Shut down GUI */
	Close_GUI(global);
}

/*
 * DRIVE/K	- select drive  (Default is current directory or scsi.device:6)
 * COMMENT/K	- set comment string
 * ALL/S	- select all tests
 * DIR/S	- setect DIR tests
 * SEEK/S	- select SEEK tests
 * CHIP/S	- select CHIP memory buffer tests
 * FAST/S	- select FAST memory buffer tests
 * LONG/S	- select LONG aligned tests
 * WORD/S	- select WORD aligned tests
 * BYTE/S	- select BYTE aligned tests
 * NOCPU/S	- turn off CPU availability tests
 * BUF1/K/N	- select buffer size 1
 * BUF2/K/N	- select buffer size 2
 * BUF3/K/N	- select buffer size 3
 * BUF4/K/N	- select buffer size 4
 * MINTIME/K/N	- select the minimum test time (default=8) in seconds
 * WINDOW/S	- use the GUI even though started from the CLI
 */

/*
 * do the command line parsing here...
 */
BOOL ParseArg(struct DiskSpeed *global,int argc,char *argv[],int start)
{
int	loop;
char	*arg;
char	*next;
BOOL	working=TRUE;
BOOL	window=FALSE;

#ifdef	SCSI_SPEED
	global->Align_Types=4;
#endif	/* SCSI_SPEED */

	for (loop=start;loop<argc;loop++)
	{
		arg=argv[loop];
		if (*arg=='?')
		{
			loop=argc;
			working=FALSE;
		}
		else if (next=Check_String(arg,"DRIVE"))
		{
			if (!(*next))
			{
				loop++;
				if (loop<argc) next=argv[loop];
			}
			if (strlen(next)>255) *next='\0';
			if (*next) strcpy(global->Device,next);
			else working=FALSE;
		}
		else if (next=Check_String(arg,"COMMENT"))
		{
			if (!(*next))
			{
				loop++;
				if (loop<argc) next=argv[loop];
			}
			if (strlen(next)>255) *next='\0';
			if (*next) strcpy(global->Comments,next);
			else working=FALSE;
		}
		else if (Check_String(arg,"ALL"))
		{
			/* All tests */

#ifndef	SCSI_SPEED

			global->Test_DIR=TRUE;
			global->Test_SEEK=TRUE;
			global->Align_Types=4|2|1;

#endif	/* !SCSI_SPEED */

			global->Mem_TYPES=MEMF_CHIP | MEMF_FAST;
		}
		else if (next=Check_String(arg,"BUF1"))
		{
			if (!(*next))
			{
				loop++;
				if (loop<argc) next=argv[loop];
			}
			if (*next) stcd_l(next,&(global->Test_Size[0]));
			else working=FALSE;
		}
		else if (next=Check_String(arg,"BUF2"))
		{
			if (!(*next))
			{
				loop++;
				if (loop<argc) next=argv[loop];
			}
			if (*next) stcd_l(next,&(global->Test_Size[1]));
			else working=FALSE;
		}
		else if (next=Check_String(arg,"BUF3"))
		{
			if (!(*next))
			{
				loop++;
				if (loop<argc) next=argv[loop];
			}
			if (*next) stcd_l(next,&(global->Test_Size[2]));
			else working=FALSE;
		}
		else if (next=Check_String(arg,"BUF4"))
		{
			if (!(*next))
			{
				loop++;
				if (loop<argc) next=argv[loop];
			}
			if (*next) stcd_l(next,&(global->Test_Size[3]));
			else working=FALSE;
		}
		else if (next=Check_String(arg,"MINTIME"))
		{
			if (!(*next))
			{
				loop++;
				if (loop<argc) next=argv[loop];
			}
			if (*next) stcd_l(next,&(global->Min_Time));
			else working=FALSE;
		}
#ifndef	SCSI_SPEED

		else if (Check_String(arg,"DIR")) global->Test_DIR=TRUE;
		else if (Check_String(arg,"SEEK")) global->Test_SEEK=TRUE;
		else if (Check_String(arg,"LONG")) global->Align_Types|=4;
		else if (Check_String(arg,"WORD")) global->Align_Types|=2;
		else if (Check_String(arg,"BYTE")) global->Align_Types|=1;

#endif	/* !SCSI_SPEED */

		else if (Check_String(arg,"CHIP")) global->Mem_TYPES|=MEMF_CHIP;
		else if (Check_String(arg,"FAST")) global->Mem_TYPES|=MEMF_FAST;
		else if (Check_String(arg,"NOCPU")) CPU_Use_Base=FALSE;
		else if (Check_String(arg,"WINDOW")) window=TRUE;
		else
		{	/* Did not match, so error */
			working=FALSE;
		}
	}

	if (global->Min_Time < 1) global->Min_Time=1;

	if (working) if (window) working=Open_GUI(global);

	return(working);
}

/*
 * This routine is called when we want to run from a GUI
 * Normally, it is only called when started from Workbench
 * or when the CLI WINDOW option is given...
 */
void DoWorkbench(struct DiskSpeed *global,int argc,char *argv[])
{
struct	WBStartup	*wbmsg;
struct	WBArg		*wbarg;
	BPTR		lock;
struct	DiskObject	*icon;

	wbmsg=(struct WBStartup *)argv;

	if ((wbarg=wbmsg->sm_ArgList) && (wbmsg->sm_NumArgs))
	{
		/*
		 * Check if we were started as a project and
		 * use that icon insted...
		 */
		if ((wbmsg->sm_NumArgs) > 1) wbarg++;

		lock=CurrentDir(wbarg->wa_Lock);

		argc=0;
		if (icon=GetDiskObject(wbarg->wa_Name))
		{
			argv=icon->do_ToolTypes;
			while (argv[argc]) argc++;
			/*
			 * Don't care about argument errors in tooltypes
			 * since other things may have been in there...
			 */
			ParseArg(global,argc,argv,0);
			FreeDiskObject(icon);
		}

		if (!argc)
		{
			/* All tests */
			global->Test_DIR=TRUE;
			global->Test_SEEK=TRUE;
			global->Align_Types=4|2|1;
			global->Mem_TYPES=MEMF_CHIP | MEMF_FAST;
		}

		if (Open_GUI(global)) Do_GUI(global);

		CurrentDir(lock);
	}
}

/*
 * This is the CLI starting point.  We do the command line parsing here...
 */
void DoCLI(struct DiskSpeed *global,int argc,char *argv[])
{
	if (ParseArg(global,argc,argv,1))
	{
		if (global->Window) Do_GUI(global);
		else StartTest(global);
	}
	else
	{
#ifdef	SCSI_SPEED

		AddDisplayLine(global,"DRIVE/K,ALL/S,CHIP/S,FAST/S,NOCPU/S,BUF1/K/N,BUF2/K/N,BUF3/K/N,BUF4/K/N,MINTIME/K/N,WINDOW/S");

#else	/* SCSI_SPEED */

		AddDisplayLine(global,"DRIVE/K,ALL/S,DIR/S,SEEK/S,CHIP/S,FAST/S,LONG/S,WORD/S,BYTE/S,NOCPU/S,BUF1/K/N,BUF2/K/N,BUF3/K/N,BUF4/K/N,MINTIME/K/N,WINDOW/S");

#endif	/* SCSI_SPEED */
	}
}

#ifdef	SCSI_SPEED

void main(int argc, char *argv[])
{
struct	DiskSpeed	*global;

	CPU_Use_Base=TRUE;	/* We want to test with CPU */

	if (IntuitionBase=OpenLibrary("intuition.library",33L))
	{
		if (GfxBase=OpenLibrary("graphics.library",33L))
		{
			if (LayersBase=OpenLibrary("layers.library",33L))
			{
				if (IconBase=OpenLibrary("icon.library",33L))
				{
					if (global=AllocMem(sizeof(struct DiskSpeed),MEMF_PUBLIC|MEMF_CLEAR))
					{
						NewList((struct List *)&(global->TextList));
						SetVersionStrings(global);
						global->Me=(struct Process *)FindTask(NULL);

						/* Standard MinTime */
						global->Min_Time=MIN_TEST_TIME;

						/* Standard sizes */
						global->Test_Size[0]=0x200;
						global->Test_Size[1]=0x1000;
						global->Test_Size[2]=0x8000;
						global->Test_Size[3]=0x40000;

						if (global->timer=Init_Timer())
						{
							/*
							 * Now either set up Window or Output
							 * depending on where we were started...
							 *
							 * If we can not get Output, we set up the window...
							 */
							if ((argc) && (global->Output=Output()))
							{
								DoCLI(global,argc,argv);
							}
							else DoWorkbench(global,argc,argv);

							Free_Timer(global->timer);
						}

						FreeDisplayList(global);
						FreeMem(global,sizeof(struct DiskSpeed));
					}
					CloseLibrary(IconBase);
				}
				CloseLibrary(LayersBase);
			}
			CloseLibrary(GfxBase);
		}
		CloseLibrary(IntuitionBase);
	}
}

#else	/* SCSI_SPEED */

void main(int argc, char *argv[])
{
struct	DiskSpeed	*global;

	CPU_Use_Base=TRUE;	/* We want to test with CPU */

	if (IntuitionBase=OpenLibrary("intuition.library",33L))
	{
		if (GfxBase=OpenLibrary("graphics.library",33L))
		{
			if (LayersBase=OpenLibrary("layers.library",33L))
			{
				if (IconBase=OpenLibrary("icon.library",33L))
				{
					if (global=AllocMem(sizeof(struct DiskSpeed),MEMF_PUBLIC|MEMF_CLEAR))
					{
						NewList((struct List *)&(global->TextList));
						SetVersionStrings(global);
						global->Me=(struct Process *)FindTask(NULL);

						/* Standard MinTime */
						global->Min_Time=MIN_TEST_TIME;

						/* Standard sizes */
						global->Test_Size[0]=0x200;
						global->Test_Size[1]=0x1000;
						global->Test_Size[2]=0x8000;
						global->Test_Size[3]=0x40000;

						if (global->fib=AllocMem(sizeof(struct FileInfoBlock),MEMF_PUBLIC))
						{
							if (global->timer=Init_Timer())
							{
								/*
								 * Now either set up Window or Output
								 * depending on where we were started...
								 *
								 * If we can not get Output, we set up the window...
								 */
								if ((argc) && (global->Output=Output()))
								{
									DoCLI(global,argc,argv);
								}
								else DoWorkbench(global,argc,argv);

								Free_Timer(global->timer);
							}
							FreeMem(global->fib,sizeof(struct FileInfoBlock));
						}

						FreeDisplayList(global);
						FreeMem(global,sizeof(struct DiskSpeed));
					}
					CloseLibrary(IconBase);
				}
				CloseLibrary(LayersBase);
			}
			CloseLibrary(GfxBase);
		}
		CloseLibrary(IntuitionBase);
	}
}

#endif	/* SCSI_SPEED */
