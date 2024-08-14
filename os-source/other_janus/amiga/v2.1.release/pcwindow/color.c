
/**** color.c ***************************************************************
 * 
 * ColorWindow Routines  --  the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * Late 85     =RJ Mical=  Created this file
 * 27 Feb 86   =RJ Mical=  Modified these routines for Zaphod
 * January 86  =RJ=        Modified the originals for Mandelbrot
 *
 ***************************************************************************/

#include "zaphod.h"
#include <exec/memory.h>

#define COPYCOLOR   1
#define RANGE_FIRST  2
#define RANGE_SECOND 3


extern struct Gadget ColorTemplateGadgets[COLOR_GADGETS_COUNT];

static USHORT SavePalette[32];
#define COLOR_COLOR_ROWS   (4 * 10)
#define COLOR_COLOR_COLS   (8 * 15)
#define COLOR_COLOR_RIGHT   (COLOR_BOX_LEFT + COLOR_COLOR_COLS - 1)
#define COLOR_COLOR_BOTTOM  (COLOR_COLOR_TOP + COLOR_COLOR_ROWS - 1)

static USHORT *ChipRGB = NULL;


static struct NewWindow ColorNewWindow =
   {
   /* SHORT LeftEdge, TopEdge;      /* screen dimensions of window */
   /* SHORT Width, Height;            /* screen dimensions of window */
   20, 12,
   COLORWINDOW_WIDTH, COLORWINDOW_HEIGHT,

   /* UBYTE DetailPen, BlockPen;      /* for bar/border/gadget rendering */
   -1, -1,

   /* ULONG IDCMPFlags;            /* User-selected IDCMP flags */
   GADGETDOWN | GADGETUP | MOUSEBUTTONS  
         | MENUPICK | MOUSEMOVE | ACTIVEWINDOW | INACTIVEWINDOW,

   /* ULONG Flags;                  /* see Window struct for defines */
   BORDERLESS | SMART_REFRESH | NOCAREREFRESH | ACTIVATE,

   /* struct Gadget *FirstGadget;*/
   &ColorTemplateGadgets[COLOR_GADGETS_COUNT - 1],

   /* struct Image *CheckMark;*/
   NULL,

   /* UBYTE *Title;                    /* the title text for this window */
   NULL,
   
   /* struct Screen *Screen;*/
   NULL,
   
   /* struct BitMap *BitMap;*/
   NULL,

   /* SHORT MinWidth, MinHeight;       /* minimums */
   0, 0,
   /* SHORT MaxWidth, MaxHeight;       /* maximums */
   0, 0,

   /*      USHORT Type;*/
   CUSTOMSCREEN,
};

static USHORT ColorMode;
static USHORT RangeFirst;
static struct Window *ColorWindow = NULL;
static struct RastPort *ColorRPort;
static struct ViewPort *ColorVPort;
static SHORT RowCount, RowHeight, ColCount, ColWidth;

/**** colordat.c ************************************************************
 * 
 * Color Window Data  --  Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 27 Feb 86   =RJ Mical=  Modified these routines for Zaphod
 * January 86  =RJ=        Modified the originals for Mandelbrot
 * Late 85     =RJ=        Created the color window for Graphicraft
 *
 ***************************************************************************/
                           


#define COLOR_PROP_LEFT       38
#define COLOR_PROP_TOP            4
#define COLOR_PROP_WIDTH      165
#define COLOR_PROP_HEIGHT      10
#define COLOR_CLUSTER_LEFT      141
/*???RANGE_GONE #define COLOR_CLUSTER_TOP      41*/
#define COLOR_CLUSTER_TOP      35
#define COLOR_CLUSTER_WIDTH      (CHARACTER_WIDTH * 6 + 4)
#define COLOR_CLUSTER_HEIGHT      9
#define COLOR_HSL_TOP            (COLOR_BOX_TOP - 2)
#define COLOR_HSL_LEFT            (COLOR_BOX_RIGHT + 3)



