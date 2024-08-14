/*** menu.c ***************************************************************
*
*   menu.c	- Menu routines for gadget toolkit.
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: menu.c,v 39.5 92/07/16 10:54:39 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Public: */
struct Menu * __asm
LIB_CreateMenusA (register __a0 struct NewMenu *newmenu,
		 register __a1 struct TagItem *taglist);

void __asm
LIB_FreeMenus (register __a0 struct Menu *menu);

LONG __asm
LIB_LayoutMenusA (register __a0 struct Menu *firstmenu,
		 register __a1 struct VisualInfo *vi,
		 register __a2 struct TagItem *taglist);

LONG __asm
LIB_LayoutMenuItemsA (register __a0 struct MenuItem *firstitem,
		     register __a1 struct VisualInfo *vi,
		     register __a2 struct TagItem *taglist);

/* Private: */
static ULONG TallyMenuMemory (struct NewMenu *nm, BOOL fullmenu );
static ULONG InitMenuMemory (struct NewMenu *nm, UBYTE *memory, UBYTE frontpen);
static void DoMenuItemLayout (struct RastPort *, struct MenuItem *, struct TextAttr *,
    WORD, WORD, ULONG, LONG, struct VisualInfo *);
static void PlaceItems (struct RastPort *, struct MenuItem *, struct TextAttr *,
    WORD, WORD, WORD, WORD, WORD, ULONG, struct VisualInfo *);
static void SizeItems (struct MenuItem *, UWORD, struct TextAttr *, LONG);
static void AboutColumn (struct ColumnInfo *, struct RastPort *, struct MenuItem *,
    UWORD, ULONG, struct VisualInfo *);
static ULONG MenuImageWidths(struct VisualInfo *vi, struct TagItem *ti);
static LONG GetFrontPen(struct VisualInfo *vi, struct TagItem *ti);

/*------------------------------------------------------------------------*/

/* This structure contains sizing information about a column of a menu: */
struct ColumnInfo
{
    UWORD width;	/* width of select area of column */
    UWORD height;	/* height of select areas of column */
    UWORD count;	/* number of items */
    struct MenuItem *nextitem;	/* first item of next column */
};

/*------------------------------------------------------------------------*/

/* Pixels between columns of menus */
#define MULTICOLUMN_GAP	8

#define ITEXT(foo) ((struct IntuiText *)(foo))
#define IMAGE(fum) ((struct Image *)(fum))

/*------------------------------------------------------------------------*/

/****** gadtools.library/CreateMenusA ****************************************
*
*   NAME
*	CreateMenusA -- allocate and fill out a menu structure. (V36)
*	CreateMenus -- varargs stub for CreateMenus(). (V36)
*
*   SYNOPSIS
*	menu = CreateMenusA(newmenu, tagList)
*	D0                  A0       A1
*
*	struct Menu *CreateMenusA(struct NewMenu *, struct TagItem *);
*
*	menu = CreateMenus(newmenu, firsttag, ...)
*
*	struct Menu *CreateMenus(struct NewMenu *, Tag, ...);
*
*   FUNCTION
*	CreateMenusA() allocates and initializes a complete menu
*	structure based on the supplied array of NewMenu structures.
*	Optionally, CreateMenusA() can allocate and initialize a complete
*	set of menu items and sub-items for a single menu title.  This
*	is dictated by the contents of the array of NewMenus.
*
*   INPUTS
*	newmenu - pointer to an array of initialized struct NewMenus.
*	tagList - pointer to an array of tags providing optional extra
*		  parameters, or NULL.
*
*   TAGS
*	GTMN_FrontPen (UBYTE) - Pen number to be used for menu text.
*	    (Under V39 and higher, this tag also exists for LayoutMenusA()
*	    and LayoutMenuItemsA()).  (defaults to zero).
*	GTMN_FullMenu (BOOL) - Requires that the NewMenu specification
*	    describes a complete menu strip, not a fragment.  If a fragment
*	    is found, CreateMenusA() will fail with a secondary error of
*	    GTMENU_INVALID.  (defaults to FALSE). (V37)
*	GTMN_SecondaryError (ULONG *) - Supply a pointer to a NULL-initialized
*	    ULONG to receive a descriptive error code.  Possible values:
*	    GTMENU_INVALID - NewMenu structure describes an illegal
*	        menu.  (CreateMenusA() will fail with a NULL result).
*	    GTMENU_TRIMMED - NewMenu structure has too many menus, items,
*	        or subitems (CreateMenusA() will succeed, returning a
*	        trimmed-down menu structure).
*	    GTMENU_NOMEM - CreateMenusA() ran out of memory.
*	    (V37)
*
*   RESULT
*	menu - pointer to the resulting initialized menu structure (or
*              the resulting FirstItem), with all the links for menu items
*              and subitems in place.
*              The result will be NULL if CreateMenusA() could not allocate
*              memory for the menus, or if the NewMenu array had an
*              illegal arrangement (eg. NM_SUB following NM_TITLE).
*              (see also the GTMN_SecondaryError tag above).
*
*   NOTES
*	The strings you supply for menu text are not copied, and must
*	be preserved for the life of the menu.
*	The resulting menus have no positional information.  You will
*	want to call LayoutMenusA() (or LayoutMenuItemsA()) to supply that.
*	CreateMenusA() automatically provides you with a UserData field
*	for each menu, menu-item or sub-item.  Use the GTMENU_USERDATA(menu)
*	or GTMENUITEM_USERDATA(menuitem) macro to access it.
*
*   BUGS
*	Prior to V39, if you put images into menus using IM_ITEM
*	or IM_SUB for a NewMenu->nm_Type, the image supplied had to
*	be an ordinary struct Image.  Starting with V39, you can use
*	boopsi images.
*
*   SEE ALSO
*	LayoutMenusA(), FreeMenus(), gadtools.h/GTMENU_USERDATA(),
*	gadtools.h/GTMENUITEM_USERDATA()
*
******************************************************************************
*
* Created:  21-Nov-89, Peter Cherna
* Modified: 30-Apr-90, Peter Cherna
*
*/

