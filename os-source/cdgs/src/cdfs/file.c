/***********************************************************************
****************                                        ****************
****************        -=   CD FILE SYSTEM   =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
*** Copyright (C) 1991 Commodore Electronics Ltd. All rights reserved***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
***********************************************************************/

#include "main.h"

#define D(x)    ;       /* Enable/disable tracking debugs */
#define DD(x)   ;

#define NO_FUNNY_ALIGNMENT

IMPORT  CDLOCK *LockFile();
IMPORT  char *ReadBlock();
IMPORT  APTR *AllocElem();
IMPORT  LONG InCache();
IMPORT  LONG AbortCacheAllBut();
IMPORT	void StartCacheRead();
IMPORT  LONG Remove();

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

    fc = (FCNTRL *)AllocElem(FilePool); /* CES 20.7 */
    if (!fc)
    {
        FreeLock(new);
        PktRes2 = ERROR_NO_FREE_STORE;
        return DOS_FALSE;
    }
    D(Debug0("Open  : fc=%lx : %s\n",fc,&name[1]));

    fh->fh_Type  = FSPort;
    fh->fh_Port  = NULL;
    fh->fh_Args  = (LONG)fc;/* passed as arg to read */

    fc->Lock    = new;
    fc->ReadPtr = 0;
    fc->BaseLBN = new->Key; 
    fc->Size    = new->ByteSize;
    fc->Flags   = 0;    /* V24.4 added */

    return DOS_TRUE;
}

/***********************************************************************
***
***  OpenFileFromLock
***
***********************************************************************/
OpenFileFromLock(fh,lock)
    FHANDL *fh;
    CDLOCK *lock;
{
    FCNTRL *fc;

    if (!lock) return DOS_FALSE;

    if (IsDirLock(lock))
    {
        PktRes2 = ERROR_OBJECT_WRONG_TYPE;
        return DOS_FALSE;
    }

    fc = (FCNTRL *)AllocElem(FilePool); /* CES 20.7 */
    if (!fc)
    {
        PktRes2 = ERROR_NO_FREE_STORE;
        return DOS_FALSE;
    }
    D(Debug0("OpenFromLock  : fc=%lx\n",fc));

    fh->fh_Type  = FSPort;
    fh->fh_Port  = NULL;
    fh->fh_Args  = (LONG)fc;/* passed as arg to read */

    fc->Lock    = lock;
    fc->ReadPtr = 0;
    fc->BaseLBN = lock->Key; 
    fc->Size    = lock->ByteSize;
    fc->Flags   = 0;    /* V24.4 added */

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
    D(Debug0("Close : fc=%lx\n",fc));
    

    FreeLock(fc->Lock);
    FreeElem(FilePool,fc);  /* CES 20.7 */
    return DOS_TRUE;
}

