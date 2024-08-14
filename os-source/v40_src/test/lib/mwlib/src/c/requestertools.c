/****** mwlib.library/RequesterTools ******************************************
    RequesterTools.c

    Mitchell/Ware Systems           Version 4.03            06-Dec-88

    Tools used for Custom Requesters
  *****************************************************************************
    NOTE1: Windows must have the IDCMP REQCLEAR flag set.

    NOTE2: If rtg[].rfunct is non-null, then it is called:
                BOOL (*rfunct)(req, window, class, r, gadget, x, y, code, udata)
                    strucr Requester *req
                    struct Window *window;  Window requester is in
                    ULONG class, code;      IDCMP class & code of message
                    struct RTGList *r;             RTG structure for this gadget
                    struct Gadget *gadget;  Gadget requesting service
                    int x, y;               Mouse x&y pos. in message
                    void *udata;             user data

                If (*rfunct)() returns TRUE, then this is taken
                as a notice to immediatly EndRequest() and clean up.
                AT THE MOMENT, THIS IS DANGEROUS- if user-gadget
                activity is taking place when this occurs, you'll
                lock-up the system!!!

    NOTE3: MutualExclude will not work for gadgets until 1.4, so....
            They are implemented here. GadgetTools DOES NOT use them
            anymore (version 3.05), and will be dealt with later.
            GADGHCOMP must be used with MutualExclude for now, because
            RequesterTools uses RectFill to uncomplement the area.

    NOTE3a: The FIX field is now used by BOOLEAN TOGGLE Gadgets for 16
            more mutually exclusive gadgets Total number is now 48.

    DoRequest(pkey, bitmap, list, text, window, left, top, width, height, udata)
            list - the special RTGLIST in tools.h
            text - an IntuiText structure describing text
            pkey - the Remember key, or NULL. If null, then all the
                    gadgets allocated during the creation of the
                    requestor will be deleted. Otherwise, they will be
                    retained.
            window - The window the requestor is to appear in. Later,
                    a NULL will prompt a window to be created for this.
                    For now, a window MUST be given.
            udata  - pointer to user data area. Not needed. Will be passed to
                    callbacks.

    AllocateReqStrBuffs(key, rtg_list)
        struct Remember **key;
        struct RTGList rtg_list[]

        Allocates all STRGADGET r->string areas to Buffer Length + 1.
        All allocation is performed using AllocRemember.

    ClearSelect(rtg_list)
        struct RTGList rtg_list[]

        Clears the Selected flags

    ClearGadgets(rtg_list)

        Clears the gadget pointers in list.

    UpdateStringEntry(r)
        struct RTGLIST *r; - entry to be updated
        Updates the .value or .string entry of given RTGLIST entry.
        WARNING: Use on strings ONLY!

    SetReqBackFill(n)
        Does as it says!

    SetIdleRoutine(rou)
        Routine to execute when no events are pending
        Routine is passed:
            (*rou)(key, rtglist, window, request);
        Routine must either return periodically or check the window UserPort
        periodically for any impending messages, at which point it
        must return immediately.
        NOTE: idle routine is cleared on exit from DoRequest.

    UpdateRTGList(rlist)
        Updates the RTGList from the gadgets

    void DoRugTopRequest (pkey, rlist, ilist, win, req)
        add this sub-requester

    void RugPullRequest (rlist, win, req)
        Snatch this sub-requester out, and all those on top
        If rlist is null, then snatch out ALL of them!


    short RegisterCGC(class, create, handler, remove, refresh, update, idle)
        short class;
        BOOL (*create)();
        BOOL (*handler)();
        BOOL (*remove)();
        BOOL (*refresh)();
        BOOL (*update)();
        BOOL (*idle)();

        Registers a new class of gadget with the RequesterTools. The class
        can either be specified, or NULL. If NULL, then a class value will
        be generated. In either case, the class will be returned, and will
        always be negative. (If the return is 0 then either a duplicate
        class was specified, or you're out of memory.) The three methods,
        (*create)(), (*handler)(), and (*remove)() perform the expected
        functions. (*create)() actually will create this gadget by allocating
        a Gadget structure, setting it up, and allocating any other resources
        that it needs.  (*handler)() will handle (and perhaps modify)
        incomming input events.  (*remove)() will be responsible for removing
        the gadget. Not all of these need be present with the exception of
        (*create)(); as long as the memory allocated was done so with the
        provided key (and no other types of resources were used) (*remove)()
        need not be done, and can just be NULL. It is also not mandatory to
        have a (*handler)(), unless special rendering will be done for this
        class.

        The methods are as follows:

            (*create)(key, rtg, cgc)    - Create this gadget

                struct Remember **key;      -All memory needed by this gadget
                                             is to be allocated here.

                RTGList *rtg;               -The single RTG entry that is an
                                             instance of this class.

                CustomGadgetClass *cgc;     -The class structure.

                Return TRUE if successful, or FALSE if not.

            (*handle)(key, rp, rtg, pclass, pcode, px, py)

                struct Remember **key;      -You shouldn't have to allocate
                                             more memory, but just in case...
                                             Be warned- this key may belong to
                                             the Requester and will be freed
                                             upon exit from DoRequest()!

                struct RastPort *rp;        -A special RastPort that represents
                                             your imagery area. Origin is set
                                             to the upper-left corner of this
                                             instance of the gadget, and is
                                             clipped to the width and height.

                RTGList *rtg;               -The instance of this class

                ULONG *pclass;              -The Intuition Class responsible
                                             for this invocation. You are free
                                             to modify this entry, as this will
                                             also be passed on to the gadget's
                                             method. If you make this NULL, then
                                             the gadget's method WILL NOT be
                                             called.

                USHORT *pcode;              -The Intuition Code. You are free
                                             to modify this entry. This entry
                                             will also be passed to the gadget's
                                             method.

                short *px, *py              -These are equivalent to the
                                             MouseX and MouseY of IntuiMessage,
                                             adjusted for the new origin. You
                                             may modify these, as well, and
                                             the degree of modification will
                                             be reflected in the original
                                             IntuiMessage that will eventually
                                             go out to the Gadget's method.

                Return TRUE if successful, or FALSE if not.

            (*remove)(key, rtg, cgc)    - Remove this gadget

                struct Remember **key;      -You shouldn't need to use this.

                RTGList *rtg;               -The single RTG entry that is an
                                             instance of this class.

                CustomGadgetClass *cgc;     -The class structure.

                Return TRUE if successful, or FALSE if not.

            (*refresh)(key, rp, rtg)    - Refresh Gadgets with new information

                struct Remember **key;      -A key to be used for memory allocation.
                                             Normally, you should avoid allocating
                                             more memory at this point, but it is
                                             diffucult to forsee all possible
                                             new uses. Your application
                                             will supply this, or make it NULL.
                                             NOTE: You should be able to handle
                                             the NULL case here.

                struct RastPort *rp;        -A special RastPort that represents
                                             your imagery area. Origin is set
                                             to the upper-left corner of this
                                             instance of the gadget, and is
                                             clipped to the width and height.
                                             Since you will not be calling
                                             this directly, this will automagically
                                             be set up for you.

                RTGEntry *rtg;              -The instance of this class.

            (*update)(key, rtg)         - Update RTGEntry from Gadget (note that
                                            this would normally be done from
                                            within your handler).

                struct Remember **key;      -A key to be used for memory allocation.
                                             Your application will supply this,
                                             or make it NULL.

                                             NOTE: You should be able to handle
                                             the NULL case here.

                RTGEntry *rtg;              -The instance of this class.

    NOTE 4:
        for now, the Rug Requesters' gadgets are totally removed when pulled!

******************************************************************************

    EDIT HISTORY:

        23-Feb-89   -   ilist was being processed at the end of DoRequest
                        when NULL! Fixed.
        24-Feb-89   -   Added mousex & mousey to gadget function call.
        02-Mar-89   -   Process fix point string events to r->value
        03-Mar-89   -   MutualExclude now resets the SELECTED flag.
                    -   Move the IF statement for MutualExclude so that
                        it is called first before the (*rfunct)() on
                        GADGETUP only. So, if (*rfunct)() uses the SELECTED
                        flag, it must do it only on a GADGETUP class.

        5/10/89     -   Added DoRugTopRequest() and RugPullRequest()
        5/29/89     -   Enhanced RugPullRequest() to pull all if rlist NULL
        2/21/90     -   Installation of Custom Gadget Class Handling
        6/3/90  &&& -   We MUST clear PushButton Gadgets upon startup!!!! (NIY)
        6/12/90     -   Added udata user data field

*****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#include "Tools.h"
#include "protos.h"

#define key     ((pkey) ? pkey : &tkey)

#define RMAX    10

static int BackFill = 0;
static void (*idle)() = NULL;

struct RTGList *rrug[RMAX];    /* sub-requester stack */
struct ITList  *irug[RMAX];
USHORT  crug[RMAX];
struct Remember *ckey[RMAX];
USHORT  rugcnt = 0;     /* sub-requester count */