static USHORT RGBData[] =
   {
   0xFC00,
   0x6600,
   0x6600,
   0x7C00,
   0x6C00,
   0x6600,
   0xE300,
   0x0000,
   0x0000,
   0x0000,
   0x0000,
   0x3C00,
   0x6600,
   0xC000,
   0xCE00,
   0xC600,
   0x6600,
   0x3E00,
   0x0000,
   0x0000,
   0x0000,
   0x0000,
   0xFC00,
   0x6600,
   0x6600,
   0x7C00,
   0x6600,
   0x6600,
   0xFC00,
   };


static struct Image ColorRGBImage =
   {
   3, 1,
   8, 
   29,
   1,
   &RGBData[0],
   0x1, 0x0,
   NULL,
   };


static SHORT ClusterBorderVectors[] =
   {
   -1, -1,
   -1, COLOR_CLUSTER_HEIGHT,
   COLOR_CLUSTER_WIDTH, COLOR_CLUSTER_HEIGHT,
   COLOR_CLUSTER_WIDTH, -1,
   -1, -1,
   };


static struct Border ColorClusterBorder =
   {
   0, 0, 
   1, 0,
   JAM1,
   5,
   &ClusterBorderVectors[0],
   NULL,
   };

static struct IntuiText ColorClusterText[4] =
   {
   /* "COPY" */
      {
      1, 0,
      JAM2,
      2 + CHARACTER_WIDTH, 1, 
      &SafeFont,
      (UBYTE *)"Copy",
      NULL,
      },

   /* "RANGE" */
      {
      1, 0, 
      JAM2,
      2 + (CHARACTER_WIDTH >> 1L), 1, 
      &SafeFont,
      (UBYTE *)"Range",
      NULL,
      },

   /* "OK" */
      {
      1, 0, 
      JAM2,
      2L + (CHARACTER_WIDTH << 1L), 1, 
      &SafeFont,
      (UBYTE *)"OK",
      NULL,
      },

   /* "CANCEL" */
      {
      1, 0, 
      JAM2,
      2, 1, 
      &SafeFont,
      (UBYTE *)"Cancel",
      NULL,
      },

   };




/* ======================================================================== */
/* ======================================================================== */
/* ======================================================================== */

static struct Image ColorPropsImages[3];


static struct PropInfo ColorPropsInfos[3] = 
   {
      {
      /* COLOR_GREEN */
      AUTOKNOB | FREEHORIZ,
      0,
      0,
      COLOR_KNOB_BODY,
      0,
      0, 0, 0, 0, 0, 0,
      },

      {
      /* COLOR_RED */
      AUTOKNOB | FREEHORIZ,
      0,
      0,
      COLOR_KNOB_BODY,
      0,
      0, 0, 0, 0, 0, 0,
      },

      {
      /* COLOR_BLUE */
      AUTOKNOB | FREEHORIZ,
      0,
      0,
      COLOR_KNOB_BODY,
      0,
      0, 0, 0, 0, 0, 0,
      },

   };


