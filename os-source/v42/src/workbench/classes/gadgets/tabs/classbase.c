#include <exec/types.h>
#include <intuition/classes.h>

#include <clib/intuition_protos.h>

#include <pragmas/intuition_pragmas.h>

/*****************************************************************************/

#include "classbase.h"
#include "classdata.h"

/****** tabs.gadget/--datasheet-- *********************************************
*
*    NAME
*       tabs.gadget--File folder tabs gadget.                   (V42)
*
*    SUPERCLASS
*	gadgetclass
*
*    DESCRIPTION
*	The tabs gadget class provides a custom control that has imagry
*	similar in style to the tabs seen in a drawer full of file folders.
*	The action of the gadget is the same as a conventional
*	mutual-exclusion control in that only one tab can be active at a time
*	and a tab is selected by clicking upon it.
*
*	The purpose of the tabs gadget class is to provide functionality like
*	the page selection cycle gadget on the top-right side of the AmigaOS
*	2.1 PrinterPS preferences editor, but allowing all the choices to be
*	visible at the same time.
*
*	This gadget class requires 2.04 (V37) or greater.
*
*    METHODS
*	OM_NEW--Create the button gadget.  Passed to superclass, then OM_SET.
*
*	OM_GET--Get an object attribute.  Passed to superclass for unknown
*	    attributes.
*
*	OM_SET--Set object attributes.  Passed to superclass first.
*
*	OM_UPDATE--Set object notification attributes.  Passed to superclass
*	    first.
*
*	GM_LAYOUT--Calculate gadget imagry positioning.  Passed to superclass
*	    first.
*
*	GM_RENDER--Renders the gadget imagry.  Overrides the superclass.
*
*	GM_HITTEST--Determines if mouse is within the gadget rectangle.  Over-
*	    rides the superclass.
*
*	GM_GOACTIVE--Handles activation, toggle-select and button-select.
*	    Overrides the superclass.
*
*	GM_GOINACTIVE--Deselects the button.  Overrides the superclass.
*
*	All other methods are passed to the superclass, including OM_DISPOSE.
*
*    ATTRIBUTES
*	GA_Disabled (BOOL) -- Determines whether the button is disabled or
*	    not.  Changing disable state will invoke GM_RENDER.  A disabled
*	    button's border and label are all rendered in SHADOWPEN and then
*	    dusted in a ghosting pattern that is rendered in SHADOWPEN.
*	    Defaults to FALSE.
*
*	GA_TextAttr (struct TextAttr *) -- Text attribute for the font to
*	    use for the labels.
*
*	LAYOUTA_ChildMaxWidth (BOOL) -- Indicate whether the tabs should be
*	    the width of the widest label.  Defaults to TRUE.
*
*	TABS_Labels (TabLabelP) -- An array of TabLabel structures used to
*	    indicate the labels for each of the tabs.
*
*	TABS_Current (LONG) -- Currently selected tab.  Defaults to zero.
*
*    NOTES
*	This gadget class requires 2.04 or beyond to run.
*
*******************************************************************************
*
* David N. Junod
*
*/

VOID CallCHook(void);

/*****************************************************************************/

BOOL __asm CreateClass (register __a6 struct ClassLib *cb)
{
    Class *cl;

    if (cl = MakeClass ("tabs.gadget", "gadgetclass", NULL, sizeof (struct objectData), 0))
    {
        cl->cl_Dispatcher.h_Entry    = CallCHook;
        cl->cl_Dispatcher.h_SubEntry = ClassDispatcher;
	cl->cl_Dispatcher.h_Data     = cb;
	cl->cl_UserData              = (ULONG) cb;
	AddClass (cl);
    }

    /* Set the class pointer */
    cb->cb_Library.cl_Class = cl;

    return (BOOL)(cl != NULL);
}


/*****************************************************************************/


BOOL __asm DestroyClass (register __a6 struct ClassLib *cb)
{
    BOOL result;

    if (result = FreeClass (cb->cb_Library.cl_Class))
	cb->cb_Library.cl_Class = NULL;

    return result;
}
