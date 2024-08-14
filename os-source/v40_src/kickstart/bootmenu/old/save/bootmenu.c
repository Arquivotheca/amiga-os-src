#include "support.h"
#include <string.h>

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
   SHORT				MaxBootPage; /* Max page number for bootable nodes*/
   struct List          NodeList;
   SHORT                Nodes;      /* Number of EB_Mounlist nodes */
   SHORT                MaxGad;     /* current Max Gadgets on screen */
   SHORT                FirstGad;   /* node number of first displayed node*/
   SHORT                SS_Disable;
   BOOL                 Cancel;
   SHORT                BootNodes; /* total number of bootable nodes */
   BOOL					AdvancedMode; /* TRUE if advanvced options */
   };

USHORT GhostPattern[] = {
      0x8888,
      0x2222
   };


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

#define G0W       102                  /* gadget 0 width */
#define G1W       254                  /* gadget 1 width */
#define G2W       44                   /* gadget 2 width */
#define G3W       44                   /* gadget 3 width */
#define G4W       92                   /* gadget 3 width */
#define G5W       36                   /* gadget 3 width */

#define T0W       64                   /* gadget 0 width */
#define T1W       240                  /* gadget 1 width */
#define T2W       32                   /* gadget 2 width */
#define T3W       32                   /* gadget 3 width */
#define T4W       80                   /* gadget 3 width */
#define T5W       24                   /* gadget 3 width */

#define GYS       30                   /* gadget y start   */
#define GYSP      15                   /* gadget y spacing */
#define GXSP      10                   /* gadget x spacing */
#define GH        13                   /* gadget height    */
#define TH        8                    /* text height      */
#define TYS       (GYS+((GH-TH)>>1)+TH-1)/* text   y start   */

#define C0GX      ((WIDTH-1-(G0W+GXSP+G1W+GXSP+G2W+GXSP+G3W+GXSP+G4W+GXSP+G5W))>>1)
#define C0TX      (C0GX+((G0W-T0W)>>1))

#define C1GX      (C0GX+G0W+GXSP)      
#define C1TX      (C1GX+((G1W-T1W)>>1))

#define C2GX      (C1GX+G1W+GXSP)      
#define C2TX      (C2GX+((G2W-T2W)>>1))

#define C3GX      (C2GX+G2W+GXSP)      
#define C3TX      (C3GX+((G3W-T3W)>>1))

#define C4GX      (C3GX+G3W+GXSP)      
#define C4TX      (C4GX+((G4W-T4W)>>1))

#define C5GX      (C4GX+G4W+GXSP)      
#define C5TX      (C5GX+((G5W-T5W)>>1))

#define LY        27                   /* Label y pos */

/************************************/
/* First Page main button constants */
/************************************/
#define FP0GX      ((WIDTH-1-(G1W))>>1)
#define FP0TX      (FP0GX+((G1W-T1W)>>1))

/**************************************************************************/
/*																		  */
/* Position and size definitions for Advanced Options Page command buttons*/
/*																		  */
/* TW = Text Width in pixels											  */
/* TH = Text Height in pixels											  */
/* GW = Gadget Width in pixels											  */
/* GH = Gadget Height in pixels											  */
/* GX = Gadget X location in pixels										  */
/* GY = Gadget Y location in pixels										  */
/* TX = Text X location in pixels										  */
/* TY = Text Y location in pixels										  */
/* GN = Gadget Number, internal gadget identification number returned by  */
/*	    GadgetHit();													  */
/*																		  */
/**************************************************************************/

/***********************************************/
/* Startup Sequence Enable/Disable NWay button */
/***********************************************/
#define DATW      64
#define DATH      8 
#define DAGW      (DATW+14+24)
#define DAGH      13  
#define DAGX      176
#define DAGY      185  
#define DATX      (DAGX+((DAGW-DATW)>>1))
#define DATY      (DAGY+DATH+((DAGH-DATH)>>1)-1)
#define DAGN      (101) 

#define STARTX    DAGX-136-10
#define STARTY    185+8+((13-8)>>1)-1

