��INTUITION_SCREENS_H�INTUITION_SCREENS_H�EXEC_TYPES_H�"exec/types.h"��GRAPHICS_GFX_H�"graphics/gfx.h"��GRAPHICS_CLIP_H�"graphics/clip.h"��GRAPHICS_VIEW_H�"graphics/view.h"��GRAPHICS_RASTPORT_H�"graphics/rastport.h"��GRAPHICS_LAYERS_H�"graphics/layers.h"�
�Screen
{
�Screen*NextScreen;
�Window*FirstWindow;
�LeftEdge,TopEdge;
�Width,Height;
�MouseY,MouseX;
��;
�*Title;
�*DefaultTitle;
�BarHeight,BarVBorder,BarHBorder,MenuVBorder,MenuHBorder;
�WBorTop,WBorLeft,WBorRight,WBorBottom;
�TextAttr*Font;
�ViewPort ViewPort;
�RastPort RastPort;
�BitMap BitMap;
�Layer_Info LayerInfo;
��*FirstGadget;
�DetailPen,BlockPen;
�SaveColor0;
�Layer*BarLayer;
�*ExtData;
�*UserData;
};�SCREENTYPE 15�WBENCHSCREEN 1�CUSTOMSCREEN 15�SHOWTITLE 16�BEEPING 32�CUSTOMBITMAP 64�SCREENBEHIND 128�SCREENQUIET 256�STDSCREENHEIGHT -1
�NewScreen
{
�LeftEdge,TopEdge,Width,Height,Depth;
�DetailPen,BlockPen;
�ViewModes;
�Type;
�TextAttr*Font;
�*DefaultTitle;
��*Gadgets;
�BitMap*CustomBitMap;
};