/*****************************************************************************
 *
 *  Includes
 *
 *****************************************************************************/

#include <exec/types.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>

#include <clib/intuition_protos.h>

#include <pragmas/intuition_pragmas.h>

#include "classbase.h"
#include "classdata.h"

/****** label.image/--datasheet-- ***********************************************
*
*    NAME
*       label.image--Text, image, or border label.                 (V42)
*
*    SUPERCLASS
*   imageclass
*
*    DESCRIPTION
*   The label.image image class provides a text or glyph label.
*
*    METHODS
*   OM_NEW--Create the label image.  Passed to superclass, then OM_SET.
*
*   OM_SET--Set object attributes.  Passed to superclass first.
*
*   OM_UPDATE--Set object notification attributes.  Passed to superclass
*       first.
*
*   IM_DRAW--Renders the images.  Overrides the superclass.
*
*   IM_DOMAIN--Query domain (position and dimensions). The implementation
*              supports constraints (i.e., given a fixed width, query
*              to determine the height). Only meaningful for text-type
*              labels (specified with IA_Text). Overrides the superclass.
*
*   All other methods are passed to the superclass, including OM_DISPOSE.
*
*    ATTRIBUTES
*   SYSIA_DrawInfo (struct DrawInfo *) -- Contains important information
*       required for layout and rendering. This tag must be specified,
*       unless intuition.library/DrawImageState() (with a DrawInfo
*       argument) is used to render the image.
*
*   IA_FGPen (LONG) -- Pen to use to draw the lit segments.  If -1
*       is specified then TEXTPEN is used.
*
*   IA_BGPen (LONG) -- Pen to use to draw the unlit segments or
*       background.  If -1 is specified then BACKGROUNDPEN is used.
*
*   IA_Width (LONG) -- Width of the image.
*
*   IA_Height (LONG) -- Height of the image.
*
*   IA_Text (STRPTR) -- Text label for the image. Mutually-exclusive
*                          with IA_BitMap. This text may contain a
*                          subset of the attributes supported by
*                          AmigaGuide. The following attributes, fully
*                          documented in the AmigaGuide documentation,
*                          are supported:
*
*                           \@{APEN <nPen>}  -   set foreground color
*                                               to pen <nPen>
*                           \@{B}            -   bold on
*                           \@{BG <color>}   -   set background color
*                                               to <color> (where <color>
*                                               is one of Text, Shine,
*                                               Shadow, Fill, FillText,
*                                               Background, Highlight)
*                           \@{CLEARTABS}    -   restore default tab
*                                               stops (every 8 columns)
*                           \@{FG <color>}   -   set foreground color
*                                               to <color> (where <color>
*                                               is one of Text, Shine,
*                                               Shadow, Fill, FillText,
*                                               Background, Highlight)
*                           \@{I}            -   italic on
*                           \@{JCENTER}      -   center justification
*                           \@{JLEFT}        -   left justification
*                           \@{JRIGHT}       -   right justification
*                           \@{PLAIN}        -   turn off bold, italic,
*                                               and underline attribute
*                           \@{SETTABS <n> ... <n>} - set tab stops
*                                                    at columns
*                           \@{TAB}          -   tab; equivalent to
*                                               literal TAB ('\t' or $09)
*                                               in string
*                           \@{U}            -   underline on
*                           \@{UB}           -   bold off
*                           \@{UI}           -   italic off
*                           \@{UU}           -   underline off
*                           newline ('\n')  -   new line
*
*   IA_TextAttr (struct TextAttr *) -- Text attributes for text label.
*                                         Only meaningful with IA_Text.
*                                         Defaults to system default font.
*                                         Styles may be modified with
*                                         attributes in text (see above).
*
*   IA_Underscore (UBYTE) -- Underscoring prefix character. Any
*                               character in IA_Text preceeded by this
*                               character will be underscored when displayed.
*                               Only meaningful with IA_Text.
*                               Defaults to '_'.
*
*   IA_BitMap (struct BitMap *) - Bitmap for the label. Mutually-exclusive
*                                 with IA_Text.
*
*******************************************************************************
*
* John J. Szucs
*
*/

/*****************************************************************************
 *
 *  Local Prototypes
 *
 *****************************************************************************/

VOID CallCHook(void);

/*****************************************************************************
 *
 *  CreateClass()   -   create class
 *
 *****************************************************************************/

BOOL __asm CreateClass (register __a6 struct ClassLib *ClassBase)
{

    Class *cl;

#ifdef DEBUG
    kprintf("CreateClass(): Entry\n");
    kprintf("   ClassBase=$%08lx\n", ClassBase);
    kprintf("   lib_OpenCnt=%ld\n", ClassBase->cb_Library.cl_Lib.lib_OpenCnt);
#endif /* DEBUG */

    if (cl = MakeClass(
        "label.image", IMAGECLASS, NULL, sizeof (struct objectData), 0)
    )
    {
#ifdef DEBUG
        kprintf("   MakeClass() good\n");
#endif /* DEBUG */
        cl->cl_Dispatcher.h_Entry    = (unsigned long (*)()) CallCHook;
        cl->cl_Dispatcher.h_SubEntry = (unsigned long (*)()) ClassDispatcher;
        cl->cl_Dispatcher.h_Data     = ClassBase;
        cl->cl_UserData              = (ULONG) ClassBase;
        AddClass (cl);


    } else {

#ifdef DEBUG
        kprintf("   MakeClass() failure\n");
#endif /* DEBUG */

    }

    /* Set the class pointer */
    ClassBase->cb_Library.cl_Class = cl;

    return (BOOL)(cl != NULL);
}


/*****************************************************************************
 *
 *  DestroyClass()  -   destroy class
 *
 *****************************************************************************/

BOOL __asm DestroyClass (register __a6 struct ClassLib *ClassBase)
{

    BOOL result;

#ifdef DEBUG
    kprintf("DestroyClass(): Entry\n");
    kprintf("   ClassBase=$%08lx\n", ClassBase);
    kprintf("   lib_OpenCnt=%ld\n", ClassBase->cb_Library.cl_Lib.lib_OpenCnt);
#endif /* DEBUG */

    /* Remove class */
#ifdef DEBUG
    kprintf("   Removing class\n");
#endif /* DEBUG */
/*    RemoveClass(ClassBase->cb_Library.cl_Class); */

    /* Free class */
#ifdef DEBUG
    kprintf("   Freeing class\n");
#endif /* DEBUG */
    if (result = FreeClass (ClassBase->cb_Library.cl_Class)) {
                /* was FreeClass(ClassBase->cb_Class) */
        ClassBase->cb_Library.cl_Class = NULL;
    }

    return result;

}
