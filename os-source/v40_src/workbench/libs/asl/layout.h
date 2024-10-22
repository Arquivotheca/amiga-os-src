#ifndef LAYOUT_H
#define LAYOUT_H


/*****************************************************************************/


#include <exec/types.h>
#include "aslbase.h"
#include "texttable.h"


/*****************************************************************************/


struct ASLGadget
{
    UBYTE           ag_GadgetKind;
    UBYTE           ag_Command;
    UWORD           ag_GadgetFlags;
    AppStringsID    ag_Label;
    struct IBox     ag_LOSize;		/* Coordinate box */
    WORD            ag_LOGroup;		/* Group */
    WORD            ag_LOSub;		/* Sub group (such as column or row) */
    WORD            ag_LOPriority;	/* Priority of object (or group) */
    struct TagItem *ag_CreationTags;
    struct IBox     ag_LOWork;		/* Work box */
    struct IBox     ag_LOGSize;		/* Group box */
    WORD            ag_MinWidth;
    WORD            ag_MinHeight;
    struct Gadget  *ag_Gadget;		/* Pointer to the created gadget */
};


#define	HGROUP_KIND 240  /* Horizontal group */
#define	VGROUP_KIND 241  /* Vertical group   */
#define	DGROUP_KIND 242  /* Diagonal group   */
#define	END_KIND    255  /* End              */


/*****************************************************************************/


#define	LGM_SETMIN 0
#define	LGM_CREATE 1


/*****************************************************************************/


BOOL LayoutGadgets(struct ReqInfo *ri, WORD mode);
VOID FreeLayoutGadgets(struct ReqInfo *ri, BOOL cleanup);
VOID CalculatePaneSize(struct ReqInfo *ri);
VOID SetGadgetAttr(struct ReqInfo *ri, WORD id, ULONG tags, ...);


/*****************************************************************************/


#endif /* LAYOUT_H */
