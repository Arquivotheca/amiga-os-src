#include "exec/types.h"
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include "list.h"

#include "protos.h"

static LONG Depth, NumEntries, TotalBlocks;

VOID AddFNode(FILELIST *list,struct FileInfoBlock *fib,LONG flag)
{
   FILENODE *FN;

   if ((FN = AllocMem(sizeof(FILENODE), MEMF_PUBLIC)) != NULL) {
      FN->DirEntryType = fib->fib_DirEntryType;
      strcpy(FN->Name, fib->fib_FileName);
      FN->NumBlocks = fib->fib_NumBlocks + 1;
      FN->Depth = Depth;
      FN->Flags = flag;
      FN->Num = NumEntries;
      AddTail((struct List *)list,(struct Node *) FN);
   }
#if DEBUG
   else {
      kprintf("AFN: Couldn't get mem for FileNode!\n");
   }
#endif
}

VOID RemoveFNode(FILENODE *FNode)
{
   Remove((struct Node *)FNode);
   FreeMem(FNode, sizeof(FILENODE));
}

/*
   This routine examines all entries (files and directories) in a given
   directory and makes a list of all these extries.  It also keeps the
   total number of blocks used by all the entries and the total number
   of entries.  Note that it calls itself.
*/
LONG ExamineDir(FILELIST *list,UBYTE *name,LONG flag)
{
   struct FileLock *lock;
   struct FileInfoBlock *fib;
   char path[128];

   D2( kprintf("ED: depth=%ld, locking name='%s'\n", Depth, name); )
   /* get lock on file/dir */
   lock = (struct FileLock *)Lock(name, ACCESS_READ);
   /* if got lock */
   if (lock != NULL) {
      D2( kprintf("ED: calling AllocMem\n"); )
      /* get mem for fib */
      fib = AllocMem(sizeof(struct FileInfoBlock), MEMF_PUBLIC);
      /* if got mem */
      if (fib != NULL) {
         D2( kprintf("ED: calling Examine\n"); )
         /* try to examine dir/file */
         if (Examine((BPTR)lock, fib)) {
            /* if this is a dir */
            if (fib->fib_DirEntryType > 0) {
               /* get first entry in this dir, while more entries */
               while (ExNext((BPTR)lock, fib)) {
                  D2( kprintf("ED: calling AddFNode\n"); )
                  AddFNode(list, fib, flag);
                  TotalBlocks += fib->fib_NumBlocks + 1;
                  NumEntries++;
                  /* if found another dir, build path and recurse */
                  if (fib->fib_DirEntryType > 0) {
                     Depth++;
                     strcpy(path, name);
                     if (path[strlen(path) - 1] != ':') {
                        strcat(path, "/");
                     }
                     strcat(path, fib->fib_FileName);
                     D2( kprintf("ED: calling ExamineDir\n"); )
                     ExamineDir(list, path, flag);
                  }
               }
            }
            else { /* top entry not a dir */
               NumEntries++;
               TotalBlocks += fib->fib_NumBlocks + 1;
            }
         }
#if DEBUG
         else {
            kprintf("ED: Error, couldn't examine '%s'!\n", name);
         }
#endif
         FreeMem(fib, sizeof(struct FileInfoBlock));
      }
#if DEBUG
      else {
         kprintf("ED: Error, couldn't get mem for fib!\n");
      }
#endif
      UnLock((BPTR)lock);
   }
#if DEBUG
   else {
      kprintf("ED: Error, couldn't get lock on '%s'!\n", name);
   }
#endif
   if (Depth > 0) {
      Depth--;
   }
   return(TotalBlocks);
}

LONG DoExamineDir(FILELIST *list,UBYTE *name,LONG flag)
{
   Depth = 0;
   NumEntries = 0;
   TotalBlocks = 0;
   return(ExamineDir(list, name, flag));
}

VOID EmptyDirList(FILELIST *list)
{
   FILENODE *FNode, *FNext;

   /* free up mem used by all entries in list */
   for (FNode=list->FirstFN; FNode->FNext != NULL; FNode=FNext) {
      FNext = FNode->FNext;
      RemoveFNode(FNode);
   }
}