/**************/
/* USE button */
/**************/
#define GOTW      24  
#define GOTH      8
#define GOGW      82 
#define GOGH      13 
#define GOGX      DAGX+DAGW+10
#define GOGY      185    
#define GOTX      (GOGX+((GOGW-GOTW)>>1))
#define GOTY      (GOGY+GOTH+((GOGH-GOTH)>>1)-1)
#define GOGN      (102) 

/********************/
/* NEXT PAGE button */
/********************/
#define NPTW      72  
#define NPTH      8  
#define NPGW      (NPTW+10) 
#define NPGH      13   
#define NPGX      GOGX+GOGW+10
#define NPGY      185  
#define NPTX      (NPGX+((NPGW-NPTW)>>1))
#define NPTY      (NPGY+NPTH+((NPGH-NPTH)>>1)-1)
#define NPGN      (103) 

/**************************************************************************/
/*																		  */
/* Position and size definitions for First Page command buttons			  */
/*																		  */
/* TW = Text Width in pixels											  */
/* TH = Text Height in pixels											  */
/* GW = Gadget Width in pixels											  */
/* GH = Gadget Height in pixels											  */
/* GX = Gadget X location in pixels										  */
/* GY = Gadget Y location in pixels										  */
/* TX = Text X location in pixels										  */
/* TY = Text Y location in pixels										  */
/* GN = Gadget Number, internal gadget identification number returned by  */
/*	    GadgetHit();													  */
/*																		  */
/**************************************************************************/

/****************************/
/* First Page CANCEL button */
/****************************/
#define FPCBTW    48 
#define FPCBTH    8  
#define FPCBGW    82 
#define FPCBGH    13 
#define FPCBGX    151 
#define FPCBGY    185    
#define FPCBTX    (FPCBGX+((FPCBGW-FPCBTW)>>1))
#define FPCBTY    (FPCBGY+FPCBTH+((FPCBGH-FPCBTH)>>1)-1)
#define FPCBGN    (104) 

/**************************************/
/* First Page Advanced Options button */
/**************************************/
#define FPAOTW    152  
#define FPAOTH    8    
#define FPAOGW    (FPAOTW+10)  
#define FPAOGH    13 
#define FPAOGX    FPCBGX+FPCBGW+10/*(((WIDTH-1-CBGW)>>1)+74)*/
#define FPAOGY    185 
#define FPAOTX    (FPAOGX+((FPAOGW-FPAOTW)>>1))
#define FPAOTY    (FPAOGY+FPAOTH+((FPAOGH-FPAOTH)>>1)-1)
#define FPAOGN    (105) 

/*******************************/
/* First Page NEXT PAGE button */
/*******************************/
#define FPNPTW    72 
#define FPNPTH    8  
#define FPNPGW    (FPNPTW+10)  
#define FPNPGH    13      
#define FPNPGX    FPAOGX+FPAOGW+10 
#define FPNPGY    185  
#define FPNPTX    (FPNPGX+((FPNPGW-FPNPTW)>>1))
#define FPNPTY    (FPNPGY+FPNPTH+((FPNPGH-FPNPTH)>>1)-1)
#define FPNPGN    (106)  

#define UP_DELAY  15    /* 60ths of a second after a gadget is released */
                        /* and before the screen closes                 */

#define PAGE_SIZE 10    /* Number of nodes per page                     */

/*************************************/
/* Location for 'Page xx of nn' text */
/*************************************/
#define PAGEX     268
#define PAGEY     17    

/**************************************************************************/
/*																		  */
/* Function Prototypes													  */
/*																		  */
/**************************************************************************/
int    MyInit(void);
D( void   printbstr(BSTR); )
void   DoImagery(struct World *);
struct InputEvent * __asm HandlerInterface(
                             register __a0 struct InputEvent *ev,
                             register __a1 struct World *world);
void   GadgetUpAction(struct World *);
void   GetData(struct World *);
struct MyNode *GetNode(struct World *world,SHORT num);
/* void   FreeData(struct World *);  */
void   DrawBSTR(struct World *,BSTR,SHORT,SHORT,LONG);
SHORT  GadgetHit(struct World *);
void   ComplementGadget(struct World *,SHORT);
void   MakeChanges(struct World *world);
BOOL   DrawEntry(struct World *world,SHORT x);
BOOL   DrawFPEntry(struct World *world,SHORT x);
VOID   DrawNWay(struct World *world,SHORT x,SHORT y,BOOL enabled,BOOL sunken);
void   DrawByte(struct World *,SHORT,SHORT,UBYTE);
void   MyDelay(struct World *world,SHORT n);

