#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/expansionbase.h>
#include <libraries/filehandler.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <proto/all.h>
#include <string.h>

void kprintf(char *,...);
void sprintf(char *,char *,...);
#define  D(x) ;
#define  D1(x) ;
#define printf kprintf

typedef int (*PFI)(void);

PFI MyFunc13= (PFI)(0x7F80000L+0x40000L+8L+4L);
PFI MyFunc14= (PFI)(0x7F80000L+0x7F000L+8L+4L);

LONG  *Config13  = (LONG *)(0x7F80000L+0x40000L+8L);
LONG  *Config14  = (LONG *)(0x7F80000L+0x7F000L+8L);

#define DEPTH     2
#define WIDTH     640
#define HEIGHT    200
#define VPMODES   HIRES|SPRITES

   USHORT colortable[]={0x777,0xfff,0x000,0xec3,
                        0,0,0,0,
                        0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,};

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

struct World {
   struct ExpansionBase *ExpansionBase;
   struct DosLibrary    *DOSBase;
   struct IntuitionBase *IntuitionBase;
   struct GfxBase       *GfxBase;
   struct Display       display;
   };

int    MyInit(void);
BOOL   DisplayOn(struct World *);
void   DisplayOff(struct World *);
void   printbstr(BSTR);
BOOL   isfloppy(BSTR);
void   GetData(struct World *);
BOOL   ReadFile(struct World *world,char *name,ULONG offset,ULONG count,char *address);
VOID   ShowMessage(struct World *world,char *s);
VOID   ShowError(struct World *world,char *s);

int MyInit()
{
   struct Library *library;
   struct World   *world;
   struct World   myworld;
   char           *mymem;

   D( kprintf("###################\n") );
   D( kprintf("kickad Init Begins\n") );

   world=&myworld;

   world->IntuitionBase=0;
   world->DOSBase=0;
   world->ExpansionBase=0;
   world->GfxBase=0;

   library = OpenLibrary("expansion.library",0);
   if(library==0)
   {
      D( kprintf("No expansion.library\n") );
      goto out;
   }
   world->ExpansionBase=(struct ExpansionBase *)library;

   library = OpenLibrary("dos.library",0);
   if(library==0)
   {
      D( kprintf("No dos.library\n") );
      goto out;
   }
   world->DOSBase=(struct DosLibrary *)library;

   library = OpenLibrary("intuition.library",36);
   if(library==0)
   {
      D( kprintf("No intuition.library\n") );
      goto out;
   }
   world->IntuitionBase=(struct IntuitionLibrary *)library;

   library = OpenLibrary("graphics.library",36);
   if(library==0)
   {
      D( kprintf("No graphics.library\n") );
      goto out;
   }
   world->GfxBase=(struct GfxBase *)library;

   if(world->ExpansionBase->Flags & EBF_KICKBACK33)
   {
         D( printf("kickad Kicking WB_1.3\n"); )
         DisplayOn(world);

         world->ExpansionBase->Flags &= ~EBF_KICKBACK33;

         mymem=AllocAbs(0x80000L,0x7f80000L);

         D1( printf("mymem=0x%lx\n",mymem); )

         if( ! mymem )
            ShowError(world,"Error can't get memory!");

         D1( printf("Clearing memory..."); )
         { LONG x; for(x=0;x<0x80000;x++) { ((char *)mymem)[x]=0; } }
         D1( printf("done.\n"); )

         D1( printf("Reading 1.3 Kickstart..."); )
         ShowMessage(world,"Loading Kickstart 1.3");
         if( ! ReadFile(world,"WB_1.3:devs/kickstart",0L,0x40000L,mymem))
         {
            D( kprintf("\nError reading 1.3 kickstart file\n") );
            ShowError(world,"Error reading 1.3 Kickstart file! Reboot system.");
            goto out;
         }
         D1( printf("done.\n"); )

         D1( printf("Reading bonus Image..."); )
         /* ShowMessage(world,"Reading Bonus image."); */
         if(  ReadFile(world,"WB_1.3:devs/kickstart",0x40000L,0x1ABFFL,(char *)(mymem+0x40000L)))
         {
            D( kprintf("\nError reading bonus file\n") );
            ShowError(world,"Error reading Bonus file. Reboot system.");
            goto out;
         }
         D1( printf("done.\n"); )

         ShowMessage(world,"Rebooting system.");
         *Config13=2;
         (*MyFunc13)();
         ShowError(world,"Error MMU routine returned! Please reboot system.");
   } else
      if(world->ExpansionBase->Flags & EBF_KICKBACK36)
      {
         D( printf("kickad Kicking WB_2.x\n"); )
         DisplayOn(world);

         world->ExpansionBase->Flags &= ~EBF_KICKBACK36;

         mymem=AllocAbs(0x80000L,0x7f80000L);

         D1( printf("mymem=0x%lx\n",mymem); )

         if( ! mymem )
            ShowError(world,"Error can't get memory!");

         D1( printf("Clearing memory..."); )
         { LONG x; for(x=0;x<0x80000;x++) { ((char *)mymem)[x]=0; } }
         D1( printf("done.\n"); )

         D1( printf("Reading 2.x Kickstart Image..."); )
         ShowMessage(world,"Loading Kickstart 2.x");
         if( ! ReadFile(world,"WB_2.x:devs/kickstart",0L,0x80000L,mymem))
         {
            D( kprintf("\nError reading 2.x kickstart file\n") );
            ShowError(world,"Error reading 2.x Kickstart file. Reboot system.");
            goto out;
         }
         D1( printf("done.\n"); )

         D1( printf("Reading Bonus2.x Image..."); )
         /* ShowMessage(world,"Reading Bonus2.x file."); */
         if(  ReadFile(world,"WB_2.x:devs/kickstart",0x80000L,0x1000L,(char *)(mymem+0x7F000L)))
         {
            D( kprintf("\nError Bonus2.x.FFF000 file\n") );
            ShowError(world,"Error reading Bonus2.x file. Reboot system.");
            goto out;
         }
         D1( printf("done.\n"); )
         ShowMessage(world,"Rebooting system.");
         *Config14=2;
         (*MyFunc14)();
         ShowError(world,"Error MMU routine returned! Please reboot system.");
      } else {
         D( printf("kickad No Flag Set, exiting...\n"); )
         goto out;
      }

out:
   D( kprintf("kickad exiting\n") );
   if(world->IntuitionBase) CloseLibrary((struct Library *)world->IntuitionBase);
   if(world->GfxBase)       CloseLibrary((struct Library *)world->GfxBase);
   if(world->DOSBase)       CloseLibrary((struct Library *)world->DOSBase);
   if(world->ExpansionBase) CloseLibrary((struct Library *)world->ExpansionBase);
   return(0);
}

