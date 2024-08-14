/*
   Installation utility
*/
#include "exec/types.h"
#include <libraries/dosextens.h>
#include <exec/memory.h>
#include "extern.h"
#include "list.h"

#include "protos.h"

extern struct Gadget OkGadget;

#define DEBUG2   0
#define DEBUG3   0

#define MAXPATHLEN   128

#define MSG0 0
#define MSG1 1
#define MSG2 2
#define MSG3 3
#define MSG4 4
#define MSG5 5
#define MSG6 6
#define MSG7 7
#define MSG8 8
#define MSG9 9
#define MSG10 10
#define MSG11 11
#define MSG12 12
#define MSG13 13
#define MSG14 14
#define MSG15 15
#define MSG16 16
#define MSG17 17
#define MSG18 18
#define MSG19 19
#define MSG20 20
#define MSG21 21
#define MSG22 22
#define MSG23 23
#define MSG24 24
#define MSG25 25

/*UBYTE InstallNames[10][MAXPATHLEN]=0;*/ /* buf for Installation path */
UBYTE InstallNames[10][MAXPATHLEN]; /* buf for Installation path */

LONG GetDiskInfo(struct InfoData *info,UBYTE *name)
{
   struct FileLock *lock;
   LONG err= 0;

   if ((lock = (struct FileLock *)Lock(name, ACCESS_READ)) != NULL) {
      err=Info((BPTR)lock, info);
      D( kprintf("InfoData->id_NumSoftErrors=0x%lx\n",info->id_NumSoftErrors); )
      D( kprintf("InfoData->id_UnitNumber   =0x%lx\n",info->id_UnitNumber); )
      D( kprintf("InfoData->id_DiskState    =0x%lx\n",info->id_DiskState); )
      D( kprintf("InfoData->id_NumBlocks    =0x%lx\n",info->id_NumBlocks); )
      D( kprintf("InfoData->id_NumBlocksUsed=0x%lx\n",info->id_NumBlocksUsed); )
      D( kprintf("InfoData->id_BytesPerBlock=0x%lx\n",info->id_BytesPerBlock); )
      D( kprintf("InfoData->id_DiskType     =0x%lx\n",info->id_DiskType); )
      D( kprintf("InfoData->id_VolumeNode   =0x%lx\n",info->id_VolumeNode); )
      D( kprintf("InfoData->id_InUse        =0x%lx\n",info->id_InUse); )
      UnLock((BPTR)lock);
   }
   D( kprintf("GetDiskInfo() reuturns err=%ld\n",err); )
   return(err);
}

LONG Pass;

static struct List DiskList; /* list of disk devices */
static FILELIST IList;   /* list of installation files */
static FILELIST TList;   /* list of target files */
static struct FileHandle *fhin = NULL; /* file handle for lastword for exe cmd */
static struct FileHandle *fhout = NULL; /* file handle on NIL: for exe cmd */
static struct FileHandle *fhdef = NULL; /* file handle on RAM:DefaultFiles */
static struct InfoData *TInfo = NULL; /* info about target disk */
static char cmdbuf[128]; /* cmd buffer for Execute */
static LONG oldLock = -1;

VOID RemoveLastDir(UBYTE *path,LONG *depth)
{
   char *ptr;
   LONG len;

   if ((len = strlen(path)) != 0) { /* if path not null */
      ptr = &path[len - 1]; /* get ptr to last char */
      while (*ptr != '/' && *ptr != ':' && ptr != path) {
         ptr--;
      }   
      if (*ptr == ':') {
         ptr ++; /* dont want to overwrite device specifier */
      }
      *ptr = '\0';
   }
   (*depth)--;
}

LONG FixPath(FILENODE *node,UBYTE *path,LONG *depth)
{
   LONG len;

   while (node->Depth <= *depth) {
      RemoveLastDir(path, depth); /* get rid of previous dirs */
   }
   if (node->DirEntryType > 0) { /* dir, add to path */
      if ((len = strlen(path)) != 0) { /* if path not null */
         if (path[len - 1] != ':') { /* if not at device point */
            strcat(path, "/"); /* sufix path with dir spec */
         }
      }
      strcat(path, node->Name); /* tack on dir name */
      *depth = node->Depth;
   }
   return(node->DirEntryType > 0); /* return TRUE if entry was a dir */
}

