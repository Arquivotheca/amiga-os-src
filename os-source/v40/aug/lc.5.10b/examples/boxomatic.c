/* 
   boxomatic.c    by Gregg Williams               finished 29 May 86
                     BYTE Magazine
                     PO Box 372
                                 Hancock NH  03449

                                     Phone: 603-924-9281
                                     BIX: greggw

   This program shows:
                   dual playfield with bottom playfield visible "under" top
                     playfield and scrolling in two dimensions

           use of the following functions: OpenLibrary,
                     InitBitMap, InitView, InitVPort,
                     GetColorMap, AllocRaster, BltClear, MakeVPort, MrgCop,
                         LoadView, WaitTOF, FreeMemory, CloseLibrary, SetRast,
                         SetAPen, RectFill, FreeRaster, FreeVPortCopLists,
                         FreeCprList, FreeColorMap

   I have kept this program as simple as possible to make the workings
   of a scrolling dual-playfield as obvious as possible.  I am placing
   this source code in the public domain as a small repayment of the
   debt I owe the many people who have similarly contributed their work
   over the years.

   Please feel free to study this program, modify it, and use it as
   a "skeleton" for a larger, more powerful program that needs a dual
   playfield (game designers, take note).


                                          --Gregg Williams
*/

#include <exec/types.h>
#include <stdio.h>
#include <graphics/gfx.h>
#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <graphics/gfxmacros.h>
#include <graphics/copper.h>
#include <graphics/view.h>
#include <graphics/rastport.h>
#include <graphics/gels.h>
#include <graphics/regions.h>
#include <graphics/clip.h>
#include <exec/exec.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>
#include <proto/graphics.h>
#include <proto/exec.h>



/*   An overview of Amiga graphics--what's where:

  View  (what you see on the screen, contains important Modes variables
   |     for things like interlaced screen, dual playfield, and others)
   |
   |--(points to)
   V
  ViewPort1 --> ViewPort2 --> ... --> ViewPortN
   |   (A ViewPort is a horizontal slice of the screen; here you specify
   |  height, width, depth (determines # of colors available),
   |  sprites, type of display: dual-playfield, resolution, interlace,
   |  hold-and-modify, extra-halfbright)
   |    (The variables DxOffset and DyOffset position the upper left 
   |  corner of the viewport on the screen)
   |    (If there are multiple viewports, the first one points to the
   |  second, the second to the third, etc.)
   |\
   | \
   | V
   | ColorMap (establishes the colors available to this viewport)
   V
  RasInfo (contains RxOffset, RyOffset, which mark the pixel within 
   |       the bitmap (see below) that will be positioned in the
   |       upper left corner of the viewport--you can scroll the 
   |       bitmap in a viewport by changing these values)
   |
   |     RastPort (contains info on pen colors, text info, pointers
   |     /         to GelsInfo, AreaInfo (when needed)
   |    /
   |   /
   V  V
  BitMap (contains actual memory that represents an area of image;
          the image can be larger than what is visible on the screen)
*/

#define DEPTH 3L      /* depth of playfield 1, (scrolling, on bottom) */
#define WIDTH 640L    /* dimensions of playfield 1 */
#define HEIGHT 400L

/* Since pf1 is behind pf2, it can be any size you want it--you could
   make it a long, thin horizontal strip if you wanted to.  Just be
   sure that nothing unexpected shows through the "hole" in the top pf....
*/

#define DEPTH2 2L    /* depth of playfield 2 (stationary, on top) */
#define WIDTH2 322L  /* dimensions of playfield 2 */
#define HEIGHT2 200L

#define VPWIDTH 322L   /* dimensions of the viewport */
#define VPHEIGHT 200L

/* In this simple case, the view (i.e., what you see on the screen)
   is made entirely from one viewport.  If the 322 number looks odd
   to you, you're not alone.  I originally had WIDTH2 and VPWIDTH to
   be 320, but a thin strip of pf1 showed through the right margin of
   the screen.  If you can explain why you need the 322 value, please
   let me know....
*/

#define NOT_ENOUGH_MEMORY -1000

#define PF1_BACKGD 7L
#define PF2_BACKGD 1L
#define PF2_FRAME  2L


/* construct a simple display */

struct View view;
struct ViewPort viewport;

/* pointer to colormap structure, dynamically allocated */
struct ColorMap *cm;

struct RasInfo rasinfo, rasinfo2;
struct BitMap bitmap, bitmap2;
struct RastPort rastport, rastport2;

struct GfxBase *GfxBase;

/* save the pointer to the old view so we can restore it later */
struct View *oldview;

