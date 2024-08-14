/* Collect CDGS subcodes */

#include "exec/types.h"
#include "exec/io.h"
#include "exec/memory.h"
#include "dos/dos.h"
#include "hardware/dmabits.h"
#include "hardware/custom.h"
#include "graphics/gfxmacros.h"
#include "clib/exec_protos.h"
#include "clib/dos_protos.h"
#include "/cdg_cr_pragmas.h"
#include "/cdgprefs.h"
#include "cdtv.h"

UBYTE *fire = (UBYTE *)0xbfe001;
UBYTE *mouse = (UBYTE *)0xdff016;

UBYTE *sub1 = (UBYTE *)0x120100;
UBYTE *sub2 = (UBYTE *)0x120180;
UBYTE *volatile bank = (UBYTE *volatile)0xb80018;
UBYTE *volatile inter = (UBYTE *volatile)0xb80004;

int main(int argc, char **argv[])
{
register UBYTE *hold, *copy;
register ULONG len,max;
register int x;
UBYTE lastbank = 0;
UBYTE *saveit;
BOOL spin;
ULONG col1, col2;
ULONG size;
BPTR fp;

	max = 256*1024L;
	size = max;
	size = size-200L;

	col1 = 0L;
	col2 = 0L;

	if(hold = AllocMem(max,MEMF_CLEAR))
	{
		spin = TRUE;
		saveit = hold;
		len = 0;
		Disable();
		while(spin)
		{
			if(*bank != lastbank)
			{
				if(lastbank)
 				{
					lastbank = *bank;
 
					copy = sub2;
					col2++;
	
					if(lastbank == 0x63)
					{
 						copy = sub1;
						col1++;
						col2--;
					}
	

					for(x=0;x<96;x++)
					{
						*hold = (*copy) & 0x3F;
 						hold++;
						copy++;
					}

						len = len+96L;

				}
				else
				{
					lastbank = *bank;

				}
			}

/*
			if((*mouse & 0x04)==0x0)
			{
				spin = FALSE;
			}
*/
			if(len > size)
			{
				spin = FALSE;
			}

		}
		Enable();
split:
		if(fp=Open("ram:data",MODE_NEWFILE))
		{
			Write(fp,saveit,len);
			Close(fp);
			printf("%ld bytes saved\n",len);
		}
		FreeMem(saveit,max);
	}
	return(0);
}