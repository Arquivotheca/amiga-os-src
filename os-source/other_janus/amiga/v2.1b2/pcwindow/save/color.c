
/* *** color.c ***************************************************************
 * 
 * ColorWindow Routines  --  the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * Late 85     =RJ Mical=  Created this file
 * 27 Feb 86   =RJ Mical=  Modified these routines for Zaphod
 * January 86  =RJ=        Modified the originals for Mandelbrot
 *
 * **************************************************************************/

#include "zaphod.h"
#include <exec/memory.h>

void InitColorSizes();
void ColorWindowHit();
void ModifyColors();
void GDrawBox();
void DrawColorWindow();
void SetColorProps();
void ColorRectFill();

#define COPYCOLOR   1
#define RANGE_FIRST  2
#define RANGE_SECOND 3


extern struct Gadget ColorTemplateGadgets[COLOR_GADGETS_COUNT];
extern struct Image ColorPropsImages[3];
extern struct PropInfo ColorPropsInfos[3];
extern struct IntuiText ColorClusterText[4];
extern struct Image ColorRGBImage;
extern USHORT RGBData[];
extern struct Image SuperColorImages[32];

USHORT SavePalette[32];
#define COLOR_COLOR_ROWS   (4 * 10)
#define COLOR_COLOR_COLS   (8 * 15)
#define COLOR_COLOR_RIGHT   (COLOR_BOX_LEFT + COLOR_COLOR_COLS - 1)
#define COLOR_COLOR_BOTTOM  (COLOR_COLOR_TOP + COLOR_COLOR_ROWS - 1)

USHORT *ChipRGB = NULL;


struct NewWindow ColorNewWindow =
   {
   /*      SHORT LeftEdge, TopEdge;      /* screen dimensions of window */
   /*      SHORT Width, Height;            /* screen dimensions of window */
   20, 12,
   COLORWINDOW_WIDTH, COLORWINDOW_HEIGHT,

   /*      UBYTE DetailPen, BlockPen;      /* for bar/border/gadget rendering */
   -1, -1,

   /*      ULONG IDCMPFlags;            /* User-selected IDCMP flags */
   GADGETDOWN | GADGETUP | MOUSEBUTTONS  
         | MENUPICK | MOUSEMOVE | ACTIVEWINDOW | INACTIVEWINDOW,

   /*      ULONG Flags;                  /* see Window struct for defines */
   BORDERLESS | SMART_REFRESH | NOCAREREFRESH | ACTIVATE,

   /*      struct Gadget *FirstGadget;*/
   &ColorTemplateGadgets[COLOR_GADGETS_COUNT - 1],

   /*      struct Image *CheckMark;*/
   NULL,

   /*      UBYTE *Title;                    /* the title text for this window */
   NULL,
   
   /*      struct Screen *Screen;*/
   NULL,
   
   /*      struct BitMap *BitMap;*/
   NULL,

   /*      SHORT MinWidth, MinHeight;       /* minimums */
   0, 0,
   /*      SHORT MaxWidth, MaxHeight;       /* maximums */
   0, 0,

   /*      USHORT Type;*/
   CUSTOMSCREEN,
};

USHORT ColorMode;
USHORT RangeFirst;
struct Window *ColorWindow = NULL;
struct RastPort *ColorRPort;
struct ViewPort *ColorVPort;
SHORT RowCount, RowHeight, ColCount, ColWidth;



/* ======================================================================== */
/* ======================================================================== */
/* ======================================================================== */

struct Window *OpenColorWindow(display)
REGISTER struct Display *display;
{
   REGISTER SHORT i;
   REGISTER struct SuperWindow *superwindow;

   if (ChipRGB == NULL)
      {
      if (ChipRGB = (USHORT *)AllocRemember(&GlobalKey, 29 * 2, MEMF_CHIP))
         {
         ColorRGBImage.ImageData = (USHORT *)ChipRGB;
         for (i = 0; i < 29; i++) *ChipRGB++ = RGBData[i];
         }
      }

   superwindow = display->FirstWindow;

   if (ColorNewWindow.Screen = superwindow->DisplayScreen)
      ColorNewWindow.Type = CUSTOMSCREEN;
   else ColorNewWindow.Type = WBENCHSCREEN;

   SetAPen(superwindow->DisplayWindow->RPort, 0);
   SetColorProps(&superwindow->DisplayWindow->WScreen->ViewPort,
         superwindow->DisplayWindow->RPort);

   ColorMode = NULL;

   if ((ColorWindow = OpenWindow(&ColorNewWindow)) == 0)
      {
      MyAlert(ALERT_NO_MEMORY, NULL);
      return(NULL);
      }

   ColorVPort = &ColorWindow->WScreen->ViewPort;
   ColorRPort = ColorWindow->RPort;
   for (i = 0; i < 32; i++)
      SavePalette[i] = GetRGB4(ColorVPort->ColorMap, i);

   SetAPen(ColorRPort, 0);

   InitColorSizes(ColorRPort->BitMap->Depth);
   DrawColorWindow(ColorWindow);

   return(ColorWindow);
}




