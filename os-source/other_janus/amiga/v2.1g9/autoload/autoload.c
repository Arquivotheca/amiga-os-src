/* *** autoload.c **********************************************************
 * 
 * autoload.c  --  hey baby, we'll be back to pick you up later
 *                  (Janus Autoloader)
 * 
 * Created by RJ Mical
 * 
 * Date        Name               Description
 * ----------  -----------------  ------------------------------------------
 * 11 Oct 88   -RJ Mical-         Created this file!
 * 
 * ********************************************************************** */

#define LINT_ARGS
#define PRAGMAS

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <workbench/icon.h>
#include <workbench/workbench.h>
#include <janus/janus.h>
#include <proto/all.h>
#include <string.h>

#include "autoload.h"

#define D(x) ;
#define printf kprintf

#define SetFlag(v,f)       ((v)|=(f))
#define ClearFlag(v,f)     ((v)&=~(f))
#define ToggleFlag(v,f)    ((v)^=(f))
#define FlagIsSet(v,f)     ((BOOL)(((v)&(f))!=0))
#define FlagIsClear(v,f)   ((BOOL)(((v)&(f))==0))

struct   JanusBase *JanusBase = NULL;
struct   IconBase  *IconBase  = NULL;
struct   FileHandle *nilfh    = NULL;

/* === ================================================================== */
/* ===  main() ========================================================== */
/* === ================================================================== */

VOID Autoload();

VOID   main(argc, argv)
int   argc;
char   **argv;
{
   ULONG    signal,error,sigmask;
   struct   ServiceData *sdata;
   struct   ServiceData *sdata_WA;
   struct   ServiceData *sdata_BA;
   struct   AutoLoad    *al;
   USHORT   i;

   /* Initialize our signal so we know whether or not to free it */
   signal = -1;
   sdata  = NULL;

   /* Is janus.library out there?  If not, we've little to do */
   JanusBase = (struct JanusBase *)OpenLibrary("janus.library",33);
   if (JanusBase==0) goto DONE;

   D( kprintf("AL:janus.library = 0x%lx\n", JanusBase); )

   /* Any signals for us to allocate? */
   signal = AllocSignal(-1);
   if (signal == -1) goto DONE;
   sigmask = 1<<signal;

   D( kprintf("AL:signal=$%lx\n", (ULONG)(signal)); )

   /* Add the Autoloader service */
   error = AddService(&sdata, AUTOLOAD_APPID, AUTOLOAD_LOCALID,
         sizeof(struct AutoLoad),
         MEMF_BUFFER | MEM_WORDACCESS,
         signal, ADDS_LOCKDATA);
   if (error != JSERV_OK)
   {
      D( kprintf("AL:couldn't AddS()\n"); )
      /* ??? If couldn't add, try to get and then expunge and release */
      if (GetService(&sdata, AUTOLOAD_APPID, AUTOLOAD_LOCALID,
            signal, NULL) == JSERV_OK)
      {
         D( kprintf("AL:GetS() good!\n"); )
#if STONED
         SetFlag(sdata->sd_Flags, EXPUNGE_SERVICE);
         CallService(sdata);
#endif
         ReleaseService(sdata);
      }
      goto DONE;
   }

   nilfh = (struct FileHandle *)Open("NIL:",MODE_NEWFILE);
   if( ! nilfh ) goto DONE;

   D( kprintf("AL:AddS() good! sdata = %lx\n",sdata); )
   sdata_BA = (struct ServiceData *)MakeBytePtr(sdata);
   sdata_WA = (struct ServiceData *)MakeWordPtr(sdata);

   /* Initialize the buffer */
   D( kprintf("AL:AmigaMemPtr = %lx\n",sdata->sd_AmigaMemPtr); )
   al = (struct AutoLoad *) sdata->sd_AmigaMemPtr;
   al->Entries = AUTOLOAD_ENTRYCOUNT;
   D( kprintf("AL:al = %lx\n",al); )
   D( kprintf("AL:al->Entries = %ld\n",al->Entries); )
   for (i = 0; i < al->Entries; i++)
   {
      al->Entry[i].AppID   = 0;
      al->Entry[i].LocalID = 0;
   }

   /* When we added, we specified ADDS_LOCKDATA so the ServiceData 
    * would be locked by us before being added to the system.  
    * This allowed us to initialize the buffer memory before 
    * anyone else could sneak in and start using it.
    * Now we musy unlock the ServiceData structure so anyone can 
    * use it.  As we're the Amiga-side, we must also release 
    * the semaphore.
    */
   /* Unlock the ServiceData structure so anyone can use it.  */
   UnlockServiceData(sdata_BA);

   for(;;)
   {
      /* Wait for autoload requests */
      D( kprintf("AL: waiting\n"); )
      Wait(sigmask);

      D( kprintf("AL:Called\n"); )

      /* Does someone want us to expunge? */
      if (FlagIsSet(sdata_WA->sd_Flags, EXPUNGE_SERVICE))
      {
         /* If someone wants us to expunge, 
          * DeleteService() and split
          */
         D(kprintf("AL: deleting service\n");)
         DeleteService(sdata);
         D(kprintf("AL: deleted service\n");)
         goto DONE;
      }

      /* Lock ServiceData semaphore */
      Forbid();

      /* Lock ServiceData lock */
      LockServiceData(sdata_BA);

      D( kprintf("AL:AmigaMemPtr = %lx\n",sdata->sd_AmigaMemPtr); )
      /* Process any autoload requests, resetting as we go */
      for (i = 0; i < al->Entries; i++)
      {
         if (al->Entry[i].AppID)
         {
            D( kprintf("AL:Calling aload for %ld,%ld\n",al->Entry[i].AppID,al->Entry[i].LocalID); )
            Autoload(al->Entry[i].AppID, al->Entry[i].LocalID);
            al->Entry[i].AppID = 0;
         }
      }

      /* Unlock ServiceData lock */
      UnlockServiceData(sdata_BA);

      /* Unlock ServiceData semaphore */
      Permit();
   }


DONE:
   if(nilfh) Close(nilfh);
   if (signal != -1)
   {
      FreeSignal(signal);
      signal = -1;
   }
   if (JanusBase)
   {
      CloseLibrary(JanusBase);
      JanusBase = NULL;
   }

   D(kprintf("AL: bye bye!\n");)
}

