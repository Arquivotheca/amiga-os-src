
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

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

BPTR afile;

VOID IOStreamProc(VOID)
{
    BPTR file;
    struct DataPacket dp;

    SysBase = (*(struct Library **)4L);

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        if(file = Open("dat:celt.mpg",MODE_OLDFILE))
        {
//            if(afile = Open("max:mpegfiles/seal.vbs2",MODE_NEWFILE))
//            {
                while((dp.dp_DataSize = Read(file,&dp.dp_Packet,CD_SECTOR_SIZE)) == CD_SECTOR_SIZE)
                {
                    if(SetSignal(0,0) & SIGBREAKF_CTRL_C)
                        break;
                    DoSystemDemux(NULL,&dp);
                }
//                Close(afile);
//	    }
            Close(file);
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
/*
void PrintAudioFrame(struct DataPacket *dp)
{
//	Printf("Pack %4.4ld\tAudio\n",packNum);

	Write(afile,dp->dp_Packet,CD_SECTOR_SIZE);
}
*/
void PrintAudioFrame(struct DataPacket *dp)
{
//	BPTR pk;
//	UBYTE buff[80];

	return;
/*
                    packNum++;

	Printf("Pack %4.4ld\tAudio\n",packNum);
	sprintf(buff,"Pack%ld",packNum);
	if(pk = Open(buff,MODE_NEWFILE))
	{
	    Write(pk,dp->dp_Packet,CD_SECTOR_SIZE);
	    Close(pk);
	}
*/
}

void PrintVideoFrame(struct DataPacket *dp)
{
	ULONG pts;

	pts = ((dp->dp_PTSHi & 3) <<30) |
	      ((dp->dp_PTSMid & 0x7fff) << 15) |
	      (dp->dp_PTSLo & 0x7fff);

	if(dp->dp_Flags & DPFLAGF_ValidPTS)
	    Printf("%08lx\n",pts);
}
