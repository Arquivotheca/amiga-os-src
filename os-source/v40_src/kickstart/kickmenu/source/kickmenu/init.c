#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/expansionbase.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/view.h>
#include <graphics/sprite.h>
#include <graphics/copper.h>
#include <libraries/filehandler.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <devices/input.h>
#include <devices/trackdisk.h>
#include <proto/all.h>

#include <string.h>

void __stdargs ColdReboot(void);
#pragma syscall ColdReboot 2d6 0

void kprintf(char *,...);
void sprintf(char *,char *,...);
#define  D(x) ;
#define  D1(x) ;
#define printf kprintf

#define FLOPPY_MULTI 1

ULONG SysBase;

typedef int (*PFI)(void);

PFI   MyFunc13   = (PFI)(0x7F80000L+0x40000L+8L+4L);
PFI   MyFunc14   = (PFI)(0x7F80000L+0x7F000L+8L+4L);

LONG  *Config13  = (LONG *)(0x7F80000L+0x40000L+8L);
LONG  *Config14  = (LONG *)(0x7F80000L+0x7F000L+8L);

USHORT GhostPattern[] = {
      0x8888,
      0x2222
   };
/*
 * 2 = HD
 * 1 = Floppy
 */

#define DEPTH     2
#define WIDTH     640
#define HEIGHT    200
#define VPMODES   HIRES|SPRITES

#define ALLOC_SPRITE 1

#define GYSP      8                   /* gadget y spacing */

#define CBTW      48                   /* cancel button text witdh    */
#define CBTH      8                    /* cancel button text height   */
#define CBGW      (CBTW+10)            /* cancel button gadget width  */
#define CBGH      13                   /* cancel button gadget height */
#define CBGX      ((WIDTH-1-CBGW)>>1)  /* cancel button gadget x      */
#define CBGY      185                  /* cancel button gadget y      */
#define CBTX      (CBGX+((CBGW-CBTW)>>1))/* cancel button gadget x      */
#define CBTY      (CBGY+CBTH+((CBGH-CBTH)>>1)-1)/* cancel button gadget y     */
#define CBGN      (100)                /* cancel button gadget number */

#define BH       (50)
#define BW       (250)
#define B0X       ((WIDTH-1-BW)>>1)
#define B0Y       35

#define B1X       B0X
#define B1Y       (B0Y+BH+15)

#define BTX       (B0X+15)
#define B0TY      (B0Y+((BH)>>1)+2)
#define B1TY      (B1Y+((BH)>>1)+2)

#define H4TW      80                   /* disable all button text witdh    */
#define H4TH      8                    /* disable all button text height   */
#define H4GW      (H4TW+10)            /* disable all button gadget width  */
#define H4GH      13                   /* disable all button gadget height */
#define H4GX      (B0X+BW-H4GW-15)                 /* disable all button gadget x      */
#define H4GY      (((BH-H4GH-F4GH-GYSP)>>1)+B0Y)                 /* disable all button gadget y      */
#define H4TX      (H4GX+((H4GW-H4TW)>>1))/* disable all button gadget x      */
#define H4TY      (H4GY+H4TH+((H4GH-H4TH)>>1)-1)/* disable all button gadget y     */
#define H4GN      (104)                /* disable all button gadget number */

#define F4TW      80                   /* disable all button text witdh    */
#define F4TH      8                    /* disable all button text height   */
#define F4GW      (F4TW+10)            /* disable all button gadget width  */
#define F4GH      13                   /* disable all button gadget height */
#define F4GX      H4GX                /* disable all button gadget x      */
#define F4GY      (H4GY+H4GH+GYSP)                  /* disable all button gadget y      */
#define F4TX      (F4GX+((F4GW-F4TW)>>1))/* disable all button gadget x      */
#define F4TY      (F4GY+F4TH+((F4GH-F4TH)>>1)-1)/* disable all button gadget y     */
#define F4GN      (106)                /* disable all button gadget number */

#define H3TW      80                   /* disable all button text witdh    */
#define H3TH      8                    /* disable all button text height   */
#define H3GW      (H3TW+10)            /* disable all button gadget width  */
#define H3GH      13                   /* disable all button gadget height */
#define H3GX      H4GX                  /* disable all button gadget x      */
#define H3GY      (((BH-H3GH-F3GH-GYSP)>>1)+B1Y)                  /* disable all button gadget y      */
#define H3TX      (H3GX+((H3GW-H3TW)>>1))/* disable all button gadget x      */
#define H3TY      (H3GY+H3TH+((H3GH-H3TH)>>1)-1)/* disable all button gadget y     */
#define H3GN      (105)                /* disable all button gadget number */


#define F3TW      80                   /* disable all button text witdh    */
#define F3TH      8                    /* disable all button text height   */
#define F3GW      (F3TW+10)            /* disable all button gadget width  */
#define F3GH      13                   /* disable all button gadget height */
#define F3GX      H4GX                 /* disable all button gadget x      */
#define F3GY      (H3GY+H3GH+GYSP)                  /* disable all button gadget y      */
#define F3TX      (F3GX+((F3GW-F3TW)>>1))/* disable all button gadget x      */
#define F3TY      (F3GY+F3TH+((F3GH-F3TH)>>1)-1)/* disable all button gadget y     */
#define F3GN      (107)                /* disable all button gadget number */

