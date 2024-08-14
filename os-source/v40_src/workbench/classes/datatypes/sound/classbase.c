/* classbase.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

#define	DB(x)	;

/****** sound.datatype/sound.datatype ****************************************
*
*    NAME
*	sound.datatype -- root data type for sounds.
*
*    FUNCTION
*	The sound.datatype is the super-class for any sound related
*	classes.
*
*    METHODS
*	OM_NEW -- Create a new sound object.
*
*	OM_GET -- Obtain the value of an attribute.
*
*	OM_SET -- Set the values of multiple attributes.
*
*	OM_UPDATE -- Update the values of multiple attributes.
*
*	OM_DISPOSE -- Dispose of a sound object.
*
*	GM_LAYOUT -- Layout the object and notify the application of the
*	    title and size.
*
*	GM_HITTEST -- Determine if the object has been hit with the
*	    mouse.
*
*	GM_GOACTIVE -- Tell the object to go active.  On SELECTDOWN, the
*	    sound will start playing.
*
*	GM_HANDLEINPUT -- Handle input.  Currently input (other than
*	    SELECTDOWN) doesn't affect the sound.
*
*	GM_RENDER -- Cause the graphic to render.  Currently the graphic
*	    for the sound is just a static icon.
*
*	DTM_TRIGGER -- Cause an event to occur.  Currently the only
*	    trigger event is STM_PLAY, which will cause the sound to start
*	    playing.
*
*	DTM_COPY -- Copy the entire sound to the clipboard as 8SVX.
*
*	DTM_WRITE -- Write the entire sound to a file as 8SVX.
*
*    TAGS
*	SDTA_VoiceHeader (struct VoiceHeader *) -- Set and get the base
*	    information for the sound.  VoiceHeader is defined in
*	    <datatypes/soundclass.h>.
*
*	    Applicability is (ISG).
*
*	SDTA_Sample (UWORD *) -- Set and get the sound data.  Starting
*	    V40 the sample data does not need to be in CHIP memory.
*
*	    Applicability is (ISG).
*
*	SDTA_SampleLength (ULONG) -- Length of the sound data.
*
*	    Applicability is (ISG).
*
*	SDTA_Period (UWORD) -- Set and get the period of the sound.
*	    This attribute can be used to affect a playing sound.
*
*	    Default for this tag is 394.  Applicability is (ISG).
*
*	SDTA_Volume (UWORD) -- Set and get the volume of the sound. This
*	    attribute can be used to affect a playing sound.
*
*	    Valid range is from 0 to 64.  Default for this tag is 64.
*	    Applicability is (ISG).
*
*	SDTA_Cycles (UWORD) -- Set and get the number of cycles the
*	    sound will be played.
*
*	    Default for this tag is 1.  Applicability is (ISG).
*
*	The following tags are new for V40.
*
*	SDTA_SignalTask (struct Task *) -- Task to signal when the
*	    is complete, or if SDTA_Continuous is TRUE, when
*	    the next buffer is needed.
*
*	SDTA_SignalBit (BYTE) -- Signal bit to use with SDTA_SignalTask
*	    or -1 to disable.
*
*	SDTA_Continuous (BOOL) -- Used to indicate that the sound
*	    datatype will be fed a continuous stream of data.  Defaults
*	    to FALSE.
*
*    BUGS
*	Still doesn't support stereo or channel selection.
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

/****o* sound.datatype/ObtainSoundClass **************************************
*
*    NAME
*	ObtainSoundClass - Obtain the pointer to the sound class.
*
*    SYNOPSIS
*	class = ObtainSoundClass ();
*	d0
*
*	Class *ObtainSoundClass (void);
*
*    FUNCTION
*	This function is used to obtain the pointer to the class
*	structure for this public BOOPSI class.
*
*    RETURNS
*	Returns a pointer to a valid public BOOPSI Class pointer.
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

Class *ASM ObtainSoundEngine (REG (a6) struct ClassBase *cb)
{
    return (cb->cb_Class);
}

/*****************************************************************************/

/* periods for scale starting at   65.40Hz (C) with 128 samples per cycle
 *                            or  130.81Hz (C) with  64 samples per cycle
 *                            or  261.63Hz (C) with  32 samples per cycle
 *                            or  523.25Hz (C) with  16 samples per cycle
 *                            or 1046.50Hz (C) with   8 samples per cycle
 *                            or 2093.00Hz (C) with   4 samples per cycle
 */

