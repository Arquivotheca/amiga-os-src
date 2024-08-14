#include <exec/types.h>
#include <clib/exec_protos.h>
#include "/view.h"
#include <stdio.h>
#include <dos/dos.h>
#define NO_PRAGMAS 1
#include "pd:ifflib/iff.h"

#pragma libcall IFFBase OpenIFF 1e 801
#pragma libcall IFFBase CloseIFF 24 901
#pragma libcall IFFBase FindChunk 2a 902
#pragma libcall IFFBase GetBMHD 30 901
#pragma libcall IFFBase GetColorTab 36 8902
#pragma libcall IFFBase DecodePic 3c 8902
#pragma libcall IFFBase SaveBitMap 42 a9804
/*#pragma libcall IFFBase SaveClip 48 210a9808*/
#pragma libcall IFFBase IFFError 4e 0
#pragma libcall IFFBase GetViewModes 54 901
#pragma libcall IFFBase NewOpenIFF 5a 802
#pragma libcall IFFBase ModifyFrame 60 8902

struct Library *IFFBase;
ULONG *infile;

void Fail(char *msg)
{
	if (msg) printf("%s\n",msg);
	if (infile) CloseIFF(infile);
	if (IFFBase) CloseLibrary(IFFBase);
	exit(0);
}


struct Library *openlib(char *name,ULONG version)
{
	struct Library *t1;
	t1=OpenLibrary(name,version);
	if (! t1)
	{
		printf("error- needs %s version %d\n",name,version);
		Fail(0l);
	}
	else return(t1);
}




main(argc,argv)
int argc;
char **argv;
{
	IFFBase=openlib("iff.library",0);
	if (argc==2)
	{
		if (infile=OpenIFF(argv[1]))
		{
			ULONG *form,*chunk;
			ULONG count;
			UBYTE *ptr;
			ULONG i;

			chunk=FindChunk(infile,ID_CMAP);
			if (! chunk) Fail("no color table");
			chunk++;
			count=(*(chunk++))/3;
			ptr=chunk;
			if (count>256) count=256;
			printf("\tdc.l\t%08lx\n",(count<<16)+0);
			for(i=0;i<count;i++)
			{
				printf("\tdc.l\t$%08lx,$%08lx,$%08lx  ; color #%d\n",*(ptr++)<<24,*(ptr++)<<24,*(ptr++)<<24,i);
			}
			printf("\tdc.l\t0\t; terminator\n");
			Fail(0);
		}
	} else Fail("can't open file");
}
