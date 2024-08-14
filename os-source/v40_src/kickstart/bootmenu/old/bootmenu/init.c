#include <exec/types.h>

/*
struct XTextAttr {
   UBYTE CUCA;
   };
*/

#include <exec/memory.h>
#include <libraries/expansionbase.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/sprite.h>
#include <graphics/gfxmacros.h>
#include <libraries/filehandler.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <devices/input.h>
/* #include <proto/all.h> */
#include "all.h"

void kprintf(char *,...);
void sprintf(char *,char *,...);
#define  D(x) ;
#define printf kprintf

USHORT GhostPattern[] = {
      0x8888,
      0x2222
   };

#define GLYPH 1

#if GLYPH
/*
extern LONG __asm BltBitMapRastPort(register __a0 struct BitMap *srcbm,
                                    register __d0 SHORT srcx,
                                    register __d1 SHORT srcy,
                                    register __a1 struct RastPort *destrp,
                                    register __d2 SHORT destx,
                                    register __d3 SHORT desty,
                                    register __d4 SHORT sizex,
                                    register __d5 SHORT sizey,
                                    register __d6 UBYTE minterm);
*/
extern LONG __asm MyBltBitMapRastPort(
                                    register __a6 struct GfxBase *GfxBase,
                                    register __a0 struct BitMap *srcbm,
                                    register __d0 SHORT srcx,
                                    register __d1 SHORT srcy,
                                    register __a1 struct RastPort *destrp,
                                    register __d2 SHORT destx,
                                    register __d3 SHORT desty,
                                    register __d4 SHORT sizex,
                                    register __d5 SHORT sizey,
                                    register __d6 UBYTE minterm);

#define VANILLA_COPY  0xC0
#define NWAYBMWIDTH   11
#define NWAYBMHEIGHT   9

UWORD NWayGlyphData[] =
    {
    0x7F00,   /* 0111 1111 0000 0000 */
    0xC180,   /* 1100 0001 1000 0000 */
    0xC180,   /* 1100 0001 1000 0000 */
    0xC7E0,   /* 1100 0111 1110 0000 */
    0xC3C0,   /* 1100 0011 1100 0000 */
    0xC180,   /* 1100 0001 1000 0000 */
    0xC000,   /* 1100 0000 0000 0000 */
    0xC180,   /* 1100 0001 1000 0000 */
    0x7F00,   /* 0111 1111 0000 0000 */
    };
#endif


#define DEPTH     2
#define WIDTH     640
#define HEIGHT    200
#define VPMODES   HIRES|SPRITES

#define ALLOC_SPRITE 1

/*#define G0W       76  */                 /* gadget 0 width */
#define G0W       102                   /* gadget 0 width */
#define G1W       254                  /* gadget 1 width */
#define G2W       44                   /* gadget 2 width */
#define G3W       44                   /* gadget 3 width */

#define T0W       64                   /* gadget 0 width */
#define T1W       240                  /* gadget 1 width */
#define T2W       32                   /* gadget 2 width */
#define T3W       32                   /* gadget 3 width */

#define GYS       30                   /* gadget y start   */
#define GYSP      15                   /* gadget y spacing */
#define GXSP      10                   /* gadget x spacing */
#define GH        13                   /* gadget height    */
#define TH        8                    /* text height      */
#define TYS       (GYS+((GH-TH)>>1)+TH-1)/* text   y start   */

#define C0GX      ((WIDTH-1-(G0W+GXSP+G1W+GXSP+G2W+GXSP+G3W))>>1)/* col 0 gadget x pos */
#define C0TX      (C0GX+((G0W-T0W)>>1))/* col 0 text   x pos */

#define C1GX      (C0GX+G0W+GXSP)      /* col 1 gadget x pos */
#define C1TX      (C1GX+((G1W-T1W)>>1))/* col 1 text   x pos */

#define C2GX      (C1GX+G1W+GXSP)      /* col 2 gadget x pos */
#define C2TX      (C2GX+((G2W-T2W)>>1))/* col 2 text   x pos */
#define C3GX      (C2GX+G2W+GXSP)      /* col 3 gadget x pos */
#define C3TX      (C3GX+((G3W-T3W)>>1))/* col 3 text   x pos */

#define LY        27                   /* Label y pos */

#define STARTX    0
#define STARTY    185+8+((13-8)>>1)-1

#define DATW      64                   /* disable all button text witdh    */
#define DATH      8                    /* disable all button text height   */
#define DAGW      (DATW+14+24)            /* disable all button gadget width  */
#define DAGH      13                   /* disable all button gadget height */
#define DAGX      STARTX+136+10 /*515*/                  /* disable all button gadget x      */
#define DAGY      185                  /* disable all button gadget y      */
#define DATX      (DAGX+((DAGW-DATW)>>1))/* disable all button gadget x      */
#define DATY      (DAGY+DATH+((DAGH-DATH)>>1)-1)/* disable all button gadget y     */
#define DAGN      (101)                /* disable all button gadget number */

#define GOTW      24                   /* disable all button text witdh    */
#define GOTH      8                    /* disable all button text height   */
#define GOGW      82 /*CBGW*/ /*(GOTW+10)*/            /* disable all button gadget width  */
#define GOGH      13                   /* disable all button gadget height */
#define GOGX      DAGX+DAGW+10 /*285*/                  /* disable all button gadget x      */
#define GOGY      185                  /* disable all button gadget y      */
#define GOTX      (GOGX+((GOGW-GOTW)>>1))/* disable all button gadget x      */
#define GOTY      (GOGY+GOTH+((GOGH-GOTH)>>1)-1)/* disable all button gadget y     */
#define GOGN      (102)                /* disable all button gadget number */

#define CBTW      48                   /* cancel button text witdh    */
#define CBTH      8                    /* cancel button text height   */
#define CBGW      82 /*(CBTW+10)*/            /* cancel button gadget width  */
#define CBGH      13                   /* cancel button gadget height */
#define CBGX      GOGX+GOGW+10 /*(((WIDTH-1-CBGW)>>1)+74)*/  /* cancel button gadget x      */
#define CBGY      185                  /* cancel button gadget y      */
#define CBTX      (CBGX+((CBGW-CBTW)>>1))/* cancel button gadget x      */
#define CBTY      (CBGY+CBTH+((CBGH-CBTH)>>1)-1)/* cancel button gadget y     */
#define CBGN      (100)                /* cancel button gadget number */