LONG DeleteFlaggedFiles(FILELIST *list,UBYTE *dir,LONG flag)
{
   FILENODE *node;
   LONG Depth = -1;
   /* LONG len; */
   LONG err = 0; /* init to no error */
   LONG status;
   char path[128];
   /* char  *ptr; */

   D( kprintf("DFF enter: path='%s', flag=%ld\n", dir, flag); )
   strcpy(path, dir);
   for (node=list->FirstFN; node->FNext!=NULL; node=node->FNext) {
         status=FixPath(node, path, &Depth);

         if (node->Flags == flag) { /* if selected */
             strcpy(cmdbuf, path);
             if (*path != '\0') { /* if path not null */
            /* if not at device level */
            if (path[strlen(path) - 1] != ':')
                strcat(cmdbuf, "/"); /* add dir spec */
             }
             if(status)strzap(cmdbuf); /* a directory */
             else {
                 if((*(node->Name)=='*')&&(*((node->Name)+1)!='*'))
                strcat(cmdbuf,"*");
            strcat(cmdbuf, node->Name);
             }
             err=delete(cmdbuf);
             if((err == 205) || (err == 216))err=0;
             if(err) break;
         }
   }
   D( kprintf("DFF exit: err=%ld\n", err); )
   return(err); /* return error status */
}

LONG ReadLine(struct FileHandle *fh,UBYTE *buf)
{
   LONG err;

   do {
      err = Read((BPTR)fh, buf, 1);
   } while (err > 0 && *buf++ != 0x0a);
   *--buf = '\0';
   return(err > 0);
}