VOID Autoload(AppID,LocalID)
ULONG AppID;
USHORT LocalID;
{
   char   servicedir[] = "SYS:PC/Services";
   struct FileInfoBlock *fib = NULL;
   struct Lock *servicelock = NULL;
   struct Lock *olddir = NULL;
   int    len;
   struct DiskObject *dobj = NULL;
   char   savechar;
   char   *toolstring;
   char   exstring[255];

   IconBase = (struct IconBase *)OpenLibrary(ICONNAME,0);
   if( ! IconBase ) goto end;

   fib = (struct FileInfoBlock *) AllocMem(sizeof(struct FileInfoBlock),MEMF_CLEAR);
   if( ! fib ) goto end;

   servicelock = (struct Lock *)Lock(servicedir,SHARED_LOCK);
   if(servicelock==0) goto end;

   olddir = (struct Lock *)CurrentDir(servicelock);

   if( ! Examine(servicelock,fib) || fib->fib_DirEntryType <= 0)
      goto setcurdir;

   while( ExNext(servicelock,fib))
   {
      len = strlen(fib->fib_FileName);
      if((strcmp(&fib->fib_FileName[len-5],".info")==0)&&(len>5))
      {
         savechar = fib->fib_FileName[len-5];
         fib->fib_FileName[len-5]='\0';
         dobj = GetDiskObject(fib->fib_FileName);
         if(dobj)
         {
            toolstring = FindToolType(dobj->do_ToolTypes,"AUTOLOAD");
            if(toolstring)
            {
               D( kprintf("AL:ToolType = %s\n",toolstring); )
               if(MatchIDs(toolstring,AppID,LocalID))
               {
                  /* execute the damn thing! */
                  D( kprintf("AL:MATCH FOUND for file %s!\n",fib->fib_FileName); )
                  D( kprintf("AL:nilfh = %lx\n",nilfh); )
                  sprintf(exstring,"run <NIL: >NIL: SYS:PC/Services/%s <NIL: >NIL:",fib->fib_FileName);
                  D( kprintf("AL:Execute %s\n",exstring); )

/*
                  Execute(exstring,0,0);
*/
                  Execute(exstring,0,nilfh);
               }
            }
            FreeDiskObject(dobj);
            dobj=0;
         }
         fib->fib_FileName[len-5]=savechar;
      }

   }

setcurdir:
   CurrentDir(olddir);
end:
   if(servicelock) UnLock(servicelock);
   if(fib) FreeMem(fib,sizeof(struct FileInfoBlock));
   if(IconBase)
   {
      CloseLibrary(IconBase);
      IconBase=NULL;
   }
}
MatchIDs(p,AppID,LocalID)
char   *p;
ULONG  AppID;
USHORT LocalID;
{
   int len;
   int i;
   char *p2;
   char savechar;
   int slash = 0;
   int saveindex;
   int result = FALSE;

   len = strlen(p);
   for(i=0;i<len;i++)
   {
      if(p[i]=='/')
      {
         savechar=p[i];
         p[i]='\0';
         p2=&p[i+1];
         slash = TRUE;
         saveindex=i;
      }
   }
   if( ! slash )
      return(FALSE); /* need to find two numbers */

   if(AppID != atoi(p))
   {
      result=FALSE;
      goto DONE;
   }
   if(LocalID != atoi(p2))
   {
      result=FALSE;
      goto DONE;
   }
      result = TRUE;
DONE:
      p[saveindex]=savechar;
      return(result);
}