#define NPTW      72                   /* disable all button text witdh    */
#define NPTH      8                    /* disable all button text height   */
#define NPGW      (NPTW+10)            /* disable all button gadget width  */
#define NPGH      13                   /* disable all button gadget height */
#define NPGX      CBGX+CBGW+10 /*448*/ /*125   */               /* disable all button gadget x      */
#define NPGY      185                  /* disable all button gadget y      */
#define NPTX      (NPGX+((NPGW-NPTW)>>1))/* disable all button gadget x      */
#define NPTY      (NPGY+NPTH+((NPGH-NPTH)>>1)-1)/* disable all button gadget y     */
#define NPGN      (103)                /* disable all button gadget number */

#define UP_DELAY  15    /* 60ths of a second after a gadget is released */
                        /* and before the screen closes                 */

#define PAGE_SIZE 10    /* Number of nodes per page                     */
#define PAGEX     268
#define PAGEY     19 /* NPTY */

extern struct Custom custom;

struct Display {
   struct View v;
   struct ViewPort vp;
   struct ColorMap *cm;
   struct RasInfo ri;
   struct BitMap b;
   struct RastPort rp;
   BOOL   Success;
   struct TextFont *font8;
   struct TextFont *font9;
   };

struct IH {
   struct MsgPort    *port;
   struct IOStdReq   *msg;
   struct InputEvent dummy;
   struct InputEvent copy;
   struct Interrupt  hstuff;
   };

struct Sprite {
   SHORT num;
   struct SimpleSprite sprite;
   APTR   image;
   SHORT  x,y;
   };

struct MyNode {
   struct Node Node;
   BOOL  Highlited;
   BOOL  Enabled;
   BOOL  NoEnable;
   BOOL  Bootable;
   struct BootNode *node;
   };

struct World {
   struct GfxBase       *GfxBase;
   struct ExpansionBase *ExpansionBase;
   struct Display       display;
   struct IH            ih;
   struct Sprite        sprite;
   struct Task          *maintask;
   LONG                 signal;
   SHORT                gad;     /* gad currently depressed or 0 */
   SHORT                CurrentPage; /* 1 - MaxPage */
   SHORT                MaxPage;     /* Max Page number */
   struct List          NodeList;
   SHORT                Nodes;      /* Number of EB_Mounlist nodes */
   SHORT                MaxGad;     /* current Max Gadgets on screen */
   SHORT                FirstGad;   /* node number of first displayed node*/
   SHORT                SS_Disable;
   BOOL                 Cancel;
   SHORT                BootNodes; /* total number of bootable nodes */
   };

#if ALLOC_SPRITE

USHORT IPointer[] = 
    {
    0x0000, 0x0000,   /* one word each for position and control */

    0x0000, 0xFC00,
    0x7C00, 0xFE00,
    0x7C00, 0x8600,
    0x7800, 0x8C00,
    0x7C00, 0x8600,
    0x6E00, 0x9300,
    0x0700, 0x6980,
    0x0380, 0x04C0,
    0x01C0, 0x0260,
    0x0080, 0x0140,
    0x0000, 0x0080,

    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,
    0x0000, 0x0000,
    };
#endif

int    MyInit(void);
BOOL   Activated(void);
void   printbstr(BSTR);
void   DoImagery(struct World *);
BOOL   DisplayOn(struct World *);
void   DisplayOff(struct World *);
BOOL   OpenIH(struct World *);
void   CloseIH(struct World *);
struct InputEvent * __asm HandlerInterface(
                             register __a0 struct InputEvent *ev,
                             register __a1 struct World *world);
void   UpdateMouse(struct World *,struct InputEvent *);
void   GadgetDownAction(struct World *);
void   GadgetUpAction(struct World *);
void   OpenSprite(struct World *);
void   CloseSprite(struct World *);
void   DrawBox(struct World *,SHORT,SHORT,SHORT,SHORT,SHORT);
void   DrawBevelBox(struct World *,SHORT,SHORT,SHORT,SHORT,SHORT);
void   GetData(struct World *);
struct MyNode *GetNode(struct World *world,SHORT num);
void   FreeData(struct World *);
void   DrawBSTR(struct World *,BSTR,SHORT,SHORT);
void   DrawByte(struct World *,SHORT,SHORT,UBYTE);
void   DrawDOSType(struct World *,struct DosEnvec *,SHORT,SHORT);
void   DrawBOOTPri(struct World *,BYTE,SHORT,SHORT);
SHORT  GadgetHit(struct World *);
void   ComplementGadget(struct World *,SHORT);
void   MakeChanges(struct World *world);
BOOL   DrawEntry(struct World *world,SHORT x);
void   DisSprite(struct World *world,int num);
void   DrawGlyph(struct World *world,SHORT x,SHORT y);
VOID   DrawNWay(struct World *world,SHORT x,SHORT y,BOOL enabled,BOOL sunken);

int MyInit()
{
   struct Library *library;
   struct World   *world;
   struct World   myworld;

   D( kprintf("###################\n") );
   D( kprintf("bootmenu Init Begins\n") );

   world=&myworld;

   if( ! Activated() ) return(0);

   /* our presence is requested so open the view */
   library = OpenLibrary("graphics.library",0);
   if(library==0)
   {
      D( kprintf("No graphics.library\n") );
      goto out;
   }
   world->GfxBase=(struct GfxBase *)library;

   library = OpenLibrary("expansion.library",0);
   if(library==0)
   {
      D( kprintf("No expansion.library\n") );
      goto out;
   }
   world->ExpansionBase=(struct ExpansionBase *)library;

   world->signal=AllocSignal(-1);
   world->maintask=FindTask(0);

   DisplayOn(world);
   D( kprintf("After DisplayOn()\n") );

   GetData(world);
   world->Cancel=FALSE;

   DoImagery(world);
   D( kprintf("After DoImagery()\n") );

   OpenSprite(world);
   D( kprintf("After OpenSprite()\n") );

   OpenIH(world);
   D( kprintf("After OpenIH()\n") );

   Wait(1<<world->signal);

   if( ! world->Cancel)
      MakeChanges(world);

   FreeData(world);

   CloseIH(world);
   D( kprintf("After CloseIH()\n") );

   CloseSprite(world);
   D( kprintf("After CloseSprite()\n") );

   DisplayOff(world);

out:
   D( kprintf("Bootmenu exiting\n") );
   FreeSignal(world->signal);
   if(world->ExpansionBase) CloseLibrary((struct Library *)world->ExpansionBase);
   if(world->GfxBase) CloseLibrary((struct Library *)world->GfxBase);
   return(0);
}

#define GfxBase       (world->GfxBase)
#define ExpansionBase (world->ExpansionBase)