#if 0
#define HDTW      8                   /* disable all button text witdh    */
#define HDTH      8                    /* disable all button text height   */
#endif
#define HDGW      2            /* disable all button gadget width  */
#define HDGH      2                   /* disable all button gadget height */
#define HDGX      0                    /* disable all button gadget x      */
#define HDGY      0                    /* disable all button gadget y      */
#if 0
#define HDTX      (H3GX+((H3GW-H3TW)>>1))/* disable all button gadget x      */
#define HDTY      (H3GY+H3TH+((H3GH-H3TH)>>1)-1)/* disable all button gadget y     */
#endif
#define HDGN      (108)                /* disable all button gadget number */

#define UP_DELAY  15    /* 60ths of a second after a gadget is released */
                        /* and before the screen closes                 */

#if 0
#define PAGE_SIZE 10    /* Number of nodes per page                     */
#define PAGEX     65
#define PAGEY     NPTY
#endif

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
/*
struct MyNode {
   struct Node Node;
   BOOL  Highlited;
   BOOL  Enabled;
   BOOL  NoEnable;
   BOOL  Bootable;
   struct BootNode *node;
   };
*/

struct World {
   struct GfxBase       *GfxBase;
   struct ExpansionBase *ExpansionBase;
   struct Display       display;
   struct IH            ih;
   struct Sprite        sprite;
   struct Task          *maintask;
   LONG                 signal;
   SHORT                gad;     /* gad currently depressed or 0 */
/*   SHORT                CurrentPage; *//* 1 - MaxPage */
/*   SHORT                MaxPage;  */   /* Max Page number */
/*   struct List          NodeList; */
/*   SHORT                Nodes; */     /* Number of EB_Mounlist nodes */
/*   SHORT                MaxGad;  */   /* current Max Gadgets on screen */
/*   SHORT                FirstGad; */  /* node number of first displayed node*/
/*   SHORT                SS_Disable; */
   BOOL                 Cancel;
   LONG                 HighPriNode;
   LONG                 Bonus13_size;
   LONG                 Bonus14_size;
   BOOL                 Exists13;
   BOOL                 Exists14;
#if 0
   struct IOExtTD       *diskreq;
   struct MsgPort       *diskport;
   BOOL                 TDOpen;
#endif
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
VOID   ShowError(struct World *world,char *s);
VOID   ShowMessage(struct World *world,char *s);
BOOL   ReadFloppy(struct World *world,ULONG offset,ULONG count,APTR address);
BOOL   DiskInDrive(struct World *world);
BOOL   MagicFloppy(struct World *world);
BOOL   SuperDisk(struct World *world);
BOOL   MatchPart(BSTR bp,char *s);
LONG   GetHighPri(struct World *world);
BOOL   isfloppy(BSTR bp);
void   DisSprite(struct World *world,int num);
BOOL   PartExists(struct World *world,char *s);
BOOL   MatchBSTR(BSTR bp,char *s);

int MyInit()
{
   struct Library *library;
   struct World   *world;
   struct World   myworld;

   D( kprintf("###################\n") );
   D( kprintf("Kickmenu Init Begins\n") );

   world=&myworld;

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

#if 0
   world->diskport=NULL;
   world->diskreq=NULL;

   if((world->diskport=CreatePort(NULL,NULL))==NULL)
      goto out;

   if((world->diskreq=(struct IOExtTD *)CreateExtIO(world->diskport,sizeof(struct IOExtTD)))==NULL)
      goto out;

   world->TDOpen=FALSE;
   if(OpenDevice(TD_NAME,0L,(struct IORequest *)world->diskreq,0L))
      goto out;
   world->TDOpen=TRUE;
#endif

   world->Exists13=PartExists(world,"WB_1.3");
   world->Exists14=PartExists(world,"WB_2.x");
   world->HighPriNode=GetHighPri(world);

   if( ! Activated() )
   {
      if( ! MagicFloppy(world))
      {
         if(world->HighPriNode==3)
         {
            D( printf("Not Activated, No Magic Floppy, Default part = 1.3\n"); )
            world->ExpansionBase->Flags |= EBF_KICKBACK33;
         } else
            if(world->HighPriNode==2)
            {
               D( printf("Not Activated, No Magic Floppy, Default part = 2.x\n"); )
               world->ExpansionBase->Flags |= EBF_KICKBACK36;
            } else
               if(world->HighPriNode==4)
               {
                  D( printf("Not Activated, No Magic Floppy, No Part Default = meunu\n"); )
                  goto in;
               }
         D( printf("Not Activated, No Magic Floppy, Default part=%d\n",world->HighPriNode); )
         goto out;
      }
      D( printf("Not Activated, Magic Floppy = ROM\n"); )
      goto out;
   }
in:

   DisplayOn(world);
   D1( kprintf("After DisplayOn()\n") );

/*
   GetData(world);
   world->Cancel=FALSE;
*/

   DoImagery(world);
   D1( kprintf("After DoImagery()\n") );

   OpenSprite(world);
   D1( kprintf("After OpenSprite()\n") );

   OpenIH(world);
   D1( kprintf("After OpenIH()\n") );

   Wait(1<<world->signal);

/*
   if( ! world->Cancel)
      MakeChanges(world);
*/

/*
   FreeData(world);
*/

   CloseIH(world);
   D1( kprintf("After CloseIH()\n") );

   CloseSprite(world);
   D1( kprintf("After CloseSprite()\n") );

   DisplayOff(world);

out:
   D( kprintf("Kickmenu exiting\n") );
#if 0
   if(world->TDOpen) CloseDevice((struct IORequest *)world->diskreq);
   if(world->diskreq)  DeleteExtIO((struct IORequest *)world->diskreq);
   if(world->diskport) DeletePort(world->diskport);
#endif
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
      D1( kprintf("No Left button. Exiting\n") );
      return(FALSE);
   }
   if( (1<<10) & *((UWORD *)0xDFF016) )
   {
      D1( kprintf("No Right button. Exiting\n") );
      return(FALSE);
   }
   return(TRUE);
}
#if 1
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
#endif
void DoImagery(world)
struct World *world;
{
   SHORT tl;

