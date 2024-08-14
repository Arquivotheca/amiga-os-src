#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/expansionbase.h>
#include <proto/all.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/view.h>
#include <graphics/sprite.h>
#include <libraries/filehandler.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <devices/input.h>

void kprintf(char *,...);
void sprintf(char *,char *,...);
#define  D(x) ;
#define printf kprintf

#define DEPTH     2
#define WIDTH     640
#define HEIGHT    200
#define VPMODES   HIRES|SPRITES

#define ALLOC_SPRITE 1

#define T0W       24                   /* gadget 0 width */
#define T1W       40                   /* gadget 1 width */
#define T2W       24                   /* gadget 2 width */
#define T3W       10                   /* gadget 3 width */

#define G0W       48                   /* gadget 0 width */
#define G1W       96                   /* gadget 1 width */
#define G2W       56                   /* gadget 2 width */
#define G3W       20                   /* gadget 3 width */

#define GYS       30                   /* gadget y start   */
#define GYSP      15                   /* gadget y spacing */
#define GXSP      20                   /* gadget x spacing */
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

#define CBTW      64                   /* cancel button text witdh    */
#define CBTH      8                    /* cancel button text height   */
#define CBGW      (CBTW+10)            /* cancel button gadget width  */
#define CBGH      13                   /* cancel button gadget height */
#define CBGX      ((WIDTH-1-CBGW)>>1)  /* cancel button gadget x      */
#define CBGY      185                  /* cancel button gadget y      */
#define CBTX      (CBGX+((CBGW-CBTW)>>1))/* cancel button gadget x      */
#define CBTY      (CBGY+CBTH+((CBGH-CBTH)>>1)-1)/* cancel button gadget y     */
#define CBGN      (100)                /* cancel button gadget number */

#define UP_DELAY  15    /* 60ths of a second after a gadget is released */
                        /* and before the screen closes                 */

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

struct MyGadget {
   BOOL  HighLighted;
   BOOL  RolledOff;
   BOOL  Enabled;
   struct BootNode *node;
   };

struct World {
   struct GfxBase       *GfxBase;
   struct ExpansionBase *ExpansionBase;
   struct Display       display;
   struct IH            ih;
   struct Sprite        sprite;
   struct MyGadget      gadget[20];
   struct Task          *maintask;
   LONG                 signal;
   SHORT                nodes;   /* Number of BOOTNODES found */
   SHORT                gad;     /* gad currently depressed or 0 */
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
BOOL   Activated(struct World *);
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
void   DoData(struct World *);
void   DrawStatus(struct World *,SHORT,SHORT,BOOL);
void   DrawWord(struct World *,SHORT,SHORT,SHORT);
void   DrawByte(struct World *,SHORT,SHORT,BYTE);
void   DrawChain(struct Wolrd *,SHORT);
SHORT  GadgetHit(struct World *);
void   ComplementGadget(struct World *,SHORT);

int MyInit()
{
   struct Library *library;
   struct World   *world;
   struct World   myworld;

   D( kprintf("###################\n") );
   D( kprintf("syscheck Init Begins\n") );

   world=&myworld;
   world->signal=-1;

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

   if( ! Activated(world) ) goto out;

   world->signal=AllocSignal(-1);
   world->maintask=FindTask(0);

   DisplayOn(world);
   D( kprintf("After DisplayOn()\n") );

   DoData(world);

   DoImagery(world);
   D( kprintf("After DoImagery()\n") );

   OpenSprite(world);
   D( kprintf("After OpenSprite()\n") );

   OpenIH(world);
   D( kprintf("After OpenIH()\n") );

   Wait(1<<world->signal);

   CloseIH(world);
   D( kprintf("After CloseIH()\n") );

   CloseSprite(world);
   D( kprintf("After CloseSprite()\n") );

   DisplayOff(world);

out:
   D( kprintf("syscheck exiting\n") );
   if(world->signal != -1)  FreeSignal(world->signal);
   if(world->ExpansionBase) CloseLibrary((struct Library *)world->ExpansionBase);
   if(world->GfxBase)       CloseLibrary((struct Library *)world->GfxBase);
   return(0);
}

#define GfxBase       (world->GfxBase)
#define ExpansionBase (world->ExpansionBase)

BOOL Activated(world)
struct World *world;
{
   D( kprintf("Activate Found Bad Memory!\n") );
   if(ExpansionBase->Flags & EBF_BADMEM)
      return(TRUE);

