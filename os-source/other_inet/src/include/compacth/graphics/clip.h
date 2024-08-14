ÄàGRAPHICS_CLIP_HÄGRAPHICS_CLIP_HàGRAPHICS_GFX_Hå<graphics/gfx.h>áàEXEC_SEMAPHORES_Hå<exec/semaphores.h>áÄNEWLOCKS
ÉLayer
{
ÉLayer*front,*back;
ÉClipRect*ClipRect;
ÉRastPort*rp;
ÉRectangle bounds;
äreserved[4];
âpriority;
â¶;
ÉBitMap*SuperBitMap;
ÉClipRect*SuperClipRect;
îWindow;
ïScroll_X,Scroll_Y;
ÉClipRect*cr,*cr2,*crnew;
ÉClipRect*SuperSaveClipRects;
ÉClipRect*_cliprects;
ÉLayer_Info*LayerInfo;
ÉSignalSemaphore Lock;
äreserved3[8];
ÉRegion*ClipRegion;
ÉRegion*saveClipRects;
äreserved2[22];
ÉRegion*DamageList;
};
ÉClipRect
{
ÉClipRect*Next;
ÉClipRect*prev;
ÉLayer*lobs;
ÉBitMap*BitMap;
ÉRectangle bounds;
ÉClipRect*_p1,*_p2;
íreserved;∞NEWCLIPRECTS_1_1
í¶;á
};ÄCR_NEEDS_NO_CONCEALED_RASTERS 1ÄISLESSX 1ÄISLESSY 2ÄISGRTRX 4ÄISGRTRY 8á