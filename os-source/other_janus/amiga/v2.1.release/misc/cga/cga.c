#include <intuition/intuition.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <janus/janus.h>

#define  DISPLAY_TASK_STACK_SIZE 2000

/*
 *    Library Base pointers
 */
struct JanusBase     *JanusBase     = 0;
struct IntuitionBase *IntuitionBase = 0;
struct GfxBase       *GfxBase       = 0;
ULONG                *DiskfontBase  = 0;

/*
 *    Ptrs for runtime memory
 */
UBYTE                *TaskStack     = 0;
struct Task          *DisplayTask   = 0;
UWORD                *ChangeBuff    = 0;
UBYTE                *NFuncs        = 0;
UBYTE                *CFuncs        = 0;
ULONG                *NFuncTable    = 0;
ULONG                *CFuncTable    = 0;

ULONG                sigmask;

UBYTE                *JBoardBase;
UWORD *CGA_Base;
struct NewScreen     MyNewScreen;
struct Screen        *MyScreen      = 0;
struct NewWindow     MyNewWindow;
struct Window        *MyWindow      = 0;
struct TextAttr      f;
struct TextFont      *font          = 0;
ULONG BPR;
UBYTE *Plane0;
UBYTE *Plane1;
UBYTE *Plane2;
UBYTE *Plane3;
UBYTE *CursorH;
UBYTE *CursorL;
UBYTE *StartH;
UBYTE *StartL;
UBYTE *ModeReg;
ULONG CPos;
ULONG RamWritten = 1;
ULONG CursorX;
ULONG CursorY;
UBYTE DTaskRun;

ULONG Width;
ULONG Height;
UWORD Rows;
UWORD Cols;

#define  BW40  (1<<0)
#define  CO40  (1<<1)
#define  BW80  (1<<2)
#define  CO80  (1<<3)
#define  BWLO  (1<<4)
#define  COLO  (1<<5)
#define  BWHI  (1<<6)


UBYTE CMode;


void main();
void MakeCharTable();
extern void ColorInt();
extern void RefreshCGA();

