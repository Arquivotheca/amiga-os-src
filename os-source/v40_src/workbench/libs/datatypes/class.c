/* class.c
 *
 */

#include "datatypesbase.h"

#include <dos/dos.h>
#include <dos/dostags.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>

#include <clib/dtclass_protos.h>

#include <pragmas/dtclass_pragmas.h>

/*****************************************************************************/

#define	G(o)	((struct Gadget *)(o))

/*****************************************************************************/

/* Magic number used to determine how long to wait around for something to finish */
#define	MAGIC_DELAY	50

/*****************************************************************************/

static struct Library *openclasslibrary (struct DataTypesLib *dtl, STRPTR libname, ULONG version)
{
    UBYTE buffer[128];

    sprintf (buffer, "datatypes/%s.datatype", libname);
    return OpenLibrary (buffer, version);
}

/****** datatypes.library/NewDTObjectA ****************************************
*
*    NAME
*	NewDTObjectA - Create an data type object.              (V39)
*
*    SYNOPSIS
*	o = NewDTObjectA (name, attrs);
*	d0		  d0   a0
*
*	Object *NewDTObjectA (APTR, struct TagItem *);
*
*	o = NewDTObject (name, tag1, ...);
*
*	Object *NewDTObject (APTR, Tag tag1, ...);
*
*    FUNCTION
*	This is the method for creating datatype objects from
*	'boopsi' classes.  Boopsi' stands for "basic object-oriented
*	programming system for Intuition".)
*
*	You further specify initial "create-time" attributes for the
*	object via a TagItem list, and they are applied to the
*	resulting datatype object that is returned.
*
*    INPUTS
*	name - Name of the data source.  Usually an existing file name.
*
*	attrs - Pointer to a taglist containing additional arguments.
*
*    TAGS
*	DTA_SourceType - Specify the type of source data; such as coming
*	    from a file or clipboard (defaults to DTST_FILE).  If
*	    source type is clipboard, then the name field contains the
*	    numeric clipboard unit.
*
*	DTA_Handle - Can optionally be used instead of the name field.
*	    Must be a valid FileHandle if DTA_SourceType is DTST_FILE.
*	    Must be a valid IFFHandle if DTA_SourceType is DTST_CLIPBOARD.
*
*	DTA_DataType - Specify the class of data.  Data is a pointer to a
*	    valid DataType.  This is only used when attempting to create
*	    a new object that doesn't have any source data.
*
*	DTA_GroupID - Specify that the object must be of this type, or
*	    NewDTObject() will fail with IoErr() of
*	    ERROR_OBJECT_WRONG_TYPE.
*
*	GA_Left, GA_RelRight, GA_Top, GA_RelBottom, GA_Width, GA_RelWidth,
*	GA_Height, GA_RelHeight - Specify the placement of the object
*	    within the destination window.
*
*	GA_ID - Specify the object ID.
*
*	GA_UserData - Specify the application specific data for the
*	    object.
*
*
*    RETURNS
*	A boopsi object, which may be used in different contexts such
*	as a gadget or image, and may be manipulated by generic functions.
*	You eventually free the object using DisposeDTObject().
*
*	A NULL return indicates failure.  Use IoErr() to get error value.
*	Following is a summary of the error number used and there meaning
*	as it relates to DataTypes.
*
*	ERROR_REQUIRED_ARG_MISSING - Indicates that a required attribute
*	    wasn't passed in.
*	ERROR_BAD_NUMBER - An invalid group ID was passed in.
*	ERROR_OBJECT_WRONG_TYPE - Object data type doesn't match
*	    DTA_GroupID.
*	ERROR_NO_FREE_STORE - Not enough memory.
*	DTERROR_UNKNOWN_DATATYPE - Unable to open the class library
*	    associated with the data type.
*	DTERROR_COULDNT_OPEN - Unable to open the data object.
*	ERROR_NOT_IMPLEMENTED - Unknown handle type.
*
*    NOTES
*	This function invokes the OM_NEW "method" for the class specified.
*
*    EXAMPLE
*	STRPTR fileName = "s:startup-sequence"
*	Object *o;
*
*	\* Simplest use is to just open an existing file *\
*	if (o = NewDTObject ((APTR)fileName, NULL))
*	{
*	    \* Free the object when we are done with it *\
*	    DisposeDTObject (o);
*	}
*
*    SEE ALSO
*	AddDTObject(), DisposeDTObject(), RemoveDTObject(),
*	intuition.library/NewObjectA()
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

Object *ASM NewDTObjectA (REG (a6) struct DataTypesLib * dtl, REG (d0) APTR name, REG (a0) struct TagItem * attrs)
{
    struct IFFHandle *iff = NULL;
    struct Library *DTClassBase;
    struct DataType *dtn;
    STRPTR libname = NULL;
    BPTR lock = NULL;
    ULONG groupid;
    APTR handle;
    ULONG stype;
    Class *cl;
    Object *o;

    if (stype = (ULONG) GetTagData (DTA_SourceType, DTST_FILE, attrs))
    {
	dtn     = (struct DataType *) GetTagData (DTA_DataType,	NULL, attrs);
	handle  =              (APTR) GetTagData (DTA_Handle,	NULL, attrs);
	groupid =                     GetTagData (DTA_GroupID,	NULL, attrs);

	if ((stype == DTST_RAM) && groupid)
	{
	    switch (groupid)
	    {
		case GID_TEXT:
		    libname = "ascii";
		    break;

		case GID_DOCUMENT:
		    libname = "document";
		    break;

		case GID_SOUND:
		    libname = "sound";
		    break;

		case GID_INSTRUMENT:
		    libname = "instrument";
		    break;

		case GID_MUSIC:
		    libname = "music";
		    break;

		case GID_PICTURE:
		    libname = "picture";
		    break;

		case GID_ANIMATION:
		    libname = "animation";
		    break;

		case GID_MOVIE:
		    libname = "movie";
		    break;

		default:
		    SetIoErr (ERROR_BAD_NUMBER);
		    break;
	    }
	}
	else if (!dtn)
	{
	    if (handle)
	    {
		dtn = ObtainDataTypeA (dtl, stype, handle, attrs);
	    }
	    else
	    {
		switch (stype)
		{
		    case DTST_FILE:
			if (lock = Lock (name, ACCESS_READ))
			{
			    if (dtn = ObtainDataTypeA (dtl, stype, (APTR) lock, attrs))
			    {
				if (groupid && (dtn->dtn_Header->dth_GroupID != groupid))
				{
				    SetIoErr (ERROR_OBJECT_WRONG_TYPE);
				    ReleaseDataType (dtl, dtn);
				    dtn = NULL;
				}
				else
				{
				    handle = (APTR) lock;
				}
			    }
			}
			break;

		    case DTST_CLIPBOARD:
			if (iff = AllocIFF ())
			{
			    if (iff->iff_Stream = (ULONG) OpenClipboard ((LONG)name))
			    {
				InitIFFasClip (iff);
				if (OpenIFF (iff, IFFF_READ) == 0)
				{
				    if (dtn = ObtainDataTypeA (dtl, stype, (APTR) iff, attrs))
				    {
					if (groupid && (dtn->dtn_Header->dth_GroupID != groupid))
					{
					    ReleaseDataType (dtl, dtn);
					    SetIoErr (ERROR_OBJECT_WRONG_TYPE);
					    dtn = NULL;
					}
					else
					{
					    handle = (APTR) iff;
					    break;
					}
				    }
				    CloseClipIFF (dtl, iff);
				}
				CloseClipboard ((struct ClipboardHandle *) iff->iff_Stream);
			    }
			    FreeIFF (iff);
			    iff = NULL;
			}
			else
			    SetIoErr (ERROR_NO_FREE_STORE);
			break;
		}
	    }
	}

	if (dtn)
	    libname = dtn->dtn_Header->dth_BaseName;

	if (libname)
	{
	    if (DTClassBase = openclasslibrary (dtl, libname, 0))
	    {
		if (cl = ObtainEngine ())
		{
		    struct TagItem list[4];

		    list[0].ti_Tag  = DTA_Name;
		    list[0].ti_Data = (ULONG) name;
		    list[1].ti_Tag  = DTA_DataType;
		    list[1].ti_Data = (ULONG) dtn;
		    list[2].ti_Tag  = DTA_Handle;
		    list[2].ti_Data = (ULONG) handle;
		    list[3].ti_Tag  = TAG_MORE;
		    list[3].ti_Data = (ULONG) attrs;

		    if (o = NewObjectA (cl, NULL, list))
			return (o);

		    /* Dispose in the object will close these out */
		    lock = NULL;
		    iff = NULL;
		}
		CloseLibrary (DTClassBase);
	    }
	    else
		SetIoErr (DTERROR_UNKNOWN_DATATYPE);
	}
    }
    else
    {
	SetIoErr (ERROR_REQUIRED_ARG_MISSING);
    }

    if (lock)
	UnLock (lock);

    if (iff)
    {
	CloseClipIFF (dtl, iff);
	CloseClipboard ((struct ClipboardHandle *) iff->iff_Stream);
	FreeIFF (iff);
    }

    if (IoErr() == ERROR_OBJECT_NOT_FOUND)
	SetIoErr (DTERROR_COULDNT_OPEN);

    return (NULL);
}

