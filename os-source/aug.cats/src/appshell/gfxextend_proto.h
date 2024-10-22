/*
***********************************************************************
*
* Copyright (c) 1989 Commodore-Amiga, Inc.
*
* Executables based on this information may be used in software
* for Commodore Amiga computers.  All other rights reserved.
* This information is provided "as is"; no warranties are made.  All
* use is at your own risk. No liability or responsibility is assumed.
*
***********************************************************************
*/

/* DynamicImages */
VOID InitDynamicImage (struct DynamicImage *, USHORT, USHORT, USHORT);
BOOL AllocDynamicImage (struct DynamicImage *);
VOID FreeDynamicImage (struct DynamicImage *);

/* AnimTools */
struct GelsInfo *setupGelSys(struct RastPort *, BYTE reserved);
VOID cleanupGelSys(struct GelsInfo *, struct RastPort *);
struct VSprite *makeVSprite(NEWVSPRITE *);
struct Bob *makeBob(NEWBOB *);
struct Bob *makeImageBob(NEWIMAGEBOB *);
struct AnimComp *makeComp(NEWBOB *, NEWANIMCOMP *);
struct AnimComp *makeSeq(NEWBOB *, NEWANIMSEQ *);
VOID freeVSprite(struct VSprite *);
VOID freeBob(struct Bob *, LONG rasdepth);
VOID freeComp(struct AnimComp *, LONG rasdepth);
VOID freeSeq(struct AnimComp *, LONG rasdepth);
VOID freeOb(struct AnimOb *, LONG rasdepth);

/* RenderInfo */
VOID GetRenderData (struct Screen *, struct RenderInfo *);
VOID FreeRenderData (struct RenderInfo *);

/* Style */
BOOL InitSlider (struct RenderInfo *, struct Window *, struct Gadget *);
VOID FreeSlider (struct Gadget *);
BOOL InitButton (struct RenderInfo *, struct Gadget *);
VOID FreeButton (struct Gadget *);
SHORT GetSliderVal (struct Gadget *, SHORT);
SHORT UpdateNum (struct Window * win, struct Gadget * gad, SHORT dir);
BOOL UpdateSlider (struct Window * win, struct Gadget * gad, SHORT dir);
VOID SetSlider (struct Window * win, struct Gadget * gad, SHORT val);
VOID Draw3DBorder (struct RenderInfo *, struct RastPort *,SHORT,SHORT,SHORT,SHORT,SHORT);
VOID MakeButtonBorder (struct Border * bd, SHORT w, SHORT h);
VOID Make3DBorder (struct RenderInfo *, struct Border *, SHORT, SHORT, SHORT);

/* SketchPad.c: protos */
struct SketchPad *OpenSketchPad (struct NewSketchPad *);
VOID CloseSketchPad (struct SketchPad *);
VOID SPSetRepeat (struct SketchPad *, struct DynamicImage *, SHORT, SHORT);
VOID SPDraw (struct SketchPad * sp, struct IntuiMessage * rmsg);
VOID SPRefresh (struct SketchPad * sp);
VOID SPClear (struct SketchPad * sp);
VOID SPUndo (struct SketchPad * sp);
VOID SPSaveToUndo (struct SketchPad * sp);

/* Arrows.c: protos */
VOID AddMoveBox (struct SketchPad *, SHORT, SHORT);
VOID SPScroll (struct SketchPad *, struct IntuiMessage *);

/* Tools.c: protos */
VOID AddToolBox (struct SketchPad *, WORD, WORD);
SHORT SPSelectTool (struct SketchPad * sp, struct IntuiMessage * msg);

/* Palette.c: function prototypes */
struct Gadget *
 AllocPaletteGadget (struct Window *, WORD, WORD, WORD, WORD);
struct Gadget *
 FreePaletteGadget (struct Gadget *);

/* eborder.c */
struct EBorder *CreateBorder (struct RenderInfo *,WORD,WORD,WORD,struct TagItem *tags );
VOID FreshenBorder(struct RenderInfo *,struct EBorder *,WORD,WORD);
struct EBorder *DeleteBorder (struct EBorder *eb);