   D1( printf("init.c: DoImagery(world=0x%lx) enter\n",world); )

/*   SetRast(&world->display.rp,0); */
   SetAPen(&world->display.rp,3);
   SetBPen(&world->display.rp,0);
   SetDrMd(&world->display.rp,JAM2);
   SetFont(&world->display.rp,world->display.font9);

   /*********/
   /* Title */
   /*********/
   tl=TextLength(&world->display.rp,"Kickstart Menu",14);
   Move(&world->display.rp,(WIDTH-1-tl)>>1,8);
   Text(&world->display.rp,"Kickstart Menu",14);

   SetFont(&world->display.rp,world->display.font8);

   /*******************/
   /* Command Buttons */
   /*******************/
   SetAPen(&world->display.rp,2);
   Move(&world->display.rp,CBTX,CBTY);
   Text(&world->display.rp,"CANCEL",6);
   DrawBox(world,CBGX,CBGY,CBGW,CBGH,1);

   /****************/
   /* Main Gadgets */
   /****************/

   Move(&world->display.rp,H4TX,H4TY);
   Text(&world->display.rp,"Hard Drive",10);
   DrawBox(world,H4GX,H4GY,H4GW,H4GH,1);

   Move(&world->display.rp,F4TX,F4TY);
   Text(&world->display.rp,"  Floppy  ",10);
   DrawBox(world,F4GX,F4GY,F4GW,F4GH,1);

   Move(&world->display.rp,BTX,B0TY);
   Text(&world->display.rp,"Kickstart 2.x",13);
   DrawBox(world,B0X,B0Y,BW,BH,0);

   SetAPen(&world->display.rp,2);
   Move(&world->display.rp,H3TX,H3TY);
   Text(&world->display.rp,"Hard Drive",10);
   DrawBox(world,H3GX,H3GY,H3GW,H3GH,1);

   Move(&world->display.rp,F3TX,F3TY);
   Text(&world->display.rp,"  Floppy  ",10);
   DrawBox(world,F3GX,F3GY,F3GW,F3GH,1);

   Move(&world->display.rp,BTX,B1TY);
   Text(&world->display.rp,"Kickstart 1.3",13);
   DrawBox(world,B1X,B1Y,BW,BH,0);

   SetAPen(&world->display.rp,2);
   SetAfPt(&world->display.rp,GhostPattern,1);
   SetDrMd(&world->display.rp,JAM1);

   if( ! world->Exists13)
      RectFill(&world->display.rp,H3GX+2,H3GY+1,H3GX+H3GW-4,H3GY+H3GH-2);

   if( ! world->Exists14)
      RectFill(&world->display.rp,H4GX+2,H4GY+1,H4GX+H4GW-4,H4GY+H4GH-2);

   SetAfPt(&world->display.rp,NULL,0);

   D1( printf("init.c: DoImagery() returns\n"); )
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
      OFF_DISPLAY
      LoadView(0);
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
               D1( kprintf("Left Button UP\n") );
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
   char *mymem;

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
      case F3GN:
            D( printf("UpAction() F3GN\n"); )
            mymem=(char *)AllocAbs(0x80000L,(void *)0x7f80000L);

            D1( printf("mymem = 0x%lx\n",mymem); )

            if( ! mymem)
               ShowError(world,"Error can't get memory! Reboot system.");

            { LONG x; for(x=0;x<0x80000;x++) {  ((char *)mymem)[x]=0; } }

