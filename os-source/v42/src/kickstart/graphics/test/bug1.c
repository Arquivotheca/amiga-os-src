/****
 * DESCRIPTION
 *	This program demonstrates two bugs in the graphics.library.
 *
 *	1. If you are running version 37.175 and upwards, together
 *	   with an old Denise, you will quite clearly see that 
 *	   the screen is jumping up and down horrendously!
 *
 *	2. In version 36.184, the screen will jump up and down,
 *	   this does not happen in the earlier versions of the
 *	   os(ie 1.3). Fixed in later versions.
 *
 *	3. If you are running 1.3, then you will see that the lower
 *	   part of the picture is cut of(the PAL part).
 *	   This bug is removed in 2.0, and you can also use PatchMrg
 *	   to fix it. No fix has ever been provided by Commodore.
 *
 *	compile with "lc -L" to make ntsc version
 *	compile with "lc -L -dPALMACHINE" to make PAL version.
 ***/


#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <graphics/copper.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/gfxnodes.h>
#include <graphics/videocontrol.h>
#include <graphics/view.h>
#include <graphics/displayinfo.h>

struct GfxBase *GfxBase;

/****
* Test 
****/

#ifdef PALMACHINE
#define VIEW_HEIGHT 580
#define VIEW_SECHEIGHT 512
#else
#define VIEW_HEIGHT 480
#define VIEW_SECHEIGHT 400
#endif

#define VIEW_OFFSET (-((VIEW_HEIGHT-VIEW_SECHEIGHT)/2))

#define VIEW_WIDTH 704
#define VIEW_DEPTH 4
#define VIEW_MODES (LACE|HIRES)


/****
 * FreeBitMap.
 ***/

void FreeBitMap(struct BitMap *BitMap)
{
int i;
   for (i=0;i<BitMap->Depth;i++)
   {
      if (BitMap->Planes[i])
      {
         FreeMem(BitMap->Planes[i], BitMap->BytesPerRow*BitMap->Rows);
         BitMap->Planes[i]=NULL;
      } else
      {
         break;
      }
   }
}

struct ViewPortExtra *vpe,*vpe1;

/****
 * Allocate a BitMap.
 ***/

int AllocBitMap(struct BitMap *BitMap)
{
int i;
   for (i=0;i<BitMap->Depth;i++)
   {
      if (!(BitMap->Planes[i]=
            AllocMem(BitMap->BytesPerRow*BitMap->Rows, MEMF_CHIP)))
      {
         for (i--;i>=0;i--)
         {
            FreeMem(BitMap->Planes[i], BitMap->BytesPerRow*BitMap->Rows);
         }
         BitMap->Planes[0]=NULL;
         return FALSE;
      }
   }
   return TRUE;
}




ULONG tags[]={VTAG_VIEWPORTEXTRA_SET,0,0,0};

struct DimensionInfo mybuf;

