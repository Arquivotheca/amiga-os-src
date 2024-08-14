/* :ts=4
*
*       bookmarks.c -- Bookmarks
*
*       William A. Ware                                                 B116
*       Leo L. Schwab  (expanded autodocs)              B214
*
* NOTE: The CODE is formatted for 4-space tabs.  The AUTODOCS are formatted
*               for 8-space tabs.
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY                                            *
*   Copyright (C) 1991, Silent Software, Incorporated.                                          *
*   All Rights Reserved.                                                                                                        *
****************************************************************************/

#include <exec/types.h>

#include <exec/execbase.h>

#include <exec/interrupts.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <hardware/intbits.h>

#include <devices/timer.h>
#include <devices/input.h>
#include <devices/inputevent.h>

#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <hardware/blit.h>

#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/rastport.h>

#include <cdtv/debox.h>
#include <cdtv/cdtv.h>
#include <cdtv/cdtvprefs.h>
#include <cdtv/bookmark.h>

#include <libraries/dos.h>

#include <resources/battclock.h>

#include "/playerprefsbase.h"
#include "/playerprefs_pragmas.h"
#include "/playerprefs_protos.h"

#include "/screensaver/screensaver.h"
#include "keyclick.h"

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include "cdtv/battclock_protos.h"
#include <cdtv/debox_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>


char __far BookMark_Name[] = "bookmark.device";

/* SPOOF THE PRAGMAS */

#define SysBase                         (PlayerPrefsBase->ppb_ExecBase)
#define DeBoxBase                       (PlayerPrefsBase->ppb_DeBoxBase)
#define GfxBase                         (PlayerPrefsBase->ppb_GfxBase)


struct CDTVPrefs __far DefaultCDTVPrefs = {

    128,                                                    /* DisplayX */
    45,                                                     /* DisplayY */
    CDTVLANG_DEFAULT_NTSC,                                  /* Language  - I think I will check for pal and alter this if it is so */
    0x7fff,                                                 /* Volume - Full */
    CDTVPF_AUDIOVOL | CDTVPF_AMPM | CDTVPF_KEYCLICK,        /* Flags */
/*    1,                                                      /* time for a blank */
    10,                                                     /* time for a blank */
    0                                                       /* reserved */
    };



struct IOStdReq * __regargs openbookmark(register ULONG mpic, register BYTE pri,
                                         register struct PlayerPrefsBase *PlayerPrefsBase) {

    Forbid();

    if (!FindName(&((struct ExecBase *)PlayerPrefsBase->ppb_ExecBase)->DeviceList,BookMark_Name)) {

        Permit();
        return(NULL);
        }

    Permit();
    return( AllocIORequest( BookMark_Name,mpic,0,sizeof( struct IOStdReq ) ));
    }



int __regargs stdbookrequest(ULONG mpic, BYTE pri, UWORD command, ULONG offset,
                             ULONG length, void *data, struct PlayerPrefsBase *PlayerPrefsBase) {

register struct IOStdReq *booki;
register int err = -1;

    if (booki = openbookmark(mpic, pri, PlayerPrefsBase)) {

        booki->io_Command = command;
        booki->io_Offset  = offset;
        booki->io_Length  = length;
        booki->io_Data    = (APTR)data;
        err = DoIO( booki );
        }

    if ((command == CMD_READ) || (command == CMD_WRITE))  err = (err ? 0:booki->io_Actual);   /* For read write commands we want to return a zero if an error or size if not an error */

    if (booki) FreeIORequest(booki);

    return(err);
    }



/****** playerprefs.library/ReadBookMark **********************************
*
*   NAME
*       ReadBookMark -- Reads a bookmark into buffer.
*
*   SYNOPSIS
*       actuallength = ReadBookMark (data, size, mpic)
*         d0                          a0    d0    d1
*
*       LONG ReadBookMark (void *, ULONG, ULONG);
*
*   FUNCTION
*       Opens the bookmark.device using the supplied MPIC, reads 'size'
*       bytes into the buffer pointed to by 'data', and closes the device.
*
*   INPUTS
*       data    - buffer in which to store the data.
*       size    - size of the buffer in bytes.
*       mpic    - MPIC (Manufacturer and Product ID Code)
*
*   RESULT
*       actuallength - the number of bytes read into buffer or a zero if
*                      an error occured.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       WriteBookMark()
*
*****************************************************************************
*/