/****** datatypes.library/DisposeDTObject *****************************************
*
*    NAME
*	DisposeDTObject - Delete a data type object.            (V39)
*
*    SYNOPSIS
*	DisposeDTObject (o);
*			 a0
*
*	VOID DisposeDTObject (Object *);
*
*    FUNCTION
*	This function is used to dispose of a data type object that was
*	obtained with NewDTObjectA().
*
*    INPUTS
*	o - Pointer to an object as returned by NewDTObjectA().
*	    NULL is a valid input.
*
*    SEE ALSO
*	NewDTObjectA()
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

VOID ASM DisposeDTObject (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o)
{
    if (o)
    {
	struct Library *DTClassBase;
	struct DTSpecialInfo *si;

	si = (struct DTSpecialInfo *) G (o)->SpecialInfo;

	while (si->si_Flags & (DTSIF_PRINTING | DTSIF_LAYOUTPROC))
	    Delay (MAGIC_DELAY);

	DTClassBase = (struct Library *)OCLASS(o)->cl_UserData;
	DisposeObject (o);
	CloseLibrary (DTClassBase);
    }

    return;
}

/****** datatypes.library/SetDTAttrsA *****************************************
*
*    NAME
*	SetDTAttrsA - Set attributes for an object.             (V39)
*
*    SYNOPSIS
*	retval = SetDTAttrsA (o, win, req, attrs);
*	d0		      a0  a1   a2   a3
*
*	ULONG SetDTAttrsA (Object *, struct Window *, struct Requester *,
*			   struct TagItem *);
*
*	retval = SetDTAttrs (o, win, req, tag1, ...);
*
*	ULONG SetDTAttrs (Object *, struct Window *, struct Requester *,
*			  Tag tag1, ...);
*
*    FUNCTION
*	This function is used to set the attributes of a data type
*	object.
*
*    INPUTS
*	o - Pointer to an object as returned by NewDTObjectA().
*
*	win - Window that the object has been added to.
*
*	attrs - Attributes to set, terminated with TAG_DONE.
*
*    TAGS
*	see <datatypes/datatypesclass.h> for tags.
*
*    SEE ALSO
*	GetDTAttrsA(), intuition.library/SetGadgetAttrsA()
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

ULONG ASM SetDTAttrsA (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o, REG (a1) struct Window *win, REG (a2) struct Requester * req, REG (a3) struct TagItem *attrs)
{
    return (SetGadgetAttrsA ((struct Gadget *)o, win, req, attrs));
}

/****** datatypes.library/GetDTAttrsA *****************************************
*
*    NAME
*	GetDTAttrsA - Obtain attributes for an object.          (V39)
*
*    SYNOPSIS
*	retval = GetDTAttrsA (o, attrs);
*	d0		      a0  a2
*
*	ULONG GetDTAttrsA (Object *, struct TagItem *);
*
*	retval = GetDTAttrs (o, tag1, ...);
*
*	ULONG GetDTAttrs (Object *, Tag tag1, ...);
*
*    FUNCTION
*	This function is used to get the attributes of a data type
*	object.
*
*    INPUTS
*	o - Pointer to an object as returned by NewDTObjectA().
*
*	attrs - Attributes to get, terminated with TAG_DONE.  The data
*	    element of each pair contains the address of the storage
*	    variable.
*
*    RETURNS
*	retval - Contains the number of attributes the system was able
*	    to obtain.
*
*    SEE ALSO
*	SetDTAttrsA(), intuition.library/GetAttr()
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

ULONG ASM GetDTAttrsA (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o, REG (a2) struct TagItem *attrs)
{
    struct TagItem *tstate = attrs;
    struct TagItem *tag;
    ULONG retval = 0L;
    struct opGet opg;

    opg.MethodID = OM_GET;
    while (tag = NextTagItem (&tstate))
    {
	opg.opg_AttrID = tag->ti_Tag;
	opg.opg_Storage = (ULONG *) tag->ti_Data;
	if (DoMethodA (o, (Msg) & opg))
	    retval++;
	else
	    *(opg.opg_Storage) = NULL;
    }
    return (retval);
}

/****** datatypes.library/AddDTObject *****************************************
*
*    NAME
*	AddDTObject - Add an object to a window or requester.   (V39)
*
*    SYNOPSIS
*	realposition = AddDTObject (window, requester, object, position);
*	d0			    a0      a1	       a2      d0
*
*	LONG AddDTObject (struct Window *, struct Requester *,
*			  Object *, LONG);
*
*    FUNCTION
*	This function adds a single object to the gadget list of the given
*	window or requester at the position in the list specified by the
*	position argument.
*
*	When the object is added to the gadget list, the object will
*	receive a GM_LAYOUT message with the gpl_Initial field set to
*	one.
*
*    INPUTS
*	window - Pointer to the window.
*
*	requester - Must be NULL.
*
*	object - Pointer to an object as returned by NewDTObjectA().
*
*	position - Integer position in the list for the new gadget.
*	    -1 to add to the end of the list.
*
*    RETURNS
*	Returns the position of where the object was actually added.
*
*    SEE ALSO
*	RemoveDTObject(), intuition.library/AddGList()
*
*******************************************************************************
*
* Created:  11-Mar-92, David N. Junod
*
*******************************************************************************
*
*	Used to do a DTM_FRAMEBOX with the FRAMEF_SPECIFY bit set
*/

