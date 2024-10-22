/* classbase.c
 *
 */

#include "classbase.h"
#include "classdata.h"

/*****************************************************************************/

#define	DB(x)	;

/****** animation.datatype/animation.datatype ****************************************
*
*    NAME
*	animation.datatype -- root data type for animations.
*
*    FUNCTION
*	The animation.datatype is the super-class for any animation related
*	classes.
*
*	This class is responsible for creating the controls, scaling,
*	remapping and synchronization.
*
*    METHODS
*	OM_NEW -- Create a new animation object.
*
*	OM_GET -- Obtain the value of an attribute.
*
*	OM_SET -- Set the values of multiple attributes.
*
*	OM_UPDATE -- Update the values of multiple attributes.
*
*	OM_DISPOSE -- Dispose of a animation object.
*
*	GM_LAYOUT -- Layout the object and notify the application of the
*	    title and size.
*
*	GM_HITTEST -- Determine if the object has been hit with the
*	    mouse.
*
*	GM_GOACTIVE -- Tell the object to go active.  On SELECTDOWN, the
*	    animation will start playing.
*
*	GM_HANDLEINPUT -- Handle input.  Currently input (other than
*	    SELECTDOWN) doesn't affect the animation.
*
*	GM_RENDER -- Cause the current frame to render.
*
*	DTM_FRAMEBOX -- Obtain the display environment that the animation
*	    requires.
*
*	DTM_TRIGGER -- Cause an event to occur.  Currently the only
*	    trigger event is STM_PLAY, which will cause the animation to start
*	    playing.
*
*	DTM_COPY -- Copy the current frame to the clipboard as an IFF ILBM.
*
*	DTM_WRITE -- Write the current frame to a file as an IFF ILBM.
*
*	DTM_PRINT -- Print the current frame.
*
*	ADTM_LOADFRAME -- Load a frame of the animation.
*
*	ADTM_UNLOADFRAME -- Deallocate any memory allocated by ADTM_LOADFRAME.
*
*	ADTM_START -- Start the animation.  This MUST be passed to the
*	    super-class AFTER the sub-class has started.
*
*	ADTM_PAUSE -- Pause the animation.  This MUST be passed to the
*	    super-class BEFORE the sub-class pauses.
*
*	ADTM_STOP -- Stop the animation.  This MUST be passed to the
*	    super-class BEFORE the sub-class stops.
*
*	ADTM_LOCATE -- Used to locate a frame of the animation.
*
*    TAGS
*	DTA_ControlPanel (BOOL) -- Determine whether the control
*	    panel is shown.  Defaults to TRUE.
*
*	    Applicability is (I).
*
*	DTA_Immediate (BOOL) -- Indicate whether the animation
*	    should immediately begin playing.  Defaults to
*	    FALSE.
*
*	    Applicability is (I).
*
*	ADTA_Remap (BOOL) -- Indicate whether the animation should be
*	    remapped or not.
*
*	    Applicability is (I).
*
*	ADTA_ModeID (ULONG) -- Set and get the graphic mode id of the
*	    picture.
*
*	    Applicability is (ISG).
*
*	ADTA_Width (ULONG) -- Width of a frame in pixels.
*
*	    Applicability is (IG).
*
*	ADTA_Height (ULONG) -- Height of a frame in pixels.
*
*	    Applicability is (IG).
*
*	ADTA_Depth (ULONG) -- Depth of the frame.
*
*	    Applicability is (IG).
*
*	ADTA_Frames (ULONG) -- Number of frames in animation.
*
*	    Applicability is (ISG).
*
*	ADTA_KeyFrame (struct BitMap *) -- Pointer to the key
*	    frame.
*
*	    Applicability is (ISG).
*
*	ADTA_FramesPerSecond (ULONG) -- Number of frames per
*	    second to play.
*
*	ADTA_NumColors (WORD) -- Number of colors used by the picture.
*
*	    Applicability is (ISG).
*
*	ADTA_ColorRegisters (struct ColorRegister *) -- Color table.
*
*	    Applicability is (G).
*
*	ADTA_CRegs (ULONG *) -- Color table to use with SetRGB32CM().
*
*	    Applicability is (G).
*
*	ADTA_GRegs (ULONG *) -- Color table.
*
*	    Applicability is (G).
*
*	ADTA_ColorTable (ULONG *) -- Shared pen table.
*
*	    Applicability is (G).
*
*	ADTA_ColorTable2 (ULONG *) -- Shared pen table.
*
*	    Applicability is (G).
*
*	ADTA_Allocated (ULONG) --  Number of shared colors allocated.
*
*	    Applicability is (G).
*
*	ADTA_NumAlloc (WORD) -- Number of colors allocated by the picture.
*
*	    Applicability is (G).
*
*	ADTA_BitMapHeader (struct BitMapHeader *) -- Set and get the
*	    base information for the animation.  BitMapHeader is defined in
*	    <datatypes/pictureclass.h>
*
*	    Applicability is (G).
*
*	SDTA_Sample (BYTE *) -- Pointer to sample data.
*
*	    Applicability is (ISG).
*
*	SDTA_SampleLength (ULONG) -- Length of sample data.
*
*	    Applicability is (ISG).
*
*	SDTA_Period (ULONG) -- Period to play back sample at.
*
*	    Applicability is (ISG).
*
*	SDTA_Volume (ULONG) -- Volume to play back sample at.
*
*	    Applicability is (ISG).
*
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

/****o* animation.datatype/ObtainAnimationClass **************************************
*
*    NAME
*	ObtainAnimationClass - Obtain the pointer to the animation class.
*
*    SYNOPSIS
*	class = ObtainAnimationClass ();
*	d0
*
*	Class *ObtainAnimationClass (void);
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

Class *ASM ObtainAnimationEngine (REG (a6) struct ClassBase *cb)
{
    return (cb->cb_Class);
}

/*****************************************************************************/

