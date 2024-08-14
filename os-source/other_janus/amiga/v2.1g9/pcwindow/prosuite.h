
#ifndef PROSUITE_H
#define PROSUITE_H


/* *** prosuite.h ***********************************************************
 *
 * Amiga Programmers' Suite  --  Primary Include File
 *     from Book 1 of the Amiga Programmers' Suite by RJ Mical
 *
 * Copyright (C) 1986, =Robert J. Mical=
 * All Rights Reserved.
 *
 * Created for Amiga developers.
 * Any or all of this code can be used in any program as long as this
 * entire notice is retained, ok?  Thanks.
 *
 * HISTORY      NAME            DESCRIPTION
 * -----------  --------------  --------------------------------------------
 * 12 Aug 86    RJ >:-{)*       Prepare (clean house) for release
 * 14 Feb 86    =RJ Mical=      Created this file.
 *
 * *********************************************************************** */



#include <exec/types.h>
#include <exec/nodes.h> 
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/tasks.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <exec/devices.h>
#include <exec/interrupts.h>

#include <libraries/dos.h>
#include <libraries/dosextens.h>

/* ALWAYS INCLUDE GFX.H before other includes */
#include <graphics/gfx.h>
/*???#include <graphics/display.h>*/
/*???#include <graphics/clip.h>*/
/*???#include <graphics/rastport.h>*/
/*???#include <graphics/view.h>*/
/*???#include <graphics/gfxbase.h>*/
/*???#include <graphics/text.h>*/
/*???#include <graphics/gfxmacros.h>*/
/*???#include <graphics/layers.h>*/
/*???#include <graphics/sprite.h>*/

/*???#include <devices/inputevent.h>*/
/*???#include <devices/gameport.h>*/
/*???#include <devices/printer.h>*/
/*???#include <devices/timer.h>*/
/*???#include <devices/clipboard.h>*/

#include <workbench/startup.h>
#include <workbench/workbench.h>


#include <intuition/intuition.h>



/* === System Macros ==================================================== */
#define SetFlag(v,f)		((v)|=(f))
#define ClearFlag(v,f)		((v)&=~(f))
#define ToggleFlag(v,f) 	((v)^=(f))
#define FlagIsSet(v,f)		((BOOL)(((v)&(f))!=0))
#define FlagIsClear(v,f)	((BOOL)(((v)&(f))==0))
#define ABS(n)				(((n)<0)?(-(n)):(n))
#define BoundaryOff(r)		((r)->Flags &= ~AREAOUTLINE)
#define MAX(a,b)			(((a)>(b))?(a):(b))
#define MIN(a,b)			(((a)<(b))?(a):(b))




#endif /* of PROSUITE_H */