LONG ASM AddDTObject (REG (a6) struct DataTypesLib *dtl, REG (a0) struct Window *win, REG (a1) struct Requester * req, REG (a2) Object * o, REG (d0) LONG pos)
{
    /* Add the object as a gadget */
    return ((LONG) AddGList (win, (struct Gadget *) o, pos, 1, req));
}

/****** datatypes.library/RefreshDTObjectA ************************************
*
*    NAME
*	RefreshDTObjectA - Refresh a datatypes object.          (V39)
*
*    SYNOPSIS
*	RefreshDTObjectA (object, window, req, attrs)
*			   a0	   a1	   a2	a3
*
*	VOID RefreshDTObjectA (Object *, struct Window *,
*			        struct Requester *, struct TagItem *);
*
*	RefreshDTObject (object, window, req, tag1, ...);
*
*	VOID RefreshDTObject (Object *, struct Window *,
*			       struct Requester *, Tag tag1, ...);
*
*    FUNCTION
*	Refreshes the specified object, by sending the GM_RENDER method to
*	the object.
*
*    INPUTS
*	object - Pointer to an object as returned by NewDTObjectA().
*
*	window - Pointer to the window.
*
*	req - Must be NULL.
*
*	attrs - Additional attributes (currently none are defined).
*
*    SEE ALSO
*	AddDTObject(), RemoveDTObject(), intuition.library/RefreshGList()
*
*******************************************************************************
*
* Created:  11-Mar-92, David N. Junod
*
*******************************************************************************
*
*	Used to do a DTM_LAYOUT also.
*
*/

