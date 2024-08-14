/************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                       - Talin & Joe Pearce                            *
*                                                                       *
* Copyright 1990 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
*
* misc.c      - Misc internal functions
*
************************************************************************/

#include <exec/execbase.h>

#include "camdlib.h"

#if OS_MIN < 36
  #include <graphics/gfxbase.h>   /* for ntsc/pal detection */
#endif


#if OS_MIN < 36
/* -------------------- EClock frequency constants for 1.3 */

#define ECLK_NTSC   715909L
#define ECLK_PAL    709379L

#endif


/* -------------------- internal functions */

#if OS_MIN < 36
  static BOOL IsPAL (void);
#endif


/* -------------------- code */

/***************************************************************
*
*   EClockFreq
*
*   FUNCTION
*       Returns EClock frequency in Hz.
*
*   INPUTS
*       None
*
*   RESULTS
*       EClock frequency in Hz.  Most likely:
*
*           NTSC: 715909
*            PAL: 709379
*
*       For 1.3, these numbers are constants and an attempt
*       is made to determine the nationality of the machine.
*       For 2.0, this value is returned from ExecBase.
*
***************************************************************/

ULONG EClockFreq (void)
{
    register struct ExecBase *sysbase = (struct ExecBase *)CamdBase->SysBase;

    return
          #if OS_MIN < 36
            sysbase->LibNode.lib_Version < 36 ? IsPAL() ? ECLK_PAL : ECLK_NTSC :
          #endif
            sysbase->ex_EClockFrequency;
}


/***************************************************************
*
*   SetErrorCode
*
*   FUNCTION
*       Set error code buffer pointed to by a tag (e.g.
*       MLINK_ErrorCode).
*
*   INPUTS
*       Tag - Error code tag.
*       TagList - Tag list to search.
*       error - Error code.
*
*   RESULTS
*       Places error in buffer pointed to by error tag.
*       Nothing happens if tag not found.
*
***************************************************************/

void SetErrorCode (Tag tagval, struct TagItem *taglist, ULONG error)
{
    ULONG *errbuf;

    if (errbuf = (ULONG *)get_tagdata (tagval, NULL, taglist)) *errbuf = error;
}


#if OS_MIN < 36

/***************************************************************
*
*   IsPAL
*
*   FUNCTION
*       Determines if machine is running as PAL or NTSC.
*
*   INPUTS
*       None
*
*   RESULTS
*       TRUE if PAL, FALSE if NTSC.
*
*   BUGS
*       doesn't currently check for NTSC bit being set.
*
***************************************************************/

static
BOOL IsPAL (void)
{
    register struct GfxBase *gfx;
    BOOL rtn = FALSE;

    if (gfx = (void *)OpenLibrary ("graphics.library", 0L)) {
        rtn = gfx->DisplayFlags & PAL;
        CloseLibrary ((struct Library *)gfx);
    }

    return rtn;
 }


/***************************************************************
*
*   next_tag()/find_tag()
*
*   FUNCTION
*       1.3 safe version of tag functions NextTagItem(),
*       FindTagItem(), and GetTagData()
*
*   INPUTS
*       same as utility.library functions.
*
*   RESULTS
*       same as utility.library functions.
*
***************************************************************/

struct TagItem *next_tag (struct TagItem **ti)
{
    if (UtilityBase) {
        return NextTagItem (ti);
    }
    else {
        struct TagItem *t;

	if (ti == NULL || (t = *ti) == NULL) return NULL;

        for (;;) switch (t->ti_Tag) {
            case TAG_DONE:
                    return NULL;

            case TAG_IGNORE:
                    t++;
                    break;

            case TAG_SKIP:
                    t += t->ti_Data + 1;
                    break;

            case TAG_MORE:
                    if ( (t = (struct TagItem *)t->ti_Data) == NULL )
			return NULL;
                    break;

            default:
                    *ti = t + 1;
                    return t;
        }
    }
}

struct TagItem *find_tag (struct TagItem *ti, Tag tval)
{
    if (UtilityBase) {
        return FindTagItem (tval, ti);
    }
    else {
        struct TagItem *tlist = ti;

        while (ti = next_tag (&tlist))
            if (ti->ti_Tag == tval) return ti;

        return NULL;
    }
}

ULONG get_tagdata (Tag tval, ULONG defaultval, struct TagItem *ti)
{
    if (UtilityBase) {
        return GetTagData (tval, defaultval, ti);
    }
    else {
        return (ti = find_tag (ti, tval)) ? ti->ti_Data : defaultval;
    }
}

#endif