struct Menu * __asm
LIB_CreateMenusA( register __a0 struct NewMenu *newmenu,
		  register __a1 struct TagItem *taglist )
{
    ULONG size;
    ULONG *errorstore;
    /* First thing that can fail is TallyMenuMemory().  That would
     * indicate an invalid NewMenu arrangement.
     */
    ULONG errorcode = GTMENU_INVALID;

    UBYTE *alloc = NULL;

    MP(("CM:  Enter\n"));
    if (size = TallyMenuMemory( newmenu, getGTTagData(GTMN_FullMenu, FALSE, taglist)))
    {
	/* If allocation fails, we want to return "no memory" */
	errorcode = GTMENU_NOMEM;
	if (alloc = AllocVec(size, MEMF_CLEAR))
	{
	    MP(("CM:  Alloc'd at $%lx\n", alloc));
	    /* errorcode will be NULL or GTMENU_TRIMMED */
	    errorcode = InitMenuMemory(newmenu, alloc,
		(UBYTE) getGTTagData(GTMN_FrontPen, 0, taglist) );
	}
    }

    /* Stuff the errorcode if a place was provided: */
    if ( errorstore = (ULONG *)getGTTagData(GTMN_SecondaryError, NULL, taglist ) )
    {
	*errorstore = errorcode;
    }

    MP(("CM:  Returning menu at $%lx with errorcode\n", alloc, errorcode));
    return((struct Menu *)alloc);
}


/*------------------------------------------------------------------------*/

/*/ TallyMenuMemory()
 *
 * Figure out how much memory this menu strip would require.
 * Returns NULL if an illegal NewMenu sequence is encountered.
 * If fullmenu is TRUE, requires that this be a complete menu
 * specification (i.e. not an otherwise valid menu fragment).
 *
 * Created:  30-Apr-90, Peter Cherna (from CreateMenusA())
 * Modified: 23-Oct-90, Peter Cherna
 *
 */

static ULONG TallyMenuMemory( struct NewMenu *nm, BOOL fullmenu )
{
    ULONG size;
    UBYTE prevtype = 0;

    /* Tally up the amount of memory needed for the whole menu. */
    size = 0;
    if ( ( fullmenu ) && ( nm->nm_Type != NM_TITLE ) )
    {
	DP(("TMM: Failing because fullmenu doesn't begin with a title\n"));
	return(0);
    }

    while (nm->nm_Type)
    {
	if ( !( nm->nm_Type & NM_IGNORE ) )
	{
	    if (nm->nm_Type == NM_TITLE)
	    {
		/* Titles need a struct Menu and a pointer */
		size += sizeof(struct Menu) + sizeof(void *);
	    }
	    else
	    {
		if (nm->nm_Label == NM_BARLABEL)
		{
		    /* Separator bars need a MenuItem, an Image, and a pointer */
		    size += sizeof(struct MenuItem) + sizeof(struct Image) +
			sizeof(void *);
		}
		else if (nm->nm_Type & MENU_IMAGE)
		{
		    /* Custom imagery needs a MenuItem, an Image structure,
		     * and a pointer, as well as a WORD to stash the original
		     * image->LeftEdge for use in re-layout:
		     */
		    size += sizeof(struct MenuItem) + sizeof(struct Image) +
			sizeof(void *) + sizeof(WORD);

		    /* This Image structure will not be used for custom images
		     * but we still allocate it cause it makes things simpler
		     */
		}
		else
		{
		    /* Text menuitems and subitems need a MenuItem, an IntuiText,
		     * and a pointer:
		     */
		    size += sizeof(struct MenuItem) + sizeof(struct IntuiText) +
			sizeof(void *);
		    /* New for V39: NM_COMMANDSTRING gives an arbitrary string
		     * right-justified in the menu.  This requires another
		     * IntuiText.
		     */
		    if ( nm->nm_Flags & NM_COMMANDSTRING )
		    {
			size += sizeof( struct IntuiText );
		    }
		}

		/* Let's handle a few things about subitems */
		if ( NM_TRUETYPE(nm->nm_Type) == NM_SUB )
		{
		    if ( prevtype == NM_TITLE )
		    {
			/* Sub-items can only follow sub-items or items! */
			return(0);
		    }
		    else if ( prevtype == NM_ITEM )
		    {
			/* This is the first submenu.  We'll need a spare IntuiText
			 * for the parent MenuItem's '>>' indicator, if the parent
			 * was a text-menuitem:
			 */
			size += sizeof(struct IntuiText);
		    }
		}
	    }
	    prevtype = nm->nm_Type;
	}
	nm++;
    }
    MP(("TMM:  Total alloc size: $%lx\n", size));
    return(size);
}

/*------------------------------------------------------------------------*/

/*/ InitMenuMemory()
 *
 * Fill the supplied memory with all the layout-insensitive information
 * from the supplied NewMenu array.
 * Returns FALSE if there were too many menus, items, or subitems
 * still initializes a valid structure, tho.
 *
 * Created:  30-Apr-90, Peter Cherna (from CreateMenusA())
 * Modified: 23-Oct-90, Peter Cherna
 *
 */

