extern struct GfxBase *GfxBase;
typedef void (*__fgptr)();
typedef UBYTE *PLANEPTR;
/*------ Text routines*/
long BltBitMap(struct BitMap *, long, long, struct BitMap*,long, long, long, long, long, long, char *);
void BltTemplate(char *, long, long, struct RastPort *, long, long, long, long);
void ClearEOL(struct RastPort *);
void ClearScreen(struct RastPort *);
long TextLength(struct RastPort *, char *, long);
long Text(struct RastPort *, char *, long);
long SetFont(struct RastPort *, struct TextFont*);
struct TextFont *OpenFont(struct TextAttr *);
void CloseFont(struct TextFont *);
long AskSoftStyle(struct RastPort *);
long SetSoftStyle(struct RastPort *, long, long);
/*------   Gels routines ------*/
void AddBob(struct Bob *, struct RastPort *);
void AddVSprite(struct VSprint *, struct RastPort *);
void DoCollision(struct RastPort *);
void DrawGList(struct RastPort *, struct ViewPort *);
void InitGels(struct VSprint *, struct VSprite *, struct GelsInfo *);
void InitMasks(struct VSprite *);
void RemIBob(struct Bob *, struct RastPort *, struct ViewPort *);
void RemVSprite(struct VSprite *);
void SetCollision(long, __fgptr, struct GelsInfo*);
void SortGList(struct RastPort *);
void AddAnimOb(struct AnimOb *, long, struct RastPort *);
void Animate(long, struct RastPort *);
void GetGBuffers(struct AnimOb *, struct RastPort *, long);
void InitGMasks(struct AnimOb *);
void DrawEllipse(struct RastPort *, long, long, long, long);
long AreaEllipse(struct RastPort *, long, long, long, long);
/*------   Remaining graphics routines ------*/
void LoadRGB4(struct ViewPort *, short *, long);
void InitRastPort(struct RastPort *);
void InitVPort(struct ViewPort *);
void MrgCop(struct View *);
void MakeVPort(struct View *, struct ViewPort *);
void LoadView(struct View *);
void WaitBlit(void);
void SetRast(struct RastPort *, long);
void Move(struct RastPort *, long, long);
void Draw(struct RastPort *, long, long);
long AreaMove(struct RastPort *, long, long);
long AreaDraw(struct RastPort *, long, long);
void AreaEnd(struct RastPort *);
void WaitTOF(void);
void QBlit(struct BlitNode *);
void InitArea(struct AreaInfo *, short *, long);
void SetRGB4(struct ViewPort *, long, long, long, long);
void QBSBlit(struct BlitNode *);
void BltClear(char *, long, long);
void RectFill(struct RastPort *, long, long, long, long);
void BltPattern(struct RastPort *, struct RastPort *, long, long, long, long, long);
long ReadPixel(struct RastPort *, long, long);
void WritePixel(struct RastPort *, long, long);
void Flood(struct RastPort *, long, long, long);
void PolyDraw(struct RastPort *, long, short *);
void SetAPen(struct RastPort *, long);
void SetBPen(struct RastPort *, long);
void SetDrMd(struct RastPort *, long);
void InitView(struct View *);
void CBump(struct UCopList *);
void CMove(struct UCopList *, long, long);
void CWait(struct UCopList *, long, long);
long VBeamPos(void);
void InitBitMap(struct BitMap *, long, long, long);
void ScrollRaster(struct RastPort *, long, long, long, long, long, long);
void WaitBOVP(struct ViewPort *);
long GetSprite(struct SimpleSprite *, long);
void FreeSprite(long);
void ChangeSprite(struct ViewPort *, struct SimpleSprite *, short *);
void MoveSprite(struct ViewPort *, struct SimpleSprite *, long, long);
void LockLayerRom(struct Layer *);
void UnlockLayerRom(struct Layer *);
void SyncSBitMap(struct Layer *);
void CopySBitMap(struct Layer *);
void OwnBlitter(void);
void DisownBlitter(void);
struct TmpRas *InitTmpRas(struct TmpRas *, char *, long);
void AskFont(struct RastPort *, struct TextAttr *);
void AddFont(struct TextFont *);
long RemFont(struct TextFont*);
PLANEPTR AllocRaster(long, long);
void FreeRaster(PLANEPTR, long, long);
void AndRectRegion(struct Region *, struct Rectangle *);
void OrRectRegion(struct Region *, struct Rectangle *);
struct Region *NewRegion(void);
long ClearRectRegion(struct Region *, struct Rectangle *);
void ClearRegion(struct Region *);
void DisposeRegion(struct Region *);
void FreeVPortCopLists(struct ViewPort *);
void FreeCopList(struct CopList *);
void ClipBlit(struct RastPort *, long, long, struct RastPort *, long, long, long, long, long);
void XorRectRegion(struct Region *, struct Rectangle *);
void FreeCprList(struct cprlist *);
struct ColorMap *GetColorMap(long);
void FreeColorMap(struct ColorMap *);
long GetRGB4(struct ColorMap *, long);
void ScrollVPort(struct ViewPort *);
void UCopperListInit(struct UCopList *, long);
void FreeGBuffers(struct AminOb *, struct RastPort *, long);
/*void BltBitMapRastPort(struct BitMap *, long, long, struct RastPort *, long, long, long, long, long);*/
long OrRegionRegion(struct Region *, struct Region *);
long XorRegionRegion(struct Region *, struct Region *);
long AndRegionRegion(struct Region *, struct Region *);
void SetRGB4CM(struct ColorMap *, long, long, long, long);
void BltMaskBitMapRastPort(struct BitMap *,long, long, struct RastPort *,long,long,long,long,long,APTR);
long AttemptLockLayerRom(struct Layer *);

