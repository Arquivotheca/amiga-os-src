/****** MWLib/MenuTools *******************************************************
    MenuTools.c

    Mitchell/Ware           Version 3.01            3/12/88

    Routines to make easy the access to windows

    EDIT History:

        3/22/90 (2.00) - Major changes, made reentrant. Code must be updated
                         to the new calls and re-compiled. See SetMenuWidth()
                         and SetMenuHeight().

        8/27/90 (2.10) - added itemdata. Made changes in parameter passing of
                         SetMenuItem() and SetSubItem() due to the fact that
                         no one is using the help facility yet.

        05 Jan 1992 (3.01)  - Changed MenuPac to struct MenuPac
  *****************************************************************************

    PREREQUSITE:    intuition and graphics libraries must be open

    ASSUMPTIONS:    width of characters in menu-strip is assumed to be 8 unless
                    otherwise specified.

    NOTE: All functions return NULL if failed, unless otherwise noted.

    struct MenuPac *InitMP()       returns the initial packet
    VOID    DInitMP(pac)    deallocate everything

    SetMenu (pac, name, seperation)
            pac is the initial packet
            name is the name of menu column
            seperation is the spacing from previous menu

    SetMenuItem(pac, name, flags, exclude, bytecmd, width, funct, itemdata, helpfile, helpfunct)
            name     -the name of this item
            flags    -the additional flags required (Flags are semi-auto)
            exclude  -the MutualExclude
            bytecmd  -NULL or the AmigaKey command
            width    -the width in pixels of the select box, or NULL
            funct    -(optional) is the call-back function to be used with DoMenu().
            data     -(optional) a ULONG item data field to be passed to functions.
                      Normally used as an identifier.
            helpfile -(optional) the name of the helpfile for this function, or NULL
            helpfunct -(optional) helpfunction to call, or NULL

    SetSubItem - same parms as SetMenuItem

    SetMenuFrontPen (pac, num)   change defaults...
    SetMenuBackPen (pac, num)
    SetMenuDrawMode (pac, flags)
    SetMenuWidth (pac, font_width)
    SetMenuHeight (pac, font_height)

    SetMenuHelpDirectory(pac, directory)
    SetMenuHelpDisplay(pac, funct_display)

    EnableMenu(pac, window, string)
    EnableMenuItem(pac, window, #menu, string)
    EnableSubItem(pac, window, #menu, #item, string)
        Where:
            pac     -the MenuPac packet
            window  -the window to which this menu is attached
            string  -string discribing the enabling (see below)
            #menu   -the menu number (first is zero)
            #item   -the item number (first is zero)

        Strings:
            Strings must match the part of the menu in length that they are
            modifying, as well as be null-terminated. Only 3 characters are
            allowed in the string -
                '1' for enable
                '0' for disable
                'x' for don't care- leave unmodified.
                    (Upper & lower case allowed.)

    BOOL DoMenu(pac, code, data1, data2, boolhelp)
            Total Menu Processing. uses callbacks.
                pac      -the MenuPac packet
                code     -the code field given by an menu selection event
                data1    -(optional) data1 packet to be passed to callback
                data2    -(optional) data2 packet to be passed to callback
                boolhelp -(required) TRUE if help is requested instead of
                          actual function.

            The callback receives:
                (*mfunct)(menu, menucode, data1, data2, itemdata)
                    menu        -the MenuPac packet
                    menucode    -the MenuCode for this menu selection
                    data1       -(optional) user data
                    data2       -(optional) user data
                    itemdata    -(optional) item data field

            If a particular menu selection has no corresponding code, it is
            ignored.

            DoMenu returns a TRUE if one or more of the callbacks returned
            TRUE. Otherwise, DoMenu returns FALSE.

            DoMenu, upon request (boolhelp), calls instead of the function
            to handle the menu selection, a specified help function, and will
            pass to it the following parameters:
                void (*helpfunction)(pac, code, pathname, data1, data2, itemdata)
                    pac         -the MenuPac packet
                    code        -the MenuCode responsible
                    pathname    -the (full) pathname for the helpfile
                    data1       -(optional) user data
                    date2       -(optional) user data

            The helpfunction is free to do whatever it wants to display the
            requested help information.

*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include "tools.h"
#include "Handy.h"
#include "protos.h"

#define PUB     (MEMF_CLEAR | MEMF_PUBLIC)

struct MenuPac *InitMP()       /* allocates first menu packet - or NULL */
{
    struct MenuPac *m;