static ULONG InitMenuMemory( struct NewMenu *nm, UBYTE *memory, UBYTE frontpen )
{
    struct Menu *menu = NULL;
    struct MenuItem *item = NULL;
    struct MenuItem *sub = NULL;
    struct MenuItem *thisitem;
    void *itemfill;
    WORD menucount = -1;
    WORD itemcount = -1;
    WORD subcount= -1;
    ULONG error = 0; /* Innocent until proven guilty */
    BOOL commandstring;

    while ( ( nm->nm_Type ) && ( menucount < NOMENU ) )
    {
	if ( !( nm->nm_Type & NM_IGNORE ) )
	{
	    commandstring = FALSE;

	    if (nm->nm_Type == NM_TITLE)
	    {
		if ( ++menucount < NOMENU )
		{
		    MP(("CM:  Creating Menu '%s' at $%lx\n", nm->nm_Label, memory));
		    if (menu)
		    {
			/* Link it to previous menu, if any */
			MP(("CM:  Linking to previous Menu ($%lx)\n", menu));
			menu->NextMenu = (struct Menu *)memory;
		    }
		    menu = (struct Menu *)memory;
		    memory += sizeof(struct Menu) + sizeof(void *);
		    menu->MenuName = nm->nm_Label;
		    /* We invert the sense of Menu->Flags MENUENABLED field,
		     * so that they're enabled by default:
		     */
		    menu->Flags = nm->nm_Flags ^ NM_MENUDISABLED;
		    GTMENU_USERDATA(menu) = nm->nm_UserData;
		    item = NULL;
		    itemcount = -1;
		}
		else
		{
		    error = GTMENU_TRIMMED;
		}
	    }
	    else
	    {
		/* It's an item or subitem */
		thisitem = (struct MenuItem *)memory;
		memory += sizeof(struct MenuItem) + sizeof(void *);

		if (NM_TRUETYPE(nm->nm_Type) == NM_ITEM)
		{
		    if ( ++itemcount < NOITEM )
		    {
			/* This is a MenuItem: */
			MP(("CM:  Creating MenuItem '%s' at $%lx\n", nm->nm_Label,
			    thisitem));
			if (item)
			{
			    /* Another item, so link it in to previous */
			    MP(("CM:  Linking into previous MenuItem ($%lx)\n", item));
			    item->NextItem = thisitem;
			}
			else
			{
			    /* The first item, so link it in to the menu itself,
			     * if there is one
			     */
			    MP(("CM:  First MenuItem of Menu at $%lx\n", menu));
			    if (menu)
			    {
				menu->FirstItem = thisitem;
			    }
			}
			item = thisitem;
			sub = NULL;
			subcount = -1;
		    }
		    else
		    {
			error = GTMENU_TRIMMED;
			subcount = NOSUB;
		    }
		}
		else /* if (NM_TRUETYPE(nm->nm_Type) == NM_SUB) */
		{
		    if ( ++subcount < NOSUB )
		    {
			MP(("CM:  Creating SubItem '%s' at $%lx\n", nm->nm_Label,
			    thisitem));
			if (sub)
			{
			    /* Another subitem, so link it into the previous */
			    MP(("CM:  Linking into previous SubItem ($%lx)\n", sub));
			    sub->NextItem = thisitem;
			}
			else
			{
			    /* The first subitem, so link it in to the
			     * menuitem itself, if there is one
			     */
			    MP(("CM:  First SubItem of MenuItem at $%lx\n", item));
			    if (item)
			    {
				item->SubItem = thisitem;
				/* Also, arrange for the item to have a second
				 * IntuiText, consisting of the '>>' symbol,
				 * provided it is a text-item:
				 */
				if (item->Flags & ITEMTEXT)
				{
				    itemfill = ITEXT(item->ItemFill)->NextText =
					ITEXT(memory);
				    memory += sizeof(struct IntuiText);
				    ITEXT(itemfill)->FrontPen = frontpen;
				    ITEXT(itemfill)->DrawMode = JAM1;
				    ITEXT(itemfill)->TopEdge = 1;
				    ITEXT(itemfill)->IText = "»";
				}
			    }
			}
			sub = thisitem;
		    }
		    else
		    {
			error = GTMENU_TRIMMED;
		    }
		}

		if ( subcount < NOSUB )
		{
		    /* We invert the sense of the MenuItem->Flags ITEMENABLED
		     * field, so that they're enabled by default:
		     */
		    thisitem->Flags = (nm->nm_Flags & NM_FLAGMASK_V39) ^ NM_ITEMDISABLED;
		    if (nm->nm_CommKey)
		    {
			/* Under V37, a non-NULL nm_CommKey implies that there's
			 * a command-key sequence, so we should set that, and set
			 * this item's COMMSEQ flag.  Starting with V39, we allow
			 * arbitrary strings stored in the nm_CommKey field to
			 * be displayed along the right-hand side of the menu.
			 * The programmer should specify the nm_Flags bit
			 * NM_COMMANDSTRING, which is numerically equal to COMMSEQ.
			 * Effectively, then, its sense is toggled.
			 */
			thisitem->Flags ^= NM_COMMANDSTRING;
			if ( thisitem->Flags & COMMSEQ )
			{
			    /* Amiga-key equivalent */
			    thisitem->Command = *nm->nm_CommKey;
			}
			else
			{
			    /* We'll need an independent IntuiText for
			     * proper right-justification
			     */
			    commandstring = TRUE;
			}
		    }
		    thisitem->MutualExclude = nm->nm_MutualExclude;
		    GTMENUITEM_USERDATA(thisitem) = nm->nm_UserData;
		    /* Whether image or text, the ItemFill follows: */
		    itemfill = thisitem->ItemFill = (APTR) memory;
		    if (nm->nm_Label == NM_BARLABEL)
		    {
			/* A separator bar: */
			memory += sizeof(struct Image);
			/* We want the Depth, ImageData, and PlanePick to be
			 * zero, which is how the memory was initialized
			 */
			IMAGE(itemfill)->LeftEdge = 2;
			IMAGE(itemfill)->TopEdge = 2;
			IMAGE(itemfill)->Height = 2;
			IMAGE(itemfill)->PlaneOnOff = frontpen;
			/* Separator bars are disabled, and don't highlight: */
			thisitem->Flags |= HIGHNONE;
			thisitem->Flags &= ~ITEMENABLED;
		    }
		    else if (nm->nm_Type & MENU_IMAGE)
		    {
                        memory += sizeof(struct Image) + sizeof(WORD);
    		        if (IMAGE(itemfill)->Depth == CUSTOMIMAGEDEPTH)
		        {
		            /* for boopsi images, point the item fill to the
                             * image itself, and don't copy it
                             */
                            itemfill = thisitem->ItemFill = (APTR)nm->nm_Label;
                            SetAttrs(itemfill,IA_Top,IMAGE(itemfill)->TopEdge + 1,
                                              TAG_DONE);
		        }
		        else
		        {
                            /* A custom image: */
                            *IMAGE(itemfill) = *IMAGE(nm->nm_Label);
                            IMAGE(itemfill)->TopEdge += 1;
                            thisitem->Flags |= HIGHCOMP;
                        }
                        ((struct ImageMenuItem *)thisitem)->imi_OrigLeft =
                                IMAGE(itemfill)->LeftEdge;
		    }
		    else
		    {
			/* A regular text item: */
			memory += sizeof(struct IntuiText);
			MP(("CM:  IntuiText at $%lx\n", itemfill.itext));
			ITEXT(itemfill)->FrontPen = frontpen;
			ITEXT(itemfill)->DrawMode = JAM1;
			ITEXT(itemfill)->TopEdge = 1;
			ITEXT(itemfill)->IText = nm->nm_Label;
			thisitem->Flags |= ITEMTEXT | HIGHCOMP;
			if ( commandstring )
			{
			    /* Connect up a second IntuiText, and start it
			     * a copy of the first.
			     */
			    *ITEXT(memory) = *ITEXT(itemfill);
			    ITEXT(itemfill)->NextText = ITEXT(memory);
			    ITEXT(memory)->IText = nm->nm_CommKey;
			    memory += sizeof(struct IntuiText);
			}
		    }
		}
	    }
	}
	nm++;
    }
    return( error );
}