VOID ASM RefreshDTObjectA (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o, REG (a1) struct Window *win, REG (a2) struct Requester *req, REG (a3) struct TagItem *attrs)
{
    /* Refresh the gadget list */
    RefreshGList ((struct Gadget *)o, win, req, 1);
}

/****** datatypes.library/RemoveDTObject ***************************************
*
*    NAME
*	RemoveDTObject - Remove an object from a window.        (V39)
*
*    SYNOPSIS
*	position = RemoveDTObject (window, object);
*	d0			    a0	    a1
*
*	LONG RemoveDTObject (struct Window *, Object *);
*
*    FUNCTION
*	Removes the object from the window's object list.  This will wait
*	until the AsyncLayout process is complete.  The object will
*	receive a DTM_REMOVEDTOBJECT message to inform the object it has
*	been removed.
*
*    INPUTS
*	window - Pointer to the window.
*
*	object - Pointer to an object as returned by NewDTObjectA().
*
*    RETURNS
*	Returns the ordinal position of the removed object.  If the
*	object wasn't found in the appropriate list then a -1 is
*	returned.
*
*    SEE ALSO
*	AddDTObject(), intuition.library/RemoveGList()
*
*******************************************************************************
*
* Created:  11-Mar-92, David N. Junod
*
*/

LONG ASM RemoveDTObject (REG (a6) struct DataTypesLib *dtl, REG (a0) struct Window *win, REG (a1) Object *o)
{
    LONG retval = 0;

    if (o)
    {
	struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;

	/* Can't remove object while it is still laying out.  Still a window for layout to
	 * to start before we actually remove the object. */
	while (si->si_Flags & (DTSIF_LAYOUTPROC))
	    Delay (MAGIC_DELAY);

	/* Tell the object it is being removed */
	DoMethod (o, DTM_REMOVEDTOBJECT, NULL);

	/* Remove the object */
	retval = (LONG) RemoveGList (win, (struct Gadget *)o, 1);
    }
    return (retval);
}

/****** datatypes.library/GetDTTriggerMethods *********************************
*
*    NAME
*	GetDTTriggerMethods - Obtain trigger methods supported by an object
*                                                               (V39)
*
*    SYNOPSIS
*	methods = GetDTTriggerMethods (object);
*	d0				a0
*
*	struct DTMethods *GetDTTriggerMethods (Object *);
*
*    FUNCTION
*	This function is used to obtain a list of trigger methods that an
*	object supports.  This is so that an application can provide
*	the appropriate controls for an object.  For example, an AmigaGuide
*	object needs controls for "Contents", "Index", "Retrace", "Browse <",
*	and "Browse >",
*
*    INPUTS
*	object - Pointer to an object as returned by NewDTObjectA().
*
*    RETURNS
*	Returns a pointer to a NULL terminated DTMethod list.  This list is
*	only valid until the object is disposed off.
*
*    EXAMPLE
*	To call the method:
*
*	    DoMethod (object, DTM_TRIGGER, dtm[button]->dtm_Method);
*
*    SEE ALSO
*	GetDTMethods()
*
*******************************************************************************
*
* Created:  11-Mar-92, David N. Junod
*
*/

struct DTMethods *ASM GetDTTriggerMethods (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o)
{
    struct DTMethods *dtm = NULL;
    struct opGet opg;

    opg.MethodID    = OM_GET;
    opg.opg_AttrID  = DTA_TriggerMethods;
    opg.opg_Storage = (ULONG *) & dtm;

    if (!DoMethodA (o, &opg))
	dtm = NULL;

    return (dtm);
}

/****** datatypes.library/GetDTMethods *********************************
*
*    NAME
*	GetDTMethods - Obtain methods an object supports.       (V39)
*
*    SYNOPSIS
*	methods = GetDTMethods (object);
*	d0			a0
*
*	ULONG GetDTMethods (Object *);
*
*    FUNCTION
*	This function is used to obtain a list of methods that an
*	object supports.
*
*    INPUTS
*	object - Pointer to an object as returned by NewDTObjectA().
*
*    RETURNS
*	Returns a pointer to a ~0 terminated ULONG array.  This array
*	is only valid until the object is disposed off.
*
*    SEE ALSO
*	GetDTTriggerMethods()
*
*******************************************************************************
*
* Created:  11-Mar-92, David N. Junod
*
*/

