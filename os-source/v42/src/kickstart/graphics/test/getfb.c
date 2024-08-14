/*
 * Copyright 1992 Ameristar Technology, Inc
 *
 * minimal set of support routines to manipulate memory mapped frame buffer
 */

#include <sys/types.h>
#include <stdlib.h>

#include <libraries/expansion.h>
#include <libraries/configvars.h>
#if 0
#include <proto/expansion.h>
#include <pragmas/exec.h>
#include <proto/exec.h>
#endif

#include "1600gx.h"

#define AMERISTAR	1053	/* mfg number for Ameristar */
#define PROD_A1600GX	20	/* product num of 1600GX */

#define VIDREG	(0x100000)	/* video register base */
#define FB_BASE	(0x200000)	/* frame buffer base */

#define D4_HRZBR	(0x40+4)	/* horizontal blank start register */
#define D4_HRZBF	(0x40+5)	/* horizontal blank end register */
#define D4_VRTBR	(0x40+10)	/* vertical start register */
#define D4_VRTBF	(0x40+11)	/* vertical end register */

#define BT468_LOADDR	(0x80+0)	/* BT468 low order address bits */
#define BT468_HIADDR	(0x80+1)	/* BT468 high order address bits */
#define BT468_DATA	(0x80+3)	/* BT468 data register */

static volatile u_char *fbp;	/* pointer to frame buffer */
static volatile u_long *vp;	/* pointer to video control registers */

struct Library *ExpansionBase;

/*
 * return pointer to first pixel (upper left corner) in frame buffer
 */
volatile u_char *
A1600GX_getfbp()
{
	char *explib = EXPANSIONNAME;
	struct ConfigDev *cd;
	u_char *base;

	ExpansionBase = (struct Library *)OpenLibrary(explib, 0);
	if(!ExpansionBase){
		return (u_char *)0;
	}

	cd = FindConfigDev((struct ConfigDev *)0, AMERISTAR, PROD_A1600GX);
	if(cd){
		base = cd->cd_BoardAddr;
		fbp = base +  FB_BASE;
		vp = (u_long *)(base + VIDREG);
	}
	CloseLibrary(ExpansionBase);
	ExpansionBase = 0;

	return fbp;
}

/*
 * return number of pixels per line
 */
int
A1600GX_getH()
{
	int H = -1;
	if(vp){
		H = 8*(vp[D4_HRZBF] - vp[D4_HRZBR]);
	}
	return H;
}

/*
 * return number of lines per frame
 */
int
A1600GX_getV()
{
	int V = -1;
	if(vp){
		V = (vp[D4_VRTBF] - vp[D4_VRTBR]);
	}
	return V;
}

/*
 * load colormap: values (R[], G[], B[]) -> [start..(start+count-1)]
 */
int
A1600GX_loadCmap(int start, int count, u_char *R, u_char *G, u_char *B)
{
	if(!vp){
		return -1;
	}

	vp[BT468_LOADDR] = start;
	vp[BT468_HIADDR] = 0;
	while(count-- > 0){
		vp[BT468_DATA] = *R++;
		vp[BT468_DATA] = *G++;
		vp[BT468_DATA] = *B++;
	}

	return 0;
}