void main(void)
{
   if (GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",0))
   {
   struct View *OldView;
   struct View View;
   struct View View2;
   struct ViewPort ViewPort;
   struct ViewPort ViewPort2;
   struct RasInfo RasInfo,RasInfo2;
   struct BitMap BitMap,BitMap2;
	ULONG handle;
   
      OldView=GfxBase->ActiView;
      
      InitView(&View);
      InitView(&View2);
      InitVPort(&ViewPort);
      InitVPort(&ViewPort2);
		ViewPort.ColorMap=GetColorMap(32);
		ViewPort2.ColorMap=GetColorMap(32);
		vpe=GfxNew(VIEWPORT_EXTRA_TYPE);
		vpe1=GfxNew(VIEWPORT_EXTRA_TYPE);
      InitBitMap(&BitMap, VIEW_DEPTH, VIEW_WIDTH, VIEW_HEIGHT);
      InitBitMap(&BitMap2, VIEW_DEPTH, VIEW_WIDTH, VIEW_SECHEIGHT);
      
      
      if (AllocBitMap(&BitMap))
      {
         if (AllocBitMap(&BitMap2))
         {
         struct RastPort RastPort;
         struct RastPort RastPort2;
         int i;
            /****
             * First(overscan view)
             ***/
            ViewPort.DyOffset=VIEW_OFFSET;
            ViewPort.DWidth=BitMap.BytesPerRow*8;
            ViewPort.DHeight=BitMap.Rows;
            ViewPort.Modes|=VIEW_MODES;
            
            ViewPort.RasInfo=&RasInfo;
            
            RasInfo.Next=NULL;
            RasInfo.RxOffset=0;
            RasInfo.RyOffset=0;
            RasInfo.BitMap=&BitMap;
            
            
            View.Modes|=VIEW_MODES;
            View.ViewPort=&ViewPort;

            InitRastPort(&RastPort);
            RastPort.BitMap=&BitMap;
            SetRast(&RastPort, 0);
            
            
            DrawEllipse(&RastPort,
                        VIEW_WIDTH/2, ((VIEW_HEIGHT-VIEW_SECHEIGHT)/2)+
                                      (VIEW_SECHEIGHT/2),
                        (VIEW_WIDTH/2)-1, (VIEW_SECHEIGHT/2)-1);
                        

            /****
             * First(overscan view)
             ***/
             
            ViewPort2.DyOffset=VIEW_OFFSET+((VIEW_HEIGHT-VIEW_SECHEIGHT)/2);
            ViewPort2.DWidth=BitMap2.BytesPerRow*8;
            ViewPort2.DHeight=BitMap2.Rows;
            ViewPort2.Modes|=VIEW_MODES;
            
            ViewPort2.RasInfo=&RasInfo2;
            
            RasInfo2.Next=NULL;
            RasInfo2.RxOffset=0;
            RasInfo2.RyOffset=0;
            RasInfo2.BitMap=&BitMap2;
            
            
            View2.Modes|=VIEW_MODES;
            View2.ViewPort=&ViewPort2;
            

            InitRastPort(&RastPort2);
            RastPort2.BitMap=&BitMap2;
            SetRast(&RastPort2, 0);
            
            
            DrawEllipse(&RastPort2,
                        VIEW_WIDTH/2, (VIEW_SECHEIGHT/2),
                        (VIEW_WIDTH/2)-1, (VIEW_SECHEIGHT/2)-1);
                        
            

			tags[0]=VTAG_VIEWPORTEXTRA_SET;
			tags[1]=vpe;
            VideoControl(ViewPort.ColorMap,tags);
			tags[0]=VTAG_VIEWPORTEXTRA_SET;
			tags[1]=vpe1;
            VideoControl(ViewPort2.ColorMap,tags);


			handle=FindDisplayInfo(VIEW_MODES);
			GetDisplayInfoData(handle,&mybuf,sizeof(mybuf),DTAG_DIMS,0);
			vpe->DisplayClip=mybuf.Nominal;
			vpe1->DisplayClip=mybuf.Nominal;

MakeVPort(&View, &ViewPort);
            MakeVPort(&View, &ViewPort2);
            
            MrgCop(&View);
            MrgCop(&View2);
                        
            for (i=0;i<50;i++)
            {
               LoadView(&View);
               WaitTOF();
               WaitTOF();
               WaitTOF();
               WaitTOF();
               LoadView(&View2);
               WaitTOF();
               WaitTOF();
               WaitTOF();
               WaitTOF();
            }
            
            LoadView(OldView);
            FreeVPortCopLists(&ViewPort);
            FreeCprList(View.SHFCprList);
            FreeCprList(View.LOFCprList);
            
            FreeVPortCopLists(&ViewPort2);
            FreeCprList(View2.SHFCprList);
            FreeCprList(View2.LOFCprList);
            FreeBitMap(&BitMap2);
         }
         FreeBitMap(&BitMap);
      }
      CloseLibrary((struct Library *)GfxBase);
   }
}

