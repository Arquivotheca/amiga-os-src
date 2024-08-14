#include "support.h"

#if SYSCHECK
/*********************************/
/* Text width for display fields */
/*********************************/
#define T0W       24  
#define T1W       40 
#define T2W       24  
#define T3W       10 

/***********************************/
/* Gadget width for display fields */
/***********************************/
#define G0W       48
#define G1W       96  
#define G2W       56 
#define G3W       20   

#define GYS       30                      /* gadget y start     */
#define GYSP      15                      /* gadget y spacing   */
#define GXSP      20                      /* gadget x spacing   */
#define GH        13                      /* gadget height      */
#define TH        8                       /* text height        */
#define TYS       (GYS+((GH-TH)>>1)+TH-1) /* text   y start     */

/*************************************/
/* Gadget and text x start positions */
/*************************************/
#define C0GX      ((WIDTH-1-(G0W+GXSP+G1W+GXSP+G2W+GXSP+G3W))>>1)
                                          /* col 0 gadget x pos */
#define C0TX      (C0GX+((G0W-T0W)>>1))   /* col 0 text   x pos */

#define C1GX      (C0GX+G0W+GXSP)         /* col 1 gadget x pos */
#define C1TX      (C1GX+((G1W-T1W)>>1))   /* col 1 text   x pos */

#define C2GX      (C1GX+G1W+GXSP)         /* col 2 gadget x pos */
#define C2TX      (C2GX+((G2W-T2W)>>1))   /* col 2 text   x pos */

#define C3GX      (C2GX+G2W+GXSP)         /* col 3 gadget x pos */
#define C3TX      (C3GX+((G3W-T3W)>>1))   /* col 3 text   x pos */

#define LY        27                      /* Label y pos        */

/*****************************/
/* Continue button constants */
/*****************************/
#define CBTW      64    
#define CBTH      8    
#define CBGW      (CBTW+10)  
#define CBGH      13     
#define CBGX      ((WIDTH-1-CBGW)>>1)  
#define CBGY      185     
#define CBTX      (CBGX+((CBGW-CBTW)>>1))
#define CBTY      (CBGY+CBTH+((CBGH-CBTH)>>1)-1)
#define CBGN      (100)  

#define UP_DELAY  15    /* 60ths of a second after a gadget is released */
                        /* and before the screen closes                 */

#if 1 
struct World {
   struct GfxBase       *GfxBase;
   struct ExpansionBase *ExpansionBase;
   struct Display       display;
   struct IH            ih;
   struct Sprite        sprite;

   struct Task          *maintask;
   LONG                 signal;
   SHORT                nodes;   /* Number of BOOTNODES found */
   SHORT                gad;     /* gad currently depressed or 0 */
   };
#endif

int    SysCheckInit(void);
void   SC_DoImagery(struct World *);
struct InputEvent * __asm SC_HandlerInterface(
                             register __a0 struct InputEvent *ev,
                             register __a1 struct World *world);
void   SC_GadgetUpAction(struct World *);
void   SC_DoData(struct World *);
SHORT  SC_GadgetHit(struct World *);
void   SC_ComplementGadget(struct World *,SHORT);