int MyInit()
{
   struct Library *library;
   struct World   *world;
   struct World   myworld;
   struct Node    *n;

   D( kprintf("###################\n") );
   D( kprintf("bootmenu Init Begins\n") );

   world=&myworld;
   world->GfxBase=NULL;
   world->ExpansionBase=NULL;
   world->signal=-1;
   world->AdvancedMode=FALSE;

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

   /* if( ! MouseActivated(world) ) goto out; */

   /* Is our magic mouse/key sequence being pressed ? */
   /* if not then just exit */
   if( (1<<6) & *((char *)0xBFE001) )
   {
      D( kprintf("No Left button. Exiting\n") );
      goto out;
   }

   *((UWORD *)0xDFF034) = 0xffff;   /* set up port to read RMB */
   MyDelay(world,2);
   /*
   WaitTOF();
   WaitTOF();
   */
   if( (1<<10) & *((UWORD *)0xDFF016) )
   {
      D( kprintf("No Right button. Exiting\n") );
      goto out;
   }

   world->signal=AllocSignal(-1);
   world->maintask=FindTask(0);

   DisplayOn(world,0xaaa,0xfff,0x000,0xec4);
   D( kprintf("After DisplayOn()\n") );

   GetData(world);
   world->Cancel=FALSE;

   DoImagery(world);
   D( kprintf("After DoImagery()\n") );

   OpenSprite(world);
   D( kprintf("After OpenSprite()\n") );

   OpenIH(world,HandlerInterface);
   D( kprintf("After OpenIH()\n") );

   Wait(1<<world->signal);

   if( ! world->Cancel)
      MakeChanges(world);

   /* FreeData(world);  */

   while(n=RemHead(&(world->NodeList)))
      FreeMem((char *)n,sizeof(struct MyNode));

   CloseIH(world);
   D( kprintf("After CloseIH()\n") );

   CloseSprite(world);
   D( kprintf("After CloseSprite()\n") );

   DisplayOff(world);

out:
   D( kprintf("Bootmenu exiting\n") );
   if(world->signal != -1) FreeSignal(world->signal);
   if(world->ExpansionBase) CloseLibrary((struct Library *)
                                            world->ExpansionBase);
   if(world->GfxBase) CloseLibrary((struct Library *)world->GfxBase);
   return(0);
}

#define GfxBase       (world->GfxBase)
#define ExpansionBase (world->ExpansionBase)

D( void printbstr(bp)
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
} )
void DoImagery(world)
struct World *world;
{
   SHORT x;
   char buf[20];

   D( printf("init.c: DoImagery(world=0x%lx) enter AdvancedMode=%lx\n",
              world,world->AdvancedMode); )