ULONG *ASM GetDTMethods (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o)
{
    ULONG *dtm = NULL;
    struct opGet opg;

    opg.MethodID    = OM_GET;
    opg.opg_AttrID  = DTA_Methods;
    opg.opg_Storage = (ULONG *) &dtm;

    if (!DoMethodA (o, &opg))
	dtm = NULL;

    return (dtm);
}

/****** datatypes.library/DoDTMethodA *****************************************
*
*    NAME
*	DoDTMethodA - Do a datatypes method.                    (V39)
*
*    SYNOPSIS
*	retval = DoDTMethodA (o, win, req, msg);
*	d0		      a0 a1   a2   a3
*
*	ULONG DoDTMethodA (Object *, struct Window *,
*			   struct Requester *, Msg);
*
*	retval = DoDTMethod (o, win, req, data, ...);
*
*	ULONG DoDTMethod (Object *, struct Window *,
*			  struct Requester *, ULONG, ...);
*
*    FUNCTION
*	This function is used to perform a datatypes method.
*
*    INPUTS
*	o - Pointer to an object as returned by NewDTObjectA().
*
*	win - Window that the object is attached to.
*
*	req - Requester that the object is attached to.
*
*	msg - The message to send to the object.
*
*    RETURNS
*	Returns the value returned by the method.
*
*    SEE ALSO
*	intuition.library/DoGadgetMethod()
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

ULONG ASM DoDTMethodA (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o, REG (a1) struct Window *win, REG (a2) struct Requester *req, REG (a3) Msg msg)
{
    switch (msg->MethodID)
    {
	case DTM_PRINT:
	case DTM_COPY:
	case DTM_WRITE:
	    /* Clear the GadgetInfo field (could only be garbage) */
	    ((struct dtGeneral *)msg)->dtg_GInfo = NULL;

	    /* We don't want locking or a GadgetInfo for these (they are doing DOS related stuff) */
	    return (DoMethodA (o, msg));

	default:
	    /* These need locking and a GadgetInfo */
	    return (DoGadgetMethodA ((struct Gadget *)o, win, req, msg));
    }
}

/****** datatypes.library/DoAsyncLayout ****************************************
*
*    NAME
*	DoAsyncLayout - Call the DTM_ASYNCLAYOUT method on a separate process.
*                                                               (V39)
*
*    SYNOPSIS
*	retval = DoAsyncLayout (object, gpl);
*	d0			a0	a1
*
*	ULONG DoAsyncLayout (Object *, struct gpLayout *);
*
*    FUNCTION
*	This function is used to asyncronously perform the object's
*	DTM_ASYNCLAYOUT method.  This is used to offload the layout method
*	from input.device.
*
*	The DTM_ASYNCLAYOUT method must exit when SIGBREAKF_CTRL_C signal
*	is set.   This indicates that the data has become obsolete and
*	the DTM_ASYNCLAYOUT method will be called again.
*
*    INPUTS
*	object - Pointer to the data types object.
*	gpl - Pointer to a gpLayout message.
*
*    RETURNS
*
*    SEE ALSO
*
*******************************************************************************
*
* Created:  14-Mar-92, David N. Junod
*
*/

/*****************************************************************************/

struct AsyncLayout
{
    struct Message	 al_Message;
    struct DataTypesLib *al_DTL;
    Object		*al_Object;
    struct Window	*al_Window;
    struct gpLayout	 al_Msg;
    struct GadgetInfo	 al_GInfo;
};

#define	ALSIZE	(sizeof (struct AsyncLayout))

/*****************************************************************************/

static void AsyncLayoutDaemon (void)
{
#undef	SysBase
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct DTSpecialInfo *si;
    struct DataTypesLib *dtl;
    struct AsyncLayout *al;
    struct Process *proc;

    /* Get a pointer to our process */
    proc = (struct Process *) FindTask (NULL);

    /* Get the startup message */
    WaitPort (&proc->pr_MsgPort);
    al = (struct AsyncLayout *) GetMsg (&proc->pr_MsgPort);
    dtl = al->al_DTL;
    si = (struct DTSpecialInfo *) G (al->al_Object)->SpecialInfo;

    /* Obtain the object lock */
    ObtainSemaphore (&(si->si_Lock));

    /* Perform the asynchronous layout */
    al->al_Msg.MethodID = DTM_ASYNCLAYOUT;
    do
    {
	si->si_Flags &= ~DTSIF_NEWSIZE;
	DoMethodA (al->al_Object, &al->al_Msg);

    } while (si->si_Flags & DTSIF_NEWSIZE);

    /* Set the layout process field */
    setattrs (dtl, al->al_Object, DTA_LayoutProc, NULL, TAG_DONE);

    /* Clear the flags */
    si->si_Flags &= ~(DTSIF_LAYOUT | DTSIF_NEWSIZE);

    /* Release the lock */
    ReleaseSemaphore (&(si->si_Lock));

    /* Tell the object that it has new data */
    notifyGAttrChanges (dtl, al->al_Object, al->al_Window, NULL, NULL,
		       GA_ID,		G(al->al_Object)->GadgetID,
		       DTA_Data,	al->al_Object,

		       DTA_TopVert,	si->si_TopVert,
		       DTA_VisibleVert,	si->si_VisVert,
		       DTA_TotalVert,	si->si_TotVert,

		       DTA_TopHoriz,	si->si_TopHoriz,
		       DTA_VisibleHoriz,si->si_VisHoriz,
		       DTA_TotalHoriz,	si->si_TotHoriz,

		       DTA_Sync,	TRUE,
		       DTA_Busy,	FALSE,
		       TAG_DONE);

    /* Leave this place */
    Forbid ();
    si->si_Flags &= ~DTSIF_LAYOUTPROC;
    FreeVec (al);
}
#define SysBase		dtl->dtl_SysBase