static struct CustomGadgetClass cgc_ar[] = {   /* permanent classes */
    {   &cgc_ar[1], PROP_HORIZ, NULL, NULL, NULL },
    {   NULL /* &cgc_ar[2] */, PROP_VERT,  NULL, NULL, NULL },
/*  {   &cgc_ar[3], N_STATE,    },
    {   &cgc_ar[4], LIST_BOX,   },
    {   NULL,       DISPLAY_BOX },  */
    };

static struct CustomGadgetClass *cgc = &cgc_ar;

void SetReqBackFill(int n)
{
    BackFill = n;
}

void SetIdleRoutine(id) /* reset to NULL on exit from DoRequest */
void (*id)();
{
    idle = id;
}
#
DoRequest (pkey, bmap, rlist, ilist, window, left, top, width, height, udata)

struct Remember **pkey; /* NULL for temp. - delete gadgets when finished. */
struct BitMap *bmap;    /* the BitMap structure to use - or NULL for Intuition rendered Requesters */
struct RTGList rlist[];        /* null-entry-terminated list for gadgets */
struct ITList *ilist;          /* Text to be rendered in requester - or NULL */
struct Window *window;  /* window to place requester in */
int left, top, width, height; /* where to place it, how big */
void *udata;            /* pointer to user data. Gets passed to all callbacks. */
{
    struct Requester req;
    struct Remember *tkey = NULL;
    struct IntuiMessage *m;
    struct Gadget *gadget;
    struct PropInfo *pinfo;
    struct StringInfo *sinfo;
    ULONG class;
    USHORT code;
    USHORT type; /* gadget type */
    USHORT indx;    /* index for rrug */
    short mousex, mousey;
    struct RTGList *r;
    struct ITList *i;
    int ret = 0;      /* return success */
    BOOL (*rfunct)() = NULL;

    setmem(&req, sizeof(req), NULL);
    ActivateWindow(window);

    req.OlderRequest = NULL;
    req.LeftEdge = left;
    req.TopEdge = top;
    req.Width = width;
    req.Height = height;
    req.RelLeft = req.RelTop = 0;
    req.ReqGadget = MakeRTGList(key, rlist, cgc);
    req.ReqBorder = MakeRTGBorders(key, rlist);
    req.ReqText = (ilist) ? MakeITList(key, ilist) : NULL;
    req.Flags = (bmap) ? PREDRAWN | NOISYREQ : NOISYREQ;
    req.BackFill = BackFill;
    req.ReqLayer = NULL;
    req.ImageBMap = bmap;
    if (req.ReqGadget && req.ReqBorder && (!ilist || req.ReqText) && (ret = Request(&req, window)))
    {
        ActivateWindow(window);
        for (r = rlist; r->width; ++r)      /* activate first string gadget */
            if (r->type == STRGADGET)
            {
                ActivateWindow(window);
                ActivateGadget(r->g, window, &req);
                break;
            }

        for (;;)
        {
            while (!(m = GetMsg(window->UserPort)))
                if (idle)
                    (*idle)(pkey, rlist, window, &req);
                else
                    WaitPort(window->UserPort);

            class = m->Class;
            code = m->Code;
            gadget = m->IAddress;
            mousex = m->MouseX;
            mousey = m->MouseY;
            ReplyMsg(m);

            if (class == REQCLEAR)
            {
                break;
            }
            else if (class == GADGETDOWN || class == GADGETUP)
            {
                if (rlist[gadget->GadgetID].g == gadget)
                    r = rlist + gadget->GadgetID;
                else
                    for (indx = 0, r = NULL; indx < rugcnt; ++indx)
                        if (rrug[indx][gadget->GadgetID].g == gadget)
                        {
                            r = rrug[indx] + gadget->GadgetID;
                        }   /* problem area... no bounds checking!!! */

                if (!r) /* if gadget's not found */
                    fuckup("RequesterTools: Signal from Phantom Gadget!");

                if (gadget->Activation & TOGGLESELECT)
                    r->selected = !!(gadget->Flags & SELECTED);
                else
                    r->selected = TRUE;

                if (r->fix && (r->g->GadgetType & ~GADGETTYPE) == STRGADGET)
                {
                    sinfo = r->g->SpecialInfo;
                    r->value = sdec_fix(sinfo->Buffer, r->fix);
                }

                if ((r->type == BOOLGADGET) && r->general && (class == GADGETUP))
                    _MutualExclude(rlist, r, window, &req);

                /* Class Handler
                */
                if (r->class && r->class->handler)
                {
                    short px = mousex - r->left - left, py = mousey - r->top - top;

                    ClipRastPort(req.ReqLayer->rp, r->left, r->top, r->width, r->height);
                    ret = (*r->class->handler)(key, req.ReqLayer->rp, r, &class, &code, &px, &py);
                    mousex = px + r->left + left, mousey = py + r->top + top;
                    UnclipRastPort(req.ReqLayer->rp, r->left, r->top);
                    if (!ret)
                    {
                        EndRequest(&req, window);
                        break;
                    }
                }

                /* Gadget's Method
                */
                if ((rfunct = r->rfunct) && class)
                {
                    if ((*rfunct)(&req, window, class, r, gadget, mousex, mousey, code, udata)) /* if returns true, shutdown */
                    {
                        EndRequest(&req, window);
                        break;
                    }
                }
                /* NOTE: There used to be an ELSE here! */
                if (r->type == STRGADGET && (r->activation & RELVERIFY)
                    && (++r)->g && r->type == STRGADGET)
                {
                    ActivateGadget(r->g, window, &req);
                }
            }
            else if ((class == MOUSEMOVE || class == MOUSEBUTTONS || class == INTUITICKS) && rfunct)
            {
                /* assume that r has not been modified */
                /* Class Handler
                */
                if (r->class && r->class->handler)
                {
                    short px = mousex - r->left - left, py = mousey - r->top - top;

                    ClipRastPort(req.ReqLayer->rp, r->left, r->top, r->width, r->height);
                    ret = (*r->class->handler)(key, req.ReqLayer->rp, r, &class, &code, &px, &py);
                    mousex = px + r->left + left, mousey = py + r->top + top;
                    UnclipRastPort(req.ReqLayer->rp, r->left, r->top);
                }

                if (!ret || (class && (*rfunct)(&req, window, class, r, NULL, mousex, mousey, code, udata)))
                {
                    EndRequest(&req, window);
                    break;
                }
            }
        }

        /* finish off the lot of them... */
        UpdateRTGList(rlist);
    }

    /* Remove special classes
    */
    for (r = rlist; r->width; ++r)
        if (r->class && r->class->remove)
            (*r->class->remove)(key, r, r->class);

    if (!pkey)
    {
        FreeRemember(&tkey, TRUE);
        ClearGadgets(rlist);

        if (ilist)
            for (i = ilist; i->string; ++i)
                i->itext = NULL;
    }

    idle = NULL;
    return ret;
}