   /* Is our magic mouse/key sequence being pressed ? */
   /* if not then just exit */
#if 1
   return(FALSE);
#else
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
#endif
}
void DoImagery(world)
struct World *world;
{
   SHORT x,tl;

   SetAPen(&world->display.rp,3);
   SetDrMd(&world->display.rp,JAM1);
   SetFont(&world->display.rp,world->display.font9);

   tl=TextLength(&world->display.rp,"System Expansion Board Check",28);
   Move(&world->display.rp,(WIDTH-1-tl)>>1,8);
   Text(&world->display.rp,"System Expansion Board Check",28);

   SetFont(&world->display.rp,world->display.font8);

   Move(&world->display.rp,C0GX,LY);
   Text(&world->display.rp,"Status",6);

   Move(&world->display.rp,C1GX,LY);
   Text(&world->display.rp,"Manufacturer",12);

   Move(&world->display.rp,C2GX,LY);
   Text(&world->display.rp,"Product",7);

/*
   Move(&world->display.rp,C3GX,LY);
   Text(&world->display.rp,"Pri",3);
*/

   SetAPen(&world->display.rp,2);
   Move(&world->display.rp,CBTX,CBTY);
   Text(&world->display.rp,"CONTINUE",8);
   DrawBox(world,CBGX,CBGY,CBGW,CBGH,1);