int SysCheckInit()
{
   struct Library *library;
   struct World   *world;
   struct World   myworld;

   D( kprintf("###################\n") );
   D( kprintf("syscheck Init Begins\n") );

   world=&myworld;
   world->signal=-1;

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

   /* if( ! SC_Activated(world) ) goto out; */

   if(!(world->ExpansionBase->Flags & EBF_BADMEM))
      goto out;

   world->signal=AllocSignal(-1);
   world->maintask=FindTask(0);

   DisplayOn(world,0x800,0xfff,0x000,0xec3);
   D( kprintf("After DisplayOn()\n") );


   SC_DoData(world);

   SC_DoImagery(world);

   D( kprintf("After SC_DoImagery()\n") );

   OpenSprite(world);
   D( kprintf("After OpenSprite()\n") );

   OpenIH(world,(APTR)SC_HandlerInterface);
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

void SC_DoImagery(world)
struct World *world;
{
   SHORT x;

   SetAPen(&world->display.rp,3);

   SetFont(&world->display.rp,world->display.font9);

   Move(&world->display.rp,179,8);
   Text(&world->display.rp,"System Expansion Board Check",28);

   SetFont(&world->display.rp,world->display.font8);

   Move(&world->display.rp,C0GX,LY);
   Text(&world->display.rp,"Status",6);

   Move(&world->display.rp,C1GX,LY);
   Text(&world->display.rp,"Manufacturer",12);

   Move(&world->display.rp,C2GX,LY);
   Text(&world->display.rp,"Product",7);

   SetAPen(&world->display.rp,2);
   Move(&world->display.rp,CBTX,CBTY);
   Text(&world->display.rp,"CONTINUE",8);
   DrawBox(world,CBGX,CBGY,CBGW,CBGH,1);


   for(x=0;x<world->nodes;x++)
   {
      DrawBox(world,C0GX,(SHORT)((x*GYSP)+GYS),G0W,GH,0);
      DrawBox(world,C1GX,(SHORT)((x*GYSP)+GYS),G1W,GH,0);
      DrawBox(world,C2GX,(SHORT)((x*GYSP)+GYS),G2W,GH,0);
   }

}
struct InputEvent * __asm SC_HandlerInterface(
                        register __a0 struct InputEvent *ev,
                        register __a1 struct World *world)
{
   if(ev->ie_Class==IECLASS_RAWMOUSE)
   {
      UpdateMouse(world,ev);
      /* if world->gad and SC_GadgetHit != world->gad comp world->gad =0 */
      /* if something was selected and mouse is no longer over same gad */
      if(world->gad && (SC_GadgetHit(world)!=world->gad))
      {
         SC_ComplementGadget(world,world->gad);
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
               world->gad=SC_GadgetHit(world);
               if(world->gad)
               {
                  SC_ComplementGadget(world,world->gad);
               }
               break;
         case IECODE_LBUTTON|IECODE_UP_PREFIX:
               D( kprintf("Left Button UP\n") );
               SC_GadgetUpAction(world);
               break;
/*
         case IECODE_NOBUTTON:
               D( kprintf("No Button x = %ld, y = %ld\n",ev->ie_X,ev->ie_Y) );
               break;
         default:
               D( kprintf("Default\n") );
               break;
*/
      }
   }
   return(ev);
}
void SC_GadgetUpAction(world)
struct World *world;
{
   SHORT gad;
   SHORT i;

   if( ! world->gad) return;     /* nothing was down */
   gad=SC_GadgetHit(world);
   if(world->gad!=gad)           /* down on one up on another or none */
   {                             /* Un highlite world->gad            */
      SC_ComplementGadget(world,world->gad);
      world->gad=0;
      return;
   }
   if(gad==CBGN)
   {
      for(i=0;i<UP_DELAY;i++)
         WaitTOF();
      Signal(world->maintask,1<<world->signal);
   }
   return;
}
void SC_DoData(world)
struct World *world;
{
   struct ConfigDev *cd=NULL;
   char buf[10];

   world->nodes=0;
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

	  /**********/
      /* Status */
	  /**********/
      Move(&world->display.rp,C0TX,(SHORT)(((world->nodes-1)*GYSP)+TYS));
      Text(&world->display.rp,(cd->cd_Flags&CDF_BADMEMORY)?"BAD":" OK",3);


	  /***********************/
      /* Manufacturer number */
	  /***********************/
      sprintf(buf,"%5ld",(ULONG)cd->cd_Rom.er_Manufacturer);
      Move(&world->display.rp,C1TX,(SHORT)(((world->nodes-1)*GYSP)+TYS));
      Text(&world->display.rp,buf,5);

	  /******************/
      /* Product number */
	  /******************/
      sprintf(buf,"%3ld",(ULONG)cd->cd_Rom.er_Product);
      Move(&world->display.rp,C2TX,(SHORT)(((world->nodes-1)*GYSP)+TYS));
      Text(&world->display.rp,buf,3);

	  /******************/
      /* Chained config */
	  /******************/
      if(cd->cd_Rom.er_Type & ERTF_CHAINEDCONFIG)
	  {
         Move(&world->display.rp,C3GX,    
                      (SHORT)(((world->nodes-1)*GYSP)+GYS+(GH>>1)));
         Draw(&world->display.rp,C3GX+G3W,
		              (SHORT)(((world->nodes-1)*GYSP)+GYS+(GH>>1)));
         Draw(&world->display.rp,C3GX+G3W,
		              (SHORT)(((world->nodes)*GYSP)+  GYS+(GH>>1)));
         Draw(&world->display.rp,C3GX,    
		              (SHORT)(((world->nodes)*GYSP)+  GYS+(GH>>1)));
	  }

	  /*********/
	  /* Boxes */
	  /*********/
/*
      DrawBox(world,C0GX,(SHORT)(((world->nodes-1)*GYSP)+GYS),G0W,GH,0);
      DrawBox(world,C1GX,(SHORT)(((world->nodes-1)*GYSP)+GYS),G1W,GH,0);
      DrawBox(world,C2GX,(SHORT)(((world->nodes-1)*GYSP)+GYS),G2W,GH,0);
*/
   }
   D( kprintf("End ConfigDev Search\n") );
}
SHORT SC_GadgetHit(world)
struct World *world;
{
   if((world->sprite.x <= CBGX+CBGW-1) && (world->sprite.x >= CBGX) &&
      (world->sprite.y <= CBGY+CBGH-1) && (world->sprite.y >= CBGY) )
   {
      return(CBGN);
   }
   return(0);
}
void SC_ComplementGadget(world,gad)
struct World *world;
SHORT gad;
{
   SetDrMd(&world->display.rp,COMPLEMENT);
   if(gad==CBGN)
   {
      RectFill(&world->display.rp,CBGX,CBGY,CBGX+CBGW-1,CBGY+CBGH-1);
   }
   SetDrMd(&world->display.rp,JAM1);
}
#endif /* #if SYSCHECK */