UpdateStringEntry(r) /* send changes in text to entry. WARNING: String Gadgets ONLY! */
struct RTGList *r; /* entry to be updated */
{
    struct StringInfo *sinfo;

    sinfo = r->g->SpecialInfo;

    if (r->g->Activation & LONGINT)
        r->value = sinfo->LongInt;

    else if (r->fix)
        r->value = sdec_fix(sinfo->Buffer, r->fix);

    else if (r->string)
        memcpy(r->string, sinfo->Buffer, sinfo->MaxChars);
}

void AllocateReqStrBuffs(pkey, r)
struct Remember **pkey;
struct RTGList *r;
{
    for (; r->width; ++r)
        if (r->type == STRGADGET)
            if (!(r->string = AllocRemember(pkey, r->general+1, MEMF_CLEAR|MEMF_PUBLIC)))
                screwup("ARSB:Warning- Out Of Memory!!!");
}

void ClearSelect(rtg)
struct RTGList rtg[];
{
    struct RTGList *r;

    for (r = rtg; r->width; ++r)
        r->selected = NULL;
}

void ClearGadgets(rtg)
struct RTGList rtg[];
{
    struct RTGList *r;

    for (r = rtg; r->width; ++r)
        r->g = NULL;
}

_MutualExclude(rlist, r, win, req)  /* Internal routine */
struct RTGList rlist[], *r;
struct Window *win;
struct Requester *req;  /* could be null. If it is, then only update rlist */
{
    USHORT i;
    struct Gadget *g;
    struct RastPort *rp;
    ULONG exclude; /* flag */

