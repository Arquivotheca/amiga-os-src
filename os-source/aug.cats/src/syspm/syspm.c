;/* syspm.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 syspm.c
Blink FROM LIB:c.o,syspm.o TO syspm LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

/*
 *  mypm.c --- C. Scheppner  CBM 1987
 *     interrupt based on Dan Silva's interrupt code
 *     maxsize() lifted from avail.c
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/interrupts.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

extern struct Library *DOSBase;
extern struct Library *SysBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif


ULONG maxsize(ULONG t);
void StopVBlank(void);
void StartVBlank(void);
LONG MyVBlank(void);
VOID countRtn(void);
void cleanexit(char *s);
void cleanup(void);
void showPri(LONG pri);
void plotall(void);

#define MINARGS 1

UBYTE *vers = "\0$VER: syspm 37.1";
UBYTE *Copyright = 
  "syspm v37.1\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: syspm";


#define nCPU    0
#define nCHIP   1
#define nFAST   2

#define MYTHMAX    1600
#define RESTBLANKS 4
#define HEADROOM   48

/* display every n 50ths */
#define INITPRI    (-10)
#define MINPRI     (-128)
#define MAXPRI     (127)

#define INITOFTEN   80
#define MINOFTEN    (RESTBLANKS<<1)
#define MAXOFTEN    300

/* scroll jump at right */
#define JUMP  16

ULONG TB, BB, RB, LB, HIGH;  /* Height will change */

#define STRIPSZ   24

#define WIDE      300
#define MINWIDE   200
#define INITX     44

extern struct ExecBase *SysBase;
extern VOID  countRtn();
extern ULONG maxsize();

struct TextAttr topaz80 =
   {"topaz.font",TOPAZ_EIGHTY,FS_NORMAL,FPF_ROMFONT };

struct TextFont *tf = NULL;

struct   NewWindow      nw = {
   40, 40,                                /* LeftEdge and TopEdge */
   WIDE,0,                                /* Width and Height */
   -1, -1,                                /* DetailPen and BlockPen */
   CLOSEWINDOW|VANILLAKEY,                /* IDCMP Flags */
   WINDOWCLOSE|WINDOWDEPTH|WINDOWSIZING
   |WINDOWDRAG|SMART_REFRESH,             /* Flags */
   NULL, NULL,                            /* Gadget and Image pointers */
   " <>Speed  +-Pri ",                    /* Title string */
   NULL,                                  /* Put Screen ptr here */
   NULL,                                  /* SuperBitMap pointer */
   MINWIDE, 0,                            /* MinWidth and MinHeight */
   640, 0,                                /* MaxWidth and MaxHeight */
   WBENCHSCREEN,                          /* Type of window */
   };


struct Window   *window = NULL;
struct RastPort *rport  = NULL;
struct IntuiMessage *msg;
struct Task *countTask;

struct Library *IntuitionBase = NULL;
struct Library *GfxBase = NULL;
ULONG  class, code, howoften;
BOOL   AddedVBlank = FALSE, PrepareToDie = FALSE;
BOOL   Done = FALSE, Resting = TRUE;

char  *labels[3] = {"TASK", "CHIP", "FAST"};
ULONG progress = 0L, vblanks = 0L, counter = 0L;
ULONG xpos, tpos;
ULONG max[3], now[3], ylow[3], ypos[3];   /* CPU  CHIP  FAST */

/* For calibration CALIBS should be at least 8 */
#define CALIBS     16
ULONG calib[CALIBS] = {0}, calibi=0;
BOOL  Calib = FALSE;

LONG  tpri;

