/**
*
* THIS CODE IS DECLARED PUBLIC DOMAIN.  Do with it what you will.
*
* This module has buffered replacements for standard C fxxx routines.
*
* Admittedly, they could use some more "bullet-proofing," but they work well
* "as-is."
*
* Usage: Call fopen(), fclose(), etc. as normal.  One caveat, though - you
*        **MUST** call "fcleanup()" below to make sure everything is
*        free'd at exit.
*
* Warnings about const/volatile conversions are because Commodore needs to
* add the 'const' keyword to their prototypes...  :-)
*
* Warning - fgets not implemented, nor fputs or fputc
**/

#ifdef FASTFILE
#include <exec/memory.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#extern struct Library *SysBase, *DOSBase;
#include "pragmas/exec_pragmas.h>
#include "pragmas/dos_pragmas.h>

#define BUFMAX 8192

typedef struct IFile
{
   BPTR fd;
   const char *name;
   long filepos;
   UBYTE *ibuffer;
   UBYTE *obuffer;
   long ibufpos, ibufsiz;
   long obufpos;
   struct IFile *prevfile;
   struct IFile *nextfile;
} IFile;

static IFile *filelisthead = NULL, *filelisttail = NULL;

/* Prototypes for functions in this module. */

struct IFile *fopen(const char *name, const char *mode);
int fclose(IFile *file);
size_t fread(APTR buffer, size_t size, size_t n, IFile *file);
size_t fwrite(APTR buffer, size_t size, size_t n, IFile *file);
int fseek(IFile *file, long position, int mode);
int fgetc(IFile *file);
int ftell(IFile *file);

/*****************************************************************************/
/*                                                                           */
/* fcleanup - general cleanup for buffered file routines.                    */
/*                                                                           */
/* Must be called at program exit to make sure all buffered files are closed */
/* and memory free'd, etc.                                                   */
/*                                                                           */
/*****************************************************************************/

void fcleanup(void)
{
   IFile *tfile;

   while (filelisthead)
   {
      tfile = filelisthead->nextfile;
      fclose(filelisthead);
      filelisthead = tfile;
   }
}

/*****************************************************************************/
/*                                                                           */
/* alcmem - private memory allocate routine.                                 */
/*                                                                           */
/* Check for memory allocate failure is done here to avoid having to do it   */
/* at every instance of memory allocation.  This way code is assured of      */
/* successful allocation, or program is exited before it can go on anyway!   */
/*                                                                           */
/*****************************************************************************/

static void *alcmem(long nbytes)
{
   char *p;

   if ((p = AllocMem(nbytes, MEMF_CLEAR)) == NULL)
   {
      Write(Output(), "Insufficient memory available.\n", 30);
      fcleanup();
      exit(RETURN_FAIL);
   }

   return(p);
}

/*****************************************************************************/
/*                                                                           */
/* fopen - open a buffered file.                                             */
/*                                                                           */
/* Currently, only supports mode "w" (write mode).  Anything else will open  */
/* the file in mode "r" (read mode).  Also, mode "w" always creates the file */
/* if it doesn't exist, and truncates it if it does.                         */
/*                                                                           */
/*****************************************************************************/

IFile *fopen(const char *name, const char *mode)
{
   IFile *file;
   long omode;

   file = (IFile *)alcmem(sizeof(struct IFile));

   /* I don't handle 'append' mode here at all. */
   omode = MODE_OLDFILE;
   if (mode[0] == 'w')
   {
       omode = MODE_NEWFILE;
   }

   if ((file->fd = Open(name, omode)) == 0)
   {
      FreeMem(file, sizeof(struct IFile));
      return(NULL);
   }
   file->name = name;

   if (filelisthead == NULL)
   {
      filelisthead = filelisttail = file;
   }
   else
   {
      file->prevfile = filelisttail;
      filelisttail->nextfile = file;
      filelisttail = file;
   }
   return(file);
}

/*****************************************************************************/
/*                                                                           */
/* fclose - close a buffered file.                                           */
/*                                                                           */
/* File **MUST** have been opened with fopen() routine above.  Results are   */
/* otherwise unpredictable, but guaranteed incorrect.                        */
/*                                                                           */
/*****************************************************************************/

