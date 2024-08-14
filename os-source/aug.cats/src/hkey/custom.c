/* custom.c This file contains the custom code for a commodity */
/* you should be able to write a new commodity by changing only */
/* custom.c and custom.h */

#include "mycxapp.h"

#if WINDOW

#define V(x) ((VOID *)x)

struct Gadget *F_Gadget[10] = { NULL };

extern BOOL IDCMPRefresh;

UBYTE *ListLabels[] =
    {
    (UBYTE *)"None",
    (UBYTE *)"Shift",
    (UBYTE *)"Alt",
    (UBYTE *)"Shift Alt",
    NULL,
    };

struct TextAttr mydesiredfont =
   {
      "topaz.font",  /*  Name */
      8,             /*  YSize */
      0,             /*  Style */
      0,             /*  Flags */
   };


VOID HandleGadget(gad,code)
ULONG gad,code;
{
   D( kprintf("custom: HandleGadget(%lx)\n",gad); )
   switch(gad)
   {
      case GAD_HIDE:
            D( kprintf("custom: HandleGadget() GAD_HIDE\n"); )
            shutdownWindow();
            break;
      case GAD_DIE:
            D( kprintf("custom: HandleGadget() GAD_DIE\n"); )
            terminate();
      default:
            MyHandleGadgets(gad,code);
            break;
   }
}

VOID setupCustomMenu()
{
   struct NewMenu mynewmenu [] =
      {
         {  NM_TITLE,   "Project",  0,    0, 0, 0,             },
         {   NM_ITEM,   "Hide",     "H",  0, 0, V(MENU_HIDE),   },
         {   NM_ITEM,   "Die",      "Q",  0, 0, V(MENU_DIE),    },
         {  NM_END,     0,          0,    0, 0, 0              },
      };

   menu=CreateMenus(mynewmenu,GTMN_FrontPen,2,TAG_DONE);
   D( kprintf("custom: CreateMenus returns menu =  %lx\n",menu); )
}

VOID handleCustomMenu(code)
UWORD code;
{
   struct MenuItem *item;
   BOOL terminated=FALSE;

   D( kprintf("custom: handleCustomMenu(code=%lx)\n",code); )
   while((code!=MENUNULL)&&(!terminated))
   {
      item=ItemAddress(menu,code);
      switch((int)MENU_USERDATA(item))
      {
         case MENU_HIDE:
               shutdownWindow();
               terminated=TRUE; /* since window is gone NextSelect is invalid so...*/
               break;
         case MENU_DIE:
               terminate();
               break;
         default:
               break;
      }
      code=item->NextSelect;
      D( kprintf("custom: handleCustomMenu next code=%lx\n",code); )
   }
   D( kprintf("custom: handleCustomMenu exits"); )
}

VOID refreshWindow()
{

   if(window)
   {
   if(IDCMPRefresh) 
	{
	GT_BeginRefresh(window);
	GT_EndRefresh(window,1L);
	}
 
      /* It is possible that the user has selected a font so large */
      /* that our imagery will fall off the bottom of the window   */
      /* we RefreshWindowFrame here incase our borders were overwritten */
      if((topborder+WINDOW_INNERHEIGHT) > window->Height)
         RefreshWindowFrame(window);
   }
}

#endif /* WINDOW */

BOOL setupCustomCX()
{
   return(MySetupCX());
}
VOID shutdownCustomCX()
{
   MyShutdownCX();
}

VOID handleCustomCXMsg(id)
ULONG id;
{
   MyHandleCXMsg(id);
}

VOID handleCustomCXCommand(id)
ULONG id;
{
   switch(id)
   {
      case 0:
      default:
            break;
   }
}


VOID setupCustomGadgets(gad,vi)
struct Gadget **gad;
void *vi;
{
   extern ULONG setoffs;
   struct NewGadget ng;
   int k, gyoffs;

   setoffs = FOFF;

   D( printf("initial gad=$%lx *gad=$%lx ",gad,*gad) );
   /*  Fill out a NewGadget structure to describe the gadget we want
       to create: */
   ng.ng_LeftEdge = 10;
   ng.ng_TopEdge = topborder+165;
   ng.ng_Width = 40;
   ng.ng_Height = 12;
   ng.ng_GadgetText = "Hide";
   ng.ng_TextAttr = &mydesiredfont;
   ng.ng_GadgetID = GAD_HIDE;
   ng.ng_Flags = NULL;
   ng.ng_VisualInfo = vi;
   *gad = CreateGadget(BUTTON_KIND,*gad, &ng,TAG_DONE);
   D( printf("Hide=$%lx ",*gad) );

   ng.ng_LeftEdge = 60;
   ng.ng_TopEdge = topborder+165;
   ng.ng_Width = 40;
   ng.ng_Height = 12;
   ng.ng_GadgetText = "Die";
   ng.ng_TextAttr = &mydesiredfont;
   ng.ng_GadgetID = GAD_DIE;
   ng.ng_Flags = NULL;
   ng.ng_VisualInfo = vi;
   *gad = CreateGadget(BUTTON_KIND,*gad, &ng,TAG_DONE);
   D( printf("Die=$%lx ",*gad) );

   ng.ng_LeftEdge = 120;
   ng.ng_TopEdge = topborder+150;
   ng.ng_Width = 120;
   ng.ng_Height = 14;
   ng.ng_GadgetText = "Modifier:";
   ng.ng_GadgetID = MY_NWAY;
   ng.ng_Flags = NG_HIGHLABEL;
   ng.ng_VisualInfo = vi;
   *gad = CreateGadget(NWAY_KIND,*gad, &ng,
	GTNW_Labels,ListLabels,GTNW_Active,0,TAG_DONE);
   D( printf("Mod=$%lx ",*gad) );

#define GYDIFF 15
   for(k=0, gyoffs=topborder; k<10; k++, gyoffs += GYDIFF)
	{
   	ng.ng_LeftEdge = 40;
   	ng.ng_TopEdge = gyoffs;
   	ng.ng_Width = 240;
   	ng.ng_Height = 14;
   	ng.ng_GadgetText = keynames[k];
   	ng.ng_TextAttr = &mydesiredfont;
   	ng.ng_GadgetID = GAD_F1 + k;
   	ng.ng_Flags = NULL;
   	ng.ng_VisualInfo = vi;
   	F_Gadget[k] = *gad =
	   CreateGadget(STRING_KIND,*gad, &ng,GTST_String,
		hkeys[k].command,GTST_MaxChars,200,TAG_DONE);
   	D( printf("G%d=$%lx ",k,*gad) );
	}
D( printf("\n") );

}


VOID refreshGadgets()
{
   int k,i;
   extern ULONG setoffs;

   for(k=0,i=setoffs; k<10; k++,i++)
   	{
   	GT_SetGadgetAttrs(F_Gadget[k],window,NULL,
		GTST_String,hkeys[i].command,TAG_DONE);
   	D( printf("trying to set gad %d to str: %s\n",k,hkeys[i].command) );
   	}
}