void InitColorSizes(depth)
REGISTER SHORT depth;
{            
   RowCount = 1 << (depth >> 1);
   RowHeight = COLOR_COLOR_ROWS / RowCount;

   ColCount = 1 << ((depth + 1) >> 1);
   ColWidth = COLOR_COLOR_COLS / ColCount;
}




VOID CloseColorWindow(accept, display)
REGISTER BOOL accept;
REGISTER struct Display *display;
{
   REGISTER SHORT depth, i;
   REGISTER USHORT *colortable;

   if (ColorWindow == NULL) return;

   if (NOT accept) LoadRGB4(ColorVPort, &SavePalette[0], 32);
   else
      {
      depth = ColorRPort->BitMap->Depth;
      colortable = (USHORT *)GetColorAddress(display->Modes, depth);
      for (i = 0; i < (1 << depth); i++)
         *(colortable + i) = GetRGB4(ColorVPort->ColorMap, i);

/*??? The first try doesn't work, but the second does.  Hmm... */
/*???      if (FlagIsSet(ColorWindow->Flags, WBENCHWINDOW))*/
      if (ColorNewWindow.Type == WBENCHSCREEN)
         SetThosePrefs();
      }
   CloseWindow(ColorWindow);
   ColorWindow = NULL;
}




VOID ColorRange(first, last)
REGISTER SHORT first, last;
{
   REGISTER SHORT i;
   SHORT whole, redfraction, greenfraction, bluefraction, divisor;
   REGISTER USHORT rgb;
   SHORT firstred, firstgreen, firstblue;
   SHORT lastred, lastgreen, lastblue;
   SHORT workred, workgreen, workblue;
               
   if (first > last)
      {
      i = first;
      first = last;
      last = i;
      }

   /* I need to see a spread of at least two, where there's at least one
    * spot between the endpoints, else there's no work to do so I
    * might as well just return now.
    */
   if (first >= last - 1) return;

   rgb = GetRGB4(ColorVPort->ColorMap, first);
   firstred = (rgb >> 8) & 0xF;
   firstgreen = (rgb >> 4) & 0xF;
   firstblue = (rgb >> 0) & 0xF;

   rgb = GetRGB4(ColorVPort->ColorMap, last);
   lastred = (rgb >> 8) & 0xF;
   lastgreen = (rgb >> 4) & 0xF;
   lastblue = (rgb >> 0) & 0xF;


   divisor = last - first;

   whole = (lastred - firstred) << 8;
   redfraction = whole / divisor;

   whole = (lastgreen - firstgreen) << 8;
   greenfraction = whole / divisor;

   whole = (lastblue - firstblue) << 8;
   bluefraction = whole / divisor;

   for (i = first + 1; i < last; i++)
      {
      lastred = ((redfraction * (i - first)) + 0x0080) >> 8;
      workred = firstred + lastred;
      lastgreen = ((greenfraction * (i - first)) + 0x0080) >> 8;
      workgreen = firstgreen + lastgreen;
      lastblue = ((bluefraction * (i - first)) + 0x0080) >> 8;
      workblue = firstblue + lastblue;
      SetRGB4(ColorVPort, i, workred, workgreen, workblue);
      }
}




