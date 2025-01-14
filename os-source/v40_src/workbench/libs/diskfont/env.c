/*
**	$Id: env.c,v 38.4 91/04/09 20:09:08 kodiak Exp $
**
**	diskfont/env.c -- environment collection
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#ifdef	DEBUG
extern void kprintf(char *,...);
#define	D(a)	kprintf a
#else
#define	D(a)
#endif

#include  "dfdata.h"

#include  <dos/dos.h>
#include  <dos/rdargs.h>
#include  <clib/dos_protos.h>
#include  <pragmas/dos_pragmas.h>

#define	DOSBase	dfl->dfl_DOSBase
#define	GfxBase	dfl->dfl_GfxBase

#define  DTEMPLATE	"XDPI/N,YDPI/N,XDOTP/N,YDOTP/N,SYMSET"
#define  DOPT_XDPI	0
#define  DOPT_YDPI	1
#define  DOPT_XDOTP	2
#define  DOPT_YDOTP	3
#define  DOPT_SYMSET	4
#define  DOPT_COUNT	5

void __asm MyGetEnv(register __a0 struct DiskfontLibrary *dfl)
{
    struct RDArgs rdArgs;
    BPTR oldStdin, envIn;
    char buffer[256];
    LONG opts[DOPT_COUNT];

    if (envIn = Open("ENV:sys/diskfont", MODE_OLDFILE)) {
#ifdef  DEBUG
	{
	    char name[256];
	    NameFromFH(envIn, name, 256);
	    D(("ENV:sys/diskfont exists as \"%s\"\n", name));
	}
#endif
	oldStdin = SelectInput(envIn);
	while (FGetC(envIn) != -1) {
	    D(("new line...\n"));
#ifdef  DEBUG
	    D(("UnGetC $%lx\n", UnGetC(envIn, -1)));
#else
	    UnGetC(envIn, -1);
#endif
	    rdArgs.RDA_Source.CS_Buffer = 0;
	    rdArgs.RDA_Source.CS_Length = 0;
	    rdArgs.RDA_Source.CS_CurChr = 0;
	    rdArgs.RDA_DAList = 0;
	    rdArgs.RDA_Buffer = buffer;
	    rdArgs.RDA_BufSiz = 256;
	    rdArgs.RDA_ExtHelp = 0;
	    rdArgs.RDA_Flags = RDAF_STDIN | RDAF_NOPROMPT;
	    memset(opts, 0, DOPT_COUNT*4);
	    if (ReadArgs(DTEMPLATE, opts, &rdArgs)) {
#ifdef  DEBUG
		{
		    int i;
		    D(("ReadArgs success\n"));
		    for (i = 0; i < DOPT_COUNT; i++)
			D(("  arg %ld value $%ld: %02lx %02lx %02lx %02lx\n",
				i, opts[i], ((char *)opts[i])[0],
				((char *)opts[i])[1], ((char *)opts[i])[2],
				((char *)opts[i])[3]));
		}
#endif
		if (opts[DOPT_XDPI] && opts[DOPT_YDPI]) {
		    dfl->dfl_XDPI = (UWORD) (*((ULONG *) opts[DOPT_XDPI]));
		    dfl->dfl_YDPI = (UWORD) (*((ULONG *) opts[DOPT_YDPI]));
		    D(("ENV DPI %ld %ld\n", dfl->dfl_XDPI, dfl->dfl_YDPI));
		}
		if (opts[DOPT_XDOTP] && opts[DOPT_YDOTP]) {
		    dfl->dfl_DotSizeX = (UWORD) (*((ULONG *) opts[DOPT_XDOTP]));
		    dfl->dfl_DotSizeY = (UWORD) (*((ULONG *) opts[DOPT_YDOTP]));
		}
		FreeArgs(&rdArgs);
	    }
	}
	SelectInput(oldStdin);
	Close(envIn);
    }
}