            /* MoveSprite(0,&world->sprite.sprite,(WIDTH>>1)-1,HEIGHT-1); */
            DisSprite(world,world->sprite.num);
            /* if(!DiskInDrive(world)) */
            {
               ShowMessage(world,"Insert Super-Kickstart disk in drive DF0:");
            }
            while(1)
            {
               if(DiskInDrive(world))
               {
                  if(SuperDisk(world))
                  {
                     break;
                  }
               }
            }

            ShowMessage(world,"Loading Kickstart 1.3");
            if(!ReadFloppy(world,0x400L,0x40000L,(APTR)mymem))
            {
               ShowError(world,"Error reading floppy! Reboot system.");
            }

            /* ShowMessage(world,"Reading Bonus13 image."); */
            if(!ReadFloppy(world,0xC1400L,world->Bonus13_size,(APTR)(mymem+0x40000L)))
            {
               ShowError(world,"Error reading floppy! Reboot system.");
            }

            *Config13 = 1; /* flag it as a floppy boot */

            ShowMessage(world,"Rebooting system, please wait...");
            D( printf("Before MyFunc13\n"); )
            (*MyFunc13)();   /* kick MMU */
            ShowError(world,"Error MMU routine returned! Reboot System.");
      case H3GN:
            D( printf("UpAction() H3GN\n"); )
            ExpansionBase->Flags |= EBF_KICKBACK33;
            for(i=0;i<UP_DELAY;i++)
               WaitTOF();
            world->Cancel=TRUE;
            Signal(world->maintask,1<<world->signal);
            break;
      case H4GN:
            D( printf("UpAction() H4GN\n"); )
            ExpansionBase->Flags |= EBF_KICKBACK36;
            for(i=0;i<UP_DELAY;i++)
               WaitTOF();
            world->Cancel=TRUE;
            Signal(world->maintask,1<<world->signal);
            break;
      case F4GN:
            D( printf("UpAction() F4GN\n"); )
            mymem=(char *)AllocAbs(0x80000L,(void *)0x7f80000L);

            /* mymem=(char *)AllocMem(0x80000,MEMF_CHIP); */

            D1( printf("mymem = 0x%lx\n",mymem); )

            if( ! mymem)
               ShowError(world,"Error can't get memory! Reboot system.");

            { LONG x; for(x=0;x<0x80000;x++) {  ((char *)mymem)[x]=0; } }

            /* MoveSprite(0,&world->sprite.sprite,(WIDTH>>1)-1,HEIGHT-1); */
            DisSprite(world,world->sprite.num);
            /* if(!DiskInDrive(world)) */
            {
               ShowMessage(world,"Insert Super-Kickstart disk in drive DF0:");
            }
            while(1)
            {
               if(DiskInDrive(world))
               {
                  if(SuperDisk(world))
                  {
                     break;
                  }
               }
            }

            ShowMessage(world,"Loading Kickstart 2.x");
            if(!ReadFloppy(world,0x40400L,0x80000L,(APTR)mymem))
            {
               ShowError(world,"Error reading floppy! Reboot system.");
            }

            /* ShowMessage(world,"Reading NoHD image."); */
            if(!ReadFloppy(world,0xC0400L,world->Bonus14_size,(APTR)(mymem+0x7F000L)))
            {
               ShowError(world,"Error reading floppy! Reboot system.");
            }

            *Config14 = 1; /* flag it as a floppy boot */

            ShowMessage(world,"Rebooting system, please wait...");
            (*MyFunc14)();   /* kick MMU */
            ShowError(world,"Error MMU routine returned! Reboot system.");
      case CBGN:
            D( printf("CANCEL GADGET\n"); )
            if(world->HighPriNode==3)
            {
               D( printf("Cancel Default part = 1.3\n"); )
               ExpansionBase->Flags |= EBF_KICKBACK33;
            } else
               if(world->HighPriNode==2)
               {
                  D( printf("Cancel Default part = 2.x\n"); )
                  ExpansionBase->Flags |= EBF_KICKBACK36;
               } else
                  if(world->HighPriNode==4)
                  {
                     /* ComplementGadget(world,world->gad); */
                     ColdReboot();
                     break;
                  }
            for(i=0;i<UP_DELAY;i++)
               WaitTOF();
            world->Cancel=TRUE;
            Signal(world->maintask,1<<world->signal);
            break;
      case HDGN:
            D( printf("HIDDEN GADGET\n"); )
            for(i=0;i<UP_DELAY;i++)
               WaitTOF();
            world->Cancel=TRUE;
            Signal(world->maintask,1<<world->signal);
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
   D1( kprintf("Sprite num = %ld\n",world->sprite.num) );
   world->sprite.sprite.x=0;
   world->sprite.sprite.y=0;
   world->sprite.sprite.height=16;

   k=((world->sprite.num&0x06)*2)+16;
   D1( kprintf("k=%ld\n",(LONG)k) );
   SetRGB4(&world->display.vp,k+1,13,2,2);
   SetRGB4(&world->display.vp,k+2,0,0,0);
   SetRGB4(&world->display.vp,k+3,15,12,10);