    m = calloc(1, sizeof(*m));
    m->width = 8;   /* default width */
    m->height = 10; /* default height */
    m->FrontPen = 1;
    m->BackPen = 2;
    m->DrawMode = JAM1;

    return m;
}

void SetMenuHeight(struct MenuPac *m, short height)
{
    m->height = height + 2;
}

void SetMenuWidth(m, width)
struct MenuPac *m;
short width;
{
    m->width = width;
}

void SetMenuFrontPen(m, num)
struct MenuPac *m;
int num;
{
    m->FrontPen = num;
}

void SetMenuBackPen(m, num)
struct MenuPac *m;
int num;
{
    m->BackPen = num;
}

void SetMenuDrawMode(m, flag)
struct MenuPac *m;
ULONG flag;
{
    m->DrawMode = flag;
}

void SetMenuHelpDirectory(pac, directory)
struct MenuPac *pac;
char *directory;
{
    pac->helpdir = directory;
}

void SetMenuHelpDisplay(pac, funct)
struct MenuPac *pac;
void (*funct)();
{
    pac->helpfunct = funct;
}


void DInitMP(pac)
struct MenuPac *pac;
{
    if (!pac)
        return;

    if (pac->next)
        DInitMP(pac->next);

    FreeRemember(&pac->key, TRUE);
    free(pac);
}


struct Menu *SetMenu(struct MenuPac *pac, char *name, int sep)    // establish this as a new menustrip, return NULL if fails
{
    struct MenuPac *p, *pl;
    int offset;

    /* find the next pac that has a NULL ptr or add one to end */
    for (p = pac, offset = sep, pl = NULL; p && p->menu.MenuName; pl = p, p = p->next)
        offset += p->sep + p->length;
        /* offset contains where next menu will be displayed */

    if (!p)
        pl->next = p = (struct MenuPac *) calloc(1, sizeof(struct MenuPac));
    if (!p)
        return NULL;

    p->length = strlen(name) * pac->width;
    p->sep = sep;

    /* allocate a MENU structure and preform preliminary init. */

    if (pl)
        pl->menu.NextMenu = &p->menu;

    p->menu.LeftEdge = offset;
    p->menu.TopEdge = 0;
    p->menu.Width = p->length + pac->width;
    p->menu.Height = 0;

    p->menu.Flags = MENUENABLED;
    p->menu.MenuName = keyStrDup(&p->key, name);
    p->menu.FirstItem = NULL;

    return &p->menu;
}

struct MenuPac *_FindMenu(struct MenuPac *pac, int num) /* find menu entry in list, or last */
    //num- number of menu entry (first is zero), last is -1
{
    struct MenuPac *p;

    for (p = pac; p && num && p->next; p = p->next, --num)
           ;

    return p; /* returns NULL if and only if pac was NULL */
}

struct MenuItem * _FindMenuItem(mi, num) /* find menu item (or subitem) entry (or last) */
struct MenuItem *mi; /* first menu item in list */
int num; /* number (or -1 if to find last) */
{
    struct MenuItem *m;

    for (m = mi; m && num && m->NextItem; m = m->NextItem, --num)
        ;

    return m; /* returns NULL if and only if mi was NULL */
}


struct IntuiText *_CreateIText(m, key, text)
struct MenuPac *m;
struct Remember **key;
char *text;
{
        struct IntuiText *it;

        it = AllocRemember(key, sizeof(*it), PUB);
        if (!it)
                return NULL;

        it->FrontPen = m->FrontPen;
        it->BackPen = m->BackPen;
        it->DrawMode = m->DrawMode;
        it->LeftEdge = m->width;
        it->TopEdge = 1;
        it->ITextFont = NULL;
        it->IText = text;
        it->NextText = NULL;

        return it;
}


