/***********************************************************************
****************                                        ****************
****************        -=  CDTV FILE SYSTEM  =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
*** Copyright (C) 1991 Commodore Electronics Ltd. All rights reserved***                                                                  ***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
***********************************************************************/

#include "main.h"
#include "fse.h"	/* KMY (2721) */

IMPORT	CDLOCK *LockFile();
IMPORT	char *ReadBlock();
IMPORT	APTR *AllocElem();

/***********************************************************************
***
***  OpenFile
***
***********************************************************************/
OpenFile(fh,lock,name)
	FHANDL *fh;
	CDLOCK *lock;
	char *name;
{
	CDLOCK *new;
	FCNTRL *fc;

	Debug2("\tO%lx.%s\n",lock,&name[1]);

	new = LockFile(lock,name,ACCESS_READ); /* also checks for vol,etc */
	if (!new) return DOS_FALSE;

	if (IsDirLock(new))
	{
		FreeLock(new);
		PktRes2 = ERROR_OBJECT_WRONG_TYPE;
		return DOS_FALSE;
	}

	fc = (FCNTRL *)AllocElem(FilePool);	/* CES 20.7 */
	if (!fc)
	{
		FreeLock(new);
		PktRes2 = ERROR_NO_FREE_STORE;
		return DOS_FALSE;
	}

	fh->fh_Type  = FSPort;
	fh->fh_Port  = NULL;
	fh->fh_Args  = (LONG)fc;/* passed as arg to read */

	fc->Lock    = new;
	fc->ReadPtr = 0;
	fc->BaseLBN = new->Key;	
	fc->Size    = new->ByteSize;
	fc->Flags   = 0;	/* V24.4 added */

	return DOS_TRUE;
}

/***********************************************************************
***
***  CloseFile
***
***********************************************************************/
CloseFile(fc)
	FCNTRL *fc;
{
	FreeLock(fc->Lock);
	FreeElem(FilePool,fc);	/* CES 20.7 */
	return DOS_TRUE;
}

/***********************************************************************
***
***  ReadFile
***
***	Ewhac/KMY (2414) (code to compensate for 512 byte chunks - scsi.device)
***
***********************************************************************/
ReadFile(fc,buf,len)
	REG FCNTRL *fc;
	REG char *buf;
	REG LONG len;
{
	REG char *b;
	REG ULONG p,n;
	REG LONG s,c;
	char temp[16];	/* larger than needed */

	Debug3("\tr%lx.%lx.%lx.%lx\n",buf,len,fc->ReadPtr,fc->Size);

	if (SourLock(fc->Lock)) return NULL;

	/* Calculate max allowed size for transfer: */
	p = fc->ReadPtr;
	s = fc->Size - p;	/* max possible length */
	if (s <= 0) return 0;
	s = MIN(s,len);		/* length of the read */

	/* KMY (2721/2804/2805) */
	FSE( FSEF_FILE, ( fc->BaseLBN + ( p >> BlockShift ) ), ( ( s + 2047 ) >> BlockShift ) );

	/* V23.3, V24.1 booleans added, V24.4 constant added */
	if (((fc->Flags & FC_DIRECTREAD) || DirectRead) &&
		(s > (CacheSize << BlockShift)) &&
		!ODD(p) && !ODD(buf) && !ODD(s))	
	{
		Debug3("\td%lx\n",s);
		DirectBytes += s;	/* V34.2 */

		p += fc->BaseLBN << BlockShift;

		if (UsingCDTV) {
			Copy8(&buf[s],temp);	/* V23.1 - for DMAC H/W problem*/
			ReadBytes(buf,p,s);
			Copy8(temp,&buf[s]);	/* V23.1 - for DMAC H/W problem*/
		} else {
			/*
			 * Must assume that device driver forces a granu-
			 * larity of 512 bytes.  This is, of course, stupid.
			 */
			len = s;
			if (p & 511) {
				/*
				 * Read initial portion of first sector.
				 */
				n = p & 511;		/*  block offset  */
				c = MIN (512 - n, len);	/*  count	  */
				p &= ~511;		/*  start pos	  */

				ReadBytes (ScratchBlock, p, 512);
				CopyMem (ScratchBlock + n, buf, c);
				p += 512;
				buf += c;
				len -= c;
			}

			if (c = len & ~511)
				/*
				 * Read in-between sectors.
				 */
				ReadBytes (buf, p, c);

			if (len & 511) {
				/*
				 * Read final piece of final sector.
				 */
				buf += c;
				p += c;
				len -= c;

				ReadBytes (ScratchBlock, p, 512);
				CopyMem (ScratchBlock, buf, len);
			}
		}
	}	
	else	
	{	
		/* Figure out starting block number and inner offset: */
		n = fc->BaseLBN + (p >> BlockShift);	/* block */
		p = p & BlockMask;	/* offset within the block */
		c = MIN(BlockSize-p,s);

		b = ReadBlock(n++);
		CopyMem(b+=p,buf,c);
		buf += c;

		/* Now aligned on block, transfer rest: */
		for (c = s - c; c > 0; c -= BlockSize)
		{
			b = ReadBlock(n++);
			CopyMem(b,buf,MIN(c,BlockSize));
			buf += BlockSize;
		}	
	}	

	fc->ReadPtr += s;
	return s;
}

#asm
	XDEF	_Copy8	; V23.1 for DMAC H/W problem
_Copy8:
		movem.l	4(sp),a0/a1
		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)+
		rts
#endasm

/***********************************************************************
***
***  SeekFile
***
***********************************************************************/
SeekFile(fc,pos,mode)
	REG FCNTRL *fc;
	REG LONG pos;
	REG LONG mode;
{
	REG LONG size;
	REG LONG oldpos;

	size = fc->Size;	/* max possible length */
	oldpos = fc->ReadPtr;

	if (SourLock(fc->Lock)) return -1;

	if (mode == 0) pos += fc->ReadPtr; /* OFFSET_CURRENT - Add the current position */
	else if (mode >  0) pos = size - pos;   /* OFFSET_END - Offset from end */
	/* else OFFSET_BEGINNING - Already a byte position */

	if (pos < 0 || pos > size)
	{
		PktRes2 = ERROR_SEEK_ERROR;
		return -1;
	}

	fc->ReadPtr = pos;

	return oldpos;
}