static UWORD per_ntsc[12] =
{
    428, 404, 380, 360,
    340, 320, 302, 286,
    270, 254, 240, 226
};

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct ClassBase *cb, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    register ULONG i;

    cb->cb_SegList = seglist;
    SysBase = (struct ExecBase *) sysbase;
    InitSemaphore (&cb->cb_Lock);
    if (((struct Library *)SysBase)->lib_Version >= 39)
    {
	cb->cb_IntuitionBase = OpenLibrary ("intuition.library",39);
	cb->cb_GfxBase       = OpenLibrary ("graphics.library", 39);
	cb->cb_DOSBase       = OpenLibrary ("dos.library",      39);
	cb->cb_UtilityBase   = OpenLibrary ("utility.library",  39);

	/* Get the system clock rate */
	if (cb->cb_GfxBase->DisplayFlags & REALLY_PAL)
	    cb->cb_TClock = PAL_CLOCK;
	else
	    cb->cb_TClock = NTSC_CLOCK;

	/* Compute the periods */
	for (i = 0; i < 12; i++)
	    cb->cb_Period[i] = ((per_ntsc[i] * cb->cb_TClock) + (NTSC_CLOCK >> 1)) / NTSC_CLOCK;

	return cb;
    }
    else
    {
	return NULL;
    }
}

/*****************************************************************************/

LONG ASM LibOpen (REG (a6) struct ClassBase *cb)
{
    LONG retval = (LONG) cb;
    BOOL success = TRUE;

    ObtainSemaphore (&cb->cb_Lock);

    /* Use an internal use counter */
    cb->cb_Lib.lib_OpenCnt++;
    cb->cb_Lib.lib_Flags &= ~LIBF_DELEXP;

    if (cb->cb_Lib.lib_OpenCnt == 1)
    {
	if (cb->cb_Class == NULL)
	{
	    success = FALSE;
	    if (cb->cb_IFFParseBase = OpenLibrary ("iffparse.library", 0))
		if (cb->cb_DataTypesBase = OpenLibrary ("datatypes.library", 0))
		     if (cb->cb_Class = initClass (cb))
			success = TRUE;
	}
    }

    if (!success)
    {
	CloseLibrary (cb->cb_DataTypesBase);
	CloseLibrary (cb->cb_IFFParseBase);
	cb->cb_Lib.lib_OpenCnt--;
	retval = NULL;
    }

    ReleaseSemaphore (&cb->cb_Lock);

    return (retval);
}

/*****************************************************************************/

LONG ASM LibClose (REG (a6) struct ClassBase *cb)
{
    LONG retval = NULL;

    ObtainSemaphore (&cb->cb_Lock);

    if (cb->cb_Lib.lib_OpenCnt)
	cb->cb_Lib.lib_OpenCnt--;

    if ((cb->cb_Lib.lib_OpenCnt == 0) && cb->cb_Class)
    {
	if (FreeClass (cb->cb_Class))
	{
	    cb->cb_Class = NULL;
	    CloseLibrary (cb->cb_SuperClassBase);
	    CloseLibrary (cb->cb_DataTypesBase);
	    CloseLibrary (cb->cb_IFFParseBase);
	}
	else
	{
	    cb->cb_Lib.lib_Flags |= LIBF_DELEXP;
	}
    }

    if (cb->cb_Lib.lib_Flags & LIBF_DELEXP)
	retval = LibExpunge (cb);

    ReleaseSemaphore (&cb->cb_Lock);

    return (retval);
}

/*****************************************************************************/

LONG ASM LibExpunge (REG (a6) struct ClassBase *cb)
{
    BPTR seg = cb->cb_SegList;

    if (cb->cb_Lib.lib_OpenCnt)
    {
	cb->cb_Lib.lib_Flags |= LIBF_DELEXP;
	return (NULL);
    }

    Remove ((struct Node *) cb);

    CloseLibrary (cb->cb_UtilityBase);
    CloseLibrary (cb->cb_DOSBase);
    CloseLibrary (cb->cb_GfxBase);
    CloseLibrary (cb->cb_IntuitionBase);

    FreeMem ((APTR)((ULONG)(cb) - (ULONG)(cb->cb_Lib.lib_NegSize)), cb->cb_Lib.lib_NegSize + cb->cb_Lib.lib_PosSize);

    return ((LONG) seg);
}