   world->sprite.image=AllocMem(19*4,MEMF_CHIP);
   p=(UWORD *)world->sprite.image;
   for(k=0;k<19*2;k++)
      p[k]=IPointer[k];

   ChangeSprite(&world->display.vp,&world->sprite.sprite,p);
   MoveSprite(0,&world->sprite.sprite,30,30);
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
SHORT GadgetHit(world)
struct World *world;
{
   SHORT retval=0;

   if(world->Exists14)
   {
      if((world->sprite.x <= H4GX+H4GW-1) && (world->sprite.x >= H4GX) &&
         (world->sprite.y <= H4GY+H4GH-1) && (world->sprite.y >= H4GY) )
      {
         retval=H4GN;
         goto exit;
      }
   }

   if(world->Exists13)
   {
      if((world->sprite.x <= H3GX+H3GW-1) && (world->sprite.x >= H3GX) &&
         (world->sprite.y <= H3GY+H3GH-1) && (world->sprite.y >= H3GY) )
      {
         retval=H3GN;
         goto exit;
      }
   }

   if((world->sprite.x <= F4GX+F4GW-1) && (world->sprite.x >= F4GX) &&
      (world->sprite.y <= F4GY+F4GH-1) && (world->sprite.y >= F4GY) )
   {
      retval=F4GN;
      goto exit;
   }

   if((world->sprite.x <= F3GX+F3GW-1) && (world->sprite.x >= F3GX) &&
      (world->sprite.y <= F3GY+F3GH-1) && (world->sprite.y >= F3GY) )
   {
      retval=F3GN;
      goto exit;
   }

