/* animr.c -- decodes opcode 5 ANIM dlta data into a bitmap */

#include <exec/types.h>
#include <graphics/gfx.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "iffp/ilbmapp.h"
#include "iffp/ilbm.h"
#include "iffp/anim.h"

#define ASM __asm
#define REG(x) register __ ## x

extern LONG ASM DecodeVKPlane (REG (a0) char *in, REG(a2) char *out, REG (d2) LONG linebytes, REG (a3) WORD *ytable);
extern LONG ASM DecodeXORPlane (REG (a0) char *in, REG(a2) char *out, REG (d2) LONG linebytes, REG (a3) WORD *ytable);

static WORD *ytable;
static LONG ytsize;

WORD *ytableptr;

ULONG returnvalue;

LONG loaddlta(struct iff *iff,struct BitMap *bitmap,AnimHeader *anhd, LONG *dltabuffer)
{
	LONG actual, size;

	struct ContextNode *cn;

	long i, linebytes;
	register unsigned char *in;
	unsigned char *out;

	UBYTE depth;

	if((anhd->operation != 5)||((anhd->bits)&&(anhd->bits != 2)))
	{
		message("Can not decompress - supports op 5 and XOR op 5 only\n");
		return(RETURN_FAIL);
	}

	if(!(currentchunkis(iff,ID_ILBM,ID_DLTA)))
	{
		message("ILBM has no DLTA\n");
		return(RETURN_FAIL);
	}

	if(!((bitmap)&&(anhd))) return(RETURN_FAIL);

	depth = bitmap->Depth;

	cn = CurrentChunk(iff);
	size = cn->cn_Size;

	actual = ReadChunkBytes(iff, dltabuffer, size);

	if(actual<size) return(RETURN_FAIL);

	linebytes = (long) bitmap->BytesPerRow;

	ytableptr = ytable;

//	printf("ytable=%d,ytableptr=%d\n",ytable,ytableptr);

	for (i=0; i<depth; i++)
	{

//		printf("dltabuffer[%d]=%d\n",i,dltabuffer[i]);

		if (dltabuffer[i])
		{
			in = (unsigned char *) dltabuffer + dltabuffer[i];
			out = (unsigned char *) (bitmap->Planes[i]);

//			printf("i=%d,in=%d,out=%d,linebytes=%d\n",i,in,out,linebytes);


			/* with kludge assume interleave 1 XOR brushs
			 * not marked as XOR
			 */
			if((anhd->bits == 0)&&(anhd->interleave != 1))
				DecodeVKPlane(in, out, linebytes, ytable);
			else if((anhd->bits == 2)||(anhd->interleave == 1))
				DecodeXORPlane(in, out, linebytes, ytable);

//			printf("returnvalue=%d\n",returnvalue);

		}
	}
	return(0);
}


BOOL make_ytable(WORD width, register WORD height)
{
	register WORD linebytes;
	register WORD *pt, acc;

	linebytes = (((width+15)>>4)<<1);
	ytsize = height * sizeof(WORD);

	if ((ytable = AllocMem(ytsize, 0L)) == NULL)
		return(FALSE);

	pt = ytable;
	acc = 0;

	while (--height >= 0)
	{
		*pt++ = acc;
		acc += linebytes;
	}

//	printf("ytable=%d\n",ytable);

	return(TRUE);
}


void free_ytable(void)
{
	if (ytable != NULL)
		FreeMem(ytable, ytsize);

	ytable = NULL;
}
