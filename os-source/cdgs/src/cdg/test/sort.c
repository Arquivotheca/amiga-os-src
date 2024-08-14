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
register ULONG fsize;
UBYTE lastbank = 0;
UBYTE *saveit;
BOOL spin;
ULONG col1, col2;
ULONG size;
BPTR fp;

	max = 256*1024L;

	col1 = 0L;
	col2 = 0L;


	if(hold = AllocMem((max*2),MEMF_CLEAR))
	{
		saveit = hold;
		copy = hold + max;

		if(fp=Open("df0:data",MODE_OLDFILE))
		{
			fsize=Read(fp,hold,max);
			Close(fp);
		}

		while(fsize > 512)
		{
			copy[0+168] = hold[0];
			copy[8+168] = hold[8];
			copy[16+168] = hold[16];
 
			copy[18+144] = hold[1];
			copy[9+144] = hold[9];
			copy[17+144] = hold[17];
 
			copy[5+120] = hold[2];
			copy[10+120] = hold[10];
			copy[1+120] = hold[18];
 
			copy[23+96] = hold[3];
			copy[11+96] = hold[11];
			copy[19+96] = hold[19];
 
			copy[4+72] = hold[4];
			copy[12+72] = hold[12];
			copy[20+72] = hold[20];
 
			copy[2+48] = hold[5];
			copy[13+48] = hold[13];
			copy[21+48] = hold[21];
 
			copy[6+24] = hold[6];
			copy[14+24] = hold[14];
			copy[22+24] = hold[22];
 
			copy[7+0] = hold[7];
			copy[15+0] = hold[15];
			copy[23+0] = hold[3];
 
			hold+=24;
			copy+=24;
 
			fsize-=24;
		}

		if(fp=Open("df0:temp",MODE_NEWFILE))
		{
			copy = saveit + max;
			Write(fp,copy,max);
			Close(fp);
		}

		FreeMem(saveit,(max*2));
	}
	return(0);
}