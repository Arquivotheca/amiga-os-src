#ifndef MYEXTERNS_H
#define MYEXTERNS_H

extern struct IntuitionBase *IntuitionBase;
extern struct Library       *CxBase;
extern struct GfxBase       *GfxBase;
extern struct DosLibrary    *DOSBase;
extern struct Library       *GadToolsBase;

extern struct MsgPort *cxport;      /* commodities messages here      */
extern ULONG          cxsigflag;    /* signal for above               */

extern struct MsgPort *iport;       /* Intuition IDCMP messages here   */
extern ULONG          isigflag;     /* signal for above                */

extern char   hotkeybuff[];   /* holds the string describing the popup */
                              /* hotkey. Used for the window title     */

extern struct Gadget *F_Gadget[];   /* the window F-key gadget structs */

extern CxObj *broker;
extern SHORT topborder;
extern VOID *vi;
extern struct Menu *menu;
extern struct Library *GadToolsBase;
extern struct Gadget  *glist;
extern char **ttypes;
extern struct MsgPort *cxport;

#endif MYEXTERNS_H