static struct Gadget ColorTemplateGadgets[COLOR_GADGETS_COUNT] =
   {
      {
      /* COLOR_COPY */
      NULL,
      COLOR_CLUSTER_LEFT,
/*???RANGE_GONE       COLOR_CLUSTER_TOP + (00 * (COLOR_CLUSTER_HEIGHT + 3)),*/
      COLOR_CLUSTER_TOP + (01 * (COLOR_CLUSTER_HEIGHT + 4)),
      COLOR_CLUSTER_WIDTH,
      COLOR_CLUSTER_HEIGHT,
      GADGHCOMP,
      RELVERIFY,
      BOOLGADGET,
      (APTR)&ColorClusterBorder,
      NULL,
      &ColorClusterText[00],
      NULL,
      NULL,
      COLOR_COPY,
      NULL,
      },

      {
      /* COLOR_RANGE */
      &ColorTemplateGadgets[COLOR_COPY],
      COLOR_CLUSTER_LEFT,
      COLOR_CLUSTER_TOP + (01 * (COLOR_CLUSTER_HEIGHT + 3)),
      COLOR_CLUSTER_WIDTH,
      COLOR_CLUSTER_HEIGHT,
      GADGHCOMP,
      RELVERIFY,
      BOOLGADGET,
      (APTR)&ColorClusterBorder,
      NULL,
      &ColorClusterText[01],
      NULL,
      NULL,
      COLOR_RANGE,
      NULL,
      },

      {
      /* COLOR_OK */
/*???RANGE_GONE       &ColorTemplateGadgets[COLOR_RANGE],*/
      &ColorTemplateGadgets[COLOR_COPY],
      COLOR_CLUSTER_LEFT,
/*???RANGE_GONE       COLOR_CLUSTER_TOP + (02 * (COLOR_CLUSTER_HEIGHT + 3)),*/
      COLOR_CLUSTER_TOP + (02 * (COLOR_CLUSTER_HEIGHT + 4)),
      COLOR_CLUSTER_WIDTH,
      COLOR_CLUSTER_HEIGHT,
      GADGHCOMP,
      RELVERIFY,
      BOOLGADGET,
      (APTR)&ColorClusterBorder,
      NULL,
      &ColorClusterText[02],
      NULL,
      NULL,
      COLOR_OK,
      NULL,
      },

      {
      /* COLOR_CANCEL */
      &ColorTemplateGadgets[COLOR_OK],
      COLOR_CLUSTER_LEFT,
/*???RANGE_GONE       COLOR_CLUSTER_TOP + (03 * (COLOR_CLUSTER_HEIGHT + 3)),*/
      COLOR_CLUSTER_TOP + (03 * (COLOR_CLUSTER_HEIGHT + 4)),
      COLOR_CLUSTER_WIDTH,
      COLOR_CLUSTER_HEIGHT,
      GADGHCOMP,
      RELVERIFY,
      BOOLGADGET,
      (APTR)&ColorClusterBorder,
      NULL,
      &ColorClusterText[03],
      NULL,
      NULL,
      COLOR_CANCEL,
      NULL,
      },

      {
      /* COLOR_GREEN */
      &ColorTemplateGadgets[COLOR_CANCEL],
      COLOR_PROP_LEFT,
      COLOR_PROP_TOP + (01 * (COLOR_PROP_HEIGHT + 1)),
      COLOR_PROP_WIDTH,
      COLOR_PROP_HEIGHT,
      GADGHCOMP | GADGIMAGE,
      FOLLOWMOUSE,
      PROPGADGET,
      (APTR)&ColorPropsImages[01],
      NULL,
      NULL,
      NULL,
      (APTR)&ColorPropsInfos[01],
      COLOR_GREEN,
      NULL,
      },

      {
      /* COLOR_RED */
      &ColorTemplateGadgets[COLOR_GREEN],
      COLOR_PROP_LEFT,
      COLOR_PROP_TOP + (00 * (COLOR_PROP_HEIGHT + 1)),
      COLOR_PROP_WIDTH,
      COLOR_PROP_HEIGHT,
      GADGHCOMP | GADGIMAGE,
      FOLLOWMOUSE,
      PROPGADGET,
      (APTR)&ColorPropsImages[00],
      NULL,
      NULL,
      NULL,
      (APTR)&ColorPropsInfos[00],
      COLOR_RED,
      NULL,
      },

      {
      /* COLOR_BLUE */
      &ColorTemplateGadgets[COLOR_RED],
      COLOR_PROP_LEFT,
      COLOR_PROP_TOP + (02 * (COLOR_PROP_HEIGHT + 1)),
      COLOR_PROP_WIDTH,
      COLOR_PROP_HEIGHT,
      GADGHCOMP | GADGIMAGE,
      FOLLOWMOUSE,
      PROPGADGET,
      (APTR)&ColorPropsImages[02],
      NULL,
      NULL,
      NULL,
      (APTR)&ColorPropsInfos[02],
      COLOR_BLUE,
      NULL,
      },

      {
      /* COLOR_HSL_RGB */
      &ColorTemplateGadgets[COLOR_BLUE],
      COLOR_HSL_LEFT,
      COLOR_HSL_TOP,
      CHARACTER_WIDTH + 5,
      COLOR_BOX_BOTTOM - COLOR_BOX_TOP + 1,
      GADGHIMAGE | GADGIMAGE | SELECTED,
      TOGGLESELECT,
      BOOLGADGET,
      (APTR)&ColorRGBImage,
      (APTR)&ColorRGBImage,
      NULL,
      NULL,
      NULL,
      COLOR_HSL_RGB,
      NULL,
      },
   };