/***********************************************************************
***
***  ReadFile
***
***********************************************************************/
ReadFile(fc,buf,len)
    REG FCNTRL *fc;
    REG char *buf;
    REG LONG len;
{
    REG char *b;
    REG char *direct_buf;
    REG ULONG direct_n,n;
    REG LONG  direct_size;
    ULONG p,eof;
    LONG s,size;
    ULONG direct_offset;

    Debug3("\tr%lx.%lx.%lx.%lx\n",buf,len,fc->ReadPtr,fc->Size);
    D(Debug0("R:$%lx ", len));

    if (SourLock(fc->Lock)) return NULL;

    /* Calculate max allowed size for transfer: */
    p = fc->ReadPtr;
    s = fc->Size - p;           /* max possible length */
    if (s <= 0) return 0;           /* must be at end of file */

    s = MIN(s,len);             /* length of the read */
    size = s;               /* one I can modify */

    /* change ReadPtr here so QuickRead is safe */
    fc->ReadPtr += s;           /* original read size */

    eof = fc->BaseLBN + (fc->Size >> BlockShift); /* last block of file */
    n   = fc->BaseLBN + (p >> BlockShift);  /* starting block */
    p   = p & BlockMask;            /* offset within the block */

    /*
    **  Note the direct read code does a backwards seek.  It
    **  Should SendIO() the block portion of the big request,
    **  then copy the available data from the buffer.
    **  Fixed - REJ
    */
    if ((size >= (BlockSize << 1)) &&
        !ODD(p) && !ODD(buf) && !ODD(size) )    
    {
        /* for large direct reads, use any cache blocks we hit */
        /* if we miss on the first, no real need to check the rest */
        /* NOTE: size MUST be larger than cache, therefor we KNOW we */
        /* will have more left to read after cache hits!!! - REJ */
        direct_n    = n;
        direct_buf  = buf;
        direct_size = size;

        /* round off to end on a block boundary */
        /* we far prefer to end the transfer via a buffer if it's */
        /* not on a block boundary (to hit the next read).    */
        direct_size -= (direct_size - (BlockSize-p)) & BlockMask;

        /* First, only look for blocks that are completed */
        while (InCache(direct_n,TRUE) &&
               direct_size >= BlockSize)
        {
            /* size will never be <= 0 because of this */
            /* was this a partial-block hit? */
            if (n == direct_n)
            {
                direct_buf  += BlockSize-p;
                direct_size -= BlockSize-p;
            } else {
                direct_buf  += BlockSize;
                direct_size -= BlockSize;
            }
            direct_n++;
        }

        /* for the first missed block, is it already underway? */
        if (direct_size >= BlockSize)
        {
            if (InCache(direct_n,FALSE))
            {
                /* abort unfinished requests other than direct_n */
                AbortCacheAllBut(direct_n);

                DD(Debug0("include unfinished block %ld\n",
                     direct_n));
                if (n == direct_n)
                {
                    direct_buf  += BlockSize-p;
                    direct_size -= BlockSize-p;
                } else {
                    direct_buf  += BlockSize;
                    direct_size -= BlockSize;
                }
                direct_n++;
            } else {
                /* abort all unfinished requests */
                AbortCacheAllBut(0);

#ifdef NO_FUNNY_ALIGNMENT
		/* if we're on a block boundary, just do a direct read.   */
		/* if not, then we need to read a partial block first.    */
		/* (we haven't started this block yet if we quick-seeked) */
		/* p is starting offset of first block */
		if (direct_n == n && p != 0) /* only if first block! */
		{
		    DD(Debug0("misaligned read start: block %ld, size $%lx\n",
			      direct_n,BlockSize-p));

		    /* Don't copy to dest now, get next read going ASAP and */
		    /* then do normal "first block in cache" code.	    */
		    /* NOTE: we must use 0 - no others are guaranteed avail */
		    /* we aborted all so we know 0 is available		    */
		    StartCacheRead(0,direct_n);

		    /* just like above */
		    direct_buf  += BlockSize-p;
		    direct_size -= BlockSize-p;
		    direct_n++;

		    /* make sure we don't try to copy as a cache hit later */
		}
#endif
            }
        } /* don't abort if we aren't going to do a direct read */

        /* ok, we've found where to start the direct read (if any) */
        if (direct_size > 0)
        {
	    /* since we rounded direct_size off to end on a block, if it's */
	    /* greater than 0 it MUST be a multiple of BlockSize!          */

            Debug3("\td%lx\n",direct_size);
            DD(Debug0("DRead!   :DirectBytes=%lx+S=%lx,fc->BaseLBN=%ld, b=%ld\n",
                 DirectBytes,direct_size,fc->BaseLBN,direct_n));
            D(Debug0("DR %ld ",direct_n));

            DirectBytes += direct_size; /* V34.2 */

            direct_offset = direct_n << BlockShift;

#ifndef NO_FUNNY_ALIGNMENT
            /* if we didn't get any hits, start on exact byte */
            if (direct_n == n)
                direct_offset += p;
#endif

            DD(Debug0("StartRead($%lx,$%lx(%ld),$%lx)\n",
                 direct_buf,direct_offset,direct_n,direct_size));
            StartReadBytes(direct_buf,direct_offset,direct_size);
        }

        /* now that the direct read is started, copy the cached data */
        /* Copy partial initial block then copy full blocks */
        for (; n < direct_n; n++)
        {
            if (p) {
                DD(Debug0("partial: b %ld, $%lx bytes to $%lx\n",
                     n,BlockSize-p,buf));
            } else {
                DD(Debug0("Full: b %ld, to $%lx\n", n,buf));
            }
            /* don't send new requests! */
            b = ReadBlock(n,eof,TRUE,FALSE);
            CopyMem(b+p,buf,BlockSize-p);
            buf  += BlockSize-p;
            size -= BlockSize-p;
            p = 0;      /* partial only on first block */
        }

        /* done copying hits, now wait for direct read to complete */
        if (direct_size > 0)
        {
            /* must pass the same args as before */
            D(Debug0("Waiting..."));
            WaitReadBytes(direct_buf,direct_offset,direct_size);
            D(Debug0("Awake.."));

            /* handle miss on first partial */
            if (p != 0)
                direct_n++; /* first full block xfered */
            n    = direct_n + (direct_size >> BlockShift);
            size -= direct_size;
            buf  = direct_buf + direct_size;
        }

        /* set up ptrs to allow "normal" code to finish last block */
        /* we always finish on a block boundary */
        p = 0;

        DD(Debug0("Left over: $%lx bytes at block %ld to $%lx\n",
             size,n,buf));
    }   

    /* handles non-aligned reads, small reads, and final blocks of big */
    /* aligned reads.  Use remaining size here!            */
    if (size > 0)
    {
        REG LONG c;

        c = MIN(BlockSize-p,size);

        b = ReadBlock(n++,eof,fc->Flags & FC_SEEKBACK,FALSE);
        CopyMem(b+=p,buf,c);
        buf += c;

        /* Now aligned on block, transfer rest: */
        for (c = size - c; c > 0; c -= BlockSize)
        {
            b = ReadBlock(n++,eof,fc->Flags & FC_SEEKBACK,FALSE);
            CopyMem(b,buf,MIN(c,BlockSize));
            buf += BlockSize;
        }
    }

    return s;
}


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

    size = fc->Size;    /* max possible length */
    oldpos = fc->ReadPtr;

    D(Debug0("Seek  : fc=%lx : mode=%08lx : pos=%08lx\n",fc,mode,pos));

    if (SourLock(fc->Lock))
        {
        D(Debug0("Sour lock %lx\n",PktRes2));
        return -1;
        }

    if (mode == 0) pos += fc->ReadPtr; /* OFFSET_CURRENT - Add the current position */
    else if (mode >  0) pos = size - pos;   /* OFFSET_END - Offset from end */
    /* else OFFSET_BEGINNING - Already a byte position */

    if (pos < 0 || pos > size)
    {
        PktRes2 = ERROR_SEEK_ERROR;
        return -1;
    }

    /* keep track of files with backwards seeks-don't read ahead wildly */
    if (pos < fc->ReadPtr)
        fc->Flags |= FC_SEEKBACK;

    DD(Debug0(" : Res1=%lx Res2=%lx Pos=%lx\n",PktRes1,PktRes2,pos));
    fc->ReadPtr = pos;

    return oldpos;
}