/*****************************************************************************/

ULONG ASM DoAsyncLayout (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o, REG (a1) struct gpLayout *gpl)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct AsyncLayout *al;
    struct Process *proc;
    ULONG retval = 0L;

    /* Why did I use window_list??? */
    ObtainSemaphore (&(dtl->dtl_Lock[WINDOW_LIST]));

    if (si->si_Flags & DTSIF_LAYOUT)
    {
	si->si_Flags |= DTSIF_NEWSIZE;
	if (GetAttr (DTA_LayoutProc, o, (ULONG *)&proc) && proc)
	    Signal ((struct Task *) proc, SIGBREAKF_CTRL_C);
	retval = 1L;
    }
    else
    {
	si->si_Flags |= DTSIF_LAYOUT | DTSIF_LAYOUTPROC;

	/* Tell the object that it has no data */
	notifyAttrChanges (dtl, o, gpl->gpl_GInfo, NULL, DTA_Data, NULL, TAG_DONE);

	if (al = AllocVec (ALSIZE, MEMF_CLEAR))
	{
	    /* Fill out the message structure */
	    al->al_Message.mn_Node.ln_Type = NT_MESSAGE;
	    al->al_Message.mn_Length = ALSIZE;
	    al->al_DTL           = dtl;
	    al->al_Object        = o;
	    al->al_Window	 = gpl->gpl_GInfo->gi_Window;
	    al->al_Msg           = *gpl;
	    al->al_GInfo         = *(gpl->gpl_GInfo);
	    al->al_Msg.gpl_GInfo = &al->al_GInfo;

	    /* Create the process */
	    if (proc = CreateNewProcTags (
				      NP_StackSize,	4096L,
				      NP_Entry,		AsyncLayoutDaemon,
				      NP_Priority,	0L,
				      NP_Name,		"AsyncLayoutDaemon",
				      TAG_DONE))
	    {
		/* Send the startup message */
		PutMsg (&(proc->pr_MsgPort), &(al->al_Message));

		/* Set the layout process field */
		setattrs (dtl, o, DTA_LayoutProc, proc, TAG_DONE);

		retval = 1L;
	    }
	    else
		FreeVec (al);
	}
    }

    ReleaseSemaphore (&(dtl->dtl_Lock[WINDOW_LIST]));
    return (retval);
}

/****** datatypes.library/PrintDTObjectA *************************************
*
*    NAME
*	PrintDTObjectA - Call the DTM_PRINT method on a separate process.
*                                                               (V39)
*
*    SYNOPSIS
*	retval = PrintDTObjectA (object, window, requester, msg);
*	d0			 a0	 a1	 a2	    a3
*
*	ULONG PrintDTObjectA (Object *, struct Window *, struct Requester *,
*			      struct dtPrint *);
*
*	retval = PrintDTObject (object, window, requester, data, ...);
*
*	ULONG PrintDTObject (Object *, struct Window *, struct Requester *,
*			     ULONG, ...);
*
*    FUNCTION
*	This function is used to asyncronously perform the object's DTM_PRINT
*	method.
*
*	Once the application has performed the PrintDTObjectA() function,
*	it must not manipulate the printerIO union until it receives a
*	IDCMP_IDCMPUPDATE message that contains the DTA_PrinterStatus
*	tag.
*
*	To abort a print, the application sends the DTM_ABORTPRINT method
*	to the object.  This in turn signals the print process with a
*	SIGBREAKF_CTRL_C.
*
*    INPUTS
*	object - Pointer to the DataTypes object.
*	window - Pointer to the window that the object has been added to.
*	requester - Pointer to the requester that the object has been
*	    added to.
*
*    RETURNS
*	Returns TRUE if successful, FALSE on error.
*
*    SEE ALSO
*
*******************************************************************************
*
* Created:  14-Mar-92, David N. Junod
*
*/

/*****************************************************************************/

struct AsyncPrint
{
    struct Message	 ap_Message;
    struct DataTypesLib *ap_DTL;
    Object		*ap_Object;
    struct Window	*ap_Window;
    struct Requester	*ap_Requester;
    struct dtPrint	 ap_Msg;
    struct GadgetInfo	 ap_GInfo;
};

#define	APSIZE	(sizeof (struct AsyncPrint))

/*****************************************************************************/