   for(x=0;x<world->nodes;x++)
   {
      DrawBox(world,C0GX,(SHORT)((x*GYSP)+GYS),G0W,GH,0);
/*
      Move(&world->display.rp,C0TX,(SHORT)((x*GYSP)+TYS));
      Text(&world->display.rp,"Enabled",7);
*/
      world->gadget[x+10].Enabled=TRUE;
      world->gadget[x+10].HighLighted=FALSE;
      world->gadget[x+10].RolledOff=FALSE;

      DrawBox(world,C1GX,(SHORT)((x*GYSP)+GYS),G1W,GH,0);
      world->gadget[x].Enabled=FALSE;
      world->gadget[x].HighLighted=FALSE;
      world->gadget[x].RolledOff=FALSE;

      DrawBox(world,C2GX,(SHORT)((x*GYSP)+GYS),G2W,GH,0);
/*
      DrawBox(world,C3GX,(SHORT)((x*GYSP)+GYS),G3W,GH,0);
*/
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
}
BOOL DisplayOn(world)
struct World *world;
{
   USHORT colortable[]={0x800,0xfff,0x000,0xec3,
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
      OFF_DISPLAY

      CloseFont(world->display.font8);
      CloseFont(world->display.font9);
      LoadView(0);
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
   MoveSprite(0,GfxBase->SimpleSprites[0],world->sprite.x>>1,world->sprite.y);
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
/*
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
            for(i=0;i<UP_DELAY;i++)
               WaitTOF();
            Signal(world->maintask,1<<world->signal);
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
            if(world->gadget[gad-1].Enabled)
            {
               DrawBox(world,C1GX,(SHORT)(((gad-1-10)*GYSP)+GYS),G1W,GH,1);
            } else {
               DrawBox(world,C1GX,(SHORT)(((gad-1-10)*GYSP)+GYS),G1W,GH,0);
            }
            world->gad=0;
            break;
*/
      case CBGN:
            for(i=0;i<UP_DELAY;i++)
               WaitTOF();
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
   SHORT k;
   UWORD *p;

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
#else
   MoveSprite(0,GfxBase->SimpleSprites[0],30,30);
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
void DoData(world)
struct World *world;
{
   struct ConfigDev *cd=NULL;

   world->nodes=0;
   SetDrMd(&world->display.rp,JAM1);
   SetFont(&world->display.rp,world->display.font8);

   D( kprintf("Begin ConfigDev Search\n") );
   while(cd=FindConfigDev((long)cd,-1,-1))
   {
      D( printf("ConfigDev Node found: ") );
      if(world->nodes == 10) break;
      world->nodes+=1;

      D( printf("BoardAddress = %lx",cd->cd_BoardAddr) );
      D( printf(" BoardSize = %ld",cd->cd_BoardSize) );
      D( printf(" SlotAddr = %lx",cd->cd_SlotAddr) );
      D( printf(" SlotSize = %ld",cd->cd_SlotSize) );
      D( printf(" Man = %ld",cd->cd_Rom.er_Manufacturer) );
      D( printf(" Prod = %ld",cd->cd_Rom.er_Product) );
      D(
         if(cd->cd_Rom.er_Type & ERTF_CHAINEDCONFIG)
            printf(" Chanied");
      )
      D( printf("\n"); )

      DrawStatus(world,C0TX,(SHORT)(((world->nodes-1)*GYSP)+TYS),(BOOL)(cd->cd_Flags&CDF_BADMEMORY));
      DrawWord(world,C1TX,(SHORT)(((world->nodes-1)*GYSP)+TYS),cd->cd_Rom.er_Manufacturer);
      DrawByte(world,C2TX,(SHORT)(((world->nodes-1)*GYSP)+TYS),cd->cd_Rom.er_Product);
      if(cd->cd_Rom.er_Type & ERTF_CHAINEDCONFIG)
         DrawChain(world,world->nodes);
   }
   D( kprintf("End ConfigDev Search\n") );
}
void DrawStatus(world,x,y,state)
struct World *world;
SHORT x,y;
BOOL state;
{
   if(state)
   {
      Move(&world->display.rp,x,y);
      Text(&world->display.rp,"BAD",3);
   } else {
      Move(&world->display.rp,x,y);
      Text(&world->display.rp," OK",3);
   }
}
void DrawWord(world,x,y,value)
struct World *world;
SHORT x,y;
SHORT value;
{
   char buf[6];

   sprintf(buf,"%5ld",(ULONG)value);
   Move(&world->display.rp,x,y);
   Text(&world->display.rp,buf,5);
}
void DrawByte(world,x,y,value)
struct World *world;
SHORT x,y;
BYTE value;
{
   char buf[4];

   sprintf(buf,"%3ld",(ULONG)value);
   Move(&world->display.rp,x,y);
   Text(&world->display.rp,buf,3);
}
void DrawChain(world,node)
struct World *world;
SHORT node;
{
   Move(&world->display.rp,C3GX,    (SHORT)(((node-1)*GYSP)+GYS+(GH>>1)));
   Draw(&world->display.rp,C3GX+G3W,(SHORT)(((node-1)*GYSP)+GYS+(GH>>1)));
   Draw(&world->display.rp,C3GX+G3W,(SHORT)(((node)*GYSP)+  GYS+(GH>>1)));
   Draw(&world->display.rp,C3GX,    (SHORT)(((node)*GYSP)+  GYS+(GH>>1)));
}
SHORT GadgetHit(world)
struct World *world;
{
   if((world->sprite.x <= CBGX+CBGW-1) && (world->sprite.x >= CBGX) &&
      (world->sprite.y <= CBGY+CBGH-1) && (world->sprite.y >= CBGY) )
   {
      return(CBGN);
   }
   return(0);
}
void ComplementGadget(world,gad)
struct World *world;
SHORT gad;
{
/*   SHORT y;*/

   SetDrMd(&world->display.rp,COMPLEMENT);
   switch(gad)
   {
      case CBGN:
            RectFill(&world->display.rp,CBGX,CBGY,CBGX+CBGW-1,CBGY+CBGH-1);
            break;
/*
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
            Move(&world->display.rp,C0TX,(SHORT)(((gad-11)*GYSP)+TYS));
            SetAPen(&world->display.rp,2);
            SetBPen(&world->display.rp,0);
            SetDrMd(&world->display.rp,JAM2);
            if(world->gadget[gad-1].Enabled)
            {
               Text(&world->display.rp,"Disabled",8);
               world->gadget[gad-1].Enabled=FALSE;
            } else {
               Text(&world->display.rp,"Enabled ",8);
               world->gadget[gad-1].Enabled=TRUE;
            }
            break;
*/
      default:
            break;
   }
}
