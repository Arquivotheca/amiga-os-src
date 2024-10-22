/*
 * $Id: internal.h,v 38.1 91/06/24 19:01:50 mks Exp $
 *
 * $Log:	internal.h,v $
 * Revision 38.1  91/06/24  19:01:50  mks
 * Changed to V38 source tree - Trimmed Log
 * 
 */

#include <exec/types.h>
#include <exec/libraries.h>

#ifndef	WBINTERNAL_H
#include "V:src/kickstart/wb/wbinternal.h"
#endif WBINTERNAL_H

#include <dos.h>

/********************************************************************
*
* library structures
*
********************************************************************/

#define TFLAlloc( free, len, type, cast ) ((cast)IFreeAlloc( free, len, type ))

struct il {
    struct Library il;
    APTR il_SysBase;
    APTR il_DOSBase;
    APTR il_WorkbenchBase;
    APTR il_SegList;
};

#define	getIconBase()	((struct il *) getreg(REG_A6))
#define	ilBASE		getIconBase()
#define	ICONBASE	struct il *IconBase = ilBASE

#define SysBase ilBASE->il_SysBase
#define DOSBase ilBASE->il_DOSBase

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/alib_protos.h>

/*
 * Internal prototypes...
 */
BOOL stricmpn(char *,char *,LONG);
VOID setResult2(LONG);

VOID MyUpdateWorkbench(char *name,BPTR parentlock,BOOL PutFlag);

putImage(LONG file, struct Image *im );
struct Image *getImage(struct FreeList *free, LONG file, struct Image *im, struct Image *srcim );

APTR IFreeAlloc( struct FreeList *free, ULONG len, ULONG type );
replenishMem( struct FreeList *free );
IRead( BPTR file, VOID *ptr, LONG len );
IWrite( BPTR file, VOID *ptr, LONG len );

LONG IGetIconFromDobj( char *name, struct DiskObject *icon, struct FreeList *free, struct DiskObject *srcdobj );
struct DiskObject *IAllocateDiskObject(VOID);
IGetIconCommon(char *name,struct DiskObject *icon,struct FreeList *free,struct DiskObject *srcdobj,long version);