void ColorWindowHit(x, y)
REGISTER SHORT x, y;
{
   REGISTER USHORT rgb, pen;
   SHORT greenpos, redpos, bluepos;

   /* Have we got a color specifier? */
   if ((x >= COLOR_BOX_LEFT) && (x <= COLOR_COLOR_RIGHT)
         && (y >= COLOR_COLOR_TOP) && (y <= COLOR_COLOR_BOTTOM))
      {
      /* Yes, it's a color gadget.  Set this pen number */
      x = x - COLOR_BOX_LEFT;
      x = x / ColWidth;
      y = y - COLOR_COLOR_TOP;
      y = y / RowHeight;
      pen = (y * ColCount) + x;
      /* first, were we in COPY COLOR mode? */
      if (ColorMode == COPYCOLOR)
         {
         /* ok, copy old color here first! */
         rgb = GetRGB4(ColorVPort->ColorMap, ColorRPort->FgPen);
         SetRGB4(ColorVPort, pen, rgb >> 8, rgb >> 4, rgb);
         ColorMode = NULL;
         }
      if (ColorMode == RANGE_FIRST)
         {
         ColorMode = RANGE_SECOND;
         RangeFirst = pen;
         }
      else if (ColorMode == RANGE_SECOND)
         {
         ColorMode = NULL;
         ColorRange(RangeFirst, pen);
         }
      SetAPen(ColorRPort, pen);
      ColorRectFill(ColorRPort, pen);

      redpos = RemoveGadget(ColorWindow,
            &ColorTemplateGadgets[COLOR_RED]);
      greenpos = RemoveGadget(ColorWindow,
            &ColorTemplateGadgets[COLOR_GREEN]);
      bluepos = RemoveGadget(ColorWindow,
            &ColorTemplateGadgets[COLOR_BLUE]);
      SetColorProps(ColorVPort, ColorRPort);
      AddGadget(ColorWindow, &ColorTemplateGadgets[COLOR_BLUE], bluepos);
      AddGadget(ColorWindow, &ColorTemplateGadgets[COLOR_GREEN], greenpos);
      AddGadget(ColorWindow, &ColorTemplateGadgets[COLOR_RED], redpos);
      RefreshGadgets(&ColorTemplateGadgets[COLOR_GADGETS_COUNT - 1], 
            ColorWindow,NULL);
      }

}




BOOL ColorGadgetGotten(gadget, display)
REGISTER struct Gadget *gadget;
REGISTER struct Display *display;
{
   switch (gadget->GadgetID)
      {
      case COLOR_OK:
         CloseColorWindow(TRUE, display);
         return(FALSE);
         break;
      case COLOR_CANCEL:
         CloseColorWindow(FALSE, display);
         return(FALSE);
         break;
      case COLOR_COPY:
         ColorMode = COPYCOLOR;
         break;
      case COLOR_RANGE:
         ColorMode = RANGE_FIRST;
         break;
      }
   return(TRUE);
}




void ModifyColors()
{
   REGISTER USHORT pen, newred, newgreen, newblue;

   pen = ColorRPort->FgPen;

   newred = ((struct PropInfo *)
         ColorTemplateGadgets[COLOR_RED].SpecialInfo)->HorizPot >> 12;
   newgreen = ((struct PropInfo *)
         ColorTemplateGadgets[COLOR_GREEN].SpecialInfo)->HorizPot >> 12;
   newblue = ((struct PropInfo *)
         ColorTemplateGadgets[COLOR_BLUE].SpecialInfo)->HorizPot >> 12;
   SetRGB4(ColorVPort, pen, newred, newgreen, newblue);
}



    
void GDrawBox(rp, left, top, right, bottom)
REGISTER struct RastPort *rp;
REGISTER SHORT left, top, right, bottom;
{
   DrawBox(rp, left, top, right - left + 1, bottom - top + 1, 1, 1, 1);
}
  


    
void DrawColorWindow()
{
   REGISTER SHORT col, row, colstart, rowstart;
   SHORT colend;
   SHORT pen;

   pen = ColorRPort->FgPen;

   ColorRectFill(ColorRPort, ColorRPort->FgPen);
   GDrawBox(ColorRPort, 1, 1, COLORWINDOW_WIDTH - 2, COLORWINDOW_HEIGHT - 2);
   GDrawBox(ColorRPort, COLOR_BOX_LEFT - 2, COLOR_BOX_TOP - 2, 
         COLOR_BOX_RIGHT + 2, COLOR_BOX_BOTTOM + 2);
   GDrawBox(ColorRPort, COLOR_BOX_LEFT - 2, COLOR_COLOR_TOP - 2, 
         COLOR_BOX_LEFT + (8 * 15) + 1, COLOR_COLOR_TOP + (4 * 10) + 1);

   colstart = COLOR_BOX_LEFT;
   colend = colstart + ColWidth - 1;
   for (col = 0; col < ColCount; col++)
      {
      rowstart = COLOR_COLOR_TOP;
      for (row = 0; row < RowCount; row++)
         {
         SetAPen(ColorRPort, (row * ColCount) + col);
         RectFill(ColorRPort, colstart, rowstart, colend, 
               rowstart + RowHeight - 1);
         rowstart += RowHeight;
         }
      colstart += ColWidth;
      colend += ColWidth;
      }

   SetAPen(ColorRPort, pen);
}