void main()
{
   int                  signum      = -1;

   struct SetupSig      *ss         =  0;
   struct IntuiMessage  *message;
   UBYTE  NotDone                   = 1;
   struct Interrupt     MyInt;
   UBYTE ResetF;
   UBYTE OldModeReg;
   UWORD Offset = 0;

/*
 *    Open Libraries
 */
   JanusBase    =(struct JanusBase *)OpenLibrary(JANUSNAME,0);
   if(JanusBase==0)
   {
      printf("Janus.library failed to open!\n");
      goto cleanup;
   }
   IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",0);
   if(IntuitionBase==0)
   {
      printf("Intuition.library failed to open!\n");
      goto cleanup;
   }
   GfxBase    =(struct GfxBase *)OpenLibrary("graphics.library",0);
   if(GfxBase==0)
   {
      printf("Graphics.library failed to open!\n");
      goto cleanup;
   }
   DiskfontBase    =(ULONG *)OpenLibrary("diskfont.library",0);
   if(DiskfontBase==0)
   {
      printf("DiskFont.library failed to open!\n");
      goto cleanup;
   }

/*
 *    Allocate runtime memory
 */
   TaskStack = (UBYTE *)AllocMem(DISPLAY_TASK_STACK_SIZE,MEMF_CLEAR);
   if(TaskStack==0)
   {
      printf("Unable to allocate DisplayTask Stack memory\n");
      goto cleanup;
   }
   DisplayTask = (struct Task *)AllocMem(sizeof(struct Task),MEMF_CLEAR|MEMF_PUBLIC);
   if(DisplayTask==0)
   {
      printf("Unable to allocate DisplayTask memory\n");
      goto cleanup;
   }
   ChangeBuff = (UWORD *)AllocMem(0x7fff,0);
   if(ChangeBuff==0)
   {
      printf("Unable to allocate ChangeBuff memory\n");
      goto cleanup;
   }
   NFuncs = (UBYTE *)AllocMem(((6*8)+2)*256,0);
   if(NFuncs==0)
   {
      printf("Unable to allocate NFuncs memory\n");
      goto cleanup;
   }
   CFuncs = (UBYTE *)AllocMem(((6*8)+2)*256,0);
   if(CFuncs==0)
   {
      printf("Unable to allocate CFuncs memory\n");
      goto cleanup;
   }
   NFuncTable = (ULONG *)AllocMem(4*256,0);
   if(NFuncTable==0)
   {
      printf("Unable to allocate NFuncTable memory\n");
      goto cleanup;
   }

   CFuncTable = (ULONG *)AllocMem(4*256,0);
   if(CFuncTable==0)
   {
      printf("Unable to allocate CFuncTable memory\n");
      goto cleanup;
   }

/*
 *    Get signal for crt register action
 */
   signum = AllocSignal(-1);
   if(signum==-1)
   {
      printf("Couldn't get signal!\n");
      goto cleanup;
   }
   sigmask = (1 << signum);


   JBoardBase = (UBYTE *)GetJanusStart();
   CursorH    = (UBYTE *)(JBoardBase + IoAccessOffset + 0x1e2dd);
   CursorL    = (UBYTE *)(JBoardBase + IoAccessOffset + 0x1e2df);
   StartH     = (UBYTE *)(JBoardBase + IoAccessOffset + 0x1e2d9);
   StartL     = (UBYTE *)(JBoardBase + IoAccessOffset + 0x1e2db);
   ModeReg    = (UBYTE *)(JBoardBase + IoAccessOffset + 0x1e23f);

   OldModeReg=(UBYTE)(*ModeReg & 0x37);
/*   printf("Mode byte = %x\n",OldModeReg);*/
   Height=200;
   Width=640;
   Rows=25;
   Cols=80;
   CMode=CO80;
   Offset = (UWORD)(((*StartH<<8) + *StartL)<<1);
   CGA_Base   = (UWORD *)(JBoardBase + WordAccessOffset + 0x10000 + Offset);
Reset:

   MyInt.is_Node.ln_Type = NT_INTERRUPT;
   MyInt.is_Node.ln_Pri  = 0;
   MyInt.is_Node.ln_Name = "CGA_Int\n";
   MyInt.is_Data         = 0;
   MyInt.is_Code         = ColorInt;

   SetJanusHandler(1L,&MyInt);
   SetJanusRequest(1L,0);
   SetJanusEnable(1L,1L);



   MyNewScreen.LeftEdge=0;
   MyNewScreen.TopEdge=0;
   MyNewScreen.Width=Width;
   MyNewScreen.Height=Height;
   MyNewScreen.Depth=4;
   MyNewScreen.DetailPen=1;
   MyNewScreen.BlockPen=2;
   MyNewScreen.ViewModes=(Width==640)?HIRES:NULL;
   MyNewScreen.Type=CUSTOMSCREEN;
   MyNewScreen.Font=NULL;
   MyNewScreen.DefaultTitle=NULL;
   MyNewScreen.Gadgets=NULL;
   MyNewScreen.CustomBitMap=NULL;

   MyScreen=(struct Screen *)OpenScreen(&MyNewScreen);
   if(MyScreen==NULL)
   {
      printf("Unable to open Screen!\n");
      goto cleanup;
   }

   MyNewWindow.LeftEdge=0;
   MyNewWindow.TopEdge=0;
   MyNewWindow.Width=Width;
   MyNewWindow.Height=Height;
   MyNewWindow.DetailPen=1;
   MyNewWindow.BlockPen=2;
   MyNewWindow.IDCMPFlags=MOUSEBUTTONS;
   MyNewWindow.Flags=SIMPLE_REFRESH|BORDERLESS;
   MyNewWindow.FirstGadget=NULL;
   MyNewWindow.CheckMark=NULL;
   MyNewWindow.Title=NULL;
   MyNewWindow.Type=CUSTOMSCREEN;
   MyNewWindow.Screen=MyScreen;
   MyNewWindow.BitMap=NULL;

   MyWindow=(struct Window *)OpenWindow(&MyNewWindow);
   if(MyWindow==NULL)
   {
      printf("Unable to open Window!\n");
      goto cleanup;
   }
   f.ta_Name="pcfont.font";
   f.ta_YSize=8;
   f.ta_Style=0;
   f.ta_Flags=0;
   font=(struct TextFont *)OpenFont(&f);
   if(font==0)
   {
      font=(struct TextFont *)OpenDiskFont(&f);
      if(font==0)
      {
         printf("Open of disk Font failed!\n");
         goto cleanup;
      }
   }

   SetRGB4(&MyScreen->ViewPort,0 ,0x0,0x0,0x0);
   SetRGB4(&MyScreen->ViewPort,1 ,0x0,0x0,0xc);
   SetRGB4(&MyScreen->ViewPort,2 ,0x0,0xc,0x0);
   SetRGB4(&MyScreen->ViewPort,3 ,0x0,0xc,0xc);
   SetRGB4(&MyScreen->ViewPort,4 ,0xc,0x0,0x0);
   SetRGB4(&MyScreen->ViewPort,5 ,0xc,0x0,0xc);
   SetRGB4(&MyScreen->ViewPort,6 ,0xc,0xc,0x0);
   SetRGB4(&MyScreen->ViewPort,7 ,0xc,0xc,0xc);
   SetRGB4(&MyScreen->ViewPort,8 ,0x8,0x8,0x8);
   SetRGB4(&MyScreen->ViewPort,9 ,0x0,0x0,0xf);
   SetRGB4(&MyScreen->ViewPort,10,0x0,0xf,0x0);
   SetRGB4(&MyScreen->ViewPort,11,0x0,0xf,0xf);
   SetRGB4(&MyScreen->ViewPort,12,0xf,0x0,0x0);
   SetRGB4(&MyScreen->ViewPort,13,0xf,0x0,0xf);
   SetRGB4(&MyScreen->ViewPort,14,0xf,0xf,0x0);
   SetRGB4(&MyScreen->ViewPort,15,0xf,0xf,0xf);


   BPR=MyWindow->RPort->BitMap->BytesPerRow;

   MakeCharTable();

   Plane0=MyWindow->RPort->BitMap->Planes[0];
   Plane1=MyWindow->RPort->BitMap->Planes[1];
   Plane2=MyWindow->RPort->BitMap->Planes[2];
   Plane3=MyWindow->RPort->BitMap->Planes[3];



   DisplayTask->tc_SPLower = (APTR)TaskStack;
   DisplayTask->tc_SPUpper = (APTR)TaskStack+2000;
   DisplayTask->tc_SPReg   = (APTR)TaskStack+2000;
   DisplayTask->tc_Node.ln_Type = NT_TASK;
   DisplayTask->tc_Node.ln_Pri  = (BYTE)4;
   DisplayTask->tc_Node.ln_Name = "CGA_DisplayTask\n";

   DTaskRun=(UBYTE)1;

   AddTask(DisplayTask,RefreshCGA,0);

   {
      ULONG *ll;
      USHORT xx;

      ll=(ULONG *)ChangeBuff;
      Forbid();
      for(xx=0;xx<(Rows*Cols*2)/4;xx++)
         *ll++=0;
      Permit();
      RamWritten=1;

      ll=(ULONG *)ChangeBuff;
      Forbid();
      for(xx=0;xx<(Rows*Cols*2)/4;xx++)
         *ll++=1;
      Permit();
      RamWritten=1;
   }

   ss=SetupJanusSig(3,signum,0,0);
   if(ss==0)
   {
      printf("SetupJanusSig failed!\n");
      goto cleanup;
   }

   NotDone=1;
   while(NotDone)
   {
      ULONG Sig;

      Sig=Wait((sigmask) | (1 << MyWindow->UserPort->mp_SigBit));

      if(Sig&sigmask)
      {
         UBYTE TMode;

/*         printf("CRT Regs changed!\n");*/

         Offset = (UWORD)(((*StartH<<8) + *StartL)<<1);
         CGA_Base=(UWORD *)(JBoardBase+WordAccessOffset+0x10000+Offset);
         TMode = *ModeReg & 0x37;
/*         printf("TMode = %x  OldModeReg = %x\n",TMode,OldModeReg);*/
         if(TMode!=OldModeReg)
         {
            OldModeReg=TMode;

            switch(TMode) {
               case 0x24:  if((CMode!=BW40)&&(CMode!=CO40))
                           {
                              CMode = BW40;
                              Width=320;
                              Cols=40;
                              ResetF=1;
                              NotDone=0;
                              DTaskRun=0;
                              Delay(5);
                           }
                           break;
               case 0x20:  if((CMode!=CO40)&&(CMode!=BW40))
                           {
                              CMode = CO40;
                              Width=320;
                              Cols=40;
                              ResetF=1;
                              NotDone=0;
                              DTaskRun=0;
                              Delay(5);
                           }
                           break;
               case 0x25:  if((CMode!=BW80)&&(CMode!=CO80))
                           {
                              CMode = BW80;
                              Width=640;
                              Cols=80;
                              ResetF=1;
                              NotDone=0;
                              DTaskRun=0;
                              Delay(5);
                           }
                           break;
               case 0x21:  if((CMode!=CO80)&&(CMode!=BW80))
                           {
                              CMode = CO80;
                              Width=640;
                              Cols=80;
                              ResetF=1;
                              NotDone=0;
                              DTaskRun=0;
                              Delay(5);
                           }
                           break;
               case 0x06:
               case 0x26:  CMode = BWLO;
                           printf("BWLO\n");
                           break;
               case 0x02:
               case 0x22:  CMode = COLO;
                           printf("COLO\n");
                           break;
               case 0x16:
               case 0x36:  CMode = BWHI;
                           printf("BWHI\n");
                           break;
            }
         }  /* end of IF MODE HAS CHANGED */
      }     /* end of IF CRT REGS CHANGED */
/*
      CPos=   ((*CursorH)<<8)+(*CursorL);
      CursorY =   ((CPos-0x50)/Cols)+1;
      CursorX =   (CPos-0x50)-((CursorY-1)*Cols);
*/

/*
      printf("CPos    = %lx\n",CPos);
      printf("CursorX = %ld\n",CursorX);
      printf("CursorY = %ld\n",CursorY);
*/
/*      DrawCursor();*/
      while(message=(struct IntuiMessage *)GetMsg(MyWindow->UserPort))
      {
         NotDone=0;
         ResetF=0;
         DTaskRun=0;
         Delay(10);
      }
   }
   RemTask(DisplayTask);

cleanup:
   SetJanusHandler(1L,0);
   if(ss) CleanupJanusSig(ss);
   if(font) CloseFont(font);
   if(MyWindow) CloseWindow(MyWindow);
   if(MyScreen) CloseScreen(MyScreen);

   if(ResetF)
   {
      ResetF=0;
      goto Reset;
   }

/*
   SetJanusEnable(1L,0);
   SetJanusHandler(1L,NULL);
*/
   if(signum!=-1)    FreeSignal(signum);
   if(CFuncTable)    FreeMem((UBYTE *)CFuncTable,4*256);
   if(NFuncTable)    FreeMem((UBYTE *)NFuncTable,4*256);
   if(CFuncs)        FreeMem(CFuncs,((6*8)+2)*256);
   if(NFuncs)        FreeMem(NFuncs,((6*8)+2)*256);
   if(ChangeBuff)    FreeMem(ChangeBuff,0x7fff);
   if(DisplayTask)   FreeMem((UBYTE *)DisplayTask,sizeof(struct Task));
   if(TaskStack)     FreeMem((UBYTE *)TaskStack,DISPLAY_TASK_STACK_SIZE);
   if(DiskfontBase)  CloseLibrary(DiskfontBase);
   if(GfxBase)       CloseLibrary(GfxBase);
   if(IntuitionBase) CloseLibrary(IntuitionBase);
   if(JanusBase)     CloseLibrary(JanusBase);
}
void MakeCharTable()
{
   UBYTE *Src;
   ULONG *l;
   USHORT x,y;
   UBYTE *ncode;
   UBYTE *ccode;
   ULONG *ntab;
   ULONG *ctab;
   UWORD *t1;
   UWORD *t2;

   ncode=NFuncs;
   ccode=CFuncs;
   ntab=NFuncTable;
   ctab=CFuncTable;
   for(y=0;y<256;y++)
   {
      Src=(UBYTE *)(font->tf_CharData);
      l=(ULONG *)font->tf_CharLoc;
      Src+=((l[y] >> 16)/8);
      *ntab++=(ULONG)ncode; /* store address of this chars func in table */
      *ctab++=(ULONG)ccode;
      for(x=0;x<8;x++)
      {
         *ncode++=0x11;
         *ccode++=0x11;
         *ncode++=0x7C;
         *ccode++=0x7C;
         *ncode++=0x00;
         *ccode++=0x00;

         *ncode++=*Src;
         *ccode++=~(*Src);

         t1=(UWORD *)ncode;
         t2=(UWORD *)ccode;
         *t1=(UWORD)(BPR)*x;
         *t2=(UWORD)(BPR)*x;
         ncode+=2;
         ccode+=2;

         Src+=font->tf_Modulo;
      }
      *ncode++=0x4E;
      *ccode++=0x4E;
      *ncode++=0x75;
      *ccode++=0x75;
   }
}
