/*
**	$Id: env.c,v 38.3 92/06/01 12:38:45 darren Exp $
**
**	Fountain/env.c -- environment collection
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include "fountain.h"

extern struct Library *SysBase;
extern struct Library *DOSBase;

extern union StaticPool S;

#define  FOPT_SIZE	0
#define  FOPT_COUNT	1

#define  DOPT_XDPI	0
#define  DOPT_YDPI	1
#define  DOPT_XDOTP	2
#define  DOPT_YDOTP	3
#define  DOPT_SYMSET	4
#define  DOPT_COUNT	5

#if FOPT_COUNT >= DOPT_COUNT
#define  BOPT_COUNT	FOPT_COUNT
#else
#define  BOPT_COUNT	DOPT_COUNT
#endif

UWORD YSizes[OT_MAXAVAILSIZES] = {
    15, 30, 45, 60, 75
};
UWORD YSizesCnt = 5;

ULONG DeviceDPI = 0x00320032;	/* 50 x 50 */
ULONG DotSize = 0x00640064;	/* 100% x 100% */
UWORD SymbolSet = 0;		/* (unused) */

MyGetEnv()
{
    struct RDArgs rdArgs;
    BPTR oldStdin, envIn;
    LONG opts[BOPT_COUNT];
    char **size;
    UWORD num, ys[OT_MAXAVAILSIZES];
    short j, y, yi, yj;

    if (envIn = Open("ENV:sys/Intellifont", MODE_OLDFILE)) {
	oldStdin = SelectInput(envIn);
	yi = 0;
	while (FGetC(envIn) != -1) {
	    UnGetC(envIn, -1);
	    rdArgs.RDA_Source.CS_Buffer = 0;
	    rdArgs.RDA_Source.CS_Length = 0;
	    rdArgs.RDA_Source.CS_CurChr = 0;
	    rdArgs.RDA_DAList = 0;
	    rdArgs.RDA_Buffer = S.t.TempBuffer;
	    rdArgs.RDA_BufSiz = 256;
	    rdArgs.RDA_ExtHelp = 0;
	    rdArgs.RDA_Flags = RDAF_STDIN | RDAF_NOPROMPT;
	    opts[FOPT_SIZE] = 0;
	    if (ReadArgs(RDARGS_FOUNTAINENV, opts, &rdArgs)) {
		for (size = (char **) opts[FOPT_SIZE]; *size; *size++) {
		    if ((sscanf(*size, "%hu", &num) == 1) &&
			    (0 < num) && (num < 1000) &&
			    (yi < OT_MAXAVAILSIZES)) {
			ys[yi++] = num;
		    }
		}
		FreeArgs(&rdArgs);
	    }
	}
	if (yi != 0) {
	    YSizesCnt = 0;
	    for (;;) {
		y = 0xffff;
		yj = -1;
		for (j = 0; j < yi; j++) {
		    if (ys[j]) {
			if (ys[j] < y) {
			    y = ys[j];
			    yj = j;
			}
			else if (ys[j] == y) {
			    /* remove duplicate */
			    ys[j] = ys[--yi];
			    --j;
			}
		    }
		}
		if (yj >= 0) {
		    YSizes[YSizesCnt++] = y;
		    ys[yj] = 0;
		}
		else
		    break;
	    }
	}
	SelectInput(oldStdin);
	Close(envIn);
    }

    if (envIn = Open("ENV:sys/diskfont", MODE_OLDFILE)) {
	oldStdin = SelectInput(envIn);
	while (FGetC(envIn) != -1) {
	    UnGetC(envIn, -1);
	    rdArgs.RDA_Source.CS_Buffer = 0;
	    rdArgs.RDA_Source.CS_Length = 0;
	    rdArgs.RDA_Source.CS_CurChr = 0;
	    rdArgs.RDA_DAList = 0;
	    rdArgs.RDA_Buffer = S.t.TempBuffer;
	    rdArgs.RDA_BufSiz = 256;
	    rdArgs.RDA_ExtHelp = 0;
	    rdArgs.RDA_Flags = RDAF_STDIN | RDAF_NOPROMPT;
	    memset(opts, 0, DOPT_COUNT*4);
	    if (ReadArgs(RDARGS_DISKFONTENV, opts, &rdArgs)) {
		if (opts[DOPT_XDPI] && opts[DOPT_YDPI]) {
		    DeviceDPI = ((*((ULONG *) opts[DOPT_XDPI]))<<16) |
			    ((*((ULONG *) opts[DOPT_YDPI]))&0xffff);
		}
		if (opts[DOPT_XDOTP] && opts[DOPT_YDOTP]) {
		    DotSize = ((*((ULONG *) opts[DOPT_XDOTP]))<<16) |
			    ((*((ULONG *) opts[DOPT_YDOTP]))&0xffff);
		}
		if ((opts[DOPT_SYMSET]) && (strlen(opts[DOPT_SYMSET]) == 2))
		    SymbolSet = (((UBYTE *) opts[DOPT_SYMSET])[0] << 8) |
			    (((UBYTE *) opts[DOPT_SYMSET])[1]);
		FreeArgs(&rdArgs);
	    }
	}
	SelectInput(oldStdin);
	Close(envIn);
    }
}