static void AsyncPrintDaemon (void)
{
#undef	SysBase
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct DTSpecialInfo *si;
    struct DataTypesLib *dtl;
    struct AsyncPrint *ap;
    union printerIO *opio;
    union printerIO *pio;
    struct Process *proc;
    struct MsgPort *mp;
    ULONG retval;

    proc = (struct Process *) FindTask (NULL);
    WaitPort (&proc->pr_MsgPort);
    ap = (struct AsyncPrint *) GetMsg (&proc->pr_MsgPort);
    dtl = ap->ap_DTL;
    si = (struct DTSpecialInfo *) G (ap->ap_Object)->SpecialInfo;

    /* Create a message port on our task */
    if (mp = CreateMsgPort ())
    {
	/* Create an IO request */
	if (pio = (union printerIO *) CreateIORequest (mp, sizeof (union printerIO)))
	{
	    /* Copy the entire old request into the new request */
	    CopyMem (ap->ap_Msg.dtp_PIO, pio, sizeof (union printerIO));

	    /* Use the new reply port */
	    pio->ios.io_Message.mn_ReplyPort = mp;

	    /* Use the new printer IO request */
	    opio = ap->ap_Msg.dtp_PIO;
	    ap->ap_Msg.dtp_PIO = pio;

	    /* Make a valid GadgetInfo structure */
	    ap->ap_Msg.dtp_GInfo = NULL;
	    if (ap->ap_Window)
	    {
		ap->ap_Msg.dtp_GInfo = &ap->ap_GInfo;
		ap->ap_GInfo.gi_Screen = ap->ap_Window->WScreen;
		ap->ap_GInfo.gi_Window = ap->ap_Window;
		ap->ap_GInfo.gi_Requester = ap->ap_Requester;
		ap->ap_GInfo.gi_RastPort = ap->ap_Window->RPort;
		ap->ap_GInfo.gi_Layer = (ap->ap_Requester) ? ap->ap_Requester->ReqLayer : ap->ap_Window->WLayer;
		ap->ap_GInfo.gi_DrInfo = GetScreenDrawInfo (ap->ap_Window->WScreen);
	    }

	    /* Perform the print */
	    retval = DoMethodA (ap->ap_Object, &ap->ap_Msg);

	    /* Free our temporary DrawInfo */
	    if (ap->ap_GInfo.gi_DrInfo)
	    {
		FreeScreenDrawInfo (ap->ap_Window->WScreen, ap->ap_GInfo.gi_DrInfo);
	    }

	    /* Restore the old printer IO request */
	    ap->ap_Msg.dtp_PIO = opio;

	    /* Delete the IO request */
	    DeleteIORequest ((struct IORequest *)pio);
	}

	/* Delete the message port */
	DeleteMsgPort (mp);
    }

    /* Clear the printer process field */
    setattrs (dtl, ap->ap_Object, DTA_PrinterProc, NULL, TAG_DONE);

    /* Send the status information */
    notifyGAttrChanges (dtl, ap->ap_Object, ap->ap_Window, ap->ap_Requester, NULL,
		       GA_ID, G (ap->ap_Object)->GadgetID,
		       DTA_PrinterStatus, retval,
		       TAG_DONE);

    /* Give them time to see it */
    Delay (MAGIC_DELAY);

    Forbid ();
    si->si_Flags &= ~DTSIF_PRINTING;
    FreeVec (ap);
}
#define SysBase		dtl->dtl_SysBase

/*****************************************************************************/

ULONG ASM PrintDTObjectA (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o, REG (a1) struct Window *w, REG (a2) struct Requester *r, REG (a3) struct dtPrint *msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct AsyncPrint *ap;
    struct Process *proc;
    ULONG retval = 0L;

    /* Why did I use window_list??? */
    ObtainSemaphore (&(dtl->dtl_Lock[WINDOW_LIST]));

    /* Can't double print! */
    if (!(si->si_Flags & DTSIF_PRINTING))
    {
	/* Show that we are printing */
	si->si_Flags |= DTSIF_PRINTING;

	/* Allocate the work data */
	if (ap = AllocVec (APSIZE, MEMF_CLEAR))
	{
	    /* Fill out the work structure */
	    ap->ap_Message.mn_Node.ln_Type = NT_MESSAGE;
	    ap->ap_Message.mn_Length = APSIZE;
	    ap->ap_DTL = dtl;
	    ap->ap_Object = o;
	    ap->ap_Window = w;
	    ap->ap_Requester = r;
	    ap->ap_Msg = *msg;

	    if (proc = CreateNewProcTags (
				      NP_StackSize,	4096L,
				      NP_Entry,		AsyncPrintDaemon,
				      NP_Priority,	0L,
				      NP_Name,		"AsyncPrintDaemon",
				      TAG_DONE))
	    {
		/* Send the startup message */
		PutMsg (&(proc->pr_MsgPort), &(ap->ap_Message));

		/* Set the printer process field */
		setattrs (dtl, o, DTA_PrinterProc, proc, TAG_DONE);

		/* Indicate success */
		retval = 1L;
	    }
	    else
	    {
		FreeVec (ap);
	    }
	}
    }

    ReleaseSemaphore (&(dtl->dtl_Lock[WINDOW_LIST]));

    return (retval);
}