void main(int argc,UBYTE *argv[]);
void main(argc, argv)
int argc;
UBYTE *argv[];
{
   FILENODE *node, *tnode;
   LONG InstallUsed, TargetUsed, TargetFree, Selected, Required;
   LONG i, err;
   UBYTE devbuf[7]; /* buffer for disk device name */
   UBYTE drbuf[1 + 7 + 70]; /* buffer for device & drawer name */
   UBYTE defbuf[MAXPATHLEN]; /* buffer for default file names */
   UBYTE *InstallPath[10]; /* pointers to strings */
   UBYTE DefPath[MAXPATHLEN]; /* path to default files to delete */
   UBYTE LastPath[MAXPATHLEN]; /* path to LastWord file (if present) */
   UBYTE BasePath[MAXPATHLEN]; /* name of install disk */
   UBYTE IPath[MAXPATHLEN], TPath[MAXPATHLEN], TName[MAXPATHLEN];
   UBYTE *npoint,*ptr;
   LONG IDepth, TDepth;

   NewList(&DiskList);
   NewList((struct List *)&TList);
   NewList((struct List *)&IList);

   LastPath[0]=0;
   BasePath[0]=0;
   DefPath[0]=0;
   Pass=0;
   oldLock = -1;

   for(i=0; i<10; i++) InstallNames[i][0] = 0;
   for(i=0; i<10; i++) InstallPath[i]  = InstallNames[i];

   D( kprintf("\n"); )

   /* global initialization */
   if (!(Init(argc, argv, InstallPath,DefPath,LastPath,BasePath)))
      ShutDown(MSG1, 20);

   /* local initialization */
   if ((fhout = (struct FileHandle *)Open("NIL:",MODE_NEWFILE)) == NULL) ShutDown(MSG2, 20);

   if ((TInfo = AllocMem(sizeof(struct InfoData), MEMF_CHIP)) == NULL)
      ShutDown(MSG3, 20);

   PrintMsg(
"Please have the backup copy of your Workbench ready to install the files on.");
   
   /* Copy freq used cmds to ram: for speedy execution */
   if((err=DoExecute("Makedir ram:c"))&&((err != 203)&&(err != 205)))
      ShutDown(MSG4, 20);

   if((err=DoExecute("Makedir ram:t"))&&((err != 203)&&(err != 205)))
   {
      /* ShutDown(MSG24, 20); */
   }

   oldLock=(LONG)Lock("RAM:t",ACCESS_READ);  /* BILL had no access mode*/
   oldLock=(LONG)CurrentDir(oldLock);

   sprintf(cmdbuf,"Copy \"%s:c/RCopy\" to ram:c\0",BasePath);
   WaitLock("ram:c","");
   if(DoExecute(cmdbuf))                  ShutDown(MSG5, 20);

/*
   if(DoExecute("Copy c:Copy ram:c"))     ShutDown(MSG6, 20);
   if(DoExecute("Copy c:Run ram:c"))      ShutDown(MSG7, 20);
   if(DoExecute("Copy c:Delete ram:c"))   ShutDown(MSG8, 20);
   if(DoExecute("Copy c:Rename ram:c"))   ShutDown(MSG8, 20);
*/

   PrintMsg("Initializing...");

/*
   if(DoExecute("Copy c:Makedir ram:c"))  ShutDown(MSG9, 20);
   if(DoExecute("Copy c:Assign ram:c"))   ShutDown(MSG10, 20);
   if(DoExecute("Copy c:Execute ram:c"))  ShutDown(MSG11, 20);
*/

/*    if(DoExecute("Assign c: ram:c"))       ShutDown(MSG12, 20); */
   if(DoExecute("Path ram:c add"))       ShutDown(MSG12, 20);

   /* Get system disk configuration */
   GetDisks(&DiskList);

   /* Copy default files list to ram:DefaultFiles */
   if(*DefPath) {
       sprintf(cmdbuf,"ram:c/RCopy \"%s\" to ram:DefaultFiles\0",DefPath);
       DoExecute(cmdbuf);
   }
   /* Copy LastWord script file to ram:LastWord */
   if(*LastPath) {
       sprintf(cmdbuf,"ram:c/RCopy \"%s\" to ram:LastWord\0",LastPath);
       DoExecute(cmdbuf);
   }

   /* Copy all files in InstallPath to ram:Installation */
   PrintMsg("Copying files to ram:...");

   if((err=DoExecute("Makedir ram:Installation"))&&
      ((err != 203)&&(err != 205))) ShutDown(MSG14, 20);
   WaitLock("ram:Installation","");

   for(i=0; (*InstallPath[i] != 0); i++) {
       for(ptr=InstallPath[i]; *ptr != ':'; ptr++);
       ptr++;

       sprintf(cmdbuf,"makedir \"ram:Installation/%s\"\0",ptr);
       if(DoExecute(cmdbuf))ShutDown(MSG15, 20);

/*
       sprintf(cmdbuf,"ram:Installation/%s\0",ptr);
       WaitLock(cmdbuf,"");
*/

       sprintf(cmdbuf,
         "ram:c/RCopy \"%s\" to \"ram:Installation/%s\" all\0",InstallPath[i],ptr);
       if(DoExecute(cmdbuf))ShutDown(MSG15, 20);

       /* now for the info file */
       sprintf(cmdbuf,
      "ram:c/RCopy \"%s.info\" to ram:Installation\0",InstallPath[i]);
       DoExecute(cmdbuf);

       /* handle the + case */
       for(ptr=InstallPath[i]; *ptr != 0; ptr++)if(*ptr == '+') {
      strcpy(cmdbuf, ptr);
      WaitLock("ram:installation/",cmdbuf);
      PlusRename(cmdbuf);

      strcat(cmdbuf, ".info");
      PlusRename(cmdbuf);
       }
   }

   /* Make a list of all files in installation directory */
   PrintMsg("Examining files to install...");
   InstallUsed = DoExamineDir(&IList, "RAM:INSTALLATION", F_TRANSFER);

   /* Prompt user with above list and allow them to make changes */
   PrintMsg(
 "Select the files you want installed.   Default: ALL  (Hit OK when ready)");
/*123456789012345678901234567890123456789012345678901234567890123456789012*/
   Required = -InstallUsed;
   if ((Prompt(&IList, &InstallUsed, &Required, F_TRANSFER)) == -1)
      ShutDown(MSG13, 20);

   /*
      Go through entire list of installation files and delete those
      that the user has marked as not wanting to be transferred.
   */
   Pass++;

   if ((err = DeleteFlaggedFiles(&IList, "ram:Installation", 0)))
      ShutDown(MSG16, 20);
   /* once more for luck */
   if ((err = DeleteFlaggedFiles(&IList, "ram:Installation", 0)))
      ShutDown(MSG16, 20);

   /* Prompt user for target disk */
   err=0;
       drbuf[0]=0;
       MakeDevRequest(&DiskList, devbuf, drbuf, w,
           "INSERT and then SELECT disk device to install files on:");
   while (err==0) {
       ClearMsg();
       if ((DevRequest(&DiskList, devbuf, drbuf, w )) == -1)
         ShutDown(MSG17, 20);
      D( kprintf("Target disk is '%s', '%s'\n", devbuf, drbuf); )
         /* See if there is enough room on the target disk */
       /* Delay(300); *//* wait for drive light to go off */
       if(GetDiskInfo(TInfo, devbuf)) { 
           npoint=((UBYTE *)BADDR(((struct DeviceList *)BADDR(TInfo->id_VolumeNode))->dl_Name))+1;
           if(!stricmp(BasePath,npoint))Notice(w,
          "You cannot install files onto the Install disk!",
          "Please insert a Workbench disk to continue.");
           else err++;
       }
   }
   TargetUsed = TInfo->id_NumBlocksUsed;
   TargetFree = TInfo->id_NumBlocks - TargetUsed;

   /*
      Examine target disk.  Go through target disk looking for any
      installation files (ie. in ram:Installation), if found mark
      them for deletion.
   */
   PrintMsg("Examining target disk...");
   strcpy(cmdbuf,npoint);      /* get target disk name */
   D( kprintf("Target npoint is %s\n",cmdbuf); )
   D( kprintf("Target drbuf is %s\n",drbuf); )
   restcat(cmdbuf,drbuf);
   D( kprintf("Target restcat drbuf is %s\n",cmdbuf); )
   strcpy(drbuf,cmdbuf);
   D( kprintf("Target name is %s\n",drbuf); )


   DoExamineDir(&TList, drbuf, 0);
   Selected = 0;
   Required = InstallUsed - TargetFree +5;
   D( kprintf("Target disk examined, looking for matching files\n"); )
   D( kprintf("Install Used=%ld, TargetFree=%ld, Required=%ld, Selected=%ld\n",InstallUsed, TargetFree, Required, Selected); )
   IDepth = -1;
   *IPath = '\0';
   /* for each installation disk file entry */
   for (node=IList.FirstFN; node->FNext!=NULL; node=node->FNext) {
      if (node->Flags & F_TRANSFER) { /* if want it transferred */
         FixPath(node, IPath, &IDepth);
         TDepth = -1;
         *TPath = '\0';
         /* for each target disk file entry */
         for (tnode=TList.FirstFN; tnode->FNext!=NULL;tnode=tnode->FNext) {
            FixPath(tnode, TPath, &TDepth);
#if DEBUG2
            kprintf("Target: comparing paths '%s' and '%s'\n",
               IPath, TPath);
#endif
            if (stricmp(IPath, TPath) == 0) {
#if DEBUG2
            kprintf("Target: comparing names '%s' and '%s'\n",
                  node->Name, tnode->Name);
#endif
               if (stricmp(node->Name, tnode->Name) == 0) {
                  D( kprintf("matched '%s %s' and '%s %s'\n",IPath, node->Name, TPath, tnode->Name); )
                  tnode->Flags |= F_DELETE;
                  Selected += tnode->NumBlocks;
                  Required -= tnode->NumBlocks;
                  break; /* advance to next installation file entry */
               }
            }
         }
      }
   }

   /*
      If STILL not enough room on the disk, go through the default file
      list looking for any files on the target disk that match.
      If found mark them for deletion.
   */
   if (Required >= 0) {
      D( kprintf("Target disk examined, looking for matching default files\n"); )
      if (*DefPath) {
          if ((fhdef = (struct FileHandle *)Open("ram:DefaultFiles", MODE_OLDFILE)) == NULL)
         ShutDown(MSG18, 20);

          /* for each default file entry */
          while (ReadLine(fhdef, defbuf) && Required >= 0) {
         TDepth = -1;
         *TPath = '\0';
         /* for each target disk file entry */
         for (tnode=TList.FirstFN; tnode->FNext!=NULL;tnode=tnode->FNext) {
            FixPath(tnode, TPath, &TDepth);
            strcpy(TName, TPath);
            if (tnode->DirEntryType < 0) { /* not a dir, add to name */
               if (*TName != '\0') { /* if path not null */
                  /* if not at device level */
                  if (TName[strlen(TName) - 1] != ':') {
                     strcat(TName, "/"); /* add dir spec */
                  }
               }
               strcat(TName, tnode->Name);
            }
#if DEBUG3
            kprintf("Target: comparing '%s' and '%s'\n", defbuf, TName);
#endif
            if (!(stricmp(defbuf, TName))) {
               D( kprintf("matched '%s' and '%s'\n", defbuf, TName); )
               tnode->Flags |= F_DELETE;
               Selected += tnode->NumBlocks;
               Required -= tnode->NumBlocks;
               break; /* advance to next default file entry */
            }
         }
          }
      }
   }
   /* Prompt user with above list and allow them to make changes */
   /* a reset prop gadget should go here */
   ResetProp();
   InformMsg(Required);
   do {
      ResetProp();
      if (Prompt(&TList, &Selected, &Required, F_DELETE) == -1)
         ShutDown(MSG19, 20);

      if(Required >= 0)Notice(w,"You must select more files to delete.",
       "There isn't enough room on the Workbench disk yet.");
   } while (Required >= 0);

   /* Go through target disk and delete selected files */
   PrintMsg("Deleting files from target disk...");
   if(!(CheckWriteProtect(drbuf)))ShutDown(MSG20, 20);
   if ((err = DeleteFlaggedFiles(&TList,drbuf,F_DELETE)))ShutDown(MSG20, 20);
   /* once more for luck */
   if ((err = DeleteFlaggedFiles(&TList,drbuf,F_DELETE)))ShutDown(MSG20, 20);

   sprintf(cmdbuf,"Assign destination: \"%s\"\0",drbuf);
   if(DoExecute(cmdbuf))ShutDown(MSG21, 20);

   /* Copy installation files from ram:Installation to target disk */
   PrintMsg("Installing files on target disk...");

   sprintf(cmdbuf,"ram:c/RCopy ram:Installation to \"%s\" all\0",drbuf);
   if(DoExecute(cmdbuf))ShutDown(MSG22, 20);

   /* execute LastWord script file */
   if(*LastPath) {
   D( kprintf("LastWord time\n"); )
       if(fhin = (struct FileHandle *)Open("ram:LastWord", MODE_OLDFILE)) {
      D( kprintf("LastWord file open\n"); )
          Execute("", (BPTR)fhin,(BPTR) fhout);
      Close((BPTR)fhin);
       }
       else ShutDown(MSG23, 20);
   }
   ShutDown(MSG0, 0);
}