__asm LIBReadBookMark(register __a0 void *data, register __d0 ULONG size,
                      register __d1 ULONG mpic, register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

    return( stdbookrequest( mpic, 0, CMD_READ, 0, size, data,PlayerPrefsBase));
    }



/****** playerprefs.library/WriteBookMark **********************************
*
*   NAME
*       WriteBookMark -- Writes a buffer to a bookmark.
*
*   SYNOPSIS
*       actuallength = WriteBookMark (data, size, mpic, pri)
*         d0                           a0    d0    d1   d2:8
*
*       LONG WriteBookMark (void *, ULONG, ULONG, BYTE);
*
*   FUNCTION
*       This routine opens the bookmark.device using the supplied MPIC,
*       writes 'size' bytes to the bookmark from the buffer pointed to by
*       'data', and closes the device.
*
*       If no bookmark is found having the supplied MPIC, the routine
*       attempts to create one and write to it as above.
*
*       'Pri' is the priority of the bookmark.  Unless you Know To Do
*       Otherwise, you should always supply a value of zero.
*
*   INPUTS
*       data    - buffer containing data to be written.
*       size    - size of the buffer in bytes.
*       mpic    - MPIC (Manufacturer and Product ID Code)
*       pri     - priority of the bookmark.
*
*   RESULT
*       actuallength    - number of bytes actually written to the bookmark.
*                         A result of zero means that an error has occured
*                         and the bookmark is not modified, or not created
*                         if it didn't exist.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       ReadBookMark(), DeleteBookMark()
*
*****************************************************************************
*/
__asm LIBWriteBookMark(register __a0 void *data, register __d0 ULONG size, register __d1 ULONG mpic,
                       register __d2 BYTE pri, register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

register int err;

    if (!(err = stdbookrequest(mpic, pri, CMD_WRITE, 0, size, data, PlayerPrefsBase))) {

        if(err = stdbookrequest(mpic, pri, BD_CREATE, mpic, size, data, PlayerPrefsBase)) return(0);

        if(!(err = stdbookrequest(mpic, pri, CMD_WRITE, 0, size, data, PlayerPrefsBase)))               /* Delete bookmark if an error occurred */
            stdbookrequest( mpic, 0, BD_DELETE,0,0,0, PlayerPrefsBase );
        }

    return( err );
    }


/****** playerprefs.library/DeleteBookMark **********************************
*
*   NAME
*       DeleteBookMark -- Deletes a bookmark.
*
*   SYNOPSIS
*       error = DeleteBookMark (mpic)
*         d0                     d0
*
*       LONG DeleteBookMark (ULONG);
*
*   FUNCTION
*       Opens the bookmark.device, attempts to delete the bookmark having
*       the supplied MPIC, and closes the device.
*
*   INPUTS
*       mpic    - bookmark MPIC to be deleted.
*
*   RESULT
*       error   - non-zero if an error occurred.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       WriteBookMark(), cdtv/bookmark.h
*
*****************************************************************************
*/
__asm LIBDeleteBookMark(register __d0 ULONG mpic, register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

    return(stdbookrequest(mpic, 0, BD_DELETE, 0, 0, 0,PlayerPrefsBase));
    }



/*======================= WRITE PREFS =============================*/


/****** playerprefs.library/SaveCDTVPrefs **********************************
*
*   NAME
*       SaveCDTVPrefs -- Save the CDTVPrefs in the bookmark.
*
*   SYNOPSIS
*       success = SaveCDTVPrefs (cdtvprefs)
*         d0                        a0
*
*       LONG SaveCDTVPrefs (struct CDTVPrefs *);
*
*   FUNCTION
*       Attempts to save the contents of 'cdtvprefs' to the cdtvprefs
*       bookmark.  
*               
*       This function also attempts to set the battclock mode to AM/PM
*       or 24-hour mode (based on the flag).
*
*   INPUTS
*       cdtvprefs - pointer to a CDTVPrefs structure.
*
*   RESULT
*       success - nonzero if successful.
*
*   EXAMPLE
*
*   NOTES
*       It is recommended that you let the system's built-in preferences
*       editor be the only thing that writes to the global preferences
*       structure, and not mess about with it yourself.
*
*   BUGS
*
*   SEE ALSO
*       FillCDTVPrefs(), cdtv/cdtvprefs.h
*
*****************************************************************************
*/