BOOL Activated()
{
   /* Is our magic mouse/key sequence being pressed ? */
   /* if not then just exit */
   if( (1<<6) & *((char *)0xBFE001) )
   {
      D( kprintf("No Left button. Exiting\n") );
      return(FALSE);
   }
   if( (1<<10) & *((UWORD *)0xDFF016) )
   {
      D( kprintf("No Right button. Exiting\n") );
      return(FALSE);
   }
   return(TRUE);
}
void printbstr(bp)
BSTR bp;
{
   int x;
   UBYTE count;
   UBYTE *p;

   p=(UBYTE *)(bp << 2);
   count=*p;
   /* D( kprintf("count = %ld ",count) ); */
   for(x=1;x<=count;x++)
      printf("%lc",(ULONG)p[x]);
}
void DoImagery(world)
struct World *world;
{
   SHORT x,tl;
   char buf[20];

   D( printf("init.c: DoImagery(world=0x%lx) enter\n",world); )

/*   SetRast(&world->display.rp,0); */
   SetAPen(&world->display.rp,3);
   SetBPen(&world->display.rp,0);
   SetDrMd(&world->display.rp,JAM2);
   SetFont(&world->display.rp,world->display.font9);

   /*********/
   /* Title */
   /*********/
   tl=TextLength(&world->display.rp,"Boot Menu",9);
   Move(&world->display.rp,(WIDTH-1-tl)>>1,8);
   Text(&world->display.rp,"Boot Menu",9);

   SetFont(&world->display.rp,world->display.font8);

   /**********/
   /* Labels */
   /**********/
   Move(&world->display.rp,C0GX,LY);
   Text(&world->display.rp,"Status",6);

   Move(&world->display.rp,C1GX,LY);
   Text(&world->display.rp,"Device Name",11);

   Move(&world->display.rp,C2GX,LY);
   Text(&world->display.rp,"Type",4);

   Move(&world->display.rp,C3GX,LY);
   Text(&world->display.rp,"Pri",3);

   /****************/
   /* Page Display */
   /****************/
   D( printf("init.c: DoImagery() CurrentPage=%ld MaxPage=%ld\n",world->CurrentPage,world->MaxPage); )

   Move(&world->display.rp,PAGEX,PAGEY);
   sprintf(buf,"Page %2ld of %2ld",world->CurrentPage,world->MaxPage);
   Text(&world->display.rp,buf,13);

   /*******************/
   /* Command Buttons */
   /*******************/
   SetAPen(&world->display.rp,2);
   Move(&world->display.rp,CBTX,CBTY);
   Text(&world->display.rp,"CANCEL",6);
   DrawBox(world,CBGX,CBGY,CBGW,CBGH,1);

   Move(&world->display.rp,GOTX,GOTY);
   Text(&world->display.rp,"USE",3);
   DrawBox(world,GOGX,GOGY,GOGW,GOGH,1);

   Move(&world->display.rp,NPTX,NPTY);
   Text(&world->display.rp,"NEXT PAGE",9);
   DrawBox(world,NPGX,NPGY,NPGW,NPGH,1);

   SetAPen(&world->display.rp,2);
   SetAfPt(&world->display.rp,GhostPattern,1);
   SetDrMd(&world->display.rp,JAM1);

   if(world->MaxPage==1)
      RectFill(&world->display.rp,NPGX+2,NPGY+1,NPGX+NPGW-4,NPGY+NPGH-2);

   SetAfPt(&world->display.rp,NULL,0);

   Move(&world->display.rp,STARTX,STARTY);
   Text(&world->display.rp,"Startup-Sequence:",17);
   DrawNWay(world,DAGX,DAGY,(BOOL)(! world->SS_Disable),FALSE);

   /****************/
   /* Main Gadgets */
   /****************/
   world->MaxGad=0;
   for(x=0;x<PAGE_SIZE;x++)
   {
      if(DrawEntry(world,x))
         world->MaxGad++;
   }

/*
   SetAPen(&world->display.rp,2);
   Move(&world->display.rp,C1TX,TYS);
   Text(&world->display.rp,"0123456789012345678901234567890123",30);
   Move(&world->display.rp,C2TX,TYS);
   Text(&world->display.rp,"0123456789012345678901234567890123",4);
   Move(&world->display.rp,C3TX,TYS);
   Text(&world->display.rp,"0123456789012345678901234567890123",4);
*/
   D( printf("init.c: DoImagery() returns\n"); )
}
BOOL DisplayOn(world)
struct World *world;
{
   USHORT colortable[]={0x777,0xfff,0x000,0xec3,
                        0,0,0,0,
                        0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,};
   UBYTE *displaymem;
   UWORD *colorpalette;
   LONG i;

   struct TextAttr ta8 ={
         "topaz.font",
         8,
         0,
         0 };
   struct TextAttr ta9 ={
         "topaz.font",
         9,
         FSF_UNDERLINED,
         0 };

   world->display.font8=OpenFont(&ta8);
   world->display.font9=OpenFont(&ta9);

   if(world->display.font8==0 || world->display.font9==0)
      D( kprintf("No font!!!\n") );

   InitView(&world->display.v);
   InitVPort(&world->display.vp);
   world->display.vp.Modes=VPMODES;
   world->display.v.ViewPort=&world->display.vp;

   InitBitMap(&world->display.b,DEPTH,WIDTH,HEIGHT);

   InitRastPort(&world->display.rp);
   world->display.rp.BitMap=&world->display.b;

   world->display.ri.BitMap=&world->display.b;
   world->display.ri.RxOffset=0;
   world->display.ri.RyOffset=0;
   world->display.ri.Next=NULL;

   world->display.vp.DWidth=WIDTH;
   world->display.vp.DHeight=HEIGHT;
   world->display.vp.RasInfo=&world->display.ri;

   if((world->display.cm=GetColorMap(32))==0)
   {
      D( kprintf("No Color Map\n") );
      world->display.Success=FALSE;
      return(FALSE);
   }
   colorpalette=(UWORD *)world->display.cm->ColorTable;
   for(i=0;i<32;i++)
      *colorpalette++=colortable[i];

   world->display.vp.ColorMap=world->display.cm;

   for(i=0;i<DEPTH;i++)
   {
      world->display.b.Planes[i]=(PLANEPTR)AllocRaster(WIDTH,HEIGHT);
      if(world->display.b.Planes[i]==NULL)
      {
         D( kprintf("No BitPlaneMem\n") );
         world->display.Success=FALSE;
         return(FALSE);
      }
   }

   MakeVPort(&world->display.v,&world->display.vp);
   MrgCop(&world->display.v);

   for(i=0;i<2;i++)
   {
      displaymem=(UBYTE *)world->display.b.Planes[i];
      BltClear(displaymem,RASSIZE(WIDTH,HEIGHT),0);
   }

   ON_DISPLAY

   LoadView(&world->display.v);
   world->display.Success=TRUE;
   return(TRUE);
}
void DisplayOff(world)
struct World *world;
{
   LONG i;