struct MenuItemExt *SetMenuItem(pac, name, flags, exc, cmd, width, mfunct, itemdata, help, helpfunct) /* set this entry */
struct MenuPac *pac;
char *name;         /* name */
USHORT flags;       /* flags for menuitem */
LONG exc;           /* mutual exclude */
UBYTE cmd;          /* Amiga command */
int width;          /* default pixel width or 0 (first entry MUST have width) */
BOOL (*mfunct)();   /* callback */
ULONG itemdata;     /* data identifying this packet */
char *help;         /* name of help file */
void (*helpfunct)(); // Ptr to help function, or NULL
{
    struct MenuPac *p;
    struct MenuItemExt *mi = NULL, *mil = NULL;
    struct IntuiText *it;

    p = _FindMenu(pac, -1); /* find last menu entry */
    mil = _FindMenuItem(p->menu.FirstItem, -1);

    if (!mil)
        mi = p->menu.FirstItem
            = AllocRemember(&p->key, sizeof(*mi), PUB);
    else
        mi = mil->mi.NextItem
            = AllocRemember(&p->key, sizeof(*mi), PUB);

    /* so mi is the current item, mil is the previous item or NULL */

    mi->mi.LeftEdge = pac->width;  /* allow room for a checkmark */
    mi->mi.TopEdge = (mil) ? mil->mi.TopEdge + pac->height : 0 ;
    mi->mi.Width = (width) ? width : ((mil) ? mil->mi.Width : strlen(name) * pac->width + pac->width);
    mi->mi.Height = pac->height;
    mi->mi.Flags = flags | ITEMTEXT | ((cmd) ? COMMSEQ : NULL) | HIGHCOMP;
    mi->mi.MutualExclude = exc;
    /* ItemFill and SelectFill is handled later */
    mi->mi.Command = cmd;
    mi->mi.SubItem = NULL;
    mi->mi.NextSelect = MENUNULL;
    mi->mfunct = mfunct;
    mi->help = help;
    mi->helpfunct = helpfunct;
    mi->itemdata = itemdata;

    /* setup an IntuiText structure */
    it = AllocRemember(&p->key, sizeof(*it), PUB);
    if (!(mi->mi.ItemFill = (APTR) _CreateIText(pac, &p->key, name)))
        return NULL;

    return mi;
}


struct MenuItemExt *SetSubItem(pac, name, flags, exc, cmd, width, mfunct, itemdata, help, helpfunct) /* set this sub entry */
struct MenuPac *pac;
char *name;         /* name */
USHORT flags;       /* flags for menuitem */
LONG exc;           /* mutual exclude */
UBYTE cmd;          /* Amiga command */
int width;          /* default pixel width or 0 (first entry MUST have width) */
BOOL (*mfunct)();
ULONG itemdata;     /* itemdata indentifyer */
char *help;         /* name of help file */
void (*helpfunct)(); // Ptr to help function, or NULL
{
        struct MenuPac *p;
        struct MenuItemExt *mi = 0, *smi = 0, *smil = 0;
        struct IntuiText *it;

        p = _FindMenu(pac, -1); /* find last menu entry */
        mi = _FindMenuItem(p->menu.FirstItem, -1);
        smil = _FindMenuItem(mi->mi.SubItem, -1);

        if (!smil)
                smi = mi->mi.SubItem
                        = AllocRemember(&p->key, sizeof(*smi), PUB);
        else
                smi = smil->mi.NextItem
                        = AllocRemember(&p->key, sizeof(*smi), PUB);

        /* so smi is the current subitem, smil is the previous item or NULL */

        smi->mi.LeftEdge = mi->mi.Width / 2;
        smi->mi.TopEdge = (smil) ? smil->mi.TopEdge + pac->height : 0 ;
        smi->mi.Width = (width) ? width : ((smil) ? smil->mi.Width : strlen(name) * pac->width + pac->width);
        smi->mi.Height = pac->height;
        smi->mi.Flags = flags | ITEMTEXT | ((cmd) ? COMMSEQ : NULL) | HIGHCOMP;
        smi->mi.MutualExclude = exc;
        /* ItemFill and SelectFill is handled later */
        smi->mi.Command = cmd;
        smi->mi.SubItem = NULL;
        smi->mi.NextSelect = MENUNULL;
        smi->mfunct = mfunct;
        smi->help = help;
        smi->helpfunct = helpfunct;
        smi->itemdata = itemdata;

        /* setup an IntuiText structure */
        it = AllocRemember(&p->key, sizeof(*it), PUB);
        if (!(smi->mi.ItemFill = (APTR) _CreateIText(pac, &p->key, name)))
                return NULL;

        return smi;
}