__asm LIBSaveCDTVPrefs(register __a0 struct CDTVPrefs *c, register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

extern char __far        BattClock_Name[];

    if (WriteBookMark( c,sizeof(struct CDTVPrefs),BID_CDTVPREFS,127 ) < sizeof(struct CDTVPrefs)) return(0);

    return(1);
    }



/*-----------------------------------------------------------------------*/


/****** playerprefs.library/FillCDTVPrefs **********************************
*
*   NAME
*       FillCDTVPrefs -- Fill a CDTVPrefs structure.
*
*   SYNOPSIS
*       FillCDTVPrefs (cdtvprefs)
*                         a0
*
*       VOID FillCDTVPrefs (struct CDTVPrefs *);
*
*   FUNCTION
*       Fills the CDTVPrefs structure with the contents of the cdtvprefs
*       bookmark.  If the bookmark.device cannot be found, or the cdtvprefs
*       bookmark does not yet exist, or it can't be read, then the system
*       will fill the bookmark and your structure with a set of defaults.
*
*   INPUTS
*       cdtvprefs - a pointer to a cdtvprefs structure to be filled.
*
*   EXAMPLE
*
*   NOTES
*       The default language setting for PAL and NTSC machines is different
*       (American english for NTSC, British english for PAL).
*
*   BUGS
*
*   SEE ALSO
*       ConfigureCDTV(), cdtv/cdtvprefs.h
*
*****************************************************************************
*/

VOID __asm LIBFillCDTVPrefs(register __a0 struct CDTVPrefs *cprefs, register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

struct View view;                                   /* to get defaults */

    if (!ReadBookMark(cprefs, sizeof(struct CDTVPrefs), BID_CDTVPREFS)) {                                                               /* Use the default structure - This should be desirable values! */

        CopyMem(&DefaultCDTVPrefs, cprefs, sizeof (struct CDTVPrefs));

        if (PlayerPrefsBase->ppb_GfxBase->DisplayFlags & PAL) cprefs->Language = CDTVLANG_DEFAULT_PAL;

        InitView(&view);                                                                                                                /* Get graphic library center pos */

        cprefs->DisplayX = view.DxOffset;
        cprefs->DisplayY = view.DyOffset;
 
        SaveCDTVPrefs(cprefs);                                                                                                          /* once gotten - it will be saved */

        }
    }



/****** playerprefs.library/ConfigureCDTV **********************************
*
*   NAME
*       ConfigureCDTV -- Inform system "convenience" tasks about changes to
*                        CDTVPrefs
*
*   SYNOPSIS
*       ConfigureCDTV (cdtvprefs)
*                         d0
*
*       VOID ConfigureCDTV (struct CDTVPrefs *);
*
*   FUNCTION
*       This routine deals with changes made to certain items in CDTVPrefs.
*       In particular, it updates the system screen saver's delay time,
*       disables/enables the system key clicker, and disables/enables
*       the global LACE setting.
*
*       Neither the key clicker or the screen saver task are started by this
*       routine; it merely updates their internal state if they are already
*       running.
*
*       This routine should be called if a program modifies CDTVPrefs.
*
*       If CDTVPrefs is NULL it will get the CDTVPrefs from the bookmark.
*
*   INPUTS
*       cdtvprefs - pointer to a CDTVPrefs structure or NULL.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       FillCDTVPrefs(), ScreenSaverCommand(), KeyClickCommand(),
*       cdtv/cdtvprefs.h
*
*****************************************************************************
*/

VOID __asm LIBConfigureCDTV(register __a0 struct CDTVPrefs *cp, register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

extern struct Custom __far  custom;
struct CDTVPrefs            cdtvprefs;

    if (!cp) {

        FillCDTVPrefs( &cdtvprefs );
        cp = &cdtvprefs;
        }

    if (cp->Flags & CDTVPF_LACE) GfxBase->system_bplcon0 |= LACE;
    else                         GfxBase->system_bplcon0 &= ~(LACE);

    ScreenSaverCommand((SCRSAV_TIMECMD | (cp->SaverTime * 60)));

    KeyClickCommand( CLKCMD_DISABLE, 0, !(cp->Flags & CDTVPF_KEYCLICK),0,0,0);
    }





/****** No Prefs ************************************************************/

LONG __asm DoPrefs(register __a6 struct PlayerPrefsBase *ppb) {

    return(0L);
    }





