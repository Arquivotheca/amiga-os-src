/****************************************************************************
 *	    S  T  A  N  D  A  R  D    I  N  C  L  U  D  E  S		    *
 ****************************************************************************/

#ifndef EXEC_TYPES_H
#include "exec/types.h"
#endif
#ifndef EXEC_MEMORY_H
#include "exec/memory.h"
#endif
#ifndef EXEC_IO_H
#include "exec/io.h"
#endif
#ifndef EXEC_DEVICES_H
#include "exec/devices.h"
#endif
#ifndef EXEC_LIBRARIES_H
#include "exec/libraries.h"
#endif

#ifndef LIBRARIES_DOS_H
#include "libraries/dos.h"
#endif

#ifndef  DEVICES_CONSOLE_H
#include "devices/console.h"
#endif
#ifndef  DEVICES_KEYMAP_H
#include "devices/keymap.h"
#endif

#ifndef INTUITION_INTUITIONBASE_H
#include "intuition/intuitionbase.h"
#endif
#ifndef INTUITION_INTUITION_H
#include "intuition/intuition.h"
#endif

#ifndef GRAPHICS_GFX_H
#include "graphics/gfxbase.h"
#endif
#ifndef GRAPHICS_RASTPORT_H
#include "graphics/rastport.h"
#endif


/* color definitions (workbench default) */
#define BLUE	0
#define	BLACK	1
#define WHITE	2
#define RED	3

#define IS_PAL		(((struct GfxBase *)GfxBase)->DisplayFlags & PAL)
#define IS_LACE		(((struct GfxBase *)GfxBase)->Modes & LACE)
#define NTSC_HEIGHT     200
#define PAL_HEIGHT      256

#define HDIGIT(x) ((char)((x)/10+48))
#define LDIGIT(x) ((char)((x)%10+48))


#define NUMKEYS 104
#define REALKEYS (104-8)
#define EURO 0
#define USA 1
#define LSHIFT 0
#define RSHIFT 1
#define CONTR 2
#define LALT 3
#define RALT 4

#define K_NADA	     0
#define K_TEXT	     0x01
#define K_BORDER     0x02
#define K_PREDEFINED 0x4

struct KeyTop
{
    struct Border *KBorder;
    struct IntuiText *KText;
    char KCapTxt[3];
    UBYTE Flag;
};

