/*
** MakeStream - A simple hack for building multiplexed MPEG streams that are contstrained to
** 		CD-ROM data rates.
**
**
**
** Each CD sector contains exactly 2324 bytes
**
** Each video or audio packet consists of 2250 bytes, leaving 74
** bytes in the sector.
**
** Each 2324 byte sector will look like this:
**
**  struct CDPacket
**  {
**       ULONG   PacketType;
**       ULONG   PacketSize;
**       ULONG   Padding[70];
**       UBYTE   Data[2028];
**  };
**
** The data is interleaved like this:
**
** Repeat 31 of VVVVVVA followed by VVVVVVVA
**
** This results in 225 CD sectors in 3 seconds containing 32 audio
** sectors and 193 video sectors.
**
*/

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/rdargs.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#define TEMPLATE "Video/A,Audio/A,Dest/A"

#define DATA_SIZE (2048 - 8)

struct CDPacket
{
    ULONG   PacketType;
    ULONG   PacketSize;
//    UBYTE   Padding[66];
    UBYTE   Data[DATA_SIZE];
};

ULONG MakeStream(VOID)
{
    struct Library *SysBase = (*(struct Library **)4L);
    struct Library *DOSBase;
    struct RDArgs *rdargs;
    LONG args[3];
    BPTR audio,video,dest;
    ULONG repeat,vidcnt,vsize=0,asize=0;
    ULONG vsectors=0,asectors=0;
    BOOL abort=FALSE;

    struct CDPacket cdp;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        if(rdargs = ReadArgs(TEMPLATE,args,NULL))
        {
            if(video = Open((STRPTR)args[0],MODE_OLDFILE))
            {
                if(audio = Open((STRPTR)args[1],MODE_OLDFILE))
                {
                    if(dest = Open((STRPTR)args[2],MODE_NEWFILE))
                    {

                    	for(repeat = 0; repeat < 5; repeat++)
                    	{
                                if(audio)
                                {
                                    cdp.PacketSize = Read(audio,cdp.Data,DATA_SIZE);
                                    if(cdp.PacketSize == -1)
                                    {
                                        abort = TRUE;
                                    }
                                    asize += cdp.PacketSize;
                                    asectors++;
                                    cdp.PacketType = 1;
                                    if(Write(dest,&cdp,sizeof(struct CDPacket)) == -1)
                                    	abort = TRUE;
                                }
                                if(abort)
                                    break;
                        }
                        while(TRUE)
                        {
                            if(SetSignal(0L,0L) & SIGBREAKF_CTRL_C)
                            	break;

                            for(repeat = 0; repeat < 4; repeat++)
                            {
                                for(vidcnt = 0; vidcnt < 3; vidcnt++)
                                {
                                    if(video)
                                    {
                                        cdp.PacketSize = Read(video,cdp.Data,DATA_SIZE);
                                        if(cdp.PacketSize == -1)
                                        {
                                             abort = TRUE;
                                        }
                                        vsize += cdp.PacketSize;
                                        vsectors++;
                                        cdp.PacketType = 0;
                                        if(Write(dest,&cdp,sizeof(struct CDPacket)) == -1)
                                            abort = TRUE;
                                    }
                                    if(abort)
                                        break;

                                }
                                if(abort)
                                    break;

                                if(audio)
                                {
                                    cdp.PacketSize = Read(audio,cdp.Data,DATA_SIZE);
                                    if(cdp.PacketSize == -1)
                                    {
                                        abort = TRUE;
                                    }
                                    asize += cdp.PacketSize;
                                    asectors++;
                                    cdp.PacketType = 1;
                                    if(Write(dest,&cdp,sizeof(struct CDPacket)) == -1)
                                    	abort = TRUE;
                                }
                                if(abort)
                                    break;
                            }
                            if(abort)
                            	break;

                            for(vidcnt = 0; vidcnt < 1; vidcnt ++)
                            {
                                if(video)
                                {
                                    cdp.PacketSize = Read(video,cdp.Data,DATA_SIZE);
                                    if(cdp.PacketSize == -1)
                                    {
                                         abort = TRUE;
                                    }
                                    vsize += cdp.PacketSize;
                                    vsectors++;
                                    cdp.PacketType = 0;
                                    if(Write(dest,&cdp,sizeof(struct CDPacket)) == -1)
                                    	abort = TRUE;

                                }
                                if(abort)
                                    break;

                            }
                            if(abort)
                                break;

                            if(audio)
                            {
                                cdp.PacketSize = Read(audio,cdp.Data,DATA_SIZE);
                                if(cdp.PacketSize == -1)
                                {
                                    abort = TRUE;
                                }
                                asize += cdp.PacketSize;
                                asectors++;
                                cdp.PacketType = 1;
                                if(Write(dest,&cdp,sizeof(struct CDPacket)) == 1)
                                    abort = TRUE;
                            }
                            if(abort)
                                break;

		            Printf("Sectors: %ld %ld\n",vsectors,asectors);
                        }
                        Close(dest);
                    }
                    Close(audio);
                }
                Close(video);
            }
            FreeArgs(rdargs);
        }
        Printf("Total Video Sectors: %ld (%ld bytes)\n",vsectors,vsize);
        Printf("Total Audio Sectors: %ld (%ld bytes)\n",asectors,asize);
        CloseLibrary(DOSBase);
    }
    return(0);
}