#ifndef  NO_PRAGMAS
/*------ Text routines*/
/*pragma libcall GfxBase BltBitMap 1e 3291080b*/
/*pragma libcall GfxBase BltTemplate 24 32910808*/
#pragma libcall GfxBase ClearEOL 2a 901
#pragma libcall GfxBase ClearScreen 30 901
#pragma libcall GfxBase TextLength 36 8903
#pragma libcall GfxBase Text 3c 8903
#pragma libcall GfxBase SetFont 42 8902
#pragma libcall GfxBase OpenFont 48 801
#pragma libcall GfxBase CloseFont 4e 901
#pragma libcall GfxBase AskSoftStyle 54 901
#pragma libcall GfxBase SetSoftStyle 5a 10903
/*------   Gels routines ------*/
#pragma libcall GfxBase AddBob 60 9802
#pragma libcall GfxBase AddVSprite 66 9802
#pragma libcall GfxBase DoCollision 6c 901
#pragma libcall GfxBase DrawGList 72 8902
#pragma libcall GfxBase InitGels 78 a9803
#pragma libcall GfxBase InitMasks 7e 801
#pragma libcall GfxBase RemIBob 84 a9803
#pragma libcall GfxBase RemVSprite 8a 801
#pragma libcall GfxBase SetCollision 90 98003
#pragma libcall GfxBase SortGList 96 901
#pragma libcall GfxBase AddAnimOb 9c a9803
#pragma libcall GfxBase Animate a2 9802
#pragma libcall GfxBase GetGBuffers a8 9803
#pragma libcall GfxBase InitGMasks ae 801
#pragma libcall GfxBase DrawEllipse b4 3210905
#pragma libcall GfxBase AreaEllipse ba 3210905
/*------   Remaining graphics routines ------*/
#pragma libcall GfxBase LoadRGB4 c0 9803
#pragma libcall GfxBase InitRastPort c6 901
#pragma libcall GfxBase InitVPort cc 801
#pragma libcall GfxBase MrgCop d2 901
#pragma libcall GfxBase MakeVPort d8 9802
#pragma libcall GfxBase LoadView de 901
#pragma libcall GfxBase WaitBlit e4 0
#pragma libcall GfxBase SetRast ea 902
#pragma libcall GfxBase Move f0 10903
#pragma libcall GfxBase Draw f6 10903
#pragma libcall GfxBase AreaMove fc 10903
#pragma libcall GfxBase AreaDraw 102 10903
#pragma libcall GfxBase AreaEnd 108 901
#pragma libcall GfxBase WaitTOF 10e 0
#pragma libcall GfxBase QBlit 114 901
#pragma libcall GfxBase InitArea 11a 9803
#pragma libcall GfxBase SetRGB4 120 3210805
#pragma libcall GfxBase QBSBlit 126 901
#pragma libcall GfxBase BltClear 12c 10903
#pragma libcall GfxBase RectFill 132 3210905
/*pragma libcall GfxBase BltPattern 138 32108907*/
#pragma libcall GfxBase ReadPixel 13e 10903
#pragma libcall GfxBase WritePixel 144 10903
#pragma libcall GfxBase Flood 14a 102904
#pragma libcall GfxBase PolyDraw 150 80903
#pragma libcall GfxBase SetAPen 156 902
#pragma libcall GfxBase SetBPen 15c 902
#pragma libcall GfxBase SetDrMd 162 902
#pragma libcall GfxBase InitView 168 901
#pragma libcall GfxBase CBump 16e 901
#pragma libcall GfxBase CMove 174 10903
#pragma libcall GfxBase CWait 17a 10903
#pragma libcall GfxBase VBeamPos 180 0
#pragma libcall GfxBase InitBitMap 186 210804
/*pragma libcall GfxBase ScrollRaster 18c 43210907*/
#pragma libcall GfxBase WaitBOVP 192 801
#pragma libcall GfxBase GetSprite 198 802
#pragma libcall GfxBase FreeSprite 19e 1
#pragma libcall GfxBase ChangeSprite 1a4 a9803
#pragma libcall GfxBase MoveSprite 1aa 109804
#pragma libcall GfxBase LockLayerRom 1b0 d01
#pragma libcall GfxBase UnlockLayerRom 1b6 d01
#pragma libcall GfxBase SyncSBitMap 1bc 801
#pragma libcall GfxBase CopySBitMap 1c2 801
#pragma libcall GfxBase OwnBlitter 1c8 0
#pragma libcall GfxBase DisownBlitter 1ce 0
#pragma libcall GfxBase InitTmpRas 1d4 9803
#pragma libcall GfxBase AskFont 1da 8902
#pragma libcall GfxBase AddFont 1e0 901
#pragma libcall GfxBase RemFont 1e6 901
#pragma libcall GfxBase AllocRaster 1ec 1002
#pragma libcall GfxBase FreeRaster 1f2 10803
#pragma libcall GfxBase AndRectRegion 1f8 9802
#pragma libcall GfxBase OrRectRegion 1fe 9802
#pragma libcall GfxBase NewRegion 204 0
#pragma libcall GfxBase ClearRectRegion 20a 9802
#pragma libcall GfxBase ClearRegion 210 801
#pragma libcall GfxBase DisposeRegion 216 801
#pragma libcall GfxBase FreeVPortCopLists 21c 801
#pragma libcall GfxBase FreeCopList 222 801
/*pragma libcall GfxBase ClipBlit 228 32910809*/
#pragma libcall GfxBase XorRectRegion 22e 9802
#pragma libcall GfxBase FreeCprList 234 801
#pragma libcall GfxBase GetColorMap 23a 1
#pragma libcall GfxBase FreeColorMap 240 801
#pragma libcall GfxBase GetRGB4 246 802
#pragma libcall GfxBase ScrollVPort 24c 801
#pragma libcall GfxBase UCopperListInit 252 802
#pragma libcall GfxBase FreeGBuffers 258 9803
/*pragma libcall GfxBase BltBitMapRastPort 25e 32910809*/
#pragma libcall GfxBase OrRegionRegion 264 9802
#pragma libcall GfxBase XorRegionRegion 26a 9802
#pragma libcall GfxBase AndRegionRegion 270 9802
/*pragma libcall GfxBase SetRGB4CM 276 3210805*/
/*pragma libcall GfxBase BltMaskBitMapRastPort 27c 3291080a*/
/*pragma libcall GfxBase GraphicsReserved1 282 0*/
/*pragma libcall GfxBase GraphicsReserved2 288 0*/
#pragma libcall GfxBase AttemptLockLayerRom 28e d01
#endif