   if(world->display.Success)
   {
      WaitTOF();
      /* OFF_DISPLAY */
      LoadView(0);
      OFF_DISPLAY
      CloseFont(world->display.font8);
      CloseFont(world->display.font9);
      for(i=0;i<DEPTH;i++)
         FreeRaster(world->display.b.Planes[i],WIDTH,HEIGHT);
      FreeColorMap(world->display.cm);
      FreeVPortCopLists(&world->display.vp);
      FreeCprList(world->display.v.LOFCprList);
   }
}
BOOL OpenIH(world)
struct World *world;
{
   world->ih.dummy.ie_Class=IECLASS_NULL;
   world->ih.dummy.ie_NextEvent=NULL;

   if((world->ih.port=CreatePort(0,0))==NULL)
   {
      D( kprintf(" Could not create port\n") );
      return(FALSE);
   }
   if((world->ih.msg=CreateStdIO(world->ih.port))==NULL)
   {
      D( kprintf(" Could not create StdIO\n") );
      DeletePort(world->ih.port);
      return(FALSE);
   }
   world->ih.hstuff.is_Data=(APTR)world;
   world->ih.hstuff.is_Code=(VOID (*))HandlerInterface;
   world->ih.hstuff.is_Node.ln_Pri=51;

   if(OpenDevice("input.device",0,(struct IORequest *)world->ih.msg,0))
   {
      D( kprintf(" Could not create StdIO\n") );
      DeleteStdIO(world->ih.msg);
      DeletePort(world->ih.port);
      return(FALSE);
   }
   world->ih.msg->io_Command=IND_ADDHANDLER;
   world->ih.msg->io_Data=(APTR)&world->ih.hstuff;
   DoIO((struct IORequest *)world->ih.msg);

   world->ih.copy.ie_TimeStamp.tv_secs=0;
   world->ih.copy.ie_TimeStamp.tv_micro=0;
   world->ih.copy.ie_Class=0;
}
void CloseIH(world)
struct World *world;
{
   world->ih.msg->io_Command=IND_REMHANDLER;
   world->ih.msg->io_Data=(APTR)&world->ih.hstuff;
   DoIO((struct IORequest *)world->ih.msg);
   CloseDevice((struct IORequest *)world->ih.msg);
   DeleteStdIO(world->ih.msg);
   DeletePort(world->ih.port);
}
struct InputEvent * __asm HandlerInterface(
                        register __a0 struct InputEvent *ev,
                        register __a1 struct World *world)
{
   if(ev->ie_Class==IECLASS_RAWMOUSE)
   {
      UpdateMouse(world,ev);
      switch(ev->ie_Code)
      {
         case IECODE_RBUTTON:
               /*D( kprintf("Right Button\n") );*/
               break;
         case IECODE_LBUTTON:
               /*D( kprintf("Left Button\n") );*/
               GadgetDownAction(world);
               break;
         case IECODE_LBUTTON|IECODE_UP_PREFIX:
               D( kprintf("Left Button UP\n") );
               GadgetUpAction(world);
               break;
         case IECODE_NOBUTTON:
               /* D( kprintf("No Button x = %ld, y = %ld\n",ev->ie_X,ev->ie_Y) ); */
               break;
         default:
               /*D( kprintf("Default\n") );*/
               break;
      }
   }
   return(ev);
}
void UpdateMouse(world,ev)
struct World *world;
struct InputEvent *ev;
{
   world->sprite.x += (ev->ie_X << 1);
   if(world->sprite.x < 0) world->sprite.x = 0;
   if(world->sprite.x > (WIDTH-1)) world->sprite.x = (WIDTH-1);

   world->sprite.y += ev->ie_Y;
   if(world->sprite.y < 0) world->sprite.y = 0;
   if(world->sprite.y > (HEIGHT-1)) world->sprite.y = (HEIGHT-1);

   /*D( kprintf("No Button x = %ld, y = %ld\n",world->sprite.x,world->sprite.y) );*/

#if ALLOC_SPRITE
   MoveSprite(0,&world->sprite.sprite,world->sprite.x>>1,world->sprite.y);
#else
   {
      struct SimpleSprite *ss;
      ss=GfxBase->SimpleSprites[0];
      MoveSprite(0,ss,world->sprite.x>>1,world->sprite.y);
   }
#endif

   /* if world->gad and GadgetHit != world->gad comp world->gad =0 */
   /* if something was selected and mouse is no longer over same gad */
   if(world->gad && (GadgetHit(world)!=world->gad))
   {
      ComplementGadget(world,world->gad);
      world->gad=0;
   }
}
void GadgetDownAction(world)
struct World *world;
{
   world->gad=GadgetHit(world);
   if(world->gad)
   {
      ComplementGadget(world,world->gad);
   }
}
void GadgetUpAction(world)
struct World *world;
{
   SHORT gad;
   SHORT i;
   struct MyNode *mn;

   if( ! world->gad) return;     /* nothing was down */
   gad=GadgetHit(world);
   if(world->gad!=gad)           /* down on one up on another or none */
   {                             /* Un highlite world->gad            */
      ComplementGadget(world,world->gad);
      world->gad=0;
      return;
   }
   switch(gad)                   /* down on one and up on same, HIT! */
   {
      case 0:
            break;
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
            ComplementGadget(world,world->gad);
            world->gad=0;

            for(mn=(struct MyNode *)world->NodeList.lh_Head;
                mn->Node.ln_Succ;
                mn=(struct MyNode *)mn->Node.ln_Succ)
            {
               if(mn->Highlited)
               {
                  mn->Highlited=FALSE;
                  break;
               }
            }
            mn=GetNode(world,(SHORT)(world->FirstGad+gad-1));
            mn->Highlited=TRUE;
            DoImagery(world);
            break;
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
            mn=GetNode(world,(SHORT)(world->FirstGad+gad-10-1));
            if((! mn->Enabled) && mn->Highlited)
            {
               mn->Highlited=FALSE;
               for(i=0;i<world->Nodes;i++)
               {
                  mn=GetNode(world,i);
                  if(((mn->Enabled && mn->Bootable)||(mn->NoEnable && mn->Bootable)))
                  {
                     mn->Highlited=TRUE;
                     if((i<(world->FirstGad+world->MaxGad))&&(i>=world->FirstGad))
                     {
                        D( printf("Entry i = %ld\n",i); )
                        DrawEntry(world,(SHORT)(i-world->FirstGad));
                     }
                     break;
                  }
               }
            }
            DrawEntry(world,(SHORT)(gad-10-1));
            world->gad=0;
            break;
      case CBGN:
            for(i=0;i<UP_DELAY;i++)
               WaitTOF();
            world->Cancel=TRUE;
            Signal(world->maintask,1<<world->signal);
            break;
      case DAGN:
            ComplementGadget(world,world->gad);
            (world->SS_Disable) ? (world->SS_Disable=FALSE) : (world->SS_Disable=TRUE);
            SetDrMd(&world->display.rp,JAM2);
            SetAPen(&world->display.rp,2);
            SetBPen(&world->display.rp,0);
            if(world->SS_Disable)
            {
               Move(&world->display.rp,DATX+9,DATY);
               Text(&world->display.rp,"DISABLED",8);
            } else {
               Move(&world->display.rp,DATX+9,DATY);
               Text(&world->display.rp,"        ",8);
               Move(&world->display.rp,DATX+9+4,DATY);
               Text(&world->display.rp,"ENABLED",7);
            }
            world->gad=0;
            break;
      case GOGN:
            for(i=0;i<UP_DELAY;i++)
               WaitTOF();
            Signal(world->maintask,1<<world->signal);
            break;
      case NPGN:
            ComplementGadget(world,world->gad);
            world->gad=0;
            world->CurrentPage +=1;
            if(world->CurrentPage > world->MaxPage)
               world->CurrentPage = 1;
            DoImagery(world);
            break;
      default:
            break;
   }
   return;
}
void OpenSprite(world)
struct World *world;
{
#if ALLOC_SPRITE
   SHORT k;
   UWORD *p;
#endif