/****i* PCWindow/OpenColorWindow ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

struct Window *OpenColorWindow(display)
struct Display *display;
{
   SHORT i;
   struct SuperWindow *superwindow;

#if (DEBUG1 & DBG_OPEN_COLOR_WINDOW_ENTER)
   kprintf("color.c:      OpenColorWindow(display=0x%lx)\n",display);
#endif

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
      MyAlert(ALERT_NO_COLOR_WINDOW_MEMORY, NULL);
      return(NULL);
      }

   ColorVPort = &ColorWindow->WScreen->ViewPort;
   ColorRPort = ColorWindow->RPort;
   for (i = 0; i < 32; i++)
      SavePalette[i] = GetRGB4(ColorVPort->ColorMap, i);

   SetAPen(ColorRPort, 0);

   InitColorSizes((SHORT)ColorRPort->BitMap->Depth);
   DrawColorWindow();

#if (DEBUG2 & DBG_OPEN_COLOR_WINDOW_RETURN)
   kprintf("color.c:      OpenColorWindow: Returns(0x%lx)\n",ColorWindow);
#endif

   return(ColorWindow);
}

/****i* PCWindow/InitColorSizes ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID InitColorSizes(depth)
SHORT depth;
{            
#if (DEBUG1 & DBG_INIT_COLOR_SIZES_ENTER)
   kprintf("color.c:      InitColorSizes(depth=0x%lx)\n",depth);
#endif

   RowCount = 1L << (depth >> 1L);
   RowHeight = COLOR_COLOR_ROWS / RowCount;

   ColCount = 1L << ((depth + 1L) >> 1L);
   ColWidth = COLOR_COLOR_COLS / ColCount;

#if (DEBUG2 & DBG_INIT_COLOR_SIZES_RETURN)
   kprintf("color.c:      InitColorSizes: Returns(VOID)\n");
#endif
}

/****i* PCWindow/CloseColorWindow ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID CloseColorWindow(accept, display)
BOOL accept;
struct Display *display;
{
   SHORT depth, i;
   USHORT *colortable;

#if (DEBUG1 & DBG_CLOSE_COLOR_WINDOW_ENTER)
   kprintf("color.c:      CloseColorWindow(accept=0x%lx,display=0x%lx)\n",accept,display);
#endif

   if (ColorWindow == NULL) return;

   if (NOT accept) LoadRGB4(ColorVPort, &SavePalette[0], 32);
   else
      {
      depth = ColorRPort->BitMap->Depth;
      colortable = (USHORT *)GetColorAddress(display->Modes, depth);
      for (i = 0; i < (1L << depth); i++)
         *(colortable + i) = GetRGB4(ColorVPort->ColorMap, i);

/*??? The first try doesn't work, but the second does.  Hmm... */
/*???      if (FlagIsSet(ColorWindow->Flags, WBENCHWINDOW))*/
      }
   CloseWindow(ColorWindow);
   ColorWindow = NULL;

#if (DEBUG2 & DBG_CLOSE_COLOR_WINDOW_RETURN)
   kprintf("color.c:      CloseColorWindow: Returns(VOID)\n");
#endif
}