   SetRast(&world->display.rp,0); 
   SetAPen(&world->display.rp,3);
   SetFont(&world->display.rp,world->display.font9);
   if(world->AdvancedMode)
   {
      /*********/
      /* Title */
      /*********/
      Move(&world->display.rp,179,8);
      Text(&world->display.rp,"Boot Menu - Advanced Options",28);

      SetFont(&world->display.rp,world->display.font8);

      /**********/
      /* Labels */
      /**********/
      Move(&world->display.rp,C0GX,LY);
      Text(&world->display.rp,"Status",6);

      Move(&world->display.rp,C1GX,LY);
      Text(&world->display.rp,"Partition Name",14);

      Move(&world->display.rp,C2GX,LY);
      Text(&world->display.rp,"Type",4);

      Move(&world->display.rp,C3GX,LY);
      Text(&world->display.rp,"Pri",3);

      Move(&world->display.rp,C4GX,LY);
      Text(&world->display.rp,"Device",6);

      Move(&world->display.rp,C5GX,LY);
      Text(&world->display.rp,"Unit",4);

      /****************/
      /* Page Display */
      /****************/
      D( printf("init.c: DoImagery() CurrentPage=%ld MaxPage=%ld\n",
	            world->CurrentPage,world->MaxPage); )

      Move(&world->display.rp,PAGEX,PAGEY);
      sprintf(buf,"Page %2ld of %2ld",world->CurrentPage,world->MaxPage);
      Text(&world->display.rp,buf,13);

      /*******************/
      /* Command Buttons */
      /*******************/
      SetAPen(&world->display.rp,2);

      Move(&world->display.rp,GOTX,GOTY);
      Text(&world->display.rp,"USE",3);
      DrawBox(world,GOGX,GOGY,GOGW,GOGH,1);

      Move(&world->display.rp,NPTX,NPTY);
      Text(&world->display.rp,"NEXT PAGE",9);
      DrawBox(world,NPGX,NPGY,NPGW,NPGH,1);

      SetAPen(&world->display.rp,2);
      SetAfPt(&world->display.rp,GhostPattern,1);

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
   } else {
      /*********/
      /* Title */
      /*********/
      Move(&world->display.rp,274,8);
      Text(&world->display.rp,"Boot Menu",9);

      SetFont(&world->display.rp,world->display.font8);

      /**********/
      /* Labels */
      /**********/
      Move(&world->display.rp,FP0GX,LY);
      Text(&world->display.rp,"Select Boot Partition",21);

      /****************/
      /* Page Display */
      /****************/
      D( printf("init.c: DoImagery() CurrentPage=%ld MaxBootPage=%ld\n",
	            world->CurrentPage,world->MaxBootPage); )

      Move(&world->display.rp,PAGEX,PAGEY);
      sprintf(buf,"Page %2ld of %2ld",world->CurrentPage,world->MaxBootPage);
      Text(&world->display.rp,buf,13);

      /*******************/
      /* Command Buttons */
      /*******************/
      SetAPen(&world->display.rp,2);
      Move(&world->display.rp,FPCBTX,FPCBTY);
      Text(&world->display.rp,"CANCEL",6);
      DrawBox(world,FPCBGX,FPCBGY,FPCBGW,FPCBGH,1);

      Move(&world->display.rp,FPAOTX,FPAOTY);
      Text(&world->display.rp,"Advanced Options...",19);
      DrawBox(world,FPAOGX,FPAOGY,FPAOGW,FPAOGH,1);

      Move(&world->display.rp,FPNPTX,FPNPTY);
      Text(&world->display.rp,"NEXT PAGE",9);
      DrawBox(world,FPNPGX,FPNPGY,FPNPGW,FPNPGH,1);

      SetAPen(&world->display.rp,2);
      SetAfPt(&world->display.rp,GhostPattern,1);

      if(world->MaxBootPage==1)
         RectFill(&world->display.rp,FPNPGX+2,FPNPGY+1,
		                             FPNPGX+FPNPGW-4,FPNPGY+FPNPGH-2);

      SetAfPt(&world->display.rp,NULL,0);

      /****************/
      /* Main Gadgets */
      /****************/
      world->MaxGad=0;
      for(x=0;x<PAGE_SIZE;x++)
      {
         if(DrawFPEntry(world,x))
            world->MaxGad++;
      }
   }
   D( printf("init.c: DoImagery() returns\n"); )
}
struct InputEvent * __asm HandlerInterface(
                        register __a0 struct InputEvent *ev,
                        register __a1 struct World *world)
{
   if(ev->ie_Class==IECLASS_RAWMOUSE)
   {
      UpdateMouse(world,ev);
      /* if world->gad and GadgetHit != world->gad comp world->gad =0 */
      /* if something was selected and mouse is no longer over same gad */
      if(world->gad && (GadgetHit(world)!=world->gad))
      {
         ComplementGadget(world,world->gad);
         world->gad=0;
      }
      switch(ev->ie_Code)
      {
         /*
         case IECODE_RBUTTON:
               D( kprintf("Right Button\n") );
               break;
         */
         case IECODE_LBUTTON:
               D( kprintf("Left Button\n") );
               world->gad=GadgetHit(world);
               if(world->gad)
               {
                  ComplementGadget(world,world->gad);
               }
               break;
         case IECODE_LBUTTON|IECODE_UP_PREFIX:
               D( kprintf("Left Button UP\n") );
               GadgetUpAction(world);
               break;
         /*
         case IECODE_NOBUTTON:
               D( kprintf("No Button x = %ld, y = %ld\n",ev->ie_X,ev->ie_Y));
               break;
         default:
               D( kprintf("Default\n") );
               break;
         */
      }
   }
   return(ev);
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
      /*
      case 0:
            break;
      */
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
			MyDelay(world,UP_DELAY);
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
            mn=GetNode(world,(SHORT)(world->FirstGad+gad-10-1));
            if((! mn->Enabled) && mn->Highlited)
            {
               mn->Highlited=FALSE;
               for(i=0;i<world->Nodes;i++)
               {
                  mn=GetNode(world,i);
                  if(((mn->Enabled && mn->Bootable)||
				      (mn->NoEnable && mn->Bootable)))
                  {
                     mn->Highlited=TRUE;
                     break;
                  }
               }
            }
            DrawEntry(world,(SHORT)(gad-10-1)); 
            /* world->gad=0; */
            break;
      case DAGN:
            ComplementGadget(world,world->gad);
            (world->SS_Disable) ? (world->SS_Disable=FALSE) : (world->SS_Disable=TRUE);
            SetAPen(&world->display.rp,0);
            RectFill(&world->display.rp,DAGX,DAGY,DAGX+DAGW-1,DAGY+DAGH-1);
            DrawNWay(world,DAGX,DAGY,(BOOL)(! world->SS_Disable),FALSE);
            /* world->gad=0; */
            break;
      case GOGN:
            ComplementGadget(world,world->gad);
            /* world->gad=0; */
			world->AdvancedMode=FALSE;
            world->CurrentPage =1;
            DoImagery(world);
			break;
      case NPGN:
            ComplementGadget(world,world->gad);
            /* world->gad=0; */
            world->CurrentPage +=1;
            if(world->CurrentPage > world->MaxPage)
               world->CurrentPage = 1;
            DoImagery(world);
            break;
      case FPCBGN:
	        MyDelay(world,UP_DELAY);
            world->Cancel=TRUE;
            Signal(world->maintask,1<<world->signal);
            break;
      case FPAOGN:
            ComplementGadget(world,world->gad);
            /* world->gad=0; */
			world->AdvancedMode=TRUE;
            world->CurrentPage =1;
            DoImagery(world);
            break;
      case FPNPGN:
            ComplementGadget(world,world->gad);
            /* world->gad=0; */
            world->CurrentPage +=1;
            if(world->CurrentPage > world->MaxBootPage)
               world->CurrentPage = 1;
            DoImagery(world);
            break;
      /*
      default:
            break;
      */
   }
   world->gad=0; 
   return;
}
void GetData(world)
struct World *world;
{
   struct BootNode *bn;
   struct DeviceNode *dn;
   struct MyNode     *mn;
   D( struct FileSysStartupMsg *fssm );
   D( struct DosEnvec          *ev   );

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
   world->MaxBootPage=(world->BootNodes+PAGE_SIZE-1)/PAGE_SIZE;
   D( kprintf("MaxPage=%ld, MaxBootPage=%ld, BootNodes=%ld\n",
      world->MaxPage,world->MaxBootPage,world->BootNodes); )
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
      /* n=0L; */
      D( printf("init.c: GetNode() returns MyNode=0x%lx\n",n); )
      return((struct MyNode *)0L);
   }

   n=world->NodeList.lh_Head;
   for(x=0;x<num;x++)
      n=n->ln_Succ;

   D( printf("init.c: GetNode() returns MyNode=0x%lx\n",n); )
   return((struct MyNode *)n);
}
#if 0
void FreeData(world)
struct World *world;
{
   struct Node *n;

