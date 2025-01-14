#ifndef	INTUALL_H
#define	INTUALL_H

/*** intuall.h **************************************************************
 *
 *  intuall.h, general includer for intuition
 *
 *  $Id: intuall.h,v 40.0 94/02/15 17:42:24 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 *                  		Modification History
 *	date	    author :   	Comments
 *	------      ------      -------------------------------------
 *	1-30-85     -=RJ=-      created this file!
 *
 ****************************************************************************/

#include <exec/types.h>

#include <graphics/videocontrol.h>

#include "iprefs.h"
#include "sc.h"
#include "i.h"

#include "sghooks.h"
#include "cghooks.h"

#include "intuinternal.h"
#include "ism.h"
#include "ifunctions.h"

#include "classes.h"
#include "classusr.h"

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/utility_protos.h>

/*****************************************************************************/

#define printf kprintf

/*****************************************************************************/

/* some shorthand	*/
#define GDID	GetDisplayInfoData

/*
 * don't forget to define LOCKDEBUG in lockstub.asm
 *
 * LOCKDEBUG = 0: Lock-debugging disabled
 * LOCKDEBUG = 1: "Light" lock-debugging.  Enforcer hits and recoverable alerts.
 * LOCKDEBUG = 2: Full lock-debugging with enforcer hits and serial printing.
 */
#define LOCKDEBUG 0

#if LOCKDEBUG==0
#define assertLock( a, b )	;
#define checkLock( a, b )	;
#define lockFree( a, b )	;
#define enableLockCheck()	;
#else
#define enableLockCheck()	SETFLAG( IBase->DebugFlag, IDF_LOCKDEBUG )
#if LOCKDEBUG==1
#define assertLock( a, b )	AssertLock( NULL, b )
#define checkLock( a, b )	CheckLock( NULL, b )
#define lockFree( a, b )	LockFree( NULL, b )
#else /* LOCKDEBUG==2 */
#define assertLock( a, b )	AssertLock( a, b )
#define checkLock( a, b )	CheckLock( a, b )
#define lockFree( a, b )	LockFree( a, b )
#endif
#endif

/*****************************************************************************/

/* aformat.asm */
void fmtPass1 (UBYTE *, UWORD *, LONG, LONG, LONG);
UWORD *fmtPass2 (UBYTE *, UWORD *, LONG, LONG, LONG);
void jsprintf (void *, ...);

/* classface.asm */
ULONG CM (Class *, Object *, Msg);
ULONG CoerceMessage (Class *, Object *, ULONG data, ...);
ULONG SM (Object *, Msg);
ULONG SendMessage (Object *, ULONG data, ...);
ULONG SSM (Class *, Object *, Msg);
ULONG SendSuperMessage (Class *, Object *, ULONG data, ...);

#if 0
ULONG CallHookA( struct Hook *hookPtr, Object *obj, APTR message );
ULONG CallHook( struct Hook *hookPtr, Object *obj, ... );
ULONG DoMethodA( Object *obj, Msg message );
ULONG DoMethod( Object *obj, unsigned long MethodID, ... );
ULONG DoSuperMethodA( struct IClass *cl, Object *obj, Msg message );
ULONG DoSuperMethod( struct IClass *cl, Object *obj, unsigned long MethodID, ... );
ULONG CoerceMethodA( struct IClass *cl, Object *obj, Msg message );
ULONG CoerceMethod( struct IClass *cl, Object *obj, unsigned long MethodID, ... );
ULONG SetSuperAttrs( struct IClass *cl, Object *obj, unsigned long Tag1, ... );
#endif

/* downcode.asm */
int LOCKSTATE (void);
int LOCKGADGETS (void);
int LOCKIBASE (void);
int LOCKVIEW (void);
int LOCKVIEWCPR (void);
int LOCKRP (void);
int UNLOCKSTATE (void);
int UNLOCKGADGETS (void);
int UNLOCKIBASE (void);
int UNLOCKVIEW (void);
int UNLOCKVIEWCPR (void);
int UNLOCKRP (void);
void windowBox(struct Window *w,struct IBox *box);
ULONG GetTagDataUser0 (Tag, struct TagItem *);
ULONG GetTagDataUser (Tag, ULONG, struct TagItem *);
struct Screen *openScreenOnlyTags (ULONG, ...);
struct Window *openWindowOnlyTags (ULONG, ...);
void transfPoint (struct Point *, struct Point);
void currMouse (struct Window *, struct Point *);
void limitLongPoint (struct LongPoint *, struct LongPoint *);
void limitPoint (struct Point *, struct Point *);
LONG imin (LONG a, LONG b);
LONG imax (LONG a, LONG b);
LONG iabs (LONG a);
ULONG divMod( ULONG dividend, ULONG divisor, ULONG *remainderptr );
LONG jstreq (char *a, char *b);
void jstrncpy (char *dest, char *src, int num);
LONG strlen (char *text);
void rectToBox (struct Rectangle *r, struct IBox *b);
void boxToRect(struct IBox *box, struct Rectangle *rect);
void rectHull(struct Rectangle *big,struct Rectangle *r2);
void upperRectHull(struct Rectangle *big,struct Rectangle *r2);
LONG rectWidth (struct Rectangle *r);
LONG rectHeight (struct Rectangle *r);
void offsetRect (struct Rectangle *r, LONG dx, LONG dy);
LONG boxRight (struct IBox *b);
LONG boxBottom (struct IBox *b);
LONG ptInBox (struct Point pt, struct IBox *b);
LONG collide (LONG x, LONG y, LONG boxx, LONG boxy, LONG xsize, LONG ysize);
void limitRect (struct Rectangle *r, struct Rectangle *limit);
void BorderPatrol (struct Screen *);
ULONG nonDegenerate (struct Rectangle *r);
void degenerateRect (struct Rectangle *r);
int inrect (int x, int y, struct Rectangle *j);
void boxFit(struct IBox *box, struct IBox *container, struct IBox *result);
void windowInnerBox(struct Window *w,struct IBox *box);

/* hookface.asm */
ULONG callHook (struct Hook *hook, APTR object, ULONG data, ...);
ULONG callHookPkt (struct Hook *hook, APTR object, APTR msg);
void stubReturn (void);

/* intuileap.asm */
ULONG __asm IntuiLeap (register __a0 struct InputEvent *, register __a1 void *);

/* intuitionface.asm */
void ScreenDepth (struct Screen *scr, int depth, int unknown);
struct Screen *OpenScreenTagList (struct NewScreen *, struct TagItem *);
ULONG OpenWorkBench (void);

/* mousebuttons.asm */
ULONG CheckButtons (void);

/* standard stuff */
extern void NewList (struct List *);

#define	FreeGIRPort	freeRP
void kprintf (void *, ...);

/* unknown origin */
struct Library *TaggedOpenLibrary (ULONG);
void SetDefaultMonitor (ULONG);
void SetDisplayInfoData (ULONG, UBYTE *, ULONG, ULONG);
void SetWindowPointerA (struct Window *, struct TagItem *attrs);

/*****************************************************************************/

#include "intuition_iprotos.h"

/*****************************************************************************/

#endif
