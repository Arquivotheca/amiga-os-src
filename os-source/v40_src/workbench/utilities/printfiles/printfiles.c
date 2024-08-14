;/* printfiles.c - Execute me to compile me with Lattice 5.04
LC -b0 -cfistq -v -y -j73 printfiles.c
Blink FROM RXStartup.obj,printfiles.o TO printfiles LIBRARY LIB:Amiga.lib,LIB:LC.lib
quit
*/

/*
 *  PrintFiles.c -- Prints CLI or WorkBench passed files
 *  Pure - May be made resident
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>

#ifdef LATTICE
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/icon_protos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

#include "printfiles_rev.h"

#define BUFSIZE  512
#define CMDREV  "\0$VER: " VSTRING

struct Library *IconBase;

struct PFVars {
   BPTR   pfile;
   BOOL   FromWb, FormFeed;
   struct Library *iconbase;
   UBYTE  buf[BUFSIZE];
   };

int  printIt(BPTR file, struct PFVars *pf);
void cliPrint(int argc,char **argv,struct PFVars *pf);
void wbPrint(struct WBStartup *wbMsg, struct PFVars *pf);
void cleanexit(UBYTE *, LONG, struct PFVars *);

char usage[] = "Usage: Printfiles [-f] file [file file...] (-f=formfeed)\n";

char copyright[] = 
  "Copyright 1988  Commodore-Amiga, Inc. All Rights Reserved" CMDREV ;

void main(int argc, char **argv)
   {
   struct PFVars *pf;
   struct WBStartup *WBenchMsg;

   if(!(pf=(struct PFVars *)
    AllocMem(sizeof(struct PFVars),MEMF_CLEAR|MEMF_PUBLIC)))
      {
      if(argc) Write(Output(),"Not enough memory\n",18);
      exit(RETURN_FAIL);
      }

   pf->FromWb = (argc==0) ? TRUE : FALSE;
   if(pf->FromWb) WBenchMsg = (struct WBStartup *)argv;
   pf->FormFeed = FALSE;

   if((argc)&&((argc == 1) ||  (*argv[argc-1] == '?')))
      cleanexit(usage,RETURN_OK,pf);

   if(argc>1) {                 /* Passed filenames via command line  */
      if(pf->pfile=Open("prt:",MODE_OLDFILE)) {
         cliPrint(argc,argv,pf);
         Close(pf->pfile);
         }
      }
   else if ((pf->FromWb)&&(WBenchMsg->sm_NumArgs > 1)) {
                             /* Passed filenames via  WorkBench */
      if(pf->pfile=Open("prt:",MODE_OLDFILE))
         {
         wbPrint(WBenchMsg,pf);
         Close(pf->pfile);
         }
      }
   cleanexit("",RETURN_OK,pf);
   }


void cliPrint(int argc,char **argv,struct PFVars *pf)
   {
   int i, Success=1;
   BPTR file;

   for (i=1; (i<argc)&&(Success); i++) {
      if((*argv[i] == '-')&&(*(argv[i]+1)=='f')&&(*(argv[i]+2)==NULL))
		pf->FormFeed = TRUE;
      else if(file=Open(argv[i],MODE_OLDFILE)) {
         Success = printIt(file,pf);
         Close(file);
         }
      }
   }


void wbPrint(struct WBStartup *wbMsg, struct PFVars *pf)
   {
   struct WBArg  *wbArg;
   BPTR startLock, oldLock, file;
   struct DiskObject *diskobj;
   char *name;
   char **toolarray;
   char *string;
   int i;

   wbArg = wbMsg->sm_ArgList;
   startLock    = wbArg->wa_Lock;
   
    if(pf->iconbase = OpenLibrary("icon.library", 0)) {
        IconBase = pf->iconbase;
    	diskobj=(struct DiskObject *)GetDiskObject(wbArg->wa_Name);
    	if(diskobj) {
            toolarray = (char **)diskobj->do_ToolTypes;
	    if(string=FindToolType(toolarray,"FORMFEED"))
		if(MatchToolValue(string,"TRUE")) pf->FormFeed=TRUE;
	FreeDiskObject(diskobj);
	}
    CloseLibrary(IconBase);
    }
   wbArg++;   /* Skip tool arg */

   for(i=1; i<wbMsg->sm_NumArgs; i++,wbArg++)
      {
      name = wbArg->wa_Name;
      if((name[0]=='-')&&(name[1]=='f')&&(name[2]==NULL))
         {
         pf->FormFeed = TRUE;
         }
      else if(oldLock = CurrentDir(wbArg->wa_Lock))
         {
         if(file = Open(wbArg->wa_Name, MODE_OLDFILE))
            {
            printIt(file,pf);
            Close(file);
            }
         }
      }
   CurrentDir(startLock);
   }


int printIt(BPTR file, struct PFVars *pf)
   {
   int rLen, wLen, Success;

   Success = rLen = wLen = 1;
   while((rLen>0)&&(wLen>0))
      {
      rLen = Read(file,pf->buf,BUFSIZE);
      if (rLen>0)   wLen = Write(pf->pfile,pf->buf,rLen);
      }
   if (wLen > 0) Write(pf->pfile,"\014",1);
   if((rLen==-1)||(wLen==-1))  Success = 0;
   return(Success);
   }

void cleanexit(UBYTE *s, LONG n, struct PFVars *pf)
   {
   if((pf)&&(*s)&&(!pf->FromWb))  Write(Output(),s,strlen(s));
   if(pf) FreeMem(pf,sizeof(struct PFVars));
   exit(n);
   }
