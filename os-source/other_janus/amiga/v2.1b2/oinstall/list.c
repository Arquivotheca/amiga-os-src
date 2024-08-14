#include <libraries/dosextens.h>
#include <exec/memory.h>
#include "list.h"

#include "protos.h"

extern LONG Depth;

VOID AddFNode(FILELIST *list,struct FileInfoBlock *fib,LONG n)
{
   FILENODE *FN;

   FN = AllocMem(sizeof(FILENODE), MEMF_PUBLIC);
   if (FN != NULL) {
      FN->DirEntryType = fib->fib_DirEntryType;
      strcpy(FN->Name, fib->fib_FileName);
      FN->NumBlocks = fib->fib_NumBlocks + 1;
      FN->Depth = Depth;
      FN->Flags = 0;
      FN->Num = n;
      AddTail((struct List *)list,(struct Node *) FN);
   }
}

VOID RemoveFNode(FILENODE *FNode)
{
   Remove((struct Node *)FNode);
   FreeMem(FNode, sizeof(FILENODE));
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
