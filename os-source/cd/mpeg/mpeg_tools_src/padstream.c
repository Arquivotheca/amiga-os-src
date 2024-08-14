/*
** PadStream - Quick Hack for padding out a brain damanged MPEG
**             system stream.
**
** Each CD sector contains exactly 2324 bytes
**
** Each Pack in the source file is 2304 bytes, so add 20 bytes
** of empty space to each Pack and write out a new file.
**
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "asyncio.h"

#define audio_packet_start_code_prefix	0x000001C0
#define audio_packet_start_code_mask	0xFFFFFFE0
#define video_packet_start_code_prefix	0x000001E0
#define video_packet_start_code_mask	0xFFFFFFF0

#define TEMPLATE "Source/A,Dest/A,VideoCD/S"

struct Library *SysBase;
struct Library *DOSBase;

ULONG MakeStream(VOID)
{
    struct RDArgs *rdargs;
    LONG args[3];
    struct AsyncFile *source, *dest;
    UBYTE *sectorBuff;
    ULONG i;
    ULONG val,bytes_written;

    SysBase = (*(struct Library **)4L);

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        if(rdargs = ReadArgs(TEMPLATE,args,NULL))
        {
            if(sectorBuff = AllocMem(2324,MEMF_CLEAR))
            {
                if(source = OpenAsync((STRPTR)args[0],MODE_READ,262144))
                {
                    if(dest = OpenAsync((STRPTR)args[1],MODE_WRITE,262144))
                    {
                        if(args[2])
                        {
                            for(i=0;i<87150;i++)
                                WriteLongAsync(dest,0x00000000);
                        }
                    	bytes_written = 0;

                        while(ReadLongAsync(source,&val) != -1)
                        {
                            if(val == 0x1ba)
                            {
                            	if(bytes_written == 2304)
                            	{
                            	    for(i=0;i<5;i++)
                            	    	WriteLongAsync(dest,0xffffffff);
                            	}
                            	else if(bytes_written && (bytes_written != 2324))
                            	{
                            	    Printf("Unknown Pack Size! %ld bytes\n",bytes_written);
                            	    break;
                            	}
                            	bytes_written = 0;
                            }
                            WriteLongAsync(dest,val);
                            bytes_written += 4;

			    if(bytes_written > 2324)
			    {
                                Printf("Overflow! Pack Size! %ld bytes\n",bytes_written);
				break;
			    }
                            if(SetSignal(0L,0L) & SIGBREAKF_CTRL_C)
                                break;
                        }
                        CloseAsync(dest);
                    }
                    CloseAsync(source);
                }
                FreeMem(sectorBuff,2324);
            }
            FreeArgs(rdargs);
        }
        CloseLibrary(DOSBase);
    }
    return(0);
}