void main(int argc, char **argv)
   {
   struct Screen *wb;
   int k;

   if(!(GfxBase = OpenLibrary("graphics.library",0))) cleanexit("");
   if(!(IntuitionBase = OpenLibrary("intuition.library",0))) cleanexit("");

   /* Find Max values for memory graphs */
   Forbid();
   max[nCHIP] = maxsize(MEMF_CHIP);
   max[nFAST] = maxsize(MEMF_FAST);
   Permit();

   /* Start vblank monitor for counter task */
   StartVBlank();
   AddedVBlank = TRUE;

   /*  Calibrate approximate max performance */
   Forbid();
   k = CALIBS * MYTHMAX * 4;
   counter = 0L;
   Calib = TRUE;
   while(k--) counter++;
   Calib = FALSE;
   Permit();
   max[nCPU] = 0L;
   for(k=0; k<CALIBS; k++) if(calib[k] > max[nCPU])  max[nCPU] = calib[k];
   max[nCPU] = (max[nCPU] + HEADROOM) & 0xFFFFFFF0;

   /* Start counter task */
   counter = 0L;
   tpri = INITPRI;
   if(!(countTask =
      (struct Task *)CreateTask("CAS_MyPm_CntTask",tpri,countRtn,2000)))
         cleanexit("");

   /* Open PM window */
   wb = (struct Screen *)OpenWorkBench();
   TB = wb->FirstWindow->BorderTop + 2;
   HIGH=(TB + STRIPSZ + STRIPSZ + STRIPSZ + 8);
   nw.Height =nw.MinHeight = nw.MaxHeight = HIGH;

   if(!(window = (struct Window *)OpenWindow(&nw)))  cleanexit("");
   rport = window->RPort;
   if(tf = OpenFont(&topaz80)) SetFont(rport,tf);

   TB = window->BorderTop;
   BB = window->BorderBottom;
   LB = window->BorderLeft;
   RB = window->BorderRight;

   for (k=0; k<3; k++)
      {
      ylow[k] = TB + (STRIPSZ * (k+1));
      SetAPen(rport,3);
      Move(rport,4,ylow[k]);
      Text(rport,labels[k],4);

      if(k==0)
	{
	Move(rport,4,ylow[k]-16);
	Text(rport,"PERF",4);
	}

      SetAPen(rport,k+1);
      Move(rport,INITX-4,ylow[k]+1-STRIPSZ);
      Draw(rport,INITX-4,ylow[k]);
      Move(rport,INITX-3,ylow[k]+1-STRIPSZ);
      Draw(rport,INITX-3,ylow[k]);
      }
   tpos = ylow[0] - 8;
   showPri(tpri);

   xpos = INITX;

   howoften = INITOFTEN;  /* in 50ths of a second */


   while(!Done)
      {
      Delay(howoften);

      now[nCPU]  = (progress > max[nCPU]) ? max[nCPU] : progress;
      now[nCHIP] = (ULONG)AvailMem(MEMF_CHIP);
      now[nFAST] = (ULONG)AvailMem(MEMF_FAST);
      plotall();

      while(msg=(struct IntuiMessage *)GetMsg(window->UserPort))
         {
         class = msg->Class;
         code  = msg->Code;

         ReplyMsg(msg);
         switch(class)
            {
            case CLOSEWINDOW:
               Done = TRUE;
               break;
            case VANILLAKEY:
               switch(code)
                  {
                  case 0x03:
                     Done = TRUE;
                     break;
                  case '<':
                     if(howoften < MAXOFTEN) howoften++;
                     break;
                  case '>':
                     if(howoften > MINOFTEN) howoften--;
                     break;
                  case '+':
                     if(tpri < MAXPRI)
                        {
                        tpri++;
                        SetTaskPri(countTask,tpri);
                        showPri(tpri);
                        }
                     break;
                  case '-':
                     if(tpri > MINPRI)
                        {
                        tpri--;
                        SetTaskPri(countTask,tpri);
                        showPri(tpri);
                        }
                     break;
                  default:
                     break;
                  }
            default:
               break;
            }
         }
      }
   cleanup();
   }


void showPri(LONG pri)
   {
   UBYTE tbuf[16];

   sprintf(tbuf,"%4ld",tpri);
   SetAPen(rport,3);
   Move(rport,4,tpos);
   Text(rport,tbuf,4);
   }


