#ifndef PE_UTILS_H
#define PE_UTILS_H


/*****************************************************************************/


#include <intuition/intuition.h>
#include "pe_custom.h"
#include "pe_strings.h"


/*****************************************************************************/


struct EdMenu
{
    UBYTE          em_Type;        /* NM_XXX from gadtools.h */
    AppStringsID   em_Label;
    enum EdCommand em_Command;
    UWORD	   em_ItemFlags;
};


struct EdGadget
{
    ULONG          eg_GadgetKind;

    WORD           eg_LeftEdge;
    WORD           eg_TopEdge;
    WORD           eg_Width;
    WORD           eg_Height;

    AppStringsID   eg_Label;
    enum EdCommand eg_Command;
    ULONG          eg_GadgetFlags;
};


/*****************************************************************************/


/* Macro to get longword-aligned stack space for a structure	*/
/* Uses ANSI token catenation to form a name for the char array */
/* based on the variable name, then creates an appropriately	*/
/* typed pointer to point to the first longword boundary in the */
/* char array allocated.					*/
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);


/*****************************************************************************/


struct Window *OpenPrefsWindow(EdDataPtr ed, ULONG tag1, ...);

APTR AllocPrefsRequest(EdDataPtr ed, ULONG type, ULONG tag1, ...);
BOOL RequestPrefsFile(EdDataPtr ed, ULONG tag1, ...);

VOID kprintf(STRPTR,...);

struct Menu *CreatePrefsMenus(EdDataPtr ed, struct EdMenu *em);

struct Gadget *DoPrefsGadget(EdDataPtr ed, struct EdGadget *eg,
                             struct Gadget *gad, ULONG tags, ...);

VOID SetGadgetAttr(EdDataPtr ed, struct Gadget *gad, ULONG tags, ...);

VOID ShowError1(EdDataPtr ed, AppStringsID es);

VOID ShowError2(EdDataPtr ed, EdStatus es);


/*****************************************************************************/


#endif /* PE_UTILS_H */