   ON_SPRITE

#if ALLOC_SPRITE
   world->sprite.num=GetSprite(&world->sprite.sprite,3);
   D( kprintf("Sprite num = %ld\n",world->sprite.num) );
   world->sprite.sprite.x=0;
   world->sprite.sprite.y=0;
   world->sprite.sprite.height=16;

   k=((world->sprite.num&0x06)*2)+16;
   D( kprintf("k=%ld\n",(LONG)k) );
   SetRGB4(&world->display.vp,k+1,13,2,2);
   SetRGB4(&world->display.vp,k+2,0,0,0);
   SetRGB4(&world->display.vp,k+3,15,12,10);

   world->sprite.image=AllocMem(19*4,MEMF_CHIP);
   p=(UWORD *)world->sprite.image;
   for(k=0;k<19*2;k++)
      p[k]=IPointer[k];

   ChangeSprite(&world->display.vp,&world->sprite.sprite,p);
   MoveSprite(0,&world->sprite.sprite,30,30);
   DisSprite(world,0);
#else
   D( kprintf("GfxBase->SimpleSprites[0]=%lx\n",GfxBase->SimpleSprites[0]); )
   D( kprintf("GfxBase->SimpleSprites[1]=%lx\n",GfxBase->SimpleSprites[1]); )
   D( kprintf("GfxBase->SimpleSprites[2]=%lx\n",GfxBase->SimpleSprites[2]); )
   D( kprintf("GfxBase->SimpleSprites[3]=%lx\n",GfxBase->SimpleSprites[3]); )
   D( kprintf("GfxBase->SimpleSprites[4]=%lx\n",GfxBase->SimpleSprites[4]); )
   D( kprintf("GfxBase->SimpleSprites[5]=%lx\n",GfxBase->SimpleSprites[5]); )
   D( kprintf("GfxBase->SimpleSprites[6]=%lx\n",GfxBase->SimpleSprites[6]); )
   D( kprintf("GfxBase->SimpleSprites[7]=%lx\n",GfxBase->SimpleSprites[7]); )
   D( kprintf("SS[0].x=%ld,y=%ld,num=%ld\n",GfxBase->SimpleSprites[0]->x,
                                            GfxBase->SimpleSprites[0]->y,
                                            GfxBase->SimpleSprites[0]->num); )
   SetRGB4(&world->display.vp,16+1,13,2,2);
   SetRGB4(&world->display.vp,16+2,0,0,0);
   SetRGB4(&world->display.vp,16+3,15,12,10);
   {
      struct SimpleSprite *ss;
      ss=GfxBase->SimpleSprites[0];
      MoveSprite(0,ss,30,30);
   }
#endif
   world->sprite.x=30;
   world->sprite.y=30;
}
void CloseSprite(world)
struct World *world;
{
#if ALLOC_SPRITE
   FreeSprite(world->sprite.num);
   FreeMem(world->sprite.image,19*4);
#endif
}
void DrawBox(world,l,t,w,h,m)
struct World *world;
SHORT l,t,w,h,m;
{
   SetDrMd(&world->display.rp,JAM1);

   m ? SetAPen(&world->display.rp,1) : SetAPen(&world->display.rp,2);
   Move(&world->display.rp,l,t);
   Draw(&world->display.rp,l,t+h-1);
   Draw(&world->display.rp,l+1,t+h-2);
   Draw(&world->display.rp,l+1,t);
   Draw(&world->display.rp,l+w-1,t);

   m ? SetAPen(&world->display.rp,2) : SetAPen(&world->display.rp,1);
   Move(&world->display.rp,l+1,t+h-1);
   Draw(&world->display.rp,l+w-2,t+h-1);
   Draw(&world->display.rp,l+w-2,t+1);
   Draw(&world->display.rp,l+w-1,t);
   Draw(&world->display.rp,l+w-1,t+h-1);
}
void DrawBevelBox(world,l,t,w,h,m)
struct World *world;
SHORT l,t,w,h,m;
{
   DrawBox(world,l,t,w,h,1);
   DrawBox(world,(SHORT)(l+2),(SHORT)(t+1),(SHORT)(w-4),(SHORT)(h-2),1);
}
void GetData(world)
struct World *world;
{
   struct BootNode *bn;
   struct DeviceNode *dn;
   struct FileSysStartupMsg *fssm;
   struct DosEnvec          *ev;
   struct MyNode     *mn;

   world->SS_Disable = (ExpansionBase->Flags & EBF_DOSFLAG)?TRUE:FALSE;

   world->Nodes=0;
   world->BootNodes=0;
   world->CurrentPage=1;
   NewList(&world->NodeList);