    rp = req->ReqLayer->rp;

    /* Remove those darn gadgets */
    RemoveGList(win, rlist[0].g, -1);
    SetDrMd(rp, COMPLEMENT);
    SetAPen(rp, 63);
    SetBPen(rp, 63);

    /* Scan bits for gadgets to deselect */
    for (i = 0; rlist[i].width && i < 64; ++i)
    {
        exclude = (i < 32) ? (r->general & (1 << i))
                           : (r->fix & (1 << (i-32)));
        if (exclude && (rlist[i].g->Flags & SELECTED))
        {
            g = rlist[i].g;
            rlist[i].selected = FALSE;
            g->Flags &= ~SELECTED;
            RectFill(rp, g->LeftEdge, g->TopEdge,
                        g->LeftEdge + g->Width-1, g->TopEdge + g->Height-1);
        }
    }

    AddGList(win, rlist[0].g, 0, -1, req);
}
#
/* note: This has been initially set up for Custom Requesters
                                        (or pseudo-Custom Requesters)... */

void DoRugTopRequest (pkey, rlist, ilist, win, req) /* add this sub-requester */
struct Remember **pkey; /* NOT USED JUST YET- make this NULL. */
struct RTGList rlist[];        /* null-entry-terminated list for gadgets */
struct ITList *ilist;          /* NOT USED JUST YET- Text to be rendered in requester - or NULL */
struct Window *win;     /* window to place requester in */
struct Requester *req;  /* Requester to add this to */
{
    register struct Gadget *g;