/***********************************************************************
***
***  QuickSeekFile.  There is more optimization to do here.
***
***********************************************************************/
QuickSeekFile(dp)
    REG struct DosPacket * dp;
{
    REG LONG size;
    REG LONG oldpos;
    REG FCNTRL *fc  =(FCNTRL *)dp->dp_Arg1;
    REG LONG pos    =dp->dp_Arg2;
    REG LONG mode   =dp->dp_Arg3;
        struct Message *msg;
        struct MsgPort *port;

    if (!QuickPackets)
    {
        D(Debug0("\nGACK! QS while in WaitIO!\n"));
        return 0;
    }

    size   = fc->Size;  /* max possible length */
    oldpos = fc->ReadPtr;

    DD(Debug0("QSeek    : fc=%lx : mode=%08lx : pos=%08lx\n",fc,mode,pos));

    if (SourLock(fc->Lock))
    {
        dp->dp_Res2 = PktRes2;
        dp->dp_Res1 = -1;
    }
    else
    {
        /* OFFSET_CURRENT  = 0 - Add the current position */
        /* OFFSET_END      = 1 - Offset from end */
        /* OFFSET_BEGINNING=-1 - Already a byte position */

        if (mode == 0) pos += fc->ReadPtr;
        else if (mode >  0) pos = size - pos;
        D(Debug0("QS:%lx ",pos - fc->ReadPtr));

        if (pos < 0 || pos > size)
        {
            dp->dp_Res2 = ERROR_SEEK_ERROR;
            dp->dp_Res1 = -1;
        }
        else
        {
            dp->dp_Res2 = NULL;
            dp->dp_Res1 = oldpos;
            fc->ReadPtr = pos;
        }
    }

    /* Copied from ReplyPacket() */
    msg                  = dp->dp_Link;
    Remove(msg);                /* wasn't removed before */
    port                 = dp->dp_Port;
    dp->dp_Port          = FSPort; 
/*  msg->mn_Node.ln_Name = (char *)dp;*/
    PutMsg(port,msg); 

    DD(Debug0(" : Res1=%lx Res2=%lx Port=%lx Msg=%lx Dp=%lx,pos=%lx\n",
	      dp->dp_Res1,dp->dp_Res2,port,msg,dp,pos));
    return TRUE;
}

