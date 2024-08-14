/******************************************************************************
*
* $Id: layers_internal_protos.h,v 38.28 92/03/26 08:43:39 mks Exp $
*
******************************************************************************/

/* These are the prototypes for internal layers routines */
void kprintf(char *,...);

/*
 * layerface.asm
 */
void __stdargs __asm NewList(register __a1 struct List *);

/*
 * layersallocmem.asm
 */
APTR __stdargs __asm LayersAllocMem(register __d0 ULONG,register __d1 ULONG,register __d2 struct LayerInfo *);
struct ClipRect * __stdargs __asm AllocCR(register __a0 struct Layer_Info *);
struct ClipRect * __stdargs __asm newAllocCR(register __a0 struct Layer *);
void __stdargs __asm newDeleteCR(register __a0 struct Layer *,register __a1 struct ClipRect *);
void __stdargs __asm Store_CR(register __a0 struct Layer *,register __a1 struct ClipRect *);
struct ClipRect * __stdargs __asm UnStore_CR(register __a0 struct Layer *);

/*
 * r_split.asm
 */
BOOL __stdargs __asm r_split(register __a2 struct Layer *,register __a0 struct Layer *,register __a1 struct ClipRect *);

/*
 * abort.asm
 */
void __stdargs abort(struct LayerInfo *);
BOOL __stdargs aborted(struct LayerInfo *);

/*
 * cleanup.c
 */
BOOL __stdargs __asm remember_to_free(register __a0 struct Layer_Info *,register __a1 VOID *,register __d0 ULONG);
void cleanup(struct Layer_Info *);

/*
 * lock.c
 */
void __stdargs __asm locklayers(register __a0 struct Layer_Info *);
void __stdargs __asm unlocklayers(register __a0 struct Layer_Info *);

/*
 * locklayer.asm
 */
void __stdargs __asm LockLayerInfo(register __a0 struct Layer_Info *);
void __stdargs __asm UnlockLayerInfo(register __a0 struct Layer_Info *);

/*
 * layers.c
 */
void regen_display(struct Layer_Info *);
BOOL fatten_lock(struct Layer_Info *);
void unlock_thin(struct Layer_Info *);
void default_layer(struct Layer_Info *,struct Layer *,struct Rectangle *);
void __stdargs __asm gen_onscreen_cliprects(register __a0 struct Layer_Info *);
void __stdargs __asm strip_onscreen_cliprects(register __a0 struct Layer_Info *);
BOOL genrect(struct Layer *);

/*
 * addobs.c
 */
BOOL addobs(struct ClipRect *,struct Rectangle *,struct Rectangle *,struct Layer *);

/*
 * copyrect.c
 */
void __stdargs __asm copyrect(register __a0 struct ClipRect *,register __a1 struct ClipRect *);

/*
 * newlayer.c
 */
void addrect(struct Rectangle *,struct Layer *);
void gen_sbitmap_cr(struct Layer *);
struct Layer * __stdargs createbehindlayer(struct Layer_Info *,struct BitMap *,struct Rectangle,long,struct BitMap *,struct Hook *);
void __stdargs addpiece(struct Layer *,struct Rectangle *,struct Layer_Info *);
void __stdargs myaddpiece(struct Layer *,struct Rectangle *,struct Layer_Info *);

/*
 * localclipblit.c
 */
void LayerBlit(struct Layer *,SHORT,SHORT, struct Layer *,SHORT,SHORT,SHORT,SHORT,USHORT);

/*
 * layerop.asm
 */
void __stdargs __asm intersect(register __a0 struct Rectangle *,register __a1 struct Rectangle *,register __a2 struct Rectangle *);
BOOL __stdargs __asm rectXrect(register __a0 struct Rectangle *,register __a1 struct Rectangle *);
void __stdargs __asm layerop(register __a0 struct Layer *,register __a1 void (*)(),register __a2 struct Rectangle *,register __a3 void *);

/*
 * tomiddle.c
 */
BOOL movelayerinfrontof(struct Layer *,struct Layer *,long);

/*
 * deletelayer.c
 */
BOOL __stdargs __asm deletelayer(register __a1 struct Layer *);
void __stdargs __asm deletelayer2(register __a0 struct Layer *,register __d0 long);

/*
 * upfront.c
 */
LONG __stdargs __asm upfront(register __a1 struct Layer *);

/*
 * behind.c
 */
BOOL behindcode(struct Layer *,UWORD);

/*
 * screenswap.c
 */
void screentocr(struct RastPort *,struct ClipRect *);
void __stdargs __asm crtoscreen(register __a0 struct RastPort *,register __a1 struct ClipRect *);
void __stdargs __asm screenswap(register __a0 struct RastPort *,register __a1 struct ClipRect *);

/*
 * hook.c
 */

/*
 * moreregions.c
 */
struct Region *ClipRectsToRegion(struct Layer *,struct ClipRect *,long);

/*
 * usercliprects.c
 */
void pushusercliprects(struct Layer_Info *);
void popusercliprects(struct Layer_Info *);

/*
 * dedicer.asm
 */
void __stdargs __asm optimizeClipRects(register __a0 struct Layer_Info *);

/*
 * crtools.asm
 */
void __stdargs __asm free_concealed_rasters(register __a0 struct ClipRect *);
BOOL __stdargs __asm get_concealed_rasters(register __a0 struct Layer *,register __a1 struct ClipRect *);
BOOL __stdargs __asm obscured(register __a0 struct Rectangle *,register __a1 struct Rectangle *);
void __stdargs __asm unlinklayer(register __a0 struct Layer *);
void __stdargs __asm insertlayer(register __a0 struct Layer *,register __a1 struct Layer *);
void __stdargs __asm Freecr(register __a0 struct ClipRect *);
void __stdargs __asm Freecrlist(register __a0 struct ClipRect *);
void __stdargs __asm vlink_cr(register __a0 struct Layer *,register __a1 struct ClipRect *,register __a2 struct Layer *);
void __stdargs __asm linkcr(register __a0 struct Layer *,register __a1 struct ClipRect *);

/*
 * update.c
 */
long internal_beginupdate(struct Layer *);
void internal_endupdate(struct Layer *);

/*
 * copylayer.c
 */
void copylayer(struct Layer *,struct Layer *);

/*
 * backfill.asm
 */
void __stdargs __asm CallBackFillHook(register __a1 struct Layer *,register __a0 struct ClipRect *);
void __stdargs __asm FullBackFill(register __a0 struct Layer *);