   D( kprintf("Begin MountList Search\n") );
   for(bn=(struct BootNode *)ExpansionBase->MountList.lh_Head;
       bn->bn_Node.ln_Succ;
       bn=(struct BootNode *)bn->bn_Node.ln_Succ)
   {
      D( printf("Node: ") );
      if(bn->bn_DeviceNode)
      {
         dn=(struct DeviceNode *)bn->bn_DeviceNode;
         D( fssm=(struct FileSysStartupMsg *)(((ULONG)dn->dn_Startup) << 2); )
         D( ev=(struct DosEnvec *)(((ULONG)fssm->fssm_Environ) << 2); )

         mn=(struct MyNode *)AllocMem(sizeof(struct MyNode),MEMF_CLEAR);

         world->Nodes+=1;
         mn->node=bn;
         /* mn->Enabled=TRUE;*/
         mn->Enabled=(dn->dn_Handler & 0x80000000)?FALSE:TRUE;
         if( ! mn->Enabled) mn->NoEnable=TRUE;
         mn->Bootable=(bn->bn_Node.ln_Type & NT_BOOTNODE)?TRUE:FALSE;

         if(((mn->Bootable)&&(mn->Enabled))||((mn->NoEnable)&&(mn->Bootable)) )
         {
            world->BootNodes+=1;
         }

         AddTail(&world->NodeList,(struct Node *)mn);

         D( printbstr(dn->dn_Name) );
         D( printf(" Pri=%ld",bn->bn_Node.ln_Pri) );
         D( printf(" fssm=%lx ",(ULONG)fssm) );
         D( printf(" ev=%lx ",(ULONG)ev) );
         D( printf(" DOSType=%lx",(ULONG)ev->de_DosType) );
         D( printf(" BP=%ld",ev->de_BootPri) );
         D( printf(" TS=%ld",(ULONG)ev->de_TableSize) );
         D( if(bn->bn_Node.ln_Type & NT_BOOTNODE) printf(" BOOTABLE"); )
      }
      D( printf("\n") );
   }
   mn=(struct MyNode *)world->NodeList.lh_Head;
   if(mn->Bootable && mn->Enabled)
      mn->Highlited=TRUE;

   world->MaxPage=(world->Nodes+PAGE_SIZE-1)/PAGE_SIZE;
   D( kprintf("End MountList Search\n") );
}
struct MyNode *GetNode(world,num)
struct World *world;
SHORT num;
{
   SHORT x;
   struct Node *n;

   D( printf("init.c: GetNode(world=0x%lx,num=%ld) enter\n",world,num); )

   if(num >= world->Nodes)
   {
      n=0L;
      D( printf("init.c: GetNode() returns MyNode=0x%lx\n",n); )
      return((struct MyNode *)n);
   }

   n=world->NodeList.lh_Head;
   for(x=0;x<num;x++)
      n=n->ln_Succ;

   D( printf("init.c: GetNode() returns MyNode=0x%lx\n",n); )
   return((struct MyNode *)n);
}
void FreeData(world)
struct World *world;
{
   struct Node *n;

   while(n=RemHead(&world->NodeList))
      FreeMem((char *)n,sizeof(struct MyNode));
}
void DrawBSTR(world,name,x,y)
struct World *world;
BPTR name;
SHORT x,y;
{
   UBYTE count;
   UBYTE *p;

   p=(UBYTE *)(name << 2);
   count=*p;
   /* D( kprintf("count = %ld ",count) ); */
   SetAPen(&world->display.rp,2);
   SetBPen(&world->display.rp,0);
   SetDrMd(&world->display.rp,JAM2);
   Move(&world->display.rp,x,y);
   Text(&world->display.rp,&p[1],count);
}
void DrawDOSType(world,ev,x,y)
struct World *world;
struct DosEnvec *ev;
SHORT x,y;
{
   /* char buf[4];*/

   if(ev->de_TableSize < DE_DOSTYPE)
   {
      SetAPen(&world->display.rp,2);
      Move(&world->display.rp,x,y);
      Text(&world->display.rp,"DOS",3);
      SetAPen(&world->display.rp,1);
      /*SetDrMd(&world->display.rp,JAM1|INVERSVID);*/
      Text(&world->display.rp,"0",1);
      SetAPen(&world->display.rp,2);
      /*SetDrMd(&world->display.rp,JAM1);*/
   } else {
      DrawByte(world,x,            y,(BYTE)((ev->de_DosType>>24)&0xff));
      DrawByte(world,(SHORT)(x+8), y,(BYTE)((ev->de_DosType>>16)&0xff));
      DrawByte(world,(SHORT)(x+16),y,(BYTE)((ev->de_DosType>>8) &0xff));
      DrawByte(world,(SHORT)(x+24),y,(BYTE)((ev->de_DosType)    &0xff));
/*
      buf[0]=(BYTE)((ev->de_DosType>>24)&0xff);
      buf[1]=(BYTE)((ev->de_DosType>>16)&0xff);
      buf[2]=(BYTE)((ev->de_DosType>>8)&0xff);
      buf[3]=(BYTE)((ev->de_DosType&0xff)+0x30);
      Move(&world->display.rp,x,y);
      Text(&world->display.rp,buf,4);
*/
   }
}
void DrawByte(world,x,y,value)
struct World *world;
SHORT x,y;
UBYTE value;
{
   Move(&world->display.rp,x,y);
   if (((value>=0x20)&&(value<=0x7f)) || ((value>=0xa0)&&(value<=0xff)))
   {
      SetAPen(&world->display.rp,2);
      /*SetDrMd(&world->display.rp,JAM1);*/
   } else
      if((value>=0x00)&&(value<=0x09))
      {
         value += '0';
         SetAPen(&world->display.rp,1);
         /*SetDrMd(&world->display.rp,JAM1|INVERSVID);*/
      } else
         if((value>=0x0a)&&(value<=0x1f))
         {
            value = value - 0x0a + 'a';
            SetAPen(&world->display.rp,1);
            /*SetDrMd(&world->display.rp,JAM1|INVERSVID);*/
         } else
            if((value>=0x80)&&(value<=0x9f))
            {
               value = value - 0x80 + '@';
               SetAPen(&world->display.rp,1);
               /*SetDrMd(&world->display.rp,JAM1|INVERSVID);*/
            }
   Text(&world->display.rp,&value,1);
   SetAPen(&world->display.rp,2);
   /*SetDrMd(&world->display.rp,JAM1);*/
}
void DrawBOOTPri(world,pri,x,y)
struct World *world;
BYTE pri;
SHORT x,y;
{
   char buf[5];

   sprintf(buf,"%4ld",(LONG)pri);
   SetAPen(&world->display.rp,2);
   SetBPen(&world->display.rp,0);
   SetDrMd(&world->display.rp,JAM2);
   Move(&world->display.rp,x,y);
   Text(&world->display.rp,buf,4);
}
SHORT GadgetHit(world)
struct World *world;
{
   SHORT i;
   SHORT retval=0;
   struct MyNode *mn;

   if((world->sprite.x <= CBGX+CBGW-1) && (world->sprite.x >= CBGX) &&
      (world->sprite.y <= CBGY+CBGH-1) && (world->sprite.y >= CBGY) )
   {
      retval=CBGN;
      goto exit;
   }

   if((world->sprite.x <= DAGX+DAGW-1) && (world->sprite.x >= DAGX) &&
      (world->sprite.y <= DAGY+DAGH-1) && (world->sprite.y >= DAGY) )
   {
      retval=DAGN;
      goto exit;
   }