   if((world->sprite.x <= CBGX+CBGW-1) && (world->sprite.x >= CBGX) &&
      (world->sprite.y <= CBGY+CBGH-1) && (world->sprite.y >= CBGY) )
   {
      retval=CBGN;
      goto exit;
   }
   if((world->sprite.x <= HDGX+HDGW-1) && (world->sprite.x >= HDGX) &&
      (world->sprite.y <= HDGY+HDGH-1) && (world->sprite.y >= HDGY) )
   {
      retval=HDGN;
      goto exit;
   }

exit:
   D1( printf("init.c: GadgetHit() returns %ld\n",retval); )
   return(retval);
}
void ComplementGadget(world,gad)
struct World *world;
SHORT gad;
{
   D1( printf("init.c: ComplementGadget(world=0x%lx,gad=%ld) enter\n",world,gad); )

   SetDrMd(&world->display.rp,COMPLEMENT);
   switch(gad)
   {
      case H4GN:
            RectFill(&world->display.rp,H4GX,H4GY,H4GX+H4GW-1,H4GY+H4GH-1);
            break;
      case H3GN:
            RectFill(&world->display.rp,H3GX,H3GY,H3GX+H3GW-1,H3GY+H3GH-1);
            break;
      case F4GN:
            RectFill(&world->display.rp,F4GX,F4GY,F4GX+F4GW-1,F4GY+F4GH-1);
            break;
      case F3GN:
            RectFill(&world->display.rp,F3GX,F3GY,F3GX+F3GW-1,F3GY+F3GH-1);
            break;
      case CBGN:
            RectFill(&world->display.rp,CBGX,CBGY,CBGX+CBGW-1,CBGY+CBGH-1);
            break;
      default:
            break;
   }
   D1( printf("init.c: ComplementGadget() returns\n",world,gad); )
}
VOID ShowError(struct World *world,char *s)
{
   ShowMessage(world,s);
   for(;;);
}
VOID ShowMessage(struct World *world,char *s)
{
   LONG tl;

   SetRast(&world->display.rp,0);
   tl=TextLength(&world->display.rp,s,strlen(s));
   Move(&world->display.rp,(WIDTH-1-tl)>>1,104);
   Text(&world->display.rp,s,strlen(s));
}
BOOL ReadFloppy(struct World *world,ULONG offset,ULONG count,APTR address)
{
#if FLOPPY_MULTI
   struct IOExtTD *diskreq  = NULL;
   struct MsgPort *diskport = NULL;
#endif
   BOOL           retval    = FALSE;

   D1( printf("init.c: ReadFloppy(offset=%ld,count=0x%lx,address=0x%lx) enter\n",offset,count,address); )

#if FLOPPY_MULTI
   if((diskport=CreatePort(NULL,NULL))==NULL)
      goto out;

   if((diskreq=(struct IOExtTD *)CreateExtIO(diskport,sizeof(struct IOExtTD)))==NULL)
      goto out;

   if(OpenDevice(TD_NAME,0L,(struct IORequest *)diskreq,0L))
      goto out;

   /*** MOTOR ON ***/
   diskreq->iotd_Req.io_Length=1;
   diskreq->iotd_Req.io_Command=TD_MOTOR;
   DoIO((struct IORequest *)diskreq);
#endif

   D1( printf("init.c: ReadFloppy() after motor on\n"); )

   /*** DO READ ***/
   diskreq->iotd_Req.io_Length =count;
   diskreq->iotd_Req.io_Data   =address;
   diskreq->iotd_Req.io_Command=CMD_READ;
   diskreq->iotd_Req.io_Offset =offset;
   DoIO((struct IORequest *)diskreq);
   if( ! diskreq->iotd_Req.io_Error)
   {
      retval=TRUE;
   }

   D1( printf("init.c: ReadFloppy() after read\n"); )

   /*** MOTOR OFF ***/
#if FLOPPY_MULTI
   diskreq->iotd_Req.io_Length=0;
   diskreq->iotd_Req.io_Command=TD_MOTOR;
   DoIO((struct IORequest *)diskreq);
#endif

   D1( printf("init.c: ReadFloppy() after motor off\n"); )

#if FLOPPY_MULTI
   CloseDevice((struct IORequest *)diskreq);
#endif

out:
#if FLOPPY_MULTI
   if(diskreq)  DeleteExtIO((struct IORequest *)diskreq);
   if(diskport) DeletePort(diskport);
#endif
   return(retval);
}
BOOL DiskInDrive(struct World *world)
{
#if FLOPPY_MULTI
   struct IOExtTD *diskreq  = NULL;
   struct MsgPort *diskport = NULL;
#endif
   BOOL           retval    = FALSE;

   D( printf("init.c: DiskInDrive() enter\n"); )

#if FLOPPY_MULTI
   if((diskport=CreatePort(NULL,NULL))==NULL)
      goto out;

   if((diskreq=(struct IOExtTD *)CreateExtIO(diskport,sizeof(struct IOExtTD)))==NULL)
      goto out;

   if(OpenDevice(TD_NAME,0L,(struct IORequest *)diskreq,0L))
      goto out;
#endif

   diskreq->iotd_Req.io_Length=1;
   diskreq->iotd_Req.io_Command=TD_CHANGESTATE;
   /*DoIO((struct IORequest *)diskreq);*/
   SendIO((struct IORequest *)diskreq);
   WaitIO((struct IORequest *)diskreq);
   if(diskreq->iotd_Req.io_Actual==0)
   {
      retval=TRUE;
   } else {
      { ULONG tt; for(tt=0;tt<150;tt++) { WaitTOF(); } }
      retval=FALSE;
   }

#if FLOPPY_MULTI
   CloseDevice((struct IORequest *)diskreq);
#endif

out:
#if FLOPPY_MULTI
   if(diskreq)  DeleteExtIO((struct IORequest *)diskreq);
   if(diskport) DeletePort(diskport);
#endif
   return(retval);
}
#if 0
BOOL MagicFloppy(struct World *world)
{
   struct IOExtTD *diskreq  = NULL;
   struct MsgPort *diskport = NULL;
   BOOL           retval    = FALSE;
   UBYTE          buf[513];

   D( printf("init.c: MagicFloppy() enter\n"); )

   if( ! DiskInDrive(world)) return(FALSE);

   if((diskport=CreatePort(NULL,NULL))==NULL)
      goto out;

   if((diskreq=(struct IOExtTD *)CreateExtIO(diskport,sizeof(struct IOExtTD)))==NULL)
      goto out;

   if(OpenDevice(TD_NAME,0L,(struct IORequest *)diskreq,0L))
      goto out;

   /*** MOTOR ON ***/
   diskreq->iotd_Req.io_Length=1;
   diskreq->iotd_Req.io_Command=TD_MOTOR;
   DoIO((struct IORequest *)diskreq);

   D1( printf("init.c: MagicFloppy() after motor on\n"); )

   /*** DO READ ***/
   diskreq->iotd_Req.io_Length =512;
   diskreq->iotd_Req.io_Data   =&buf[0];
   diskreq->iotd_Req.io_Command=CMD_READ;
   /* diskreq->iotd_Req.io_Offset =0x6e1ba;*/ /* ROOT block (880) offset 0x1ba */
   diskreq->iotd_Req.io_Offset =0x6e000; /* ROOT block (880) offset 0x1ba */
   DoIO((struct IORequest *)diskreq);
   if( ! diskreq->iotd_Req.io_Error)
   {
      if(  (*((ULONG *)(&buf[442])))  ==0x03010401)     /* magic value 0x03010401  */
      {
         retval=TRUE;
      }
   }

   D1( printf("init.c: MagicFloppy() after read\n"); )

   /*** MOTOR OFF ***/
   diskreq->iotd_Req.io_Length=0;
   diskreq->iotd_Req.io_Command=TD_MOTOR;
   DoIO((struct IORequest *)diskreq);

   D1( printf("init.c: MagicFloppy() after motor off\n"); )

   CloseDevice((struct IORequest *)diskreq);

out:
   if(diskreq)  DeleteExtIO((struct IORequest *)diskreq);
   if(diskport) DeletePort(diskport);
   return(retval);
}
#endif
BOOL MagicFloppy(struct World *world)
{
#if FLOPPY_MULTI
   struct IOExtTD *diskreq  = NULL;
   struct MsgPort *diskport = NULL;
#endif
   BOOL           retval    = FALSE;
   UBYTE          buf[513];

   D( printf("init.c: MagicFloppy() enter\n"); )

   if( ! DiskInDrive(world)) return(FALSE);

#if FLOPPY_MULTI
   if((diskport=CreatePort(NULL,NULL))==NULL)
      goto out;

   if((diskreq=(struct IOExtTD *)CreateExtIO(diskport,sizeof(struct IOExtTD)))==NULL)
      goto out;

   if(OpenDevice(TD_NAME,0L,(struct IORequest *)diskreq,0L))
      goto out;

   /*** MOTOR ON ***/
   diskreq->iotd_Req.io_Length=1;
   diskreq->iotd_Req.io_Command=TD_MOTOR;
   DoIO((struct IORequest *)diskreq);
#endif

   D1( printf("init.c: MagicFloppy() after motor on\n"); )

   /*** DO READ ***/
   diskreq->iotd_Req.io_Length =512;
   diskreq->iotd_Req.io_Data   =&buf[0];
   diskreq->iotd_Req.io_Command=CMD_READ;
   /* diskreq->iotd_Req.io_Offset =0x6e1ba;*/ /* ROOT block (880) offset 0x1ba */
   diskreq->iotd_Req.io_Offset =512L; /* BOOT block (880) offset 0x1ba */
   DoIO((struct IORequest *)diskreq);
   if( ! diskreq->iotd_Req.io_Error)
   {
      if(  (*((ULONG *)(&buf[508])))  ==0x03010401)     /* magic value 0x03010401  */
      {
         retval=TRUE;
      }
   }

   D1( printf("init.c: MagicFloppy() after read\n"); )

   /*** MOTOR OFF ***/
#if FLOPPY_MULTI
   diskreq->iotd_Req.io_Length=0;
   diskreq->iotd_Req.io_Command=TD_MOTOR;
   DoIO((struct IORequest *)diskreq);
#endif

   D1( printf("init.c: MagicFloppy() after motor off\n"); )

#if FLOPPY_MULTI
   CloseDevice((struct IORequest *)diskreq);
#endif

out:
#if FLOPPY_MULTI
   if(diskreq)  DeleteExtIO((struct IORequest *)diskreq);
   if(diskport) DeletePort(diskport);
#endif
   return(retval);
}
BOOL SuperDisk(struct World *world)
{
   BOOL           retval    = FALSE;
   LONG           buf[129];

   D( printf("init.c: SuperDisk() enter\n"); )

   if(ReadFloppy(world,0L,512L,&buf[0]))
   {
      D1( printf("init.c: SuperDisk() ReadFloppys OK!\n"); )
      D1( printf("buf[0]=0x%lx buf[1]=0x%lx\n",buf[0],buf[1]); )
      if((buf[0]==0x4B49434B)&&(buf[1]==0x53555030))
         retval=TRUE;
   }

   world->Bonus13_size=buf[2];
   world->Bonus14_size=buf[3];
   D1( printf("init.c: SuperDisk() returns 0x%lx\n",retval); )

   return(retval);
}
BOOL MatchPart(BSTR bp,char *s)
{
   UBYTE count;
   UBYTE *p;

   p=(UBYTE *)(bp << 2);
   count=*p;

   D( printf("MatchPart("); )
   D( printbstr(bp); )
   D( kprintf(",%s) enter\n",s) );

   if(count!=6) return(FALSE);

   /* D( kprintf("isPart() good length\n") ); */

/*   if(strnicmp(&p[1],"WB_1.3",6)!=NULL) */
   if(strnicmp(&p[1],s,6)!=NULL)
   {
      D( kprintf("MatchPart() FAIL\n") );
      return(FALSE);
   }

   D( kprintf("MatchPart() MATCH\n") );
   return(TRUE);
}
/* Returns 3 if WB_1.3 partition is highest pri
 * Returns 2 if WB_2.x partition is highest pri
 * Returns 0 otherwise
 */
LONG GetHighPri(struct World *world)
{
   struct BootNode *bn;
   struct DeviceNode *dn;
   struct FileSysStartupMsg *fssm;
   struct DosEnvec          *ev;
   LONG                     hp=-1;
   struct BootNode          *highnode=NULL;
   /* BOOL                     NoPart=TRUE; */

   D( kprintf("Begin MountList Search\n") );
   for(bn=(struct BootNode *)ExpansionBase->MountList.lh_Head;
       bn->bn_Node.ln_Succ;
       bn=(struct BootNode *)bn->bn_Node.ln_Succ)
   {
      D( printf("Node: ") );
      if(bn->bn_DeviceNode)
      {
         dn=(struct DeviceNode *)bn->bn_DeviceNode;
         fssm=(struct FileSysStartupMsg *)(((ULONG)dn->dn_Startup) << 2);
         ev=(struct DosEnvec *)(((ULONG)fssm->fssm_Environ) << 2);

         if(bn->bn_Node.ln_Type == NT_BOOTNODE)
         {
            D( printf("checking bootnode\n"); )
            if( ! isfloppy(dn->dn_Name))
            {
               /*NoPart=FALSE;*/
               if(bn->bn_Node.ln_Pri>hp)
               {
                  hp=bn->bn_Node.ln_Pri;
                  highnode=bn;
               }
            }
         }

         D( printbstr(dn->dn_Name) );
         D( printf(" Pri=%ld",bn->bn_Node.ln_Pri) );
         D( printf(" DOSType=%lx",(ULONG)ev->de_DosType) );
         D( printf(" BP=%ld",ev->de_BootPri) );
         D( if(bn->bn_Node.ln_Type & NT_BOOTNODE) printf(" BOOTABLE"); )
         D( printf(" NodeType=0x%lx\n",bn->bn_Node.ln_Type); )
      }
      D( printf("\n") );
   }
   D( kprintf("End MountList Search\n") );
   if(highnode)
   {
      bn=highnode;
      dn=(struct DeviceNode *)bn->bn_DeviceNode;
      fssm=(struct FileSysStartupMsg *)(((ULONG)dn->dn_Startup) << 2);
      ev=(struct DosEnvec *)(((ULONG)fssm->fssm_Environ) << 2);
      if(MatchPart(dn->dn_Name,"WB_1.3"))
      {
         return(3);
      } else
         if(MatchPart(dn->dn_Name,"WB_2.x"))
         {
            return(2);
         } else
            return(0);
   } else
      return(4);
}
BOOL isfloppy(BSTR bp)
{
   UBYTE count;
   UBYTE *p;

   p=(UBYTE *)(bp << 2);
   count=*p;
   D( kprintf("isfloppy() count = %ld ",count) );
   if(count!=3) return(FALSE);
   if((p[1]!='D')&&(p[1]!='d')) return(FALSE);
   if((p[2]!='F')&&(p[2]!='f')) return(FALSE);
   if((p[3]!='0')&&(p[3]!='1')&&(p[3]!='2')&&(p[3]!='4')) return(FALSE);
   D( kprintf("isfloppy() MATCH") );
   return(TRUE);
}
void DisSprite(struct World *world,int num)
{
   UWORD *k;

   k    =        GfxBase->copinit->sprstrtup + 4*num;
   k[1] = (long)(GfxBase->copinit->sprstop) >>16;
   k[3] = (long)(GfxBase->copinit->sprstop);
}
BOOL PartExists(struct World *world,char *s)
{
   struct BootNode *bn;
   struct DeviceNode *dn;
   struct FileSysStartupMsg *fssm;
   struct DosEnvec          *ev;

   D( kprintf("Begin MountList Search\n") );
   for(bn=(struct BootNode *)ExpansionBase->MountList.lh_Head;
       bn->bn_Node.ln_Succ;
       bn=(struct BootNode *)bn->bn_Node.ln_Succ)
   {
      D( printf("Node: ") );
      if(bn->bn_DeviceNode)
      {
         dn=(struct DeviceNode *)bn->bn_DeviceNode;
         fssm=(struct FileSysStartupMsg *)(((ULONG)dn->dn_Startup) << 2);
         ev=(struct DosEnvec *)(((ULONG)fssm->fssm_Environ) << 2);

         if(bn->bn_Node.ln_Type == NT_BOOTNODE)
         {
            D( printf("checking bootnode\n"); )
            if( ! isfloppy(dn->dn_Name))
            {
               /*NoPart=FALSE;*/
               if(MatchBSTR(dn->dn_Name,s))
               {
                  return(TRUE);
               }
            }
         }

         D( printbstr(dn->dn_Name) );
         D( printf(" Pri=%ld",bn->bn_Node.ln_Pri) );
         D( printf(" DOSType=%lx",(ULONG)ev->de_DosType) );
         D( printf(" BP=%ld",ev->de_BootPri) );
         D( if(bn->bn_Node.ln_Type & NT_BOOTNODE) printf(" BOOTABLE"); )
         D( printf(" NodeType=0x%lx\n",bn->bn_Node.ln_Type); )
      }
      D( printf("\n") );
   }
   D( kprintf("End MountList Search\n") );
   return(FALSE);
}
BOOL MatchBSTR(BSTR bp,char *s)
{
   UBYTE bcount,scount;
   UBYTE *p;

   p=(UBYTE *)(bp << 2);
   bcount=*p;
   scount=strlen(s);

   D( printf("MatchPart("); )
   D( printbstr(bp); )
   D( kprintf(",%s) enter\n",s) );

   if(bcount!=scount) return(FALSE);

   /* D( kprintf("isPart() good length\n") ); */

/*   if(strnicmp(&p[1],"WB_1.3",6)!=NULL) */
   if(strnicmp(&p[1],s,scount)!=NULL)
   {
      D( kprintf("MatchPart() FAIL\n") );
      return(FALSE);
   }

   D( kprintf("MatchPart() MATCH\n") );
   return(TRUE);
}
