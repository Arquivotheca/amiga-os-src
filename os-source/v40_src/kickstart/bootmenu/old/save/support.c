#include "support.h"

#if 1 
struct World {                      /* short fake definition of World */
   struct GfxBase       *GfxBase;   /* contains only common strucs for*/
   struct ExpansionBase *ExpansionBase;/* support.c */
   struct Display       display;
   struct IH            ih;
   struct Sprite        sprite;
   };
#endif


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

#define GfxBase       (world->GfxBase)
#define ExpansionBase (world->ExpansionBase)

BOOL DisplayOn(struct World *world, USHORT c0, USHORT c1, USHORT c2, USHORT c3)
{
   USHORT colortable[]={0x777,0xfff,0x000,0xec3,
                        0,0,0,0,
                        0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,};
   UBYTE *displaymem;
   UWORD *colorpalette;
   LONG i;

/*
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
*/

   colortable[0]=c0;
   colortable[1]=c1;
   colortable[2]=c2;
   colortable[3]=c3;

   world->display.font8=OpenFont(&ta8);
   world->display.font9=OpenFont(&ta9);

   D(
   if(world->display.font8==0 || world->display.font9==0)
      kprintf("No font!!!\n") );

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

   SetBPen(&world->display.rp,0); 
   SetDrMd(&world->display.rp,JAM1); 

   return(TRUE);
}
void DisplayOff(world)
struct World *world;
{
   LONG i;

   if(world->display.Success)
   {
      WaitTOF();
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
void OpenSprite(world)
struct World *world;
{
   SHORT k;
   UWORD *p;

   ON_SPRITE

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

   /****************************/
   /* Disable Intuition sprite */
   /****************************/
   p    =        GfxBase->copinit->sprstrtup + 4; /* 4*num num==0 */
   p[1] = (long)(GfxBase->copinit->sprstop) >>16;
   p[3] = (long)(GfxBase->copinit->sprstop);

   /*****************************/
   /* Remember Initial Position */
   /*****************************/
   world->sprite.x=30;
   world->sprite.y=30;
}
void CloseSprite(world)
struct World *world;
{
   FreeSprite(world->sprite.num);
   FreeMem(world->sprite.image,19*4);
}
BOOL OpenIH(struct World *world,APTR mycode)
{
   if((world->ih.port=CreateMsgPort())==NULL)
   {
      D( kprintf(" Could not create port\n") );
      return(FALSE);
   }

   if((world->ih.msg=CreateIORequest(world->ih.port,
           sizeof(struct IOStdReq)))==NULL)
   {
      D( kprintf(" Could not create StdIO\n") );
      DeleteMsgPort(world->ih.port);
      return(FALSE);
   }
   world->ih.hstuff.is_Data=(APTR)world;
   world->ih.hstuff.is_Code=(VOID (*))mycode;
   world->ih.hstuff.is_Node.ln_Pri=51;

   if(OpenDevice("input.device",0,(struct IORequest *)world->ih.msg,0))
   {
      D( kprintf(" Could not create StdIO\n") );
      DeleteIORequest(world->ih.msg);
      DeleteMsgPort(world->ih.port);
      return(FALSE);
   }

   world->ih.msg->io_Command=IND_ADDHANDLER;
   world->ih.msg->io_Data=(APTR)&world->ih.hstuff;
   DoIO((struct IORequest *)world->ih.msg);
}
void CloseIH(struct World *world)
{
   world->ih.msg->io_Command=IND_REMHANDLER;
   world->ih.msg->io_Data=(APTR)&world->ih.hstuff;
   DoIO((struct IORequest *)world->ih.msg);
   CloseDevice((struct IORequest *)world->ih.msg);
   DeleteIORequest(world->ih.msg);
   DeleteMsgPort(world->ih.port);
}
void DrawBox(struct World *world,SHORT l, SHORT t, SHORT w, SHORT h, SHORT m)
{

   SetAPen(&world->display.rp,m ? 1 : 2 );
   Move(&world->display.rp,l,t);
   Draw(&world->display.rp,l,t+h-1);
   Draw(&world->display.rp,l+1,t+h-2);
   Draw(&world->display.rp,l+1,t);
   Draw(&world->display.rp,l+w-1,t);

   SetAPen(&world->display.rp,m ? 2 : 1 );
   Move(&world->display.rp,l+1,t+h-1);
   Draw(&world->display.rp,l+w-2,t+h-1);
   Draw(&world->display.rp,l+w-2,t+1);
   Draw(&world->display.rp,l+w-1,t);
   Draw(&world->display.rp,l+w-1,t+h-1);
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

   /*D( kprintf("No Button x = %ld, y = %ld\n",
                world->sprite.x,world->sprite.y) );*/

   MoveSprite(0,&world->sprite.sprite,world->sprite.x>>1,world->sprite.y);
}