   while(n=RemHead(&world->NodeList))
      FreeMem((char *)n,sizeof(struct MyNode));
}
#endif
void DrawBSTR(world,name,x,y,maxlen)
struct World *world;
BPTR name;
SHORT x,y;
LONG maxlen;
{
   UBYTE count;
   UBYTE *p;

   p=(UBYTE *)(name << 2);
   count=*p;

   if((count>7)&&maxlen)
   {
      /* kprintf("count=%ld, %7s\n",count,&p[count-7+1]); */
      if(stricmp(".device",&p[count-7])==0)
      {
   	     count -= 8;
      }
   }

   if((maxlen!=0)&&(count>maxlen))
      count=maxlen;
   /* D( kprintf("count = %ld ",count) ); */
   Move(&world->display.rp,x,y);
   Text(&world->display.rp,&p[1],count);
}
SHORT GadgetHit(world)
struct World *world;
{
   SHORT i;
   SHORT retval=0;
   struct MyNode *mn;

   if(world->AdvancedMode)
   {  
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

      if((world->sprite.x >= (C0GX))&&(world->sprite.x <= (C0GX+G0W-1)))
      {
         for(i=0;i<world->MaxGad;i++)
         {
            /*D( kprintf("i=%ld, min=%ld, max=%ld, sprite.y=%ld\n",(LONG)i,
   		              (LONG)((i*GYSP)+GYS),(LONG)((i*GYSP)+GYS+GH),
   					  (LONG)world->sprite.y) );*/
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
   } else {
      if((world->sprite.x <= FPCBGX+FPCBGW-1) && (world->sprite.x>=FPCBGX) &&
         (world->sprite.y <= FPCBGY+FPCBGH-1) && (world->sprite.y>=FPCBGY) )
      {
         retval=FPCBGN;
         goto exit;
      }
      if((world->sprite.x <= FPAOGX+FPAOGW-1) && (world->sprite.x>=FPAOGX) &&
         (world->sprite.y <= FPAOGY+FPAOGH-1) && (world->sprite.y>=FPAOGY) )
      {
         retval=FPAOGN;
         goto exit;
      }
      if(world->MaxBootPage > 1)
      {
         if((world->sprite.x<=FPNPGX+FPNPGW-1)&&(world->sprite.x>=FPNPGX) &&
            (world->sprite.y<=FPNPGY+FPNPGH-1)&&(world->sprite.y>=FPNPGY) )
         {
            retval=FPNPGN;
            goto exit;
         }
      }
      if((world->sprite.x >= (FP0GX))&&(world->sprite.x <= (FP0GX+G1W-1)))
      {
         for(i=0;i<world->MaxGad;i++)
         {
            /*D( kprintf("i=%ld, min=%ld, max=%ld, sprite.y=%ld\n",(LONG)i,
   		              (LONG)((i*GYSP)+GYS),(LONG)((i*GYSP)+GYS+GH),
   					  (LONG)world->sprite.y) );*/
            if((world->sprite.y >= ((i * GYSP)+GYS)) &&
               (world->sprite.y <= ((i * GYSP)+GYS+GH-1)) )
            {
               mn=GetNode(world,(SHORT)(world->FirstGad+i));
               if(((mn->Enabled  && mn->Bootable)||
			       (mn->NoEnable && mn->Bootable)))
                  retval=((SHORT)(i+1));
               goto exit;
            }
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
      case DAGN:
            RectFill(&world->display.rp,DAGX,DAGY,DAGX+DAGW-1,DAGY+DAGH-1);
            break;
      case GOGN:
            RectFill(&world->display.rp,GOGX,GOGY,GOGX+GOGW-1,GOGY+GOGH-1);
            break;
      case NPGN:
            RectFill(&world->display.rp,NPGX,NPGY,NPGX+NPGW-1,NPGY+NPGH-1);
            break;
      case FPCBGN:
            RectFill(&world->display.rp,FPCBGX,FPCBGY,FPCBGX+FPCBGW-1,
			                            FPCBGY+FPCBGH-1);
            break;
      case FPAOGN:
            RectFill(&world->display.rp,FPAOGX,FPAOGY,FPAOGX+FPAOGW-1,
			                            FPAOGY+FPAOGH-1);
            break;
      case FPNPGN:
            RectFill(&world->display.rp,FPNPGX,FPNPGY,FPNPGX+FPNPGW-1,
			                            FPNPGY+FPNPGH-1);
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
	  /*
            y=(SHORT)(((gad-1)*GYSP)+GYS);
	        if(world->AdvancedMode)
			{
               RectFill(&world->display.rp,C1GX,y,C1GX+G1W-1,y+GH-1);
			} else {
               RectFill(&world->display.rp,FP0GX,y,FP0GX+G1W-1,y+GH-1);
			}
	  */
            y=(SHORT)(((gad-1)*GYSP)+GYS);
            RectFill(&world->display.rp,FP0GX,y,FP0GX+G1W-1,y+GH-1);
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
            D( printf("init.c: ComplementGadget() FirstGad=%ld\n",
			          world->FirstGad); )
            mn=GetNode(world,(SHORT)(world->FirstGad+gad-11));
            if(mn->Enabled)
            {
               if(mn->Bootable)
               {
                  if(world->BootNodes > 1)
                  {
                     mn->Enabled=FALSE;
                     world->BootNodes-=1;
                  }
               } else {
                  mn->Enabled=FALSE;
               }
            } else {
               mn->Enabled=TRUE;
               if(mn->Bootable)
                  world->BootNodes+=1;
            }
            RectFill(&world->display.rp,C0GX,(SHORT)(((gad-11)*GYSP)+GYS),
			         C0GX+G0W-1,(SHORT)(((gad-11)*GYSP)+GYS)+GH-1);
            break;
      /*
      default:
            break;
      */
   }
   SetDrMd(&world->display.rp,JAM1);
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
   if( ! ((struct MyNode *)(world->NodeList.lh_Head))->Highlited)/*Bootdef*/
      for(mn=(struct MyNode *)world->NodeList.lh_Head;
          mn->Node.ln_Succ;
          mn=(struct MyNode *)mn->Node.ln_Succ)
      {
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
   char buf[5];

   /**************/
   /* Clear slot */
   /**************/
   SetAPen(&world->display.rp,0);

   RectFill(&world->display.rp,C0GX,(x*GYSP)+GYS,
                            C5GX+G5W-1,(x*GYSP)+GYS+GH-1);

   /********************/
   /* Does node exist? */
   /********************/
   world->FirstGad=((world->CurrentPage-1)*PAGE_SIZE);
   mn=GetNode(world,(SHORT)(x+world->FirstGad));
   if( ! mn)
   {
      return(FALSE);
   }

   /****************/
   /* Get Pointers */
   /****************/
   bn=mn->node;
   dn=(struct DeviceNode *)bn->bn_DeviceNode;
   fssm=(struct FileSysStartupMsg *)(((ULONG)dn->dn_Startup) << 2);
   ev=(struct DosEnvec *)(((ULONG)fssm->fssm_Environ) << 2); 

   /**************/
   /* Draw Boxes */
   /**************/
   DrawBox(world,C1GX,(SHORT)((x*GYSP)+GYS),G1W,GH,0);
   DrawBox(world,C2GX,(SHORT)((x*GYSP)+GYS),G2W,GH,0);
   DrawBox(world,C3GX,(SHORT)((x*GYSP)+GYS),G3W,GH,0);
   DrawBox(world,C4GX,(SHORT)((x*GYSP)+GYS),G4W,GH,0);
   DrawBox(world,C5GX,(SHORT)((x*GYSP)+GYS),G5W,GH,0);

   /********/
   /* NWay */
   /********/
   DrawNWay(world,C0GX,(SHORT)((x*GYSP)+GYS),mn->Enabled,mn->NoEnable);

   /***************/
   /* Device Name */
   /***************/
   SetAPen(&world->display.rp,2);
   DrawBSTR(world,dn->dn_Name,C1TX,(SHORT)((x*GYSP)+TYS),0);

   /***********/
   /* BootPri */
   /***********/
   Move(&world->display.rp,C3TX,(SHORT)((x*GYSP)+TYS));
   if(mn->Bootable)
   {
      sprintf(buf,"%4ld",(LONG)bn->bn_Node.ln_Pri);
      Text(&world->display.rp,buf,4);
   } else {
      Text(&world->display.rp," N/A",4);
   }

   /**********/
   /* Device */
   /**********/
   DrawBSTR(world,fssm->fssm_Device,C4TX,(SHORT)(((x)*GYSP)+TYS),10);

   /********/
   /* Unit */
   /********/
   sprintf(buf,"%3ld",(LONG)fssm->fssm_Unit);
   Move(&world->display.rp,C5TX,(SHORT)((x*GYSP)+TYS));
   Text(&world->display.rp,buf,3);


   /***********/
   /* DosType */
   /***********/
   if(ev->de_TableSize < DE_DOSTYPE)
   {
      Move(&world->display.rp,C2TX,(SHORT)((x*GYSP)+TYS));
      Text(&world->display.rp,"DOS",3);
      SetAPen(&world->display.rp,1);
      Text(&world->display.rp,"0",1);
   } else {
      DrawByte(world,(SHORT)C2TX,     (SHORT)((x*GYSP)+TYS),
	                                  (UBYTE)((ev->de_DosType>>24)&0xff));
      DrawByte(world,(SHORT)(C2TX+8), (SHORT)((x*GYSP)+TYS),
	                                  (UBYTE)((ev->de_DosType>>16)&0xff));
      DrawByte(world,(SHORT)(C2TX+16),(SHORT)((x*GYSP)+TYS),
	                                  (UBYTE)((ev->de_DosType>>8) &0xff));
      DrawByte(world,(SHORT)(C2TX+24),(SHORT)((x*GYSP)+TYS),
	                                  (UBYTE)((ev->de_DosType)    &0xff));
   }


   return(TRUE);
}
BOOL DrawFPEntry(world,x)
struct World *world;
SHORT x;
{
   struct MyNode *mn;
   struct BootNode *bn;
   struct DeviceNode *dn;

   /**************/
   /* Clear slot */
   /**************/
   SetAPen(&world->display.rp,0);

   RectFill(&world->display.rp,FP0GX,(SHORT)((x*GYSP)+GYS),
                            FP0GX+G1W-1,((x*GYSP)+GYS)+GH-1);

   /********************/
   /* Does node exist? */
   /********************/
   world->FirstGad=((world->CurrentPage-1)*PAGE_SIZE);
   mn=GetNode(world,(SHORT)(x+world->FirstGad));
   if((!mn)||(!mn->Bootable))
   {
      return(FALSE);
   }

   /****************/
   /* Get Pointers */
   /****************/
   bn=mn->node;
   dn=(struct DeviceNode *)bn->bn_DeviceNode;

   /************/
   /* Draw Box */
   /************/
   DrawBox(world,FP0GX,(SHORT)((x*GYSP)+GYS),G1W,GH,(SHORT)(
      ((mn->Enabled && mn->Bootable)||(mn->NoEnable && mn->Bootable))?1:0));

   /***************/
   /* Device Name */
   /***************/
   SetAPen(&world->display.rp,2);
   DrawBSTR(world,dn->dn_Name,FP0TX,(SHORT)(((x)*GYSP)+TYS),0);

   return(TRUE);
}
VOID DrawNWay(struct World *world,SHORT x,SHORT y,BOOL enabled,BOOL sunken)
{
   struct BitMap mybitmap;
   SHORT place;

   /*******/
   /* Box */
   /*******/
   DrawBox(world,x,y,DAGW,DAGH,(BOOL)(! sunken));
   /* DrawGlyph(world,x,y); */

   /*********/
   /* Glyph */
   /*********/
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

   FreeMem((char *)mybitmap.Planes[0],sizeof(NWayGlyphData));

   /*****************/
   /* Vertical line */
   /*****************/
   SetAPen(&world->display.rp,1);
   Move(&world->display.rp,x+20,y+2);
   Draw(&world->display.rp,x+20,y+10);

   SetAPen(&world->display.rp,2);
   Move(&world->display.rp,x+19,y+2);
   Draw(&world->display.rp,x+19,y+10);

   /********/
   /* Text */
   /********/
   if(! enabled)
   {
      Move(&world->display.rp,x+19+9,y+9);
      Text(&world->display.rp,"DISABLED",8);
   } else {
      Move(&world->display.rp,x+19+9+4,y+9);
      Text(&world->display.rp,"ENABLED",7);
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
   } else {
      SetAPen(&world->display.rp,1);
      if((value>=0x00)&&(value<=0x09))
      {
         value += '0';
      } else
         if((value>=0x0a)&&(value<=0x1f))
         {
            value = value - 0x0a + 'a';
         } else
            if((value>=0x80)&&(value<=0x9f))
            {
               value = value - 0x80 + '@';
            }
   }
   Text(&world->display.rp,&value,1);
}
void MyDelay(struct World *world,SHORT n)
{
	SHORT x;

	for(x=0;x<n;x++)
	   WaitTOF();
}
