/* 
 *	dos_private.h
 *
 *	$Id: dos_private.h,v 36.2 92/04/05 23:52:55 jesup Exp $
 *	Copyright 1990, Commodore-Amiga, Inc.
 *
 *	private versions of dos structures, not for public consumption
 *	I wish we had a tool to hide these automatically!!!!
 */

#ifndef DOS_DOS_H
#include "dos/dos.h"
#endif

#ifndef DOS_STDIO_H
#include "dos/stdio.h"
#endif

struct NewFileHandle {
   ULONG fh_Link;		 /* This was never used, always had to be 0 */
#define fh_Flags fh_Link	 /* I'm therefore appropriating it for dos */

   struct MsgPort *fh_Port;	 /* Boolean, non-zero if it's interactive */
#define fh_Interactive fh_Port

   struct MsgPort *fh_Type;	 /* Port to do PutMsg() to */

   BPTR fh_Buf;		 /* buffer ptr */
   LONG fh_Pos;		 /* current position in buffer */
   LONG fh_End;		 /* size of buffer (write), or end of data (read) */

   /* do NOT play with these function pointers!!! */
   LONG fh_Funcs;
#define fh_Func1 fh_Funcs	/* function to be called for read  */
   LONG fh_Func2;		/* function to be called for write */
   LONG fh_Func3;		/* function to be called for close */

   /* these fields are private to the filesystem the file is open on */
   LONG fh_Args;
#define fh_Arg1 fh_Args		/* passed in most packets */
   LONG fh_Arg2;		/* never passed anywhere  */

   /* Fields added to the old structure - only here if FHF_EXTEND is true */
   LONG fh_BufSize;		/* buffer size */
   BPTR fh_DupBufPtr;		/* for seeing if we allocated the buffer! */

}; /* NewFileHandle */

#define FHB_EXTEND	0
#define FHF_EXTEND	1	/* private to dos */
#define FHB_FULL	BUF_FULL		/* happens to be 1 */
#define FHF_FULL	(1L << FHB_FULL)
#define FHB_NONE	BUF_NONE		/* happens to be 2 */
#define FHF_NONE	(1L << FHB_NONE)

#define FHB_USERBUF	7			/* I didn't allocate this */
#define FHF_USERBUF	(1L << FHB_USERBUF)