   if((world->sprite.x <= GOGX+GOGW-1) && (world->sprite.x >= GOGX) &&
      (world->sprite.y <= GOGY+GOGH-1) && (world->sprite.y >= GOGY) )
   {
      retval=GOGN;
      goto exit;
   }

   if(world->MaxPage > 1)
   {
      if((world->sprite.x <= NPGX+NPGW-1) && (world->sprite.x >= NPGX) &&
         (world->sprite.y <= NPGY+NPGH-1) && (world->sprite.y >= NPGY) )
      {
         retval=NPGN;
         goto exit;
      }
   }

   if((world->sprite.x >= (C1GX))&&(world->sprite.x <= (C1GX+G1W-1)))
   {
      for(i=0;i<world->MaxGad;i++)
      {
         /*D( kprintf("i=%ld, min=%ld, max=%ld, sprite.y=%ld\n",(LONG)i,(LONG)((i*GYSP)+GYS),(LONG)((i*GYSP)+GYS+GH),(LONG)world->sprite.y) );*/
         if((world->sprite.y >= ((i * GYSP)+GYS)) &&
            (world->sprite.y <= ((i * GYSP)+GYS+GH-1)) )
         {
            mn=GetNode(world,(SHORT)(world->FirstGad+i));
            /* if((world->gadget[i+10].Enabled) &&(world->gadget[i].Bootable))*/
            if(((mn->Enabled && mn->Bootable)||(mn->NoEnable && mn->Bootable))
                &&(! mn->Highlited))
               retval=((SHORT)(i+1));
            goto exit;
         }
      }
   }

   if((world->sprite.x >= (C0GX))&&(world->sprite.x <= (C0GX+G0W-1)))
   {
      for(i=0;i<world->MaxGad;i++)
      {
         /*D( kprintf("i=%ld, min=%ld, max=%ld, sprite.y=%ld\n",(LONG)i,(LONG)((i*GYSP)+GYS),(LONG)((i*GYSP)+GYS+GH),(LONG)world->sprite.y) );*/
         if((world->sprite.y >= ((i * GYSP)+GYS)) &&
            (world->sprite.y <= ((i * GYSP)+GYS+GH-1)) )
         {
            mn=GetNode(world,(SHORT)(world->FirstGad+i));
            if( ! mn->NoEnable)
               retval=((SHORT)(i+1+10));
            goto exit;
         }
      }
   }
exit:
   D( printf("init.c: GadgetHit() returns %ld\n",retval); )
   return(retval);
}
void ComplementGadget(world,gad)
struct World *world;
SHORT gad;
{
   SHORT y;
   struct MyNode *mn;

   D( printf("init.c: ComplementGadget(world=0x%lx,gad=%ld) enter\n",world,gad); )

   SetDrMd(&world->display.rp,COMPLEMENT);
   switch(gad)
   {
      case CBGN:
            RectFill(&world->display.rp,CBGX,CBGY,CBGX+CBGW-1,CBGY+CBGH-1);
            break;
      case DAGN:
            RectFill(&world->display.rp,DAGX,DAGY,DAGX+DAGW-1,DAGY+DAGH-1);
            break;
      case GOGN:
            RectFill(&world->display.rp,GOGX,GOGY,GOGX+GOGW-1,GOGY+GOGH-1);
            break;
      case NPGN:
            RectFill(&world->display.rp,NPGX,NPGY,NPGX+NPGW-1,NPGY+NPGH-1);
            break;
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
            y=(SHORT)(((gad-1)*GYSP)+GYS);
            RectFill(&world->display.rp,C1GX,y,C1GX+G1W-1,y+GH-1);
            break;
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
#if 0
            Move(&world->display.rp,C0TX,(SHORT)(((gad-11)*GYSP)+TYS));
            SetAPen(&world->display.rp,2);
            SetBPen(&world->display.rp,0);
            SetDrMd(&world->display.rp,JAM2);
#endif
            D( printf("init.c: ComplementGadget() FirstGad=%ld\n",world->FirstGad); )
            mn=GetNode(world,(SHORT)(world->FirstGad+gad-11));
            if(mn->Enabled)
            {
               if(mn->Bootable)
               {
                  if(world->BootNodes > 1)
                  {
                     /* Text(&world->display.rp,"Disabled",8); */
                     mn->Enabled=FALSE;
                     world->BootNodes-=1;
                  }
               } else {
                  /* Text(&world->display.rp,"Disabled",8); */
                  mn->Enabled=FALSE;
               }
            } else {
               /* Text(&world->display.rp,"Enabled ",8); */
               mn->Enabled=TRUE;
               if(mn->Bootable)
                  world->BootNodes+=1;
            }
#if 0
            DrawNWay(world,C0GX,(SHORT)(((gad-11)*GYSP)+GYS),mn->Enabled,mn->NoEnable);
#endif
            RectFill(&world->display.rp,C0GX,(SHORT)(((gad-11)*GYSP)+GYS),C0GX+G0W-1,(SHORT)(((gad-11)*GYSP)+GYS)+GH-1);
            break;
      default:
            break;
   }
   D( printf("init.c: ComplementGadget() returns\n",world,gad); )
}
void MakeChanges(world)
struct World *world;
{
   struct BootNode *bn;
   struct DeviceNode *dn;
   struct MyNode *mn;

   D( printf("init.c: MakeChanges() enter\n"); )

   /***********************************/
   /* Startup-Sequence Enable/Disable */
   /***********************************/
   if(world->SS_Disable)
      ExpansionBase->Flags |= EBF_DOSFLAG;
   else
      ExpansionBase->Flags &= (~EBF_DOSFLAG);

   /****************************/
   /* Partition Enable/Disable */
   /****************************/
   for(mn=(struct MyNode *)world->NodeList.lh_Head;
       mn->Node.ln_Succ;
       mn=(struct MyNode *)mn->Node.ln_Succ)
   {
      bn=mn->node;
      dn=(struct DeviceNode *)bn->bn_DeviceNode;

      if(mn->Enabled)
         dn->dn_Handler &= (~0x80000000);
      else
         dn->dn_Handler |= 0x80000000;
   }