/****i* datatypes.library/ObtainDTDrawInfoA *************************************
*
*    NAME
*	ObtainDTDrawInfoA - Obtain a DataTypes object for drawing.
*	                                                        (V39)
*
*    SYNOPSIS
*	handle = ObtainDTDrawInfoA (o, attrs);
*	d0			    a0 a1
*
*	APTR ObtainDTDrawInfoA (Object *, struct TagItem *);
*
*	handle = ObtainDTDrawInfo (o, tag1, ...);
*
*	APTR ObtainDTDrawInfo (Object *, Tag, ...);
*
*    FUNCTION
*	This function is used to prepare a DataTypes object for
*	drawing into a RastPort.
*
*	This function will send the DTM_OBTAINDRAWINFO method
*	to the object using the opSet message structure.
*
*    INPUTS
*	o - Pointer to an object as returned by NewDTObjectA().
*	attrs - Additional attributes.
*
*    RETURNS
*	Returns a opaque handle that must be passed to ReleaseDTDrawInfo()
*	when the application is done drawing the object.
*
*    TAGS
*	none defined at this time.
*
*    SEE ALSO
*	DrawDTObjectA(), ReleaseDTDrawInfo()
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

ULONG ASM ObtainDTDrawInfoA (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o, REG (a1) struct TagItem *attrs)
{
    /* Send the message using the opSet message structure */
    return DoMethod (o, DTM_OBTAINDRAWINFO, attrs, NULL);
}

/****i* datatypes.library/DrawDTObjectA ***********************************
*
*    NAME
*	DrawDTObjectA - Draw a DataTypes object.                (V39)
*
*    SYNOPSIS
*	retval = DrawDTObjectA (rp, o, x, y, w, h, th, tv, attrs);
*	d0			a0  a1 d0 d1 d2 d3 d4  d5  a2
*
*	LONG DrawDTObjectA (struct RastPort *rp, Object *, LONG x, LONG y,
*			    LONG w, LONG h, LONG th, LONG tv,
*			    struct TagItem *);
*
*	retval = DrawDTObject (rp, o, x, y, w, h, th, tv, tag1, ...);
*
*	LONG DrawDTObjectA (struct RastPort *rp, Object *, LONG x, LONG y,
*			    LONG w, LONG h, LONG th, LONG tv, Tag, ...);
*
*    FUNCTION
*	This function is used to draw a DataTypes object into a RastPort.
*
*	This function can be used for strip printing the object or
*	embedding it within a document.
*
*	You must successfully call ObtainDTDrawInfoA() before using
*	this function.
*
*	This function invokes the object's DTM_DRAW method.
*
*	Clipping MUST be turned on within the RastPort.  This means
*	that there must be a valid layer structure attached to the
*	RastPort.
*
*    INPUTS
*	rp - Pointer to the RastPort to draw into.
*	o - Pointer to an object returned by NewDTObjectA().
*	x - Left-most point of area to draw into.
*	y - Top-most point of area to draw into.
*	w - Width of area to draw into.
*	h - Height of area to draw into.
*	th - Horizontal top in units.
*	tv - Vertical top in units.
*	attrs - Additional attributes.
*
*    TAGS
*	none defined at this time.
*
*    RETURNS
*	TRUE to indicate that it was able to render, FALSE on failure.
*
*    SEE ALSO
*	ObtainDTDrawInfo(), ReleaseDTDrawInfo()
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

ULONG ASM DrawDTObjectA (REG (a6) struct DataTypesLib *dtl, REG (a0) struct RastPort *rp, REG (a1) Object *o, REG (d0) LONG x, REG (d1) LONG y, REG (d2) LONG w, REG (d3) LONG h, REG (d4) LONG th, REG (d5) LONG tv, REG (a2) struct TagItem *attrs)
{
    /* Send the message using the opSet message structure */
    return DoMethod (o, DTM_DRAW, rp, x, y, w, h, th, tv, attrs);
}

/****i* datatypes.library/ReleaseDTDrawInfo ***********************************
*
*    NAME
*	ReleaseDTDrawInfo - Release a DataTypes object from drawing.
*	                                                        (V39)
*
*    SYNOPSIS
*	ReleaseDTDrawInfo (o, handle);
*			   a0 a1
*
*	VOID ReleaseDTDrawInfo (Object *, APTR);
*
*    FUNCTION
*	This function is used to release the information obtained
*	with ObtainDTDrawInfoA().
*
*	This function invokes the object's DTM_RELEASEDRAWINFO method
*	using the opMember message structure.
*
*    INPUTS
*	handle - Pointer to an object returned by ObtainDTDrawInfoA().
*
*    SEE ALSO
*	DrawDTObjectA(), ObtainDTDrawInfo()
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

ULONG ASM ReleaseDTDrawInfo (REG (a6) struct DataTypesLib *dtl, REG (a0) Object *o, REG (a1) APTR handle)
{
    /* Send the message using the opSet message structure */
    return DoMethod (o, DTM_RELEASEDRAWINFO, handle);
}
