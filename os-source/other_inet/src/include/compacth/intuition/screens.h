€ˆINTUITION_SCREENS_H€INTUITION_SCREENS_HˆEXEC_TYPES_HŒ"exec/types.h"‡ˆGRAPHICS_GFX_HŒ"graphics/gfx.h"‡ˆGRAPHICS_CLIP_HŒ"graphics/clip.h"‡ˆGRAPHICS_VIEW_HŒ"graphics/view.h"‡ˆGRAPHICS_RASTPORT_HŒ"graphics/rastport.h"‡ˆGRAPHICS_LAYERS_HŒ"graphics/layers.h"‡
ƒScreen
{
ƒScreen*NextScreen;
ƒWindow*FirstWindow;
•LeftEdge,TopEdge;
•Width,Height;
•MouseY,MouseX;
™¦;
Š*Title;
Š*DefaultTitle;
šBarHeight,BarVBorder,BarHBorder,MenuVBorder,MenuHBorder;
šWBorTop,WBorLeft,WBorRight,WBorBottom;
ƒTextAttr*Font;
ƒViewPort ViewPort;
ƒRastPort RastPort;
ƒBitMap BitMap;
ƒLayer_Info LayerInfo;
ƒ»*FirstGadget;
ŠDetailPen,BlockPen;
™SaveColor0;
ƒLayer*BarLayer;
Š*ExtData;
Š*UserData;
};€SCREENTYPE 15€WBENCHSCREEN 1€CUSTOMSCREEN 15€SHOWTITLE 16€BEEPING 32€CUSTOMBITMAP 64€SCREENBEHIND 128€SCREENQUIET 256€STDSCREENHEIGHT -1
ƒNewScreen
{
•LeftEdge,TopEdge,Width,Height,Depth;
ŠDetailPen,BlockPen;
™ViewModes;
™Type;
ƒTextAttr*Font;
Š*DefaultTitle;
ƒ»*Gadgets;
ƒBitMap*CustomBitMap;
};‡