void plotall()
   {
   SHORT width;
   int k;

   width = window->Width;
   if(xpos > width - (RB+1))
      {
      ScrollRaster(rport,JUMP,0,INITX,TB,width-(RB+1),HIGH-(BB+1));
      xpos = width-RB-JUMP;
      }

   for(k=0; k<3; k++)
      {
      if(max[k]) ypos[k] = (ULONG)(STRIPSZ * now[k] / max[k]);
      else ypos[k] = 0L;
      SetAPen(rport,k+1);
      Move(rport,xpos,ylow[k]-ypos[k]);
      Draw(rport,xpos,ylow[k]);
      }
   xpos++;
   }


void cleanexit(char *s)
   {
   cleanup();
   exit(0);
   }

void cleanup()
   {
   if(AddedVBlank)  StopVBlank();
   if(countTask)
      {
      PrepareToDie = TRUE;
      while(PrepareToDie)  Delay(10);
      DeleteTask(countTask);
      }
   if(window)
      {
      while(msg=(struct IntuiMessage *)GetMsg(window->UserPort))
         ReplyMsg(msg);
      CloseWindow(window);
      }
   if(IntuitionBase) CloseLibrary(IntuitionBase);
   if(GfxBase)       CloseLibrary(GfxBase);
   }


VOID countRtn()
   {
   int k;

   while(!PrepareToDie)
      {
      Resting = TRUE;
      for(k=0; k<RESTBLANKS; k++) WaitTOF();
      Resting = FALSE;
      k = 32 * max[nCPU];
      while(k--) counter++;
      }
   PrepareToDie = FALSE;
   Wait(0L);
   }


LONG MyVBlank()
   {

#ifdef IS_AZTEC
#asm
       movem.l  a2-a7/d2-d7,-(sp)
       move.l   a1,a4
#endasm
#endif

if(Calib)
   {
   if(calibi < CALIBS)
      {
      calib[calibi] = counter;
      counter = 0L;
      calibi++;
      }
   }
else if(!Resting)
   {
   if(++vblanks > howoften)
      {
      progress = counter / vblanks;
      vblanks  = 0L;
      counter  = 0L;
      }
   }


#ifdef IS_AZTEC
      ;   /* this is necessary */
#asm
      movem.l  (sp)+,a2-a7/d2-d7
#endasm
#endif

   return(0);  /* interrupt routines have to do this */
   }



/*
 *  Code to install/remove cycling interrupt handler
 */

char myname[] = "MyPMVB";  /* Name of interrupt handler */
struct Interrupt intServ;

typedef void (*VoidFunc)();

void StartVBlank()  {
#ifdef IS_AZTEC
   intServ.is_Data = GETAZTEC();  /* returns contents of register a4 */
#else
   intServ.is_Data = NULL;
#endif
   intServ.is_Code = (VoidFunc)&MyVBlank;
   intServ.is_Node.ln_Succ = NULL;
   intServ.is_Node.ln_Pred = NULL;
   intServ.is_Node.ln_Type = NT_INTERRUPT;
   intServ.is_Node.ln_Pri  = 0;
   intServ.is_Node.ln_Name = myname;
   AddIntServer(5,&intServ);
   }

void StopVBlank() { RemIntServer(5,&intServ); }

/**/




/* maxsize -- determine the total maximum size for all regions
 *   of the given type.  This code must be executed while
 *   FORBIDDEN (it accesses shared system structures).
 */
ULONG maxsize(ULONG t)
    {
    ULONG size = 0;
    struct MemHeader *mem;
    struct ExecBase  *eb = SysBase;

    /* THIS CODE MUST ALWAYS BE CALLED WHILE FORBIDDEN */
    Forbid();
    for (mem = (struct MemHeader *)eb->MemList.lh_Head;
       mem->mh_Node.ln_Succ;
          mem = (struct MemHeader *)mem->mh_Node.ln_Succ)
      {
      if (mem -> mh_Attributes & t)
         {
         size += ((ULONG) mem -> mh_Upper - (ULONG) mem -> mh_Lower);
         }
      }
    Permit();
    return(size);
    }

