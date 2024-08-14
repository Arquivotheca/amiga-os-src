/*
 * GetSerial.c
 */

#include    <exec/libraries.h>

#include    <dos/dos.h>
#include    <dos/dosextens.h>

#include    <devices/serial.h>

#include    <clib/exec_protos.h>
#include    <pragmas/exec_pragmas.h>

#include    <clib/dos_protos.h>
#include    <pragmas/dos_pragmas.h>

#include    "GetSerial_rev.h"

/******************************************************************************/

/* This is the command template. */
#define TEMPLATE    "TO/A,UNIT/N" VERSTAG

#define OPT_TO		0
#define OPT_UNIT	1
#define OPT_COUNT	2

#define	BUF_SIZE	1023

#define	GO_SIGS		(SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F)

LONG cmd(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
	LONG		rc=RETURN_FAIL;

	if (DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",37))
	{
	struct	RDArgs	*rdargs;
		LONG	opts[OPT_COUNT];

		for (rc=0;rc<OPT_COUNT;opts[rc++]=0);
		rc=RETURN_FAIL;

		if (rdargs=ReadArgs(TEMPLATE,opts,NULL))
		{
		struct	MsgPort		*SerialMP;
		struct	IOExtSer	*SerialIO;
			ULONG		sigBits = SIGBREAKF_CTRL_C | GO_SIGS;
			LONG		unit=0;
			LONG		size;
			char		Buffer[BUF_SIZE+1];

			if (opts[OPT_UNIT]) unit=*((LONG *)opts[OPT_UNIT]);

			if (SerialMP=CreateMsgPort())
			{
				sigBits|=(1L << SerialMP->mp_SigBit);
				if (SerialIO=CreateIORequest(SerialMP,sizeof(struct IOExtSer)))
				{
					if (!OpenDevice("serial.device",unit,SerialIO,0))
					{
					BPTR	outFile;
					ULONG	sigs=0;

						if (outFile=Open((char *)opts[OPT_TO],MODE_NEWFILE))
						{
						BOOL	done=FALSE;
						BOOL	LoopFlag;

							rc=RETURN_OK;
							while (!done)
							{
								if (sigs & GO_SIGS)
								{
									SerialIO->IOSer.io_Command=SDCMD_QUERY;
									SerialIO->IOSer.io_Command=CMD_WRITE;
									SerialIO->IOSer.io_Length=1;
						/* CTRL-D: Send ^X */	if (sigs & SIGBREAKF_CTRL_D) SerialIO->IOSer.io_Data=(APTR)"\x18";
						/* CTRL-E: Send ^S */	if (sigs & SIGBREAKF_CTRL_E) SerialIO->IOSer.io_Data=(APTR)"\x13";
						/* CTRL-F: Send ^Q */	if (sigs & SIGBREAKF_CTRL_F) SerialIO->IOSer.io_Data=(APTR)"\x11";
									DoIO(SerialIO);
								}

								SerialIO->IOSer.io_Command=SDCMD_QUERY;
								DoIO(SerialIO);
								size=SerialIO->IOSer.io_Actual;
								if (size<1) size=1;
								else if (size>BUF_SIZE) size=BUF_SIZE;
								SerialIO->IOSer.io_Command=CMD_READ;
								SerialIO->IOSer.io_Length=size;
								SerialIO->IOSer.io_Data=(APTR)Buffer;
								SendIO(SerialIO);

								/*
								 * This loop is here to handle the case of
								 * getting a signal without the message...
								 */
								LoopFlag=TRUE;
								while (LoopFlag)
								{
									LoopFlag=FALSE;
									sigs=Wait(sigBits);
									if (sigs & SIGBREAKF_CTRL_C)
									{
										/* We got a quit signal, so die... */
										AbortIO(SerialIO);
										WaitIO(SerialIO);
										done=TRUE;
									}
									else
									{
										if (GetMsg(SerialMP))
										{
											if (size!=Write(outFile,Buffer,size)) done=TRUE;
										}
										else if (sigs & GO_SIGS)
										{
											AbortIO(SerialIO);
											WaitIO(SerialIO);
										}
										else
										{
											/* Signal without message, so loop... */
											LoopFlag=TRUE;
										}
									}
								}
							}
							Close(outFile);
						}
						CloseDevice(SerialIO);
					}
					DeleteIORequest(SerialIO);
				}
				DeleteMsgPort(SerialMP);
			}

			FreeArgs(rdargs);
			if (SetSignal(0,0) & SIGBREAKF_CTRL_C) PrintFault(ERROR_BREAK,NULL);
		}
		else PrintFault(IoErr(),NULL);

		CloseLibrary((struct Library *)DOSBase);
	}
	return(rc);
}