/*------------------------------------------------------------------------*/

/****** gadtools.library/FreeMenus *******************************************
*
*   NAME
*	FreeMenus -- frees memory allocated by CreateMenusA(). (V36)
*
*   SYNOPSIS
*	FreeMenus(menu)
*	          A0
*
*	VOID FreeMenus(struct Menu *);
*
*   FUNCTION
*	Frees the menus allocated by CreateMenusA().  It is safe to
*	call this function with a NULL parameter.
*
*   INPUTS
*	menu - pointer to menu structure (or first MenuItem) obtained
*	       from CreateMenusA().
*
*   SEE ALSO
*	CreateMenusA()
*
******************************************************************************
*
* Created:  21-Nov-89, Peter Cherna
* Modified: 26-Nov-89, Peter Cherna
*
*/

void __asm
LIB_FreeMenus( register __a0 struct Menu *menu )
{
    FreeVec(menu);
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/LayoutMenusA ****************************************
*
*   NAME
*	LayoutMenusA -- position all the menus and menu items. (V36)
*	LayoutMenus -- varargs stub for LayoutMenusA(). (V36)
*
*   SYNOPSIS
*	success = LayoutMenusA(menu, vi, tagList)
*	D0                     A0    A1  A2
*
*	BOOL LayoutMenusA(struct Menu *, APTR, struct TagItem *);
*
*	success = LayoutMenus(menu, vi, firsttag, ...)
*
*	BOOL LayoutMenus(struct Menu *, APTR, Tag, ...);
*
*   FUNCTION
*	Lays out all the menus, menu items and sub-items in the supplied
*	menu according to the supplied visual information and tag parameters.
*	This routine attempts to columnize and/or shift the MenuItems in
*	the event that a menu would be too tall or too wide.
*
*   INPUTS
*	menu - pointer to menu obtained from CreateMenusA().
*	vi - pointer returned by GetVisualInfoA().
*	tagList - pointer to an array of tags providing optional extra
*		  parameters, or NULL.
*
*   TAGS
*	GTMN_TextAttr (struct TextAttr *) - Text Attribute to use for
*	    menu-items and sub-items.  If not supplied, the screen's
*	    font will be used.  This font must be openable via OpenFont()
*	    when this function is called. (V36)
*	GTMN_NewLookMenus (BOOL) - If you ask for WA_NewLookMenus for your
*	    window, you should ask for this tag as well.  This informs GadTools
*	    to use the appropriate checkmark, Amiga-key, and colors. (V39)
*	GTMN_Checkmark (struct Image *) - If you are using a custom image for a
*	    checkmark (WA_Checkmark), also pass it to GadTools, so it can lay
*	    the menus out accordingly. (V39)
*	GTMN_AmigaKey (struct Image *) - If you are using a custom image for
*	    the Amiga-key in menus (WA_AmigaKey), also pass it to GadTools, so
*	    it can lay the menus out accordingly. (V39)
*	GTMN_FrontPen (ULONG) - This tag has existed for CreateMenus(), but now
*	    LayoutMenusA() has it too.  If a legitimate pen number is supplied,
*	    it is used for coloring the menu items (in preference to
*	    anything passed to CreateMenus()).  If GTMN_NewLookMenus
*	    has been specified, this tag defaults to using the
*	    screen's BARDETAILPEN, else it defaults to "do nothing".
*	    For visual consistency, we recommend you omit this tag in all
*	    functions, and let the defaults be used. (V39)
*
*   RESULT
*	success - TRUE if successful, FALSE otherwise (signifies that
*	          the TextAttr wasn't openable).
*
*   NOTES
*	When using this function, there is no need to also call
*	LayoutMenuItemsA().
*
*   BUGS
*	If a menu ends up being wider than the whole screen, it will
*	run off the right-hand side.
*
*   SEE ALSO
*	CreateMenusA(), GetVisualInfoA()
*
******************************************************************************
*
*   NOTES
*	The vi parameter is internally declared as a struct VisualInfo *
*
* Created:  21-Nov-89, Peter Cherna (from David Junod's menus.c)
* Modified: 16-May-90, Peter Cherna
*
*/

/* This function is declared here as long, but publicly it is known
 * as a BOOL.  This way we guarantee that the high-word of d0 is clear.
 */

LONG __asm
LIB_LayoutMenusA( register __a0 struct Menu *firstmenu,
		  register __a1 struct VisualInfo *vi,
		  register __a2 struct TagItem *taglist )
{
    struct RastPort textrp;		/* Temporary RastPort */
    struct Menu *menu;			/* Menu to adjust */
    struct TextFont *font;		/* Font to use */
    UWORD start;			/* Horiz start pixel */
    LONG retval = FALSE;
    struct TextAttr *tattr = (struct TextAttr *) getGTTagData(GTMN_TextAttr,
	(ULONG) vi->vi_Screen->Font, taglist);
    ULONG imagewidths = MenuImageWidths(vi, taglist);
    LONG frontpen = GetFrontPen(vi, taglist);

    /* open the font */
    InitRastPort(&textrp);
    DP(("LM:  About to OpenFont\n"));
    if((font = OpenFont(tattr)))
    {
	DP(("LM:  OpenFont succeeded\n"));

	/* Set the starting edge of the menu titles */
	start = 2;

	/* Step thru the menu structure and adjust it */
	menu = firstmenu;
	while (menu)
	{
	    /* Use the screen font for the menu titles, and the user's
	     * supplied font for the MenuItems:
	     */
	    SetFont(&textrp, vi->vi_ScreenFont);
	    menu->LeftEdge = start;
	    /* Intuition starts the text at BarHBorder-BarVBorder
	     * into the highlighting (believe it or not)!  So we
	     * make the extra width twice that:
	     */
	    menu->Width = TextLength(&textrp, menu->MenuName,
		    (UWORD)strlen(menu->MenuName)) +
		    2 * (vi->vi_Screen->BarHBorder - vi->vi_Screen->BarVBorder);
	    SetFont(&textrp, font);
	    /* The real left value seems to be five pixels over from the
	     * Menu->LeftEdge value, and the real top value is one
	     * greater than the BarHeight
	     */
	    DoMenuItemLayout(&textrp, menu->FirstItem, tattr,
		(WORD)(menu->LeftEdge+5), menu->Width,
		imagewidths, frontpen, vi);
	    start += (menu->Width + vi->vi_ScreenFont->tf_XSize);
	    menu = menu->NextMenu;
	}
	/* Close the font */
	closeFont(font);
	retval = TRUE;
    }
#ifdef DEBUGGING
    else
    {
	DP(("LM:  OpenFont of tattr $%lx failed\n", tattr));
	DP(("LM:  ta_Name: '%s'\n", tattr->ta_Name));
	DP(("LM:  ta_YSize: %ld\n", tattr->ta_YSize));
	DP(("LM:  ta_Style: $%lx\n", tattr->ta_Style));
	DP(("LM:  ta_Flags: $%lx\n\n", tattr->ta_Flags));

	DP(("LM:  VisualInfo at $%lx\n", vi));
	DP(("LM:  vi_Screen: $%lx\n", vi->vi_Screen));
	DP(("LM:  vi->vi_Screen->Font: $%lx\n", vi->vi_Screen->Font));
	DP(("LM:  vi->vi_Screen->Font->ta_Name: '%s'\n", vi->vi_Screen->Font->ta_Name));
	DP(("LM:  vi->vi_Screen->Font->ta_YSize: %ld\n", (LONG)vi->vi_Screen->Font->ta_YSize));
	DP(("LM:  vi->vi_Screen->Font->ta_Style: $%lx\n", (LONG)vi->vi_Screen->Font->ta_Style));
	DP(("LM:  vi->vi_Screen->Font->ta_Flags: $%lx\n", (LONG)vi->vi_Screen->Font->ta_Flags));
    }
#endif
    DP(("LM:  Exit\n"));
    return(retval);
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/LayoutMenuItemsA ************************************
*
*   NAME
*	LayoutMenuItemsA -- position all the menu items. (V36)
*	LayoutMenuItems -- varargs stub for LayoutMenuItemsA(). (V36)
*
*   SYNOPSIS
*	success = LayoutMenuItemsA(menuitem, vi, tagList)
*	D0                         A0        A1  A2
*
*	BOOL LayoutMenuItemsA(struct MenuItem *, APTR, struct TagItem *);
*
*	success = LayoutMenuItems(menuitem, vi, firsttag, ...)
*
*	BOOL LayoutMenuItemsA(struct MenuItem *, APTR, Tag, ...);
*
*   FUNCTION
*	Lays out all the menu items and sub-items according to
*	the supplied visual information and tag parameters.  You would use this
*	if you used CreateMenusA() to make a single menu-pane (with sub-items,
*	if any), instead of a whole menu strip.
*	This routine attempts to columnize and/or shift the MenuItems in
*	the event that a menu would be too tall or too wide.
*
*   INPUTS
*	menuitem - pointer to first MenuItem in a linked list of
*	           items.
*	vi - pointer returned by GetVisualInfoA().
*	tagList - pointer to an array of tags providing optional extra
*		  parameters, or NULL.
*
*   TAGS
*	GTMN_Menu (struct Menu *) - Pointer to the Menu structure whose
*	    FirstItem is the MenuItem supplied above.  If the menu items are
*	    such that they need to be columnized or shifted, the Menu
*	    structure is needed to perform the complete calculation.
*	    It is suggested you always provide this information. (V36)
*
*	For the following tags, please see the description under
*	LayoutMenusA().  Their behavior is identical when used in
*	LayoutMenuItemsA().
*
*	    GTMN_TextAttr
*	    GTMN_NewLookMenus
*	    GTMN_Checkmark
*	    GTMN_AmigaKey
*	    GTMN_FrontPen
*
*   RESULT
*	success - TRUE if successful, FALSE otherwise (signifies that
*	          the TextAttr wasn't openable).
*
*   BUGS
*	If a menu ends up being wider than the whole screen, it will
*	run off the right-hand side.
*
*   SEE ALSO
*	CreateMenusA(), GetVisualInfoA()
*
******************************************************************************
*
*   NOTES
*	The vi parameter is internally declared as a struct VisualInfo *
*
* Created:  22-Nov-89, Peter Cherna
* Modified: 22-Mar-90, Peter Cherna
*
*/

/* This function is declared here as long, but publicly it is known
 * as a BOOL.  This way we guarantee that the high-word of d0 is clear.
 */

LONG __asm
LIB_LayoutMenuItemsA( register __a0 struct MenuItem *firstitem,
		      register __a1 struct VisualInfo *vi,
		      register __a2 struct TagItem *taglist )
{
    struct RastPort textrp;		/* Temporary RastPort */
    struct TextFont *font;		/* Font to use */
    LONG retval = FALSE;
    WORD realleft;
    WORD menuwidth;
    struct Menu *menu;
    struct TextAttr *tattr = (struct TextAttr *)getGTTagData(GTMN_TextAttr,
	(ULONG)vi->vi_Screen->Font, taglist);
    ULONG imagewidths = MenuImageWidths(vi, taglist);
    LONG frontpen = GetFrontPen(vi, taglist);

    DP(("LMIA:  firstitem: $%lx, vi: $%lx\n", firstitem, vi));
    InitRastPort(&textrp);
    /* open the font */
    if((font = OpenFont(tattr)))
    {
	/* Install the font: */
	SetFont(&textrp, font);
	DP(("LMIA:  vi->vi_Screen: $%lx\n", vi->vi_Screen));

	/* Let realleft default to 5 plus the 2 that we normally have */
	realleft = 7;
	/* Let menuwidth sets a minimum, so if we don't know the menu,
	 * it can be zero
	 */
	menuwidth = 0;

	if (menu = (struct Menu *)getGTTagData(GTMN_Menu, NULL, taglist))
	{
	    DP(("LMIA:  menu: $%lx, menu->FirstItem: $%lx\n",
		menu, menu->FirstItem));
	    realleft = menu->LeftEdge+5;
	    menuwidth = menu->Width;
	}
	/* Step thru the menu structure and adjust it */
	DoMenuItemLayout(&textrp, firstitem, tattr,
		realleft, menuwidth, imagewidths, frontpen, vi);
	/* Close the font */
	closeFont(font);
	retval = TRUE;
    }
    return(retval);
}


/*------------------------------------------------------------------------*/

/*/ DoMenuItemLayout()
 *
 * Function to adjust the size/placement of all the MenuItems in a
 * menu plane (menu or sub-menu).
 *
 * Created:  21-Nov-89, Peter Cherna (from David Junod's menus.c)
 * Modified: 22-Mar-90, Peter Cherna
 *
 */

static void DoMenuItemLayout( struct RastPort *txtrp, struct MenuItem *fi,
    struct TextAttr *tattr, WORD realleft, WORD menuwidth, ULONG imagewidths,
    LONG frontpen, struct VisualInfo *vi )
{
    UWORD itemheight;

    if (fi)
    {
        /* To prevent crowding of the Amiga key when using COMMSEQ,
         * don't allow the items to be less than 8 pixels high.
         */
        itemheight = max(txtrp->Font->tf_YSize, 8) + 1;

        /* Set all the heights and fonts for the menu items */
        SizeItems(fi, itemheight, tattr, frontpen);

        /* Do the actual placement */
        PlaceItems(txtrp, fi, tattr, 0, 0, realleft, vi->vi_Screen->BarHeight+1,
            menuwidth, imagewidths, vi);
    }
}


/*------------------------------------------------------------------------*/

/*/ SizeItems()
 *
 * Sets the fields all items and sub items in this pane that are
 * independent of the actual placement, namely font and item height.
 *
 * Created:  20-Mar-90, Peter Cherna
 * Modified: 30-Apr-90, Peter Cherna
 *
 */

static void SizeItems( struct MenuItem *item, UWORD itemheight,
    struct TextAttr *tattr, LONG frontpen )
{
    struct IntuiText *itext;

    /* Go through the menuitems, filling in item height, and text
     * or graphics
     */

    while (item)
    {
	if (item->Flags & ITEMTEXT)
	{
	    /* We have a text item.  Set its height and font accordingly */
	    item->Height = itemheight;
	    itext = ITEXT(item->ItemFill);
	    /* Set font in IntuiText */
	    itext->ITextFont = tattr;
	    if (frontpen != -1)
	    {
		itext->FrontPen = frontpen;
	    }
	    /* If there's a chained text, then it's the '>>' sub-menu
	     * indicator, or a command-string.  Set its font too:
	     */
	    if (itext = itext->NextText)
	    {
		itext->ITextFont = tattr;
		if (frontpen != -1)
		{
		    itext->FrontPen = frontpen;
		}
	    }
	}
	else if ((item->Flags & HIGHFLAGS) == HIGHNONE)
	{
	    /* We have a separator bar.  Set its height and width
	     * accordingly:
	     */
	    item->Height = 6;
	    if (frontpen != -1)
	    {
		IMAGE(item->ItemFill)->PlaneOnOff = frontpen;
	    }
	}
	else
	{
	    /* A custom image. */
	    item->Height = IMAGE(item->ItemFill)->Height + 1;
	}

	/* Do for submenus too */
	SizeItems(item->SubItem, itemheight, tattr, frontpen);
	item = item->NextItem;
    }
}

/*------------------------------------------------------------------------*/

/*/ PlaceItems()
 *
 * Function to position all the MenuItems in a  menu plane (menu or sub-menu).
 *
 * Created:  21-Nov-89, Peter Cherna (from David Junod's menus.c)
 * Modified:  8-May-90, Peter Cherna
 *
 */

static void PlaceItems( struct RastPort *txtrp, struct MenuItem *fi,
    struct TextAttr *tattr, WORD leftoffset, WORD topoffset,
    WORD realleft, WORD realtop, WORD minwidth, ULONG imagewidths,
    struct VisualInfo *vi )
{
    struct IntuiText *itext;
    struct MenuItem *item = fi;
    UWORD itemtop;
    WORD sub_leftoffset;		/* offset for submenus */
    struct ColumnInfo column;
    WORD extra, panelwidth, panelheight;
    WORD maxheight;
    WORD extrawidth;
    WORD numcolumns = 0;

    if (fi)
    {
        /* !!! I think the best strategy here would be to note down the
         * desired left edge of things, but then set the left edge of the
         * menu to the extreme left, and proceed with the layout.  Then,
         * looking at widths, we try to move the menu over as far right
         * as we can, but not exceeding the desired left edge nor exceeding
         * the right of the screen, if possible !!!
         */
        /* The maximum allowable height for this column is the screen height
         * less two (for the two pixel bottom trim Intuition supplies for
         * menus) less the real top coordinate of this menu:
         */
        maxheight = vi->vi_Screen->Height - 2 - (topoffset + realtop);
        /* Figure out the total width of this group of menu items by
         * summing the width for each column
         */
        panelwidth = -MULTICOLUMN_GAP;
        panelheight = 0;
        column.nextitem = item;
        while (column.nextitem)
        {
            numcolumns++;
            AboutColumn(&column, txtrp, column.nextitem,
                maxheight, imagewidths, vi);
            panelwidth += column.width + MULTICOLUMN_GAP;
            panelheight = max(panelheight, column.height);
            DP(("PI:  panelwidth is %ld\n", panelwidth));
        }

        /* To ensure that narrow submenus are stretched to cover at least
         * the whole menu panel:
         */
        extrawidth = (max(minwidth - panelwidth, 0) / numcolumns) + 1;
        panelwidth += extrawidth * numcolumns;

        if (topoffset)
        {
            /* Only submenus have topoffset != 0.
             * If the submenu would fit without difficulty, we would like to
             * have a topoffset of -1, but we're willing to shift it up to fit
             * more subitems to avoid columnization.
             * Hence we calculate the offset needed to have the submenu panel
             * just hit the very bottom, but never let it exceed -1:
             */
            topoffset = min(-1, vi->vi_Screen->Height - 2 - realtop - panelheight);
        }

        /* There are four pixels on the right edge of the menu, past
         * the select box:
         */
        extra = (realleft + panelwidth - vi->vi_Screen->Width + 4);
        DP(("PI:  realleft %ld, extra: %ld\n", (LONG)realleft, (LONG)extra));
        if (extra > 0)
        {
            DP(("PI:  extra of %ld, leftoffset reduced from %ld", extra, leftoffset));
            /* Too wide!  Do the best we can by moving the left over.  We
             * should never reduce realleft to less than four, which is the
             * amount of left-side menu trim Intuition supplies.
             * If the menu is wider than the screen, then we say too
             * bad and you extend off the right side... (may fix later
             * by causing some menu items to be dropped)
             */
            leftoffset -= min(extra, realleft-4);
            DP((" to %ld\n", leftoffset));
        }

        /* By setting column.count to zero, we'll call AboutColumn()
         * as soon as we get into in the loop
         */
        column.count = 0;
        column.width = 0;
        leftoffset -= MULTICOLUMN_GAP;
        while (item)
        {
            /* Would this item go off the bottom of the screen? */
            if (column.count-- == 0)
            {
                itemtop = topoffset;
                leftoffset += column.width + MULTICOLUMN_GAP;
                AboutColumn(&column, txtrp, item, maxheight, imagewidths, vi);
                column.width += extrawidth;
                /* Restart loop in the new column: */
                continue;
            }

            item->TopEdge = itemtop;
            item->LeftEdge = leftoffset;
            item->Width = column.width;
            /* Do the part that depends on the strip-width */
            if (item->Flags & ITEMTEXT)
            {
                itext = ITEXT(item->ItemFill);
                DP(("PI:  Menu item '%s', LeftEdge %ld\n", itext->IText,
                    item->LeftEdge));
                /* If there's a chained text, then it's the '>>' sub-menu
                 * indicator or a command string.
                 * Position that off to the right edge:
                 */
                if (itext = itext->NextText)
                {
                    itext->LeftEdge = column.width - 2 - IntuiTextLength( itext );
                }
            }
            else if ((item->Flags & HIGHFLAGS) == HIGHNONE)
            {
                /* Only separator bars have HIGHNONE */
                DP(("PI:  Menu item Separator Bar\n"));
                /* We have a separator bar.  Set its width accordingly: */
                /* Set width of separator bar  */
                IMAGE(item->ItemFill)->Width = column.width-4;
            }
            /* else (we have a custom image), but nothing needs to be done here */

            /* Submenus begin 3/4 of the way over: */
            minwidth = column.width >> 2;
            sub_leftoffset = column.width - minwidth;
            DP(("PI:  sub_leftoffset: %ld\n", (LONG)sub_leftoffset));
            /* Layout the submenus (if any) */
            PlaceItems(txtrp, item->SubItem, tattr,
                sub_leftoffset, (WORD)(-1-itemtop),
                (WORD)(realleft+leftoffset+sub_leftoffset), (WORD)(itemtop + realtop),
                minwidth, imagewidths, vi);
            itemtop += item->Height;
            item = item->NextItem;
        }
    }
}


/*------------------------------------------------------------------------*/

/*/ AboutColumn()
 *
 * Function to determine the width of the select box for a single
 * column of a menu pane based on the longest menuitem.  Also determines
 * The number of items that will fit in this column.
 * This routine adds a little trim, and takes into account any item with
 * key equivalents.
 *
 * Created:  21-Nov-89, Peter Cherna (from David Junod's menus.c)
 * Modified: 30-Apr-90, Peter Cherna
 *
 */

static void AboutColumn( struct ColumnInfo *col, struct RastPort *txtrp,
    struct MenuItem *fi, UWORD maxheight, ULONG imagewidths,
    struct VisualInfo *vi )
{
    UWORD maxlen = 0;
    struct MenuItem *item = fi;
    struct IntuiText *itext;
    struct Image *im;
    ULONG righttrim;
    UWORD count;
    WORD checkwidth = imagewidths & 0x0000FFFF;
    WORD left;

    /* We have a default extra width (on the right) of 2 */
    righttrim = 2;

    /* Count up how many items fit in this column */
    col->count = 0;
    col->height = 0;
    col->nextitem = NULL;
    while (item)
    {
	col->height += item->Height;
	MP(("AC:  Item of height %ld would make col->height %ld\n", item->Height,
	    col->height));
	/* Two pixels for the bottom trim of the menu that Intuition
	 * supplies:
	 */
	if (col->height > maxheight)
	{
	    MP(("AC:  Cannot fit\n"));
	    /* This one would not fit, so stop */
	    col->nextitem = item;
	    col->height -= item->Height;
	    item = NULL;
	}
	else
	{
	    /* It fits, so count it and proceed to the next */
	    col->count++;
	    item = item->NextItem;
	}
    }

    /* Figure out how much room we need along the right-hand side of
     * the menu.  We need enough room for the widest of the Amiga-key
     * equivalents, command strings, or the ">>" submenu indicator.
     */
    item = fi;
    count = col->count;
    while (count--)
    {
	ULONG thisrighttrim = 0;

	/* If this item has an Amiga-key equivalent, account for that. */
	if (item->Flags & COMMSEQ)
	{
	    /* righttrim would be width of the chosen key, plus the
	     * correct amount for the command-key logo.  We also
	     * add tf_XSize as a gutter width.
	     * (commwidth is imagewidths >> 16).
	     */
	    thisrighttrim = TextLength(txtrp, &item->Command, 1) +
		(imagewidths >> 16) + txtrp->Font->tf_XSize;
	}
	/* If this item has a submenu or has an arbitrary command
	 * string, then we can detect that by discovering that
	 * the IntuiText has a NextText pointer.  In that case,
	 * the right-side width needs to account for that second
	 * text, which will either be the ">>" or else the
	 * command string.
	 */
	else if ( ( item->Flags & ITEMTEXT ) &&
	    ( itext = ITEXT(item->ItemFill)->NextText ) )
	{
	    /* righttrim would be width of the command string.
	     * We also add tf_XSize as a gutter width.
	     */
	    thisrighttrim = IntuiTextLength( itext ) + txtrp->Font->tf_XSize;
	}
	righttrim = max( righttrim, thisrighttrim );

	item = item->NextItem;
    }

    /* Find the longest item, based on TextLength() of the IntuiText
     * and its LeftEdge:
     */
    item = fi;
    count = col->count;
    while (count--)
    {
	if (item->Flags & ITEMTEXT)
	{
	    itext = ITEXT(item->ItemFill);
	    itext->LeftEdge = 2;
	    if (item->Flags & CHECKIT)
		itext->LeftEdge += checkwidth;

	    maxlen = max(maxlen, itext->LeftEdge + IntuiTextLength( itext ) );
	}
	else if ((item->Flags & HIGHFLAGS) != HIGHNONE)
	{
	    /* Custom menu imagery is not ITEMTEXT but is HIGHCOMP. */
	    im = IMAGE(item->ItemFill);
	    left = ((struct ImageMenuItem *)item)->imi_OrigLeft +
                    ((item->Flags & CHECKIT) ? (2 + checkwidth) : 2);

	    if (im->Depth == CUSTOMIMAGEDEPTH)
	    {
	        SetAttrs(im,IA_Left,left,
	                    TAG_DONE);
	    }
	    else
	    {
	        im->LeftEdge = left;
            }
	    maxlen = max(maxlen, im->LeftEdge + im->Width);
	}
	item = item->NextItem;
    }
    col->width = maxlen + righttrim;
    DP(("AC:  column width: %ld, column count: %ld\n", col->width, col->count));
}


/*------------------------------------------------------------------------*/

/*/ MenuImageWidths
 *
 * Returns the width of the command key and the checkmark for this
 * window.  The two are packed into a single ULONG, with the
 * commandkey-width in the MSW and the checkmark-width in the LSW.
 */

static ULONG MenuImageWidths( struct VisualInfo *vi, struct TagItem *taglist )
{
    ULONG commwidth, checkwidth;
    struct Image *im;

    /* For old menus, default to the old images */
    checkwidth = LOWCHECKWIDTH;
    commwidth = LOWCOMMWIDTH;

    /* This flag is marked Intuition-private, but hey, that's how
     * Intuition does it!
     */
    if (vi->vi_Screen->Flags & SCREENHIRES )
    {
	checkwidth = CHECKWIDTH;
	commwidth = COMMWIDTH;
    }

    /* If GTMN_NewLookMenus is specified, default to the images
     * in the screen.
     */
    if ( getGTTagData(GTMN_NewLookMenus, FALSE, taglist ) )
    {
	checkwidth = vi->vi_DrawInfo->dri_CheckMark->Width;
	commwidth = vi->vi_DrawInfo->dri_AmigaKey->Width;
    }

    /* Override with specifics */
    if ( im = (struct Image *)getGTTagData(GTMN_Checkmark, NULL, taglist) )
    {
	checkwidth = im->Width;
    }

    if ( im = (struct Image *)getGTTagData(GTMN_AmigaKey, NULL, taglist) )
    {
	commwidth = im->Width;
    }

    return( ( commwidth << 16 ) + checkwidth );
}

/*------------------------------------------------------------------------*/

/*/ GetFrontPen()
 *
 * Returns the correct FrontPen based on whether the caller is
 * requesting GTMN_NewLookMenus or supplying an explicit GTMN_FrontPen.
 * Defaults to dri_Pens[ BARDETAILPEN ] if GTMN_NewLookMenus is specified,
 * defaults to "do nothing" ((LONG) -1) if new-look is not specified.
 */

static LONG GetFrontPen( struct VisualInfo *vi, struct TagItem *taglist )
{
    /* GTMN_FrontPen defaults to "no effect", for compatibility */
    LONG frontpen = -1;

    if ( getGTTagData(GTMN_NewLookMenus, FALSE, taglist) )
    {
	/* If you specify GTMN_NewLookMenus, then GTMN_FrontPen
	 * defaults to the BARDETAILPEN, which is the "right thing"
	 */
	frontpen = vi->vi_DrawInfo->dri_Pens[BARDETAILPEN];
    }

    return ( (LONG)getGTTagData(GTMN_FrontPen, frontpen, taglist) );
}

/*------------------------------------------------------------------------*/