/***********************************************************************
***
***  QuickReadFile
***
*** Careful, modifies LastBlockCached and PktRes2, plus sends new request!!
***********************************************************************/
LONG
QuickReadFile(dp)
    struct DosPacket *dp;
{
    REG FCNTRL *fc;
    REG char *buf,*b;
    REG LONG size,c;
    REG ULONG p,n,i,base;
    ULONG eof;

/*  Debug3("\tr%lx.%lx.%lx.%lx\n",buf,len,fc->ReadPtr,fc->Size); */
    DD(Debug0("QuickRead    : fc=%lx : buff=%08lx : len=%08lx\n",
         dp->dp_Arg1, dp->dp_Arg2, dp->dp_Arg3));

    fc = (FCNTRL *) dp->dp_Arg1;

    /* Calculate max allowed size for transfer: */
    p = fc->ReadPtr;

    /* handle EOF first (it does get hit often) */
    size = fc->Size - p;            /* max possible length */
    if (size <= 0) {
        D(Debug0("QR:EOF "));
        dp->dp_Res1 = 0;        /* must be at end of file */
        PktRes2 = 0;
        if (SourLock(fc->Lock)) dp->dp_Res1 = -1;
        dp->dp_Res2 = PktRes2;
        goto handled;
    }

    size = MIN(size,dp->dp_Arg3);       /* length of the read */

    /* if big, can't possibly be fast */
    if (size > (CacheSize << BlockShift))
    {
        D(Debug0("too big "));
        return 0;
    }

    base = fc->BaseLBN;
    n    = base + (p >> BlockShift);    /* starting block */

    /* check if beginning is in buffer next */
    if (!InCache(n,TRUE))
    {
        D(Debug0("not-in "));
        return 0;           /* can't handle it quick */
    }

    DD(Debug0("QR:First block (%ld) in cache!\n",n));

    /* check if all other blocks are in the cache */
    for (i = n+1; i <= base + ((p+size) >> BlockShift); i++)
    {
	/* this loop may never be entered if IO falls in a single block */
        DD(Debug0("%ld ",i));
        if (!InCache(i,TRUE))
        {
            D(Debug0("not-all "));
            return 0;
        }
    }

    /* Wow! all blocks are in the cache! Check SourLock and go! */
    if (SourLock(fc->Lock))
    {
        dp->dp_Res1 = -1;
        dp->dp_Res2 = PktRes2;
        goto handled;
    }

    DD(Debug0("QR success!\n"));
    D(Debug0("QR:%lx",dp->dp_Arg3));

    eof = base + (fc->Size >> BlockShift); /* last block of file */
    p   = p & BlockMask;            /* offset within the block */
    buf = (char *) dp->dp_Arg2;

    /* handles non-aligned reads, small reads, and final blocks of big */
    /* aligned reads.  Use remaining size here!            */
    c = MIN(BlockSize-p,size);

    /* it's ok to let it send new requests */
    /* this will NEVER wait!!! */
    /* if we're in a WaitIO, it's not good to send new packets! */
    DD(Debug0("QR %ld from block %ld\n",c,n));
    b = ReadBlock(n++,eof,QuickPackets ? TRUE : fc->Flags & FC_SEEKBACK,
              TRUE);
    CopyMem(b+=p,buf,c);
    buf += c;

    /* Now aligned on block, transfer rest: */
    for (c = size - c; c > 0; c -= BlockSize)
    {
        DD(Debug0("QR %ld from block %ld\n",MIN(c,BlockSize),n));
        /* if we're in a WaitIO, it's not good to send new packets! */
        b = ReadBlock(n++,eof,
                  QuickPackets ? TRUE : fc->Flags & FC_SEEKBACK,
                  TRUE);
        CopyMem(b,buf,MIN(c,BlockSize));
        buf += BlockSize;
    }

    fc->ReadPtr += size;    /* original read size */
    dp->dp_Res1 = size;
    dp->dp_Res2 = 0;
handled:
    {
        struct Message *msg;
        struct MsgPort *port;

        /* Copied from ReplyPacket() */
        msg                  = dp->dp_Link;
        Remove(msg);    /* wasn't removed before */
        port                 = dp->dp_Port;
        dp->dp_Port          = FSPort; 
/*      msg->mn_Node.ln_Name = (char *)dp;*/
        PutMsg(port,msg);
        DD(Debug0("Sent $%lx to $%lx\n",msg,port));
    }
    return 1;       /* we handled this packet */
}