int fclose(IFile *file)
{
   if (file->obufpos > 0)
   {
      Write(file->fd, file->obuffer, file->obufpos);
   }

   if (file->ibuffer)
   {
      FreeMem(file->ibuffer, BUFMAX);
      file->ibuffer = NULL;
   }

   if (file->obuffer)
   {
      FreeMem(file->obuffer, BUFMAX);
      file->obuffer = NULL;
   }

   Close(file->fd);

   if (file->prevfile)
   {
      file->prevfile->nextfile = file->nextfile;
   }
   else
   {
      filelisthead = NULL;
   }

   FreeMem(file, sizeof(struct IFile));

   return 0;
}

/*****************************************************************************/
/*                                                                           */
/* fread - read data from a buffered file.                                   */
/*                                                                           */
/* File **MUST** have been opened with fopen() routine above.  Results are   */
/* otherwise unpredictable, but guaranteed incorrect.                        */
/*                                                                           */
/*****************************************************************************/

size_t fread(APTR buffer, size_t size, size_t n, IFile *file)
{
   long l;

   if (file->ibuffer == NULL)
   {
      file->ibuffer = alcmem(BUFMAX);
      file->ibufpos = BUFMAX;
   }

   for (l=0; l<(size*n); )
   {
      if (file->ibufpos < file->ibufsiz)
      {
         ((UBYTE *)buffer)[l++] = file->ibuffer[file->ibufpos++];
         file->filepos++;
      }

      if (l == (size*n)) break;

      if (file->ibufpos >= file->ibufsiz)
      {
         __chkabort();
         file->ibufsiz = Read(file->fd, file->ibuffer, BUFMAX);
         if (file->ibufsiz <= 0)
         {
            break;
         }
         file->ibufpos = 0;
      }
   }

   if (file->ibufsiz <= 0)
   {
      return(0L);
   }

   return(n);
}

/*****************************************************************************/
/*                                                                           */
/* fwrite - write data to a buffered file.                                   */
/*                                                                           */
/* File **MUST** have been opened with fopen() routine above.  Results are   */
/* otherwise unpredictable, but guaranteed incorrect.                        */
/*                                                                           */
/*****************************************************************************/

size_t fwrite(APTR buffer, size_t size, size_t n, IFile *file)
{
   long l;

   if (file->obuffer == NULL)
   {
      file->obuffer = alcmem(BUFMAX);
      file->obufpos = 0;
   }

   for (l=0; l<(size*n); )
   {
      file->obuffer[file->obufpos++] = ((UBYTE *)buffer)[l++];
      file->filepos++;

      if (file->obufpos == BUFMAX)
      {
         __chkabort();
         Write(file->fd, file->obuffer, BUFMAX);
         file->obufpos = 0;
      }
   }
   return(n);
}

/*****************************************************************************/
/*                                                                           */
/* fseek - set file position for next read/write in buffered file.           */
/*                                                                           */
/* File **MUST** have been opened with fopen() routine above.  Results are   */
/* otherwise unpredictable, but guaranteed incorrect.                        */
/*                                                                           */
/*****************************************************************************/

int fseek(IFile *file, long position, int mode)
{
   long tmp_pos;

   if (mode == 1)
   {
      if (file->ibuffer)
      {
         tmp_pos = file->ibufpos + position;
         if ((tmp_pos >= 0) && (tmp_pos < BUFMAX))
         {
            file->ibufpos = tmp_pos;
            file->filepos += position;
            return(file->filepos);
         }
      }
   }

   if (file->obuffer && (file->obufpos > 0))
   {
      Write(file->fd, file->obuffer, file->obufpos);
   }

   Seek(file->fd, file->filepos + position, -1);

   file->ibufpos = BUFMAX;
   file->obufpos = 0;

   return 0;
}


/*****************************************************************************/
/*                                                                           */
/* ftell - report current file position                                      */
/*                                                                           */
/* File **MUST** have been opened with fopen() routine above.  Results are   */
/* otherwise unpredictable, but guaranteed incorrect.                        */
/*                                                                           */
/*****************************************************************************/

int ftell(IFile *file)
{
   return(fseek(file,0,1));  /* 0 from offset current */
}


/*****************************************************************************/
/*                                                                           */
/* fgetc - return next character of EOF (-1)                                 */
/*                                                                           */
/* File **MUST** have been opened with fopen() routine above.  Results are   */
/* otherwise unpredictable, but guaranteed incorrect.                        */
/*                                                                           */
/*****************************************************************************/

int fgetc(IFile *file)
{
UBYTE c;

if(fread(&c,1,1,file) > 0)	return((int)c);
else				return(-1);
}

#endif /* FASTFILE */
