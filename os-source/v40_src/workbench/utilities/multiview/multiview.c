/* multiview.c
 *
 */

#include "multiview.h"

/*****************************************************************************/

LONG OpenNewData (struct GlobalData * gd, ULONG stype, ULONG unit)
{
    struct DataType *dtn;
    LONG retval = 0L;
    BPTR lock;

    /* Remember the source type */
    if ((gd->gd_SourceType = stype) == DTST_FILE)
	unit = (ULONG) gd->gd_NameBuffer;

    /* Show that we're loading a new object */
    setattrs (gd, gd->gd_WindowObj, WOA_Title, (ULONG) GetString (gd, MV_TITLE_LOADING), TAG_DONE);
    setwindowpointer (gd, window, WA_BusyPointer, TRUE, WA_PointerDelay, TRUE, TAG_DONE);

    if (gd->gd_DataObject)
    {
	/* Get rid of the object */
	if (gd->gd_Flags & GDF_BITMAP)
	{
	    CloseEnvironment (gd, 2);
	}
	else
	{
	    /* Remove the object from the window */
	    DoMethod ((Object *) gd->gd_WindowObj, WOM_REMVIEW, (ULONG) gd->gd_DataObject);

	    /* Erase its rectangle */
	    EraseRect (window->RPort,
		       window->BorderLeft,
		       window->BorderTop,
		       window->Width - window->BorderRight - 1,
		       window->Height - window->BorderBottom - 1);

	    /* Dispose of the object */
	    DisposeDTObject (gd->gd_DataObject);
	}
    }

    /* Clear the top */
    gd->gd_TotVert = gd->gd_TotHoriz = 0;

    /* Get the new object */
    if (gd->gd_DataObject = NewDTObject ((STRPTR) unit,
					DTA_ARexxPortName,	(ULONG)gd->gd_ARexxPortName,
					DTA_SourceType,		gd->gd_SourceType,
					DTA_TextAttr,		(ULONG) &gd->gd_TextAttr,
					GA_Immediate,		TRUE,
					GA_RelVerify,		TRUE,
					TAG_DONE))
    {
	/* Try opening the environment */
	if (OpenEnvironment (gd))
	{
	    /* Show success */
	    retval = 1L;

	    /* Clear the scrollers */
	    setgadgetattrs (gd, (struct Gadget *) gd->gd_WindowObj, gd->gd_Window,
			    DTA_TopVert, 0L,
			    DTA_TopHoriz, 0L,
			    TAG_DONE);
	}
	else
	{
	    /* We have problems!!!! */
	    PrintF (gd, 1, gd->gd_SecondaryResult, NULL);
	    gd->gd_Going = FALSE;
	}
    }
    else
    {
	/* Remember the error message */
	gd->gd_SecondaryResult = IoErr ();

	/* Lock the name given */
	if (lock = Lock (gd->gd_NameBuffer, ACCESS_READ))
	{
	    /* Get the object type */
	    if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) lock, NULL))
	    {
		/* If the object type is a directory, then we will bring up the file requester. */
		if (Stricmp (dtn->dtn_Header->dth_Name, "directory") == 0)
		    retval = 2;

		/* Done with the datatype, so release it */
		ReleaseDataType (dtn);
	    }
	    UnLock (lock);
	}

	if (retval == 0)
	{
	    /* Show the error message */
	    PrintF (gd, 2, gd->gd_SecondaryResult, FilePart (gd->gd_NameBuffer));

	    /* Clear the sliders */
	    setgadgetattrs (gd, (struct Gadget *) gd->gd_WindowObj, gd->gd_Window,
			    DTA_TopVert, 0L,
			    DTA_TopHoriz, 0L,
			    DTA_TotalVert, 0L,
			    DTA_TotalHoriz, 0L,
			    TAG_DONE);
	}
	else
	{
	    setattrs (gd, gd->gd_WindowObj, WOA_Title, (ULONG) GetString (gd, MV_TITLE_MULTIVIEW), TAG_DONE);
	}

	/* Clear the window pointer */
	setwindowpointer (gd, window, WA_Pointer, NULL, TAG_DONE);

	/* If we don't have a window, then we might as well stop right now. */
	if (!window)
	    gd->gd_Going = FALSE;
    }

    return (retval);
}

/*****************************************************************************/

ULONG frameobject (struct GlobalData * gd)
{
    struct FrameInfo *fri = &gd->gd_FrameInfo;
    struct DisplayInfo di;
    struct dtFrameBox dtf;
    ULONG modeid;

    /* Get the display information */
    modeid = GetVPModeID (&(gd->gd_Screen->ViewPort));
    GetDisplayInfoData (NULL, (APTR) & di, sizeof (struct DisplayInfo), DTAG_DISP, modeid);

    /* Fill in the frame info */
    fri->fri_PropertyFlags = di.PropertyFlags;
    fri->fri_Resolution = *(&di.Resolution);
    fri->fri_RedBits = di.RedBits;
    fri->fri_GreenBits = di.GreenBits;
    fri->fri_BlueBits = di.BlueBits;
    fri->fri_Dimensions.Width = gd->gd_Screen->Width;
    fri->fri_Dimensions.Height = gd->gd_Screen->Height;
    fri->fri_Dimensions.Depth = gd->gd_Screen->BitMap.Depth;
    fri->fri_Screen = gd->gd_Screen;
    fri->fri_ColorMap = gd->gd_Screen->ViewPort.ColorMap;

    /* Send the message */
    dtf.MethodID = DTM_FRAMEBOX;
    dtf.dtf_ContentsInfo = &gd->gd_FrameInfo;
    dtf.dtf_SizeFrameInfo = sizeof (struct FrameInfo);
    dtf.dtf_FrameFlags = FRAMEF_SPECIFY;
    return (DoDTMethodA (gd->gd_DataObject, gd->gd_Window, NULL, &dtf));
}

/*****************************************************************************/

void SetDataObjectAttrs (struct GlobalData * gd)
{
    if (gd->gd_DataObject)
    {
	DoMethod ((Object *) gd->gd_WindowObj, WOM_ADDVIEW, (ULONG) gd->gd_DataObject, NULL);
    }
}
