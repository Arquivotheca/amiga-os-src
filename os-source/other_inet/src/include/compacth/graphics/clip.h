��GRAPHICS_CLIP_H�GRAPHICS_CLIP_H�GRAPHICS_GFX_H�<graphics/gfx.h>��EXEC_SEMAPHORES_H�<exec/semaphores.h>��NEWLOCKS
�Layer
{
�Layer*front,*back;
�ClipRect*ClipRect;
�RastPort*rp;
�Rectangle bounds;
�reserved[4];
�priority;
��;
�BitMap*SuperBitMap;
�ClipRect*SuperClipRect;
�Window;
�Scroll_X,Scroll_Y;
�ClipRect*cr,*cr2,*crnew;
�ClipRect*SuperSaveClipRects;
�ClipRect*_cliprects;
�Layer_Info*LayerInfo;
�SignalSemaphore Lock;
�reserved3[8];
�Region*ClipRegion;
�Region*saveClipRects;
�reserved2[22];
�Region*DamageList;
};
�ClipRect
{
�ClipRect*Next;
�ClipRect*prev;
�Layer*lobs;
�BitMap*BitMap;
�Rectangle bounds;
�ClipRect*_p1,*_p2;
�reserved;�NEWCLIPRECTS_1_1
��;�
};�CR_NEEDS_NO_CONCEALED_RASTERS 1�ISLESSX 1�ISLESSY 2�ISGRTRX 4�ISGRTRY 8