Class *initClass (struct ClassBase * cb)
{
    Class *cl;

    if (cl = MakeClass (ANIMATIONDTCLASS, DATATYPESCLASS, NULL, sizeof (struct localData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = ((ULONG (* ASM)())(Dispatch));
	cl->cl_UserData = (ULONG) cb;
	AddClass (cl);
    }

    return (cl);
}

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct ClassBase *cb, REG (a0) BPTR seglist, REG (a6) struct Library * sysbase)
{
    cb->cb_SegList = seglist;
    SysBase = (struct ExecBase *) sysbase;
    InitSemaphore (&cb->cb_Lock);

    if (((struct Library *)SysBase)->lib_Version >= 39)
    {
	cb->cb_IntuitionBase = OpenLibrary ("intuition.library",39);
	cb->cb_GfxBase       = OpenLibrary ("graphics.library", 39);
	cb->cb_DOSBase       = OpenLibrary ("dos.library",      39);
	cb->cb_UtilityBase   = OpenLibrary ("utility.library",  39);
	cb->cb_LayersBase    = OpenLibrary ("layers.library",   39);

	return ((struct Library *) cb);
    }
    else
    {
	return (NULL);
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
	    if (IFFParseBase = OpenLibrary ("iffparse.library", 0))
		if (DataTypesBase = OpenLibrary ("datatypes.library", 0))
		    if (RealTimeBase = OpenLibrary ("realtime.library", 0))
			if (cb->cb_TapeDeck = OpenLibrary ("gadgets/tapedeck.gadget", 0))
			    if (cb->cb_Class = initClass (cb))
				success = TRUE;
	}
    }

    if (!success)
    {
	CloseLibrary (cb->cb_TapeDeck);
	CloseLibrary ((struct Library *) RealTimeBase);
	CloseLibrary (DataTypesBase);
	CloseLibrary (IFFParseBase);
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
	    CloseLibrary (cb->cb_TapeDeck);
	    CloseLibrary ((struct Library *) RealTimeBase);
	    CloseLibrary (DataTypesBase);
	    CloseLibrary (IFFParseBase);
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

    CloseLibrary (cb->cb_LayersBase);
    CloseLibrary (cb->cb_UtilityBase);
    CloseLibrary (cb->cb_DOSBase);
    CloseLibrary (cb->cb_GfxBase);
    CloseLibrary (cb->cb_IntuitionBase);

    FreeMem ((APTR)((ULONG)(cb) - (ULONG)(cb->cb_Lib.lib_NegSize)), cb->cb_Lib.lib_NegSize + cb->cb_Lib.lib_PosSize);

    return ((LONG) seg);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

struct Gadget *newobject (struct ClassBase * cb, APTR name, Tag tag1,...)
{
    return ((struct Gadget *) NewObjectA (NULL, name, (struct TagItem *) & tag1));
}

/*****************************************************************************/

ULONG setattrs (struct ClassBase * cb, struct Gadget *g, Tag tag1,...)
{
    return (SetAttrsA (g, (struct TagItem *) & tag1));
}

/*****************************************************************************/

ULONG setgadgetattrs (struct ClassBase * cb, struct Gadget *g, struct Window *w, Tag tag1,...)
{
    return (SetGadgetAttrsA (g, w, NULL, (struct TagItem *) & tag1));
}

/*****************************************************************************/

Object *newdtobject (struct ClassBase * cb, APTR name, Tag tag1,...)
{
    return (NewDTObjectA (name, (struct TagItem *) & tag1));
}

/*****************************************************************************/

ULONG setdtattrs (struct ClassBase * cb, Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

ULONG notifyAttrChanges (struct ClassBase * cb, Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

struct Process *createnewproc (struct ClassBase * cb, Tag tag1,...)
{
    return (CreateNewProc ((struct TagItem *) & tag1));
}

/*****************************************************************************/

struct Player *createplayer (struct ClassBase * cb, Tag tag,...)
{
    return CreatePlayerA ((struct TagItem *) & tag);
}

/*****************************************************************************/

ULONG bestmodeid (struct ClassBase * cb, ULONG data,...)
{
    return BestModeIDA ((struct TagItem *) & data);
}

/*****************************************************************************/

struct Region *installclipregion (struct ClassBase *cb, struct Window *w, struct Layer *l, struct Region *r)
{
    BOOL refresh = FALSE;
    struct Region *or;

    if (w->Flags & WINDOWREFRESH)
    {
	EndRefresh (w, FALSE);
	refresh = TRUE;
    }
    or = InstallClipRegion (l, r);
    if (refresh)
	BeginRefresh (w);
    return (or);
}