VOID ShutDown(LONG s,LONG err)
{
#if DEBUG
   kprintf("ERR: %ld, err=%ld, IoErr=%ld\n", s, err, IoErr());
   kprintf("error message: ");
   switch (s) {
      case MSG0:
      kprintf("end of program, normal exit\n");
      break;

      case MSG1:
      kprintf("Couldn't init\n");
      break;

      case MSG2:
      kprintf("Couldn't open NIL:\n");
      break;

      case MSG3:
      kprintf("Couldn't get mem for TInfo\n");
      break;

      case MSG4:
      kprintf("Couldn't Makedir ram:c\n");
      break;

      case MSG5:
      kprintf("Couldn't Copy RCopy ram:c\n");
      break;

      case MSG6:
      kprintf("Couldn't Copy c:Copy ram:\n");
      break;

      case MSG7:
      kprintf("Couldn't Copy c:Run ram:c\n");
      break;

      case MSG8:
      kprintf("Couldn't Copy sys:c/Delete ram:c\n");
      break;

      case MSG9:
      kprintf("Couldn't Copy sys:c/Makedir ram:c\n");
      break;

      case MSG10:
      kprintf("Couldn't Copy sys:c/Assign ram:c\n");
      break;

      case MSG11:
      kprintf("Couldn't Copy c:Execute ram:c\n");
      break;

      case MSG12:
      kprintf("Couldn't Assign c: ram:\n");
      break;

      case MSG13:
      kprintf("User aborted on Installation list prompt\n");
      break;

      case MSG14:
      kprintf("Couldn't Makedir ram:Installation\n");
      break;

      case MSG15:
      kprintf("Couldn't copy installation files to ram:\n");
      break;

      case MSG16:
      kprintf("DeleteFlaggedFiles(ram:Installation) failed\n");
      break;

      case MSG17:
      kprintf("User aborted on target device request\n");
      break;

      case MSG18:
      kprintf("Couldn't open ram:DefaultFiles\n");
      break;

      case MSG19:
      kprintf("User aborted on target list prompt\n");
      break;

      case MSG20:
      kprintf("DeleteFlaggedFiles(Target disk) failed\n");
      break;

      case MSG21:
      kprintf("Couldn't assign destination\n");
      break;

      case MSG22:
      kprintf("Couldn't copy ram:Installation to target disk\n");
      break;

      case MSG23:
      kprintf("Couldn't execute ram:LastWord\n");
      break;

      case MSG24:
      kprintf("Couldn't Makedir ram:t\n");
      break;

      default:
      kprintf("unknown error %ld\n",s);
   }
#endif
   if (w) PrintMsg("Cleaning up RAMdisk...");

      /* local cleanup */
      DoExecute("Delete ram:DefaultFiles");
      DoExecute("Delete ram:LastWord");
      DoExecute("Delete ram:Installation all");
      DoExecute("Assign c: sys:c");
      DoExecute("Delete ram:c all");
      DoExecute("Delete ram:t");

   if(oldLock != -1) {
      oldLock=(LONG)CurrentDir(oldLock);
      UnLock(oldLock);
   }
   if (TInfo) {
      FreeMem(TInfo, sizeof(struct InfoData));
   }
   if (fhdef)Close((BPTR)fhdef);
   if (fhout)Close((BPTR)fhout);
      
   EmptyDirList(&TList);
   EmptyDirList(&IList);
   FreeDisks(&DiskList);

   Bye(err);
}

