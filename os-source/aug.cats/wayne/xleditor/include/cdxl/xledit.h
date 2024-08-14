/*************

 xledit.h

 W.D.L 930330

**************/

/*
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 */

#ifndef	XLEDIT_H	// [
#define	XLEDIT_H

#include	"cdxl/cdxlob.h"


typedef struct DataNode
{
    struct DataNode *Next;
    ULONG Size;
    ULONG Type;

} DATA_NODE;


typedef struct xledit
{
    DATA_NODE	  dnode;
    ULONG	  flags;
    DISP_DEF	  disp_def;
    CDXLOB	* cdxlob;
    USHORT	* frameflags;
} XLEDIT;


typedef struct xlcontrol
{
    XLEDIT	* XlHead;
    XLEDIT	* XlParent;
    XLEDIT	* XlCur;
    ULONG	  flags;

} XLCONTROL;


// xledit->frameflags
#define	XL_FRAME_DELETED	0x0001

#endif			// ]
