#ifndef GADTOOLS_GTINTERNAL_PROTOS_H
#define GADTOOLS_GTINTERNAL_PROTOS_H

/*  gtinternal_protos.h
**
*   Internal-Only prototypes for Gadget Toolkit.
*
*   Copyright 1989, Commodore-Amiga, Inc.
*   All Rights Reserved.
*
*/

/*------------------------------------------------------------------------*/

ULONG __asm callCHook(  register __a0 struct Hook *hook,
    register __a2 Object *obj,
    register __a1 ULONG *msg );

struct Image *getSysImage( struct VisualInfo *vi, LONG width, LONG height, ULONG kind );

void SetListViewTop(struct ExtGadget *gad, struct Window *win, WORD top);

void printITextToFit(struct RastPort *rp, struct IntuiText *itext,
    WORD LeftOffset, WORD TopOffset, UWORD Width, UWORD justification,
    struct Rectangle *resultExtent);

void intuiTextSize (struct IntuiText *itext, Point *textsize);

void PlaceIntuiText(struct IntuiText *itext, struct Rectangle *extent,
    ULONG where);

void PlaceSizedIntuiText(struct IntuiText *itext, struct Rectangle *extent,
    ULONG where, Point *textsize);

void placeGadgetText( struct ExtGadget *gad, ULONG flags, ULONG default_place,
    struct Rectangle *rect );

VOID Ghost(struct RastPort *rp, UWORD pen, UWORD x0, UWORD y0, UWORD x1, UWORD y1);

void cloneRastPort(struct RastPort *clonerp, struct RastPort *rp );

VOID __asm EraseOldExtent(register __a0 struct RastPort *rp,
                          register __a1 struct Rectangle *oldExtent,
                          register __a2 struct Rectangle *newExtent);

VOID __asm FillOldExtent(register __a0 struct RastPort *rp,
                         register __a1 struct Rectangle *oldExtent,
                         register __a2 struct Rectangle *newExtent);

void __asm closeFont(register __a1 struct TextFont *font);

void __asm setAPen(register __a1 struct RastPort *rp, register __d0 ULONG pen);

VOID CombineRects(struct Rectangle *r1, struct Rectangle *r2,
                  struct Rectangle *result);

VOID PlaceHelpBox(struct ExtGadget *gad);

struct RastPort *GetRP(struct ExtGadget *gad, struct Window *win);

struct Requester *GetReq(struct ExtGadget *gad, struct Window *win);

struct Image *getBevelImage( LONG left, LONG top, LONG width, LONG height, ULONG type );

struct TagItem *TagAbleGadget(struct ExtGadget *gad, struct Window *win,
                              struct TagItem *taglist);

void SelectGadget(struct ExtGadget *gad, struct Window *win, BOOL select);

struct TagItem *findTagItem(Tag TagVal, struct TagItem *TagList);

struct TagItem *findTagItemGT(Tag TagVal, struct TagItem *TagList);

ULONG getTagData(Tag TagVal, ULONG Default, struct TagItem *TagList);

ULONG getTagDataGT(Tag TagVal, ULONG Default, struct TagItem *TagList);

ULONG getTagDataGA(Tag TagVal, ULONG Default, struct TagItem *TagList);

ULONG getTagDataSTRING(Tag TagVal, ULONG Default, struct TagItem *TagList);

UWORD __asm AddRefreshGadget(register __a1 struct ExtGadget *gadget,
                             register __a0 struct Window *window,
                             register __d0 UWORD position);

UWORD __asm removeGadget(register __a0 struct Window *window,
                         register __a1 struct ExtGadget *gadget);

Class *initGTButtonIClass(void);

ULONG __stdargs SSM( Class *cl, Object *o, Msg msg );

struct Border *MakeCycleBorder(struct CycleBorder *cycleborder,
    LONG height, struct VisualInfo *vi);

/*------------------------------------------------------------------------*/

/* This one promises to save much ROM */
struct ExtGadget *CreateGenericBase (struct ExtGadget *gad, struct NewGadget *ng,
	ULONG extrasize, struct TagItem *moretags);

/*------------------------------------------------------------------------*/

struct ExtGadget *CreateGenericA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateButtonGadgetA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateCheckBoxA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateIntegerGadgetA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateListViewA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateMXA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateNumberA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateCycleA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreatePaletteA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateScrollerA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateSliderA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateStringGadgetA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateTextA(struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);

/*------------------------------------------------------------------------*/

/*  Stubs I need, so I put them along with their taglist versions: */

void GT_SetGadgetAttrs(struct ExtGadget *gad, struct Window *win,
    struct Requester *req, ULONG firsttag, ...);

/*------------------------------------------------------------------------*/

extern struct GetTable Button_GetTable[];
extern struct GetTable Checkbox_GetTable[];
extern struct GetTable Integer_GetTable[];
extern struct GetTable Listview_GetTable[];
extern struct GetTable MX_GetTable[];
extern struct GetTable Number_GetTable[];
extern struct GetTable Cycle_GetTable[];
extern struct GetTable Palette_GetTable[];
extern struct GetTable Scroller_GetTable[];
extern struct GetTable Slider_GetTable[];
extern struct GetTable String_GetTable[];
extern struct GetTable Text_GetTable[];

/*------------------------------------------------------------------------*/

#endif /* !GADTOOLS_GTINTERNAL_PROTOS_H */