VOID InformMsg(LONG Required)
{

    if(Required >= 0 ) PrintMsg(
    "Select files to delete until (Still Needed) is < 0.");
    else PrintMsg(
    "Ready to delete selected files.      (Hit OK to delete)");
   /*12345678901234567890123456789012345678901234567890123456789012345*/
}

LONG CheckWriteProtect(UBYTE *s)
{
   UBYTE t[80];
   LONG file;
   LONG error;

   strcpy(t,s);
   strcat(t,"+qweinstall");
   if(!(file=(LONG)Open(t,MODE_NEWFILE)))return(0);
   if((Write(file,"A",1))<=0)error=0;
   else error=1;
   Close((BPTR)file);
   DeleteFile(t);
   return(error);
}

LONG DoExecute(UBYTE *command)
{
   LONG err1,err2;

    err1= (Execute(command, (BPTR)NULL, (BPTR)fhout)==0);
    err2=IoErr();
    D( kprintf("EXECUTE: %s  (err1=%ld,err2=%ld)\n",command,err1,err2); )
    if(err2)return(err2);
    else return(err1);
}

VOID WaitLock(UBYTE *name1,UBYTE *name2)
{
   ULONG lock=0;
   LONG err=0;
   UBYTE buf[128];
   UBYTE *ptr;

   ptr=buf;
   strcpy(ptr,name1);
   strcat(ptr,name2);

   if(*ptr == '\"')
   {
      ptr++;
      buf[ScanQuote(ptr)]=0;
   }

    D( kprintf("Waiting for Lock on %s\n",ptr); )

    while ((lock == 0) && (err++ < 2)) {
       if(!(lock=(ULONG) Lock(ptr,ACCESS_WRITE))) {
      /* Delay(50); */
      /* Delay(25); */
   }
    }
    UnLock(lock);
}

LONG ScanQuote(UBYTE *string)
{
   UBYTE *ptr=string;
   LONG i=0;

   while((*ptr != 0) && (*ptr != '\"')){ptr++; i++;}
   return(i+1);
}

VOID PlusRename(UBYTE *name)
{
   UBYTE *name1=name;
   UBYTE buf1[128],buf2[128];
   LONG err;

   name1++;
   sprintf(buf1,"ram:Installation/%s\0",name);
   sprintf(buf2,"ram:Installation/%s\0",name1);

   D( kprintf("RENAME %s to %s\n",buf1,buf2); )

   err=Rename(buf1,buf2);

   D( kprintf("rename results are %ld, ioerr=%ld\n",err,IoErr()); )
}

LONG delete(UBYTE *name)
{
   LONG err;

   D( kprintf("DELETE: %s\n",name); )

   err=DeleteFile(name);

   D( kprintf("   result: err=%ld, ioerr=%ld\n",err,IoErr()); )

   if(!err)
   {
      if(((err=IoErr()) == 205) || (err==212) || (err=216))err=0;
         return(err);
   }
   else return(0);
}
