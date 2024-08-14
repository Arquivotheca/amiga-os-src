#include <exec/types.h>
#include <intuition/classes.h>

#include <clib/intuition_protos.h>

#include <pragmas/intuition_pragmas.h>

/*****************************************************************************/

#include "classbase.h"
#include "classdata.h"

/****** button.gadget/--datasheet-- *******************************************
*
*    NAME
*       button.gadget--Action button                            (V42)
*
*    SUPERCLASS
*	gadgetclass
*
*    DESCRIPTION
*	The button gadget class is used to create action buttons that are
*	momentary, toggle and sticky.  This class also supports relativity
*	and placement within the window border.
*
*	An advantage that this class provides that the system buttongclass
*	doesn't support fully is:
*
*	 o Centered images.
*
*	 o Centered pen-sensitive glyphs.
*
*	 o Ability to change the text and background colors for normal
*	   and selected buttons.
*
*    METHODS
*	OM_NEW--Create the button gadget.  Passed to superclass, then OM_SET.
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
*	GM_HANDLEINPUT--Handles input events once active.  Handles cycle
*	    buttons, repeat and RMB abort.  Overrides the superclass.
*
*	GM_GOINACTIVE--Deselects the button.  Overrides the superclass.
*
*	All other methods are passed to the superclass, including OM_DISPOSE.
*
*    ATTRIBUTES
*	GA_ToggleSelect (BOOL) -- Indicate that the gadget is a toggle button.
*	    Defaults to FALSE.
*
*	GA_Selected (BOOL) -- Determines whether the button is selected or
*	    not.  Changing selection state will invoke GM_RENDER.  Setting
*	    selected causes BUTTON_Current to go to 1.  Clearing selected
*	    causes BUTTON_Current to go to 0.  Defaults to FALSE.
*
*	GA_Disabled (BOOL) -- Determines whether the button is disabled or
*	    not.  Changing disable state will invoke GM_RENDER.  A disabled
*	    button's border and label are all rendered in SHADOWPEN and then
*	    dusted in a ghosting pattern that is rendered in SHADOWPEN.
*	    Defaults to FALSE.
*
*	GA_Text (STRPTR) -- Used to specify the NULL terminated string to use
*	    as the text for the gadget.  The Text() function is used to draw
*	    the text.  The class does not currently support underlining of
*	    the keyboard shortcut character.  NULL is valid input.  Changing
*	    the label will invoke GM_LAYOUT and then GM_RENDER.
*
*	GA_Image (struct Image *) -- Used to specify the image to use for the
*	    label of the gadget.  The DrawImage() function is used to draw
*	    the image.  NULL is valid input.  Changing the label will invoke
*	    GM_LAYOUT and then GM_RENDER.
*
*	GA_TextAttr (struct TextAttr *) -- Text attribute for the font to
*	    use for the labels.
*
*	GA_ReadOnly (BOOL) -- Read-only gadgets ignore activation attempts.
*	    Defaults to FALSE.
*
*	BUTTON_Glyph (struct Image *) -- Used to specify the image to use for
*	    the label of the gadget.  The BltTemplate() function is used to
*	    draw each plane of the image.  NULL is valid input.  Changing the
*	    label will invoke GM_LAYOUT and then GM_RENDER.
*
*	BUTTON_PushButton (BOOL) -- Used to indicate that the button stays
*	    pressed in when the user selects it with the mouse.  The button
*	    may programmatically be deselected using {GA_Selected, FALSE}.
*	    Defaults to FALSE.
*
*	BUTTON_Array (LONG) -- Indicates that the label is an array, and
*	    indicates the number of elements in the array.  This allows the
*	    gadget to be used as a checkbox or a cycle gadget without the
*	    cycle glyph.  Defaults to 1.
*
*	BUTTON_Current (LONG) -- Used to select the current image from the
*	    BUTTON_Array.  Changing the current image will invoke GM_LAYOUT
*	    and then GM_RENDER.  Defaults to zero.
*
*	BUTTON_TextPen (LONG) -- Indicate the pen number used to render the
*	    IDS_NORMAL text.  If -1 is specified, then TEXTPEN is used.
*	    Defaults to -1.
*
*	BUTTON_BackgroundPen (LONG) -- Indicate the pen number used to render
*	    the IDS_NORMAL background.  If -1 is specified, then BACKGROUNDPEN
*	    is used.  Defaults to -1.
*
*	BUTTON_FillTextPen (LONG) -- Indicate the pen number used to render
*	    the IDS_SELECTED text.  If -1 is specified, then FILLTEXTPEN is
*	    used.  Defaults to -1.
*
*	BUTTON_FillPen (LONG) -- Indicate the pen number used to render the
*	    IDS_SELECTED background.  If -1 is specified, then FILLPEN is used.
*	    Defaults to -1.
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

    if (cl = MakeClass ("button.gadget", "gadgetclass", NULL, sizeof (struct objectData), 0))
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