    if (rugcnt < RMAX)
    {
        rrug[rugcnt] = rlist;
        irug[rugcnt] = ilist;

        g = MakeRTGList(&ckey[rugcnt], rlist);
        crug[rugcnt] = GadgetCount(g);

        /* Now add this list to the current list of gadgets
        */
        AddGList(win, g, -1, -1, req);
        RefreshGList(g, win, req, crug[rugcnt]);

        /* MakeRTGBorders(&ckey[rugcnt], rlist); /* somehow, should draw this!... */
        /* MakeITList(&ckey[rugcnt], ilist); /* somehow, should use this ... */

        ++rugcnt;
    }
    else
        fuckup("DRTReq: ??? Too many sub requesters!");
}

void RugPullRequest (rlist, win, req) /* snatch this sub-requester out, and all those on top */
struct RTGList rlist[];        /* sub-requester list to pull out */
struct Window *win;
struct Requester *req;
{
    register i, j;
    register struct RTGList *r;

    for (i = 0; i < rugcnt; ++i)
        if (!rlist || rrug[i] == rlist)
        {
            for (j = i; j < rugcnt; ++j)
            {
                RemoveGList(win, rrug[j][0].g, crug[j]);
                UpdateRTGList(rrug[j]);
                ClearGadgets(rrug[j]);
                rrug[j] = NULL;
                FreeRemember(&ckey[j], TRUE);
            }
            rugcnt = i;
            return;
        }

    if (rlist)
        fuckup("RPR: Can't find rlist");
}

void UpdateRTGList(r)
struct RTGList *r;
{
    register struct PropInfo *pinfo;

    for (; r->width; ++r)
    {
        switch (r->g->GadgetType & ~GADGETTYPE)
        {
        case BOOLGADGET:
            if (r->g->Activation & TOGGLESELECT)
                r->selected = !! (r->g->Flags & SELECTED);
            break;

        case STRGADGET:
            UpdateStringEntry(r);
            break;

        case PROPGADGET: /* could either be a normal gadget or PropArrows */
            pinfo = r->g->SpecialInfo;
            r->vertpot = pinfo->VertPot;
            r->horizpot = pinfo->HorizPot;
            r->vertbody = pinfo->VertBody;
            r->horizbody = pinfo->HorizBody;
            break;

        default:
            break;
        }
    }
}
#
/******************************************************************************

    Classic Functions - uses cgc as defined way above

******************************************************************************/

short RegisterCGC(class, create, handler, remove, refresh, update, idle)
short class;
BOOL (*create)(), (*handler)(), (*remove)();
BOOL (*refresh)(), (*update)(), (*idle)();
{
    short ncl;
    struct CustomGadgetClass *c;

    /* let's deal with the class value
    */
    if (class)
    {
        for (c = cgc; c; c = c->next)
            if (class == c->class)
                return NULL;    /* duplicate class error */
    }
    else /* find a good class value */
    {
        for (ncl = 0, c = cgc; c; c = c->next)
            if (c->class < ncl)
                ncl = c->class;
        class = --ncl;
    }

    /*----------------------------------------------
        Now, 'class' contains a new class value.
    -----------------------------------------------*/
    if (c = malloc(sizeof(*c)))
    {
        c->next = cgc; cgc = c;
        c->class = class;
        c->create = create;
        c->handler = handler;
        c->remove = remove;
        c->refresh = refresh;
        c->update = update;
        c->idle = idle;
    }
    else
        class = NULL;   /* out of memory- class isin't registered */

    return class;
}