   /****************************/
   /* Boot Partition Selection */
   /****************************/
   if( ! ((struct MyNode *)(world->NodeList.lh_Head))->Highlited) /* Booting default */
      for(mn=(struct MyNode *)world->NodeList.lh_Head;
          mn->Node.ln_Succ;
          mn=(struct MyNode *)mn->Node.ln_Succ)
      {
         bn=mn->node;
         D( dn=(struct DeviceNode *)bn->bn_DeviceNode; )

         /* if highlited then do pri swizzle and break */
         if(mn->Highlited)
         {
            Remove((struct Node *)mn->node);
            mn->node->bn_Node.ln_Pri=127;
            AddHead(&ExpansionBase->MountList,(struct Node *)mn->node);
         }
      }
   D( printf("init.c: MakeChanges() returns\n"); )
}
BOOL DrawEntry(world,x)
struct World *world;
SHORT x;
{
   struct MyNode *mn;
   struct BootNode *bn;
   struct DeviceNode *dn;
   struct FileSysStartupMsg *fssm;
   struct DosEnvec          *ev;

   world->FirstGad=((world->CurrentPage-1)*PAGE_SIZE);
   mn=GetNode(world,(SHORT)(x+world->FirstGad));
   if( ! mn)
   {
      SetAPen(&world->display.rp,0);
      SetDrMd(&world->display.rp,JAM1);

      RectFill(&world->display.rp,C0GX,(SHORT)((x*GYSP)+GYS),C3GX+G3W-1,((x*GYSP)+GYS)+GH-1);
      return(FALSE);
   }
   bn=mn->node;
   dn=(struct DeviceNode *)bn->bn_DeviceNode;
   fssm=(struct FileSysStartupMsg *)(((ULONG)dn->dn_Startup) << 2);
   ev=(struct DosEnvec *)(((ULONG)fssm->fssm_Environ) << 2);

   SetAPen(&world->display.rp,0);
   SetDrMd(&world->display.rp,JAM1);
   RectFill(&world->display.rp,C1GX,(SHORT)((x*GYSP)+GYS),C1GX+G1W-1,((x*GYSP)+GYS)+GH-1);
   DrawBSTR(world,dn->dn_Name,C1TX,(SHORT)(((x)*GYSP)+TYS));

   SetAPen(&world->display.rp,0);
   SetDrMd(&world->display.rp,JAM1);
   RectFill(&world->display.rp,C2GX,(SHORT)((x*GYSP)+GYS),C2GX+G2W-1,((x*GYSP)+GYS)+GH-1);
   DrawDOSType(world,ev,C2TX,(SHORT)(((x)*GYSP)+TYS));

   SetAPen(&world->display.rp,0);
   SetDrMd(&world->display.rp,JAM1);
   RectFill(&world->display.rp,C3GX,(SHORT)((x*GYSP)+GYS),C3GX+G3W-1,((x*GYSP)+GYS)+GH-1);
   SetAPen(&world->display.rp,2);
   if(mn->Bootable)
      DrawBOOTPri(world,bn->bn_Node.ln_Pri,C3TX,(SHORT)(((x)*GYSP)+TYS));
   else
   {
      Move(&world->display.rp,C3TX,(SHORT)(((x)*GYSP)+TYS));
      Text(&world->display.rp," N/A",4);
   }

   SetAPen(&world->display.rp,0);
   SetDrMd(&world->display.rp,JAM1);
   RectFill(&world->display.rp,C0GX,(SHORT)((x*GYSP)+GYS),C0GX+G0W-1,((x*GYSP)+GYS)+GH-1);
   DrawNWay(world,C0GX,(SHORT)((x*GYSP)+GYS),mn->Enabled,mn->NoEnable);
#if 0
   DrawBox(world,C0GX,(SHORT)((x*GYSP)+GYS),G0W,GH,(SHORT)(mn->NoEnable?0:1));
   SetAPen(&world->display.rp,2);
   Move(&world->display.rp,C0TX,(SHORT)((x*GYSP)+TYS));
   if(mn->Enabled)
      Text(&world->display.rp,"Enabled ",8);
   else
      Text(&world->display.rp,"Disabled",8);
#endif

   if(mn->Highlited)
   {
      DrawBox(world,C1GX,(SHORT)((x*GYSP)+GYS),G1W,GH,(SHORT)0);
      ComplementGadget(world,(SHORT)(x+1));
   } else
      DrawBox(world,C1GX,(SHORT)((x*GYSP)+GYS),G1W,GH,(SHORT)(
      ((mn->Enabled && mn->Bootable)||(mn->NoEnable && mn->Bootable))?1:0));

   DrawBox(world,C2GX,(SHORT)((x*GYSP)+GYS),G2W,GH,0);
   DrawBox(world,C3GX,(SHORT)((x*GYSP)+GYS),G3W,GH,0);
   return(TRUE);
}
void DisSprite(struct World *world,int num)
{
   UWORD *k;

   k    =        GfxBase->copinit->sprstrtup + 4*num;
   k[1] = (long)(GfxBase->copinit->sprstop) >>16;
   k[3] = (long)(GfxBase->copinit->sprstop);
}
#if GLYPH
void DrawGlyph(struct World *world,SHORT x,SHORT y)
{
   struct BitMap mybitmap;
   SHORT place;

   InitBitMap(&mybitmap, 8, NWAYBMWIDTH, NWAYBMHEIGHT);

   if (!(mybitmap.Planes[0] =
   AllocMem(sizeof(NWayGlyphData), MEMF_CHIP)))
   {
      return;
   }
   CopyMem((char *)NWayGlyphData, (char *)mybitmap.Planes[0],
   sizeof(NWayGlyphData));
   for (place = 1; place < 8; place++)
   {
      mybitmap.Planes[place] = mybitmap.Planes[0];
   }
    SetWrMsk(&world->display.rp,0x02);
    MyBltBitMapRastPort(GfxBase,&mybitmap, 0, 0,&world->display.rp,
                      (SHORT)(x+5),(SHORT)(y+2),
                      NWAYBMWIDTH, NWAYBMHEIGHT,
                      VANILLA_COPY);
    SetWrMsk(&world->display.rp,0xFF);
}
#endif
VOID DrawNWay(struct World *world,SHORT x,SHORT y,BOOL enabled,BOOL sunken)
{
   DrawBox(world,x,y,DAGW,DAGH,(BOOL)(! sunken));
   DrawGlyph(world,x,y);
   SetAPen(&world->display.rp,2);
   Move(&world->display.rp,x+19,y+2);
   Draw(&world->display.rp,x+19,y+10);
   SetAPen(&world->display.rp,1);
   Move(&world->display.rp,x+20,y+2);
   Draw(&world->display.rp,x+20,y+10);

   SetAPen(&world->display.rp,2);
   SetBPen(&world->display.rp,0);
   SetDrMd(&world->display.rp,JAM2);
   if(! enabled)
   {
      Move(&world->display.rp,x+19+9,y+9);
      Text(&world->display.rp,"DISABLED",8);
   } else {
      Move(&world->display.rp,x+19+9,y+9);
      Text(&world->display.rp,"        ",8);
      Move(&world->display.rp,x+19+9+4,y+9);
      Text(&world->display.rp,"ENABLED",7);
   }
}
