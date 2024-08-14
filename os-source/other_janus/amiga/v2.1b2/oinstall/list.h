#ifndef LIST_H
#define LIST_H
#define F_TRANSFER   1
#define F_DELETE   2

typedef struct FileNode {
   struct FileNode *FNext; /* next file/dir */
   struct FileNode *FPrev; /* prev file/dir */
   long DirEntryType;      /* neg - file, pos - dir */
   char Name[31];         /* file/dir name */
   int NumBlocks;         /* block size */
   int Depth;            /* directry depth */
   int Flags;            /* don't transfer */
   int Num;            /* entry number */
} FILENODE;

typedef struct FileList {
   FILENODE *FirstFN;
   FILENODE *LastFNTail;
   FILENODE *LastFN;
} FILELIST;
#endif