#define ExpansionBase (world->ExpansionBase)
#define IntuitionBase (world->IntuitionBase)
#define DOSBase       (world->DOSBase)
#define SYSBase       (world->DOSBase)
#define GfxBase       (world->GfxBase)

BOOL DisplayOn(world)
struct World *world;
{
   UBYTE *displaymem;
   UWORD *colorpalette;
   LONG i;

   D( printf("DisplayOn() enter\n"); )

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
#if 0
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
BOOL isfloppy(bp)
BSTR bp;
{
   UBYTE count;
   UBYTE *p;

   p=(UBYTE *)(bp << 2);
   count=*p;
   D1( kprintf("isfloppy() count = %ld ",count) );
   if(count!=3) return(FALSE);
   if((p[1]!='D')&&(p[1]!='d')) return(FALSE);
   if((p[2]!='F')&&(p[2]!='f')) return(FALSE);
   if((p[3]!='0')&&(p[3]!='1')&&(p[3]!='2')&&(p[3]!='4')) return(FALSE);
   D1( kprintf("isfloppy() MATCH") );
   return(TRUE);
}
void GetData(world)
struct World *world;
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
               bn->bn_Node.ln_Type=0x00;
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
}
#endif
BOOL ReadFile(struct World *world,char *name,ULONG offset,ULONG count,char *address)
{
   BPTR        fh=NULL;
   ULONG       actual;
   BOOL        retval=FALSE;

   D1( printf("\nReadFile(world=0x%lx,name=%s,offset=0x%lx,count=0x%lx,address=0x%lx) enter\n",world,name,offset,count,address); )

   fh=Open(name,MODE_OLDFILE);
   if(fh==NULL)
   {
      D( printf("Could not open file\n"); )
      goto out;
   }
   D1( printf("File = %lx\n",fh); )

   if(offset != 0)
   {
      if(Seek(fh,offset,-1)==-1)
         ShowError(world,"SeekError!");
   }

   if ((actual=Read(fh,address,count))!=count)
   {
      D( printf("Read length error\n"); )
      goto out;
   }
   retval=TRUE;
out:
   if(fh) Close(fh);
   D1( printf("ReadFile() returns %lx\n",retval); )
   return(retval);
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