USHORT colortable[]={
  /*  colors 0-7, for playfield 1
  clear, gold,  cyan,  cherry, brown, green, gray,  blue   */
  0x000, 0xfb0, 0xccc, 0xf15,  0xa50, 0x0c5, 0x777, 0x77f,
  /* colors 8-11, for playfield 2
     clear, blue,  black, rose   */
     0x000, 0x55f, 0x000, 0xc57,
  0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0
};
/* Remember that, even though you paint into playfield 2 with brush
   numbers 0-7 (actually, 0-3 in this 2-bit playfield), the computer
   maps them to colors 8-15....
*/

UWORD *colorpalette;

/* * * * * * * * * * * * * * * * * * * * * * * * * */
void draw_playfields(void);		/* declarations of functions defined later */
void draw_box(LONG, LONG, LONG, LONG);
void FreeMemory(void);

void _main()

{
  LONG i, n;
  SHORT nmax;
  SHORT pf1_xmove, pf1_ymove;

  /* 'nmax' determines how long the display runs; see below */

    nmax = 10;   /* default value */


  /* Open the graphics library.... */
  GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
  if (GfxBase == NULL) _exit(100);

  /* Save the current view to restore it later. */
  oldview = GfxBase->ActiView;

  InitView(&view);       /* initialize view */
  InitVPort(&viewport);  /* init viewport */
  view.ViewPort = &viewport;     /* link view into viewport */
  view.Modes = DUALPF + PFBA;   /* sets dual playfield w/ pf2 on top */
  viewport.Modes = DUALPF + PFBA;   /* sets dual playfield w/ pf2 on top */

  /* Init bit map (for rasinfo and rastport). */
  InitBitMap(&bitmap, DEPTH, WIDTH, HEIGHT);
  InitBitMap(&bitmap2, DEPTH2, WIDTH2, HEIGHT2);

  /* Establish rast.ports for playfield 1, then playfield 2. */
  InitRastPort(&rastport);
  rastport.BitMap = &bitmap;

  InitRastPort(&rastport2);
  rastport2.BitMap = &bitmap2;

  /* Init RasInfos for playfield 1, then playfield 2. */

  rasinfo.BitMap = &bitmap;
  rasinfo.RxOffset = 0;
  rasinfo.RyOffset = 0;
  rasinfo.Next = &rasinfo2;   /* link this RasInfo to the next one */

  rasinfo2.BitMap = &bitmap2;
  rasinfo2.RxOffset = 0;
  rasinfo2.RyOffset = 0;
  rasinfo2.Next = NULL;

  /* Now specify critical characteristics.... */
  viewport.DWidth = VPWIDTH;
  viewport.DHeight = VPHEIGHT;
  viewport.RasInfo = &rasinfo;

  /* Initialize the color map, which has 32 entries (sprites take up
     the top 16).   */

  cm = GetColorMap(32);

  /* If no memory for color map, reclaim memory and exit. */
  if (cm == NULL) {
    FreeMemory();
    _exit(100);
  }

  /* 'colorpalette' points to the color table for this color map. */
  colorpalette = (UWORD *)cm->ColorTable;
  /* We initialize this color table with the values from our array. */
  for(i=0; i<32; i++) {
    *colorpalette++ = colortable[i];
  }

  /* Copy the color table into the viewport structure. */
  viewport.ColorMap = cm;

  /* Allocate space for bitmap 1, then bitmap 2. */

  for(i=0; i<DEPTH; i++) {
    bitmap.Planes[i] = (PLANEPTR) AllocRaster(WIDTH, HEIGHT);
        if(bitmap.Planes[i] == NULL) {
		FreeMemory();
		_exit(NOT_ENOUGH_MEMORY);
	}
        BltClear(bitmap.Planes[i], RASSIZE(WIDTH, HEIGHT), 1);
  }
  for(i=0; i<DEPTH2; i++) {
    bitmap2.Planes[i] = (PLANEPTR) AllocRaster(WIDTH2, HEIGHT2);
        if(bitmap2.Planes[i] == NULL) {
		FreeMemory();
		_exit(NOT_ENOUGH_MEMORY);
	}
        BltClear(bitmap2.Planes[i], RASSIZE(WIDTH2, HEIGHT2), 1);
  }

  draw_playfields();   /* Draw desired graphics into pf1 and pf2. */

  /* These three instructions assemble and "load" the view we've defined.*/
  MakeVPort(&view, &viewport);
  MrgCop(&view);
  LoadView(&view);

  pf1_xmove = 1; pf1_ymove = 1;

  /* This is the main loop of the program; we scroll playfield 1 (the bottom
     one) by changing its 'RxOffset' and 'RyOffset' values; we change the
         direction of the scrolling (both horizontally and vertically) when
         each brings us near the edge of the playfield 1 bitmap.
           'n' starts at 1 and increments each time there is a scrolling 
     change.  This program ends when 'n' reaches the value 'nmax', which
         either defaults to 10 or is specified by the user.
   */

  for( n=1; n<=nmax;  )   {

          rasinfo.RxOffset = rasinfo.RxOffset + pf1_xmove;
          rasinfo.RyOffset = rasinfo.RyOffset + pf1_ymove;

      if (rasinfo.RxOffset <= 0  ||  rasinfo.RxOffset >= 319)  {
            pf1_xmove = -pf1_xmove;
                n = n + 1;     /* sorry, no C jargon here--code should be readable */
      }
      if (rasinfo.RyOffset <= 0  ||  rasinfo.RyOffset >= 199)  {
            pf1_ymove = -pf1_ymove;
                n = n + 1;
      }

          /* Reexecute this trio of instructions to make the changes visible. */
          MakeVPort(&view, &viewport);
          MrgCop(&view);
          LoadView(&view);

          /* Slow it down to one move per video frame: this routine waits
             until the electron gun in the video display is about to start
                 drawing a new frame.   */

          WaitTOF();
  }   /* end of 'for' loop */

  /* Restore the system to its original state. */
  LoadView(oldview);

  /* The two statements that follow return the memory allocated by
     this program to the system for general use.  If you do not do
         this, you lose access to a big chunk of memory each time you
         run this program and, eventually, the Amiga refuses to work
         properly for lack of available memory.   */

  FreeMemory();
  CloseLibrary((struct Library *)GfxBase);

}     /* end of main() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * */

void draw_playfields()
/* This routine draws playfields 1 and 2 for later display. */
{
  LONG x, y, temp, color1, color2;

  /* Draw "window" of playfield 2 (the stationary one on top). */

  SetRast( &rastport2, PF2_BACKGD );   /* flood raster with backgd color */

  SetAPen( &rastport2, PF2_FRAME );  /* this is the color of the "frame" */
  RectFill( &rastport2, 95L, 45L, 225L, 155L );

  SetAPen( &rastport2, 0);    /* this is the "transparent" window (color
                                 0) that allows us to see through to
                                 the lower playfield (pf1)         */
  RectFill( &rastport2, 100L, 50L, 220L, 150L );

  /*  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
  /* Draw the boxes in playfield 1 (the scrolling one on bottom). */

  SetRast( &rastport, PF1_BACKGD );

  temp = 1L;
  for( x=25L; x<=590L; x=x+50L )   {

    for ( y=0L; y<=350L; y=y+50L)   {

          color1 = temp;     /* this code simply generates an ever-changing
                                set of color pairs for draw_box */
          temp = temp + 1L;
          if (temp > 7L) temp = 1L;
          color2 = temp;
          temp = temp + 1L;
          if (temp > 7L) temp = 1L;

          /* Draw a box given its position and two colors. */
          draw_box( x, y, color1, color2 );
        }
  }   /* end outer 'for' loop */

}   /* end draw_playfields */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * */

void draw_box( x, y, bigsq_color, smallsq_color )
/*
   This function draws a boxlike design onto pf1 at row x, column y;
   bigsq_color and smallsq_color are the color numbers used to draw
   the double box.   */

LONG x, y, bigsq_color, smallsq_color;

{
   SetAPen( &rastport, 0 );   /* draw black shadow */
   RectFill( &rastport, x+4L, y+4L, x+40L, y+40L );

   SetAPen( &rastport, 0 );   /* draw large black box */
   RectFill( &rastport, x, y, x+36L, y+36L );

   SetAPen( &rastport, bigsq_color );   /* draw large colored box */
   RectFill( &rastport, x+2L, y+2L, x+34L, y+34L );

   SetAPen( &rastport, smallsq_color );   /* draw small colored box */
   RectFill( &rastport, x+12L, y+12L, x+24L, y+24L );

}   /* end draw_box */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * */

void FreeMemory()
/* Return user and system-allocated memory to the system. */
{
  LONG i;

  /* Free both bitmaps. */
  for (i=0; i<DEPTH; i++) {
        if (bitmap.Planes[i] != NULL) {
      FreeRaster(bitmap.Planes[i], WIDTH, HEIGHT);
        }
  }
  for (i=0; i<DEPTH2; i++) {
        if (bitmap2.Planes[i] != NULL) {
      FreeRaster(bitmap2.Planes[i], WIDTH2, HEIGHT2);
        }
  }

  /* free the color map created by GetColorMap() */
  if (cm != NULL)  FreeColorMap(cm);

  /* Free some other dynamically created structures. */
  FreeVPortCopLists(&viewport);
  FreeCprList(view.LOFCprList);

}   /* end FreeMemory */

/*---end---of---boxomatic.c---*/

/*  (pant, pant, pant) ...and this was a *simple* program... (sigh) */