/*****************************************************************************

    The active Menu enable/disable section

*****************************************************************************/

void EnableMenu(mp, win, s)
struct MenuPac *mp;
struct Window *win;
UBYTE *s;
{
    register i;

    for (i = 0; *s; ++i, ++s)
        switch(*s)
        {
        case '1':  OnMenu(win, SHIFTMENU(i) | SHIFTITEM(NOITEM)); break;
        case '0': OffMenu(win, SHIFTMENU(i) | SHIFTITEM(NOITEM)); break;
        case 'x':
        case 'X': break;
        default:  fuckup("EM:Illegal Character In String"); break;
        }
}

void EnableMenuItem(mp, win, mno, s)
struct MenuPac *mp;
struct Window *win;
int mno;
UBYTE *s;
{
    register i;

    for (i = 0; *s; ++i, ++s)
        switch(*s)
        {
        case '1':  OnMenu(win, SHIFTMENU(mno) | SHIFTITEM(i) | SHIFTSUB(NOSUB)); break;
        case '0': OffMenu(win, SHIFTMENU(mno) | SHIFTITEM(i) | SHIFTSUB(NOSUB)); break;
        case 'x':
        case 'X': break;
        default:  fuckup("EMI:Illegal Character In String"); break;
        }
}

void EnableSubItem(mp, win, mno, ino, s)
struct MenuPac *mp;
struct Window *win;
int mno, ino;
UBYTE *s;
{
    register i;

    for (i = 0; *s; ++i, ++s)
        switch(*s)
        {
        case '1':  OnMenu(win, SHIFTMENU(mno) | SHIFTITEM(ino) | SHIFTSUB(i)); break;
        case '0': OffMenu(win, SHIFTMENU(mno) | SHIFTITEM(ino) | SHIFTSUB(i)); break;
        case 'x':
        case 'X': break;
        default:  fuckup("ESI:Illegal Character In String"); break;
        }
}

BOOL DoMenu(mp, code, data1, data2, boolhelp)
struct MenuPac *mp;
ULONG code;
APTR data1, data2;
BOOL boolhelp;  /* TRUE if this is help processing */
{
    struct MenuItemExt *mie;
    BOOL ret = FALSE;

    for ( ; code != MENUNULL; code = mie->mi.NextSelect)
    {
        mie = ItemAddress(&mp->menu, code);

        if (boolhelp && mp->helpfunct)
        {
            if (mie->helpfunct || (mp->helpfunct && mie->help))
            {
                char *pathname;
                short hl;

                /* create full pathname for helpfile
                */
                pathname = NULL;
                if (mie->help)
                    if (!(pathname = calloc(sizeof(char), strlen(mie->help)+ (hl = (mp->helpdir) ? strlen(mp->helpdir) : 0) + 2)))
                        return ret; /* out of memory - forget the help */

                if (pathname)
                {
                    if (mp->helpdir)
                    {
                        strcpy(pathname, mp->helpdir);
                        if (pathname[hl-1] != ':' && pathname[hl-1] != '/')
                            pathname[hl++] = '/';
                    }
                    strcat(pathname, mie->help);
                }

                /* call function & clean up
                */
                if (mie->helpfunct)
                    (*mie->helpfunct)(mp, code, pathname, data1, data2, mie->itemdata);
                else if (mp->helpfunct)
                    (*mp->helpfunct)(mp, code, pathname, data1, data2, mie->itemdata);

                iffree(pathname);
            }
        }
        else if (mie->mfunct)
            ret = ret || (*mie->mfunct)(mp, code, data1, data2, mie->itemdata);
    }
    return ret;
}
