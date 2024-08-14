
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "cd.h"
#include "streamio.h"

/*****************************************************************************/

extern void __stdargs kputstr(STRPTR str);
extern void __stdargs kprintf(STRPTR fmt, ...);

struct Library *SysBase;
struct Library *DOSBase;
ULONG packNum=0;


#ifndef min
#define min(a,b) __builtin_min(a,b)
#endif

struct OpenMsg
{
    struct Message	om_Msg;
    struct StreamFile	*om_SF;
    STRPTR		om_FileName;
    ULONG		om_Status;
};

VOID IOStreamProc(VOID);
extern void DoSystemDemux(struct StreamFile *stream, struct DataPacket *dp);

struct TagItem MPEGList[] =
{
    { TAGCD_SECTORSIZE, 2328},
    { TAG_END, TRUE}
};

struct TagItem NORMList[] =
{
    { TAGCD_SECTORSIZE, 2048},
    { TAG_END, TRUE}
};

union CDTOC myTOC[10];

VOID IOStreamProc(VOID)
{
    BPTR file;
    struct DataPacket dp;
    struct IOStdReq *ioCD;
    struct MsgPort *port;
    UBYTE i;
    ULONG startPos;
    LONG err;
    SysBase = (*(struct Library **)4L);

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        if(port = CreateMsgPort())
        {
            if(ioCD = (struct IOStdReq *)CreateIORequest(port,sizeof(struct IOStdReq)))
            {
                if(!OpenDevice("cd.device",0L,(struct IORequest *)ioCD,0L))
                {
                    ioCD->io_Command = CD_TOCLSN;
                    ioCD->io_Data = myTOC;
                    ioCD->io_Length = 10;
                    ioCD->io_Offset = 0;

                    if(!DoIO((struct IORequest *)ioCD))
                    {
			for(i = 0; i <= myTOC[0].Summary.LastTrack; i++)
			{
			    if(!i)
			    {
			    	Printf("Table of Contents\n");
			    	Printf("First Track: %ld\n",myTOC[i].Summary.FirstTrack);
			    	Printf("Last Track:  %ld\n",myTOC[i].Summary.LastTrack);
			    	Printf("Lead Out At Sector %ld\n",myTOC[i].Summary.LeadOut.LSN);
			    	Printf("-----------------------\n");
			    }
			    else
			    {
			    	Printf("Track:    %ld\n",myTOC[i].Entry.Track);
			    	Printf("CtlAdr:   %ld\n",myTOC[i].Entry.CtlAdr);
			    	Printf("Position: %ld\n",myTOC[i].Entry.Position.LSN);
			    	Printf("\n");
			    }
			}
			ioCD->io_Command = CD_CONFIG;
			ioCD->io_Data = (APTR)MPEGList;
			ioCD->io_Length = 0;
			DoIO((struct IORequest *)ioCD);

			startPos = myTOC[3].Entry.Position.LSN * 2328;

			while(!(SetSignal(0,0) & SIGBREAKF_CTRL_C))
			{
			    ioCD->io_Command = CD_READ;
			    ioCD->io_Data = (APTR)dp.dp_Packet;
			    ioCD->io_Length = 2328;
			    ioCD->io_Offset = startPos;
			    ioCD->io_Flags = 0;
			    if(!(err = DoIO((struct IORequest *)ioCD)))
			    {
			    	packNum++;
			    	if(*((ULONG *)ioCD->io_Data))
			    	    DoSystemDemux(NULL,&dp);
			    	else
			    	    PutStr("e");
			    }
			    else
			    {
			    	Printf("Read Error: %ld\n",err);
			    	break;
			    }
			    startPos += 2328;
			}
		    }
		    ioCD->io_Command = CD_CONFIG;
		    ioCD->io_Data = (APTR)NORMList;
		    ioCD->io_Length = 0;
		    DoIO((struct IORequest *)ioCD);

		    CloseDevice((struct IORequest *)ioCD);
		}
		DeleteIORequest((struct IORequest *)ioCD);
	    }
	    DeleteMsgPort(port);
	}
	CloseLibrary(DOSBase);
    }
}

void PrintBadFrame(struct DataPacket *dp)
{
	BPTR pk;
	UBYTE buff[80];

	Printf("Pack %4.4ld\tConfused!\n",packNum);
	sprintf(buff,"Pack%ld",packNum);
	if(pk = Open(buff,MODE_NEWFILE))
	{
	    Write(pk,dp->dp_Packet,CD_SECTOR_SIZE);
	    Close(pk);
	}
}

void PrintAudioFrame(struct DataPacket *dp)
{
	BPTR pk;
	UBYTE buff[80];

	Printf("Pack %4.4ld\tAudio\n",packNum);
	sprintf(buff,"Pack%ld",packNum);
	if(pk = Open(buff,MODE_NEWFILE))
	{
	    Write(pk,dp->dp_Packet,CD_SECTOR_SIZE);
	    Close(pk);
	}
}

void PrintVideoFrame(struct DataPacket *dp)
{
	BPTR pk;
	UBYTE buff[80];

	Printf("Pack %4.4ld\tVideo Size: %ld\t",packNum,dp->dp_DataLength);
	if(dp->dp_Flags & DPFLAGF_ValidPTS)
	{
		Printf("pts\n");
	}
	else
	{
		Printf("\n");
	}
	sprintf(buff,"Pack%ld",packNum);
	if(pk = Open(buff,MODE_NEWFILE))
	{
	    Write(pk,dp->dp_Packet,CD_SECTOR_SIZE);
	    Close(pk);
	}
}