/****i* PCWindow/ColorRange ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID ColorRange(first, last)
SHORT first, last;
{
   SHORT i;
   SHORT whole, redfraction, greenfraction, bluefraction, divisor;
   USHORT rgb;
   SHORT firstred, firstgreen, firstblue;
   SHORT lastred, lastgreen, lastblue;
   SHORT workred, workgreen, workblue;

#if (DEBUG1 & DBG_COLOR_RANGE_ENTER)
   kprintf("color.c:      ColorRange(first=0x%lx,last=0x%lx)\n",first,last);
#endif
               
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
   firstred = (rgb >> 8L) & 0xF;
   firstgreen = (rgb >> 4L) & 0xF;
   firstblue = (rgb >> 0L) & 0xF;

   rgb = GetRGB4(ColorVPort->ColorMap, last);
   lastred = (rgb >> 8L) & 0xF;
   lastgreen = (rgb >> 4L) & 0xF;
   lastblue = (rgb >> 0L) & 0xF;


   divisor = last - first;

   whole = (lastred - firstred) << 8L;
   redfraction = whole / divisor;

   whole = (lastgreen - firstgreen) << 8L;
   greenfraction = whole / divisor;

   whole = (lastblue - firstblue) << 8L;
   bluefraction = whole / divisor;

   for (i = first + 1; i < last; i++)
      {
      lastred = ((redfraction * (i - first)) + 0x0080) >> 8L;
      workred = firstred + lastred;
      lastgreen = ((greenfraction * (i - first)) + 0x0080) >> 8L;
      workgreen = firstgreen + lastgreen;
      lastblue = ((bluefraction * (i - first)) + 0x0080) >> 8L;
      workblue = firstblue + lastblue;
      SetRGB4(ColorVPort, i, workred, workgreen, workblue);
      }

#if (DEBUG2 & DBG_COLOR_RANGE_RETURN)
   kprintf("color.c:      ColorRange: Returns(VOID)\n");
#endif
}

/****i* PCWindow/ColorWindowHit ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID ColorWindowHit(x, y)
SHORT x, y;
{
   USHORT rgb, pen;
   SHORT greenpos, redpos, bluepos;

#if (DEBUG1 & DBG_COLOR_WINDOW_HIT_ENTER)
   kprintf("color.c:      ColorWindowHit(x=0x%lx,y=0x%lx)\n",x,y);
#endif

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
         SetRGB4(ColorVPort, pen, rgb >> 8L, rgb >> 4L, rgb);
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

#if (DEBUG2 & DBG_COLOR_WINDOW_HIT_RETURN)
   kprintf("color.c:      ColorWindowHit: Returns(VOID)\n");
#endif

}

/****i* PCWindow/ColorGadgetGotten ****************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

BOOL ColorGadgetGotten(gadget, display)
struct Gadget *gadget;
struct Display *display;
{
#if (DEBUG1 & DBG_COLOR_GADGET_GOTTEN_ENTER)
   kprintf("color.c:      ColorGadgetGotten(gadget=0x%lx,display=0x%lx)\n",gadget,display);
#endif

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

#if (DEBUG2 & DBG_COLOR_GADGET_GOTTEN_RETURN)
   kprintf("color.c:      ColorGadgetGotten: Returns(???)\n");
#endif
   return(TRUE);
}

/****i* PCWindow/ModifyColors ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID ModifyColors()
{
   USHORT pen, newred, newgreen, newblue;

#if (DEBUG1 & DBG_MODIFY_COLORS_ENTER)
   kprintf("color.c:      ModifyColors(VOID)\n");
#endif

   pen = ColorRPort->FgPen;

   newred = ((struct PropInfo *)
         ColorTemplateGadgets[COLOR_RED].SpecialInfo)->HorizPot >> 12L;
   newgreen = ((struct PropInfo *)
         ColorTemplateGadgets[COLOR_GREEN].SpecialInfo)->HorizPot >> 12L;
   newblue = ((struct PropInfo *)
         ColorTemplateGadgets[COLOR_BLUE].SpecialInfo)->HorizPot >> 12L;
   SetRGB4(ColorVPort, pen, newred, newgreen, newblue);

#if (DEBUG2 & DBG_MODIFY_COLORS_RETURN)
   kprintf("color.c:      ModifyColors: Returns(VOID)\n");
#endif
}

/****i* PCWindow/GDrawBox ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
    
VOID GDrawBox(rp, left, top, right, bottom)
struct RastPort *rp;
SHORT left, top, right, bottom;
{
#if (DEBUG1 & DBG_G_DRAW_BOX_ENTER)
   kprintf("color.c:      GDrawBox(rp=0x%lx,left=0x%lx,top=0x%lx,right=0x%lx,bottom=0x%lx)\n",rp,left,top,right,bottom);
#endif

   DrawBox(rp, left, top,(SHORT)( right - left + 1),(SHORT)( bottom - top + 1), 1, 1, 1);

#if (DEBUG2 & DBG_G_DRAW_BOX_RETURN)
   kprintf("color.c:      GDrawBox: Returns(VOID)\n");
#endif
}
  
/****i* PCWindow/DrawColorWindow ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
    
VOID DrawColorWindow()
{
   SHORT col, row, colstart, rowstart;
   SHORT colend;
   SHORT pen;

#if (DEBUG1 & DBG_DRAW_COLOR_WINDOW_ENTER)
   kprintf("color.c:      DrawColorWindow(VOID)\n");
#endif

   pen = ColorRPort->FgPen;

   ColorRectFill(ColorRPort,(SHORT) ColorRPort->FgPen);
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

#if (DEBUG2 & DBG_DRAW_COLOR_WINDOW_RETURN)
   kprintf("color.c:      DrawColorWindow: Returns(VOID)\n");
#endif
}

/****i* PCWindow/SetColorProps ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID SetColorProps(vport, rport)
struct ViewPort *vport;
struct RastPort *rport;
{
   USHORT rgb, red, green, blue;

#if (DEBUG1 & DBG_SET_COLOR_PROPS_ENTER)
   kprintf("color.c:      SetColorProps(vport=0x%lx,rport=0x%lx)\n",vport,rport);
#endif

   rgb = GetRGB4(vport->ColorMap, rport->FgPen);
   red = (rgb >> 8L) & 0xF;
   green = (rgb >> 4L) & 0xF;
   blue = (rgb >> 0L) & 0xF;

   ((struct PropInfo *)ColorTemplateGadgets[COLOR_RED]
         .SpecialInfo)->HorizPot
         = (red << 12L) | (red << 8L) | (red << 4L) | red;
   ((struct PropInfo *)ColorTemplateGadgets[COLOR_GREEN]
         .SpecialInfo)->HorizPot
         = (green << 12L) | (green << 8L) | (green << 4L) | green;
   ((struct PropInfo *)ColorTemplateGadgets[COLOR_BLUE]
         .SpecialInfo)->HorizPot
         = (blue << 12L) | (blue << 8L) | (blue << 4L) | blue;

#if (DEBUG2 & DBG_SET_COLOR_PROPS_RETURN)
   kprintf("color.c:      SetColorProps: Returns(VOID)\n");
#endif
}

/****i* PCWindow/ColorRectFill ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID ColorRectFill(rp, pen)
struct RastPort *rp;
SHORT pen;
{
#if (DEBUG1 & DBG_COLOR_RECT_FILL_ENTER)
   kprintf("color.c:      ColorRectFill(rp=0x%lx,pen=0x%lx)\n",rp,pen);
#endif

   SetAPen(rp, pen);
   SetDrMd(rp, JAM2);
   RectFill(rp, COLOR_BOX_LEFT, COLOR_BOX_TOP, 
      COLOR_BOX_RIGHT, COLOR_BOX_BOTTOM);

#if (DEBUG2 & DBG_COLOR_RECT_FILL_RETURN)
   kprintf("color.c:      ColorRectFill: Returns(VOID)\n");
#endif
}

/****i* PCWindow/DoColorWindow ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID DoColorWindow(display)
struct Display *display;
{
   struct IntuiMessage *message;
   ULONG class;
   struct Gadget *gadget;
   struct MsgPort *saveport;
   ULONG saveidcmp;
   UBYTE *savetitle;
   struct Window *oldwindow;
   BOOL mousemoved;
   SHORT x, y, code;

#if (DEBUG1 & DBG_DO_COLOR_WINDOW_ENTER)
   kprintf("color.c:      DoColorWindow(display=0x%lx)\n",display);
#endif
      
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
      Wait((1L << ColorWindow->UserPort->mp_SigBit));

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

#if (DEBUG2 & DBG_DO_COLOR_WINDOW_RETURN)
   kprintf("color.c:      DoColorWindow: Returns(VOID)\n");
#endif
}