void SetColorProps(vport, rport)
REGISTER struct ViewPort *vport;
REGISTER struct RastPort *rport;
{
   REGISTER USHORT rgb, red, green, blue;

   rgb = GetRGB4(vport->ColorMap, rport->FgPen);
   red = (rgb >> 8) & 0xF;
   green = (rgb >> 4) & 0xF;
   blue = (rgb >> 0) & 0xF;

   ((struct PropInfo *)ColorTemplateGadgets[COLOR_RED]
         .SpecialInfo)->HorizPot
         = (red << 12) | (red << 8) | (red << 4) | red;
   ((struct PropInfo *)ColorTemplateGadgets[COLOR_GREEN]
         .SpecialInfo)->HorizPot
         = (green << 12) | (green << 8) | (green << 4) | green;
   ((struct PropInfo *)ColorTemplateGadgets[COLOR_BLUE]
         .SpecialInfo)->HorizPot
         = (blue << 12) | (blue << 8) | (blue << 4) | blue;
}




void ColorRectFill(rp, pen)
REGISTER struct RastPort *rp;
REGISTER SHORT pen;
{
   SetAPen(rp, pen);
   SetDrMd(rp, JAM2);
   RectFill(rp, COLOR_BOX_LEFT, COLOR_BOX_TOP, 
      COLOR_BOX_RIGHT, COLOR_BOX_BOTTOM);
}

                                 

/* ======================================================================== */
/* ======================================================================== */
/* ======================================================================== */

VOID DoColorWindow(display)
REGISTER struct Display *display;
{
   REGISTER struct IntuiMessage *message;
   REGISTER ULONG class;
   REGISTER struct Gadget *gadget;
   struct MsgPort *saveport;
   ULONG saveidcmp;
   UBYTE *savetitle;
   REGISTER struct Window *oldwindow;
   BOOL mousemoved;
   REGISTER SHORT x, y, code;
      
   if (ColorWindow) return;
/*???
   if (NOT MyLock(&display->DisplayLock)) return;
???*/
DeathLock(&display->DisplayLock);

   if (display->FirstWindow == NULL) goto UNLOCK_RETURN;
   if ((oldwindow = display->FirstWindow->DisplayWindow) == NULL)
       goto UNLOCK_RETURN;

   Forbid();
   saveport = oldwindow->UserPort;
   saveidcmp = oldwindow->IDCMPFlags;
   oldwindow->UserPort = NULL;
   ModifyIDCMP(oldwindow, NULL);

   ClearMenuStrip(oldwindow);

   savetitle = oldwindow->Title;
   if (display->Modes & BORDER_VISIBLE)
      {
      if (display->Modes & COLOR_DISPLAY)
         SetWindowTitles(oldwindow, &ColorInactiveTitle[0],(UBYTE *) -1);
      else
         SetWindowTitles(oldwindow, &MonoInactiveTitle[0],(UBYTE *) -1);
      }
   Permit();

   if (NOT OpenColorWindow(display)) goto RESTORE_RETURN;

   FOREVER
      {
      Wait((1 << ColorWindow->UserPort->mp_SigBit));

      mousemoved = FALSE;
      while (message = (struct IntuiMessage *)GetMsg(ColorWindow->UserPort))
         {
         class = message->Class;
         code = message->Code;
         gadget = (struct Gadget *)(message->IAddress);
         x = message->MouseX;
         y = message->MouseY;
         ReplyMsg((struct Message *)message);

         switch (class)
            {
            case GADGETDOWN:
            case GADGETUP:
               if (ColorGadgetGotten(gadget, display) == FALSE)
                  goto RESTORE_RETURN;
               break;
            case MOUSEMOVE:
               mousemoved = TRUE;
               break;
            case MOUSEBUTTONS:
               if (code == SELECTDOWN) ColorWindowHit(x, y);
               break;
            }
         }
      if (mousemoved) ModifyColors();
      }
      

RESTORE_RETURN:
   oldwindow->UserPort = saveport;
   ModifyIDCMP(oldwindow, saveidcmp);
   SetMenuStrip(oldwindow, &MenuHeaders[MENU_HEADERS_COUNT - 1]);
   if (display->Modes & BORDER_VISIBLE)
      SetWindowTitles(oldwindow, savetitle,(UBYTE *) -1);


UNLOCK_RETURN:
/*???
   Unlock(&display->DisplayLock);
???*/
LifeUnlock(&display->DisplayLock);
}


