/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: macros.h,v 39.8 93/02/11 08:23:16 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	macros.h,v $
*   Revision 39.8  93/02/11  08:23:16  chrisg
*   new SUPERBUMP, SUPERMOVE, etc. maros for makevp speedup.
*   
*   Revision 39.7  92/07/01  11:01:37  chrisg
*   CMOVER macro. generates better code.
*   
*   Revision 39.6  92/06/19  14:57:08  spence
*   ENFORCER macro
*   
*   Revision 39.5  92/05/19  13:08:30  chrisg
*   added pragma for TaggedOpenLibrary.
*   
*   Revision 39.4  92/03/24  15:37:11  chrisg
*   added LayersBase #define.
*   
*   Revision 39.3  92/02/19  15:53:58  chrisg
*   mods so that graphics can call exec via pragmas.
*   
*   Revision 39.2  92/01/21  13:38:42  chrisg
*    redefinition of includes for pragma calling.
*   
*   Revision 39.1  91/11/11  16:50:24  chrisg
*   defined macro PASSRECT tpo trick lattice into generating good code.
*   
*   Revision 39.0  91/08/21  17:11:31  chrisg
*   Bumped
*   
*   Revision 37.5  91/05/13  14:48:12  chrisg
*   added #define for SysBase.
*   
*   Revision 37.4  91/05/07  17:05:32  chrisg
*    added GBASE #define
*   
*   Revision 37.3  91/05/03  15:32:04  chrisg
*    added #define for GBASE. instead of doing:
*      register struct GfxBase *GB;
*      FETCHGBASE;
*      GB->blah=0;
*   
*    do  GBASE->blah=0;
*      this will generate CLR.L (A6)
*   
*   Revision 37.2  91/05/03  15:29:03  spence
*   *** empty log message ***
*   
*   Revision 37.1  91/05/02  13:52:51  chrisg
*    changed FETCHGBASE macro for lattice
*   
*   Revision 37.0  91/01/07  15:14:44  spence
*   initial switchover from V36
*   
*   Revision 33.4  90/07/27  16:30:07  bart
*   *** empty log message ***
*   
*   Revision 33.3  90/03/28  09:37:07  bart
*   *** empty log message ***
*   
*   Revision 33.2  89/05/02  09:30:13  bart
*   copyright 1989
*   
*   Revision 33.1  88/06/23  18:39:52  dale
*   cp
*   
*   Revision 33.0  86/05/17  14:58:13  bart
*   added to rcs for updating
*   
*
******************************************************************************/

#ifndef REG_A6
#include <dos.h>
#endif

#ifndef GRAPHICS_GFXBASE_H
#include "/gfxbase.h"
#endif

#define SHORTMIN(a,b)	(SHORT)(MIN((SHORT)(a),(SHORT)(b)))
#define SHORTMAX(a,b)	(SHORT)(MAX((SHORT)(a),(SHORT)(b)))

#define	MAX(a,b)	((a)>(b)?(a):(b))
#define	MIN(a,b)	((a)<(b)?(a):(b))
#define	ABS(x)	((x<0)?(-(x)):(x))

#define LOCKLAYER(l)	ObtainSemaphore(&(l)->Lock)
#define UNLOCKLAYER(l)	ReleaseSemaphore(&(l)->Lock)

#define CMOVE(c,a,b)    { (c)->OpCode = COPPER_MOVE;(c)->DESTADDR=(int)(&a);(c)->DESTDATA = b; }

#define REGNUM(field) (UWORD)(&(((struct Custom *)0)->field))

/* CMOVER generates better code and is more readable. it also doesn't generate f106 for bplcon3! */
#define CMOVER(c,a,b)    { (c)->OpCode = COPPER_MOVE;(c)->DESTADDR=REGNUM(a);(c)->DESTDATA = b; }
#define SUPERCMOVE(c,a,b) { *(c++)=COPPER_MOVE; *(c++)=REGNUM(a); *(c++)=b; }
#define SUPERBUMP(acl) ++((*acl)->Count)
#define SUPERWAIT(c,a,b) { *(c++)=COPPER_WAIT; *(c++)=a; *(c++)=b; }

/* CBUMPR generates good code when there is no possibility of overflow */
#define CBUMPR(acl) { ++c ; ++((*acl)->Count); }

#define CWAIT(c,a,b)    { (c)->OpCode=COPPER_WAIT;(c)->VWAITPOS = a;(c)->HWAITPOS = b; }
#define CEND(c)                 CWAIT(c,10000,255)
#define SUPERCEND(c)                 SUPERWAIT(c,10000,255)

/* acl is address of a CopList ptr */
#define CBUMP(acl)      c = cbump(acl,++c)

#define FETCHGBASE		GB = (struct GfxBase *) getreg(REG_A6)
#define GBASE ((struct GfxBase *) getreg(REG_A6))

#define SysBase (((struct GfxBase *)getreg(14))->ExecBase)
#define LayersBase (((struct GfxBase *)getreg(14))->gb_LayersBase)
#define UtilityBase (GBASE->UtilBase)

#define LOCKLAYERINFO(li)  ObtainSemaphore(&(li)->Lock)
#define UNLOCKLAYERINFO(li)  ReleaseSemaphore(&(li)->Lock)

/* pass a rectangle by value on the stack. avoid lattice inefficiency */
/* if lattice gets smarter, can do #define PASSRECT(x) x */
#define PASSRECT(x) *((ULONG *) &x),*(((ULONG *) &x)+1)


/* create an enforcer hit */
#define ENFORCER(x) ((*(UWORD *)2) = (x))

#include <pragmas/exec_pragmas.h>
#pragma libcall SysBase TaggedOpenLibrary 32a 001
