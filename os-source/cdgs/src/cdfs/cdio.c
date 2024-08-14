/***********************************************************************
****************                                        ****************
****************        -=   CD FILE SYSTEM   =-        ****************
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
#include <devices/trackdisk.h>

IMPORT  struct Task *FindTask();
IMPORT  APTR FetchA4();

/* we can't handle 64K dir block cache blocks! (not that we have that much
   mem anyways, that would be a truely impressive amount of ram). In reality
   this can't go over 256 in the forseeable future (that's 512K contiguous)
   but it's safer to handle it, since it's theoretically possible.  - REJ */

#define INVALID_DIR_INDEX       0xffff

#define D(x)    ;
#define DD(x)   ;

/***********************************************************************
***
***  DiskChanged
***
***     Low-level disk change interrupt server.
***     If there are no other disk change interrupt servers in the
***     list, then no one else is interested in disk inserts, and
***     we will reset the machine!
***
***     May want to delay if SCSI or TrackDisk is active, or if
***     we are not the Boot device.
***
***********************************************************************/
extern  VOID DiskChanged();
#asm
        XDEF    _DiskChanged
_DiskChanged:
                movem.l d2/a2/a4/a6,-(sp)
                move.l  10(a1),a4       ; data segment
                jsr     _ChangeFunc
                movem.l (sp)+,d2/a2/a4/a6
                rts

#endasm

/***********************************************************************
***
***  ChangeFunc
***
***     An interrupt level function that handles disk change events.
***
***********************************************************************/
void ChangeFunc()
{
        Debug1("\tC\n");
	D(Debug0("Disk Change\n"));

        InsertFlag = TRUE;
        Signal(FSProc, 1 << (FSPort->mp_SigBit));
}

/***********************************************************************
***
***  InitBootIO
***
***     Setup a temporary IO connection for checking out the boot disk.
***
***********************************************************************/
InitBootIO()
{
        BootPort.mp_Flags = 0;
        BootPort.mp_SigBit = AllocSignal(-1);
        BootPort.mp_SigTask = FindTask(NULL);
        NewList(&BootPort.mp_MsgList);

        DevIOReq.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
        DevIOReq.io_Message.mn_ReplyPort = &BootPort;   
        if (OpenDevice(&DeviceName[0],0,&DevIOReq,0)) return FALSE;

        return TRUE;
}

QuitBootIO()
{
        CloseDevice(&DevIOReq);
        FreeSignal(BootPort.mp_SigBit);
}

/***********************************************************************
***
***  InitCDIO
***
***********************************************************************/
InitCDIO() {

        D(Debug0("Welcome to the CD filesystem.\n"));

        if (OpenDevice("input.device",0,&InpReq,0)) return FALSE;

        /* Init and open our device: */
        DevIOReq.io_Message.mn_Node.ln_Type = NT_REPLYMSG;

        /* Share the process message port !?  Messy.
        DevIOReq.io_Message.mn_ReplyPort = FSPort; */

        DevMsgPort.mp_Node.ln_Type = NT_MSGPORT;
        DevMsgPort.mp_Flags        = PA_SIGNAL;
        DevMsgPort.mp_SigBit       = AllocSignal(-1);
        DevMsgPort.mp_SigTask      = FindTask(NULL);
        NewList(&DevMsgPort.mp_MsgList);
        DevIOReq.io_Message.mn_ReplyPort = &DevMsgPort;

        if (OpenDevice(DeviceName,UnitNumber,&DevIOReq,OpenFlags)) return FALSE;

#ifdef COMPLEX_CACHE
        DevMasterReq = DevIOReq;                /* for initing IOR's */
#endif
        DevChgReq    = DevIOReq;                /* copy struct */

        /* Setup disk change interrupt and its request: */
        DiskChgInt.is_Node.ln_Type = NT_INTERRUPT;
        DiskChgInt.is_Code = &DiskChanged;
        DiskChgInt.is_Data = (APTR)&DevChgReq;
        DevChgReq.io_Message.mn_Node.ln_Name = (char *)FetchA4();       
        DevChgReq.io_Command = CD_ADDCHANGEINT; 
        DevChgReq.io_Data = (APTR)&DiskChgInt;  
        SendIO(&DevChgReq);             

        return(TRUE);
        }

/***********************************************************************
***
***  MakeCache
***
***********************************************************************/
MakeCache()
{
#ifdef COMPLEX_CACHE
        if (CacheSize > 0x7fff)
                CacheSize = 0x7fff;

        /* guaranteed to be all 0's!!!! active = 0, block = 0, etc */
        MUST(Cache = MakeVector((WORD) CacheSize * (WORD) sizeof(struct Cache)));
        MUST(Cache[0].data = MakeVector(CacheSize << BlockShift));

        /* set up IOR's so we can use them */
        {
                REG UWORD i;
                for (i = 0; i < CacheSize; i++)
                {
                        /* structure copy */
                        Cache[i].IOR = DevMasterReq;
                        /* safe to add since data is a UBYTE * */
                        Cache[i].data = Cache[0].data + (i << BlockShift);
                }
        }
        LastCachedBlock = 0;
#else
        MUST(Cache = MakeVector(CacheSize << BlockShift));
#endif
        MUST(DirCache = MakeVector(DirCacheSize << BlockShift));
        MUST(DirCacheIndex = MakeVector(DirCacheSize * sizeof(ULONG)));
        MUST(DirCacheList = MakeVector(DirCacheSize * sizeof(UWORD)));
        DirCacheFirst = INVALID_DIR_INDEX;

        /* this must be of range -1 ... max_uword, i.e. LONG */
        DirCacheHighWater = -1;

        return TRUE;
}

/***********************************************************************
***
***  MoreCache
***
***     Add or subtract some number of blocks from the disk cache.
***     (Implicit flush in this code).
***
***********************************************************************/
MoreCache(new)
        REG LONG new;
{
        if (!new) return CacheSize;

        new += CacheSize;
        if (new <= 2) new = 2;

#ifdef COMPLEX_CACHE
        FlushDataCache();       /* make sure pending IO's are done */

        Forbid();       /* Don't want to risk losing my memory! */
        FreeVector(Cache[0].data);      /* one big allocation */
        FreeVector(Cache);

        if (new > 0x7fff)
                new = 0x7fff;

        /* guaranteed to be all 0's!!!! active = 0, block = 0, etc */
        Cache = MakeVector((WORD) new * (WORD) sizeof(struct Cache));
        if (Cache)
                Cache[0].data = MakeVector(new << BlockShift);
        while (!Cache || !Cache[0].data)
                                 /* Too big... come as close as we can. */
        {
                if (Cache) FreeVector(Cache);

                Cache = MakeVector((WORD) (--new) *
                                   (WORD) sizeof(struct Cache));
                if (Cache)
                        Cache[0].data = MakeVector(new << BlockShift);
        }
        Permit();

        CacheSize = new;

        /* set up IOR's so we can use them */
        {
                REG UWORD i;
                for (i = 0; i < CacheSize; i++)
                {
                        /* structure copy */
                        Cache[i].IOR = DevMasterReq;
                        Cache[i].data = Cache[0].data + (i << BlockShift);
                }
        }
#else
        Forbid();       /* Don't want to risk losing my memory! */
        FreeVector(Cache);
        Cache = MakeVector(new << BlockShift);
        while (!Cache) /* Too big... come as close as we can. */
        {
                Cache = MakeVector((--new) << BlockShift);
        }
        Permit();

        CacheBlock = 0;         /* Flush */
        CacheSize = new;

#endif
        return new;
}

#ifdef COMPLEX_CACHE
/***********************************************************************
***
***  StartCacheRead
***
*** Starts the IOR for a cache block
************************************************************************/
void StartCacheRead(index,block)
        UWORD index;
        ULONG block;
{
        struct Cache *cache = &Cache[index];

        if (cache->active)                      /* Aborted or not */
                WaitIO(&(cache->IOR));

        cache->block = block;
        cache->retry = Retry;   /* Max retries */
        cache->active = TRUE;

        StartDevIO(&(Cache[index].IOR),CD_READ,block << BlockShift,BlockSize,
                   cache->data);
        /* retries are handled above in ReadBlock */
}

/***********************************************************************
***
***  FlushDataCache
***
***********************************************************************/
FlushDataCache()
{
        REG UWORD i;

        D(Debug0("Flush data cache\n"));
        for (i = 0; i < CacheSize; i++)
        {
                if (Cache[i].active)
                {
                        if (!CheckIO(&(Cache[i].IOR)))
                                AbortIO(&(Cache[i].IOR));
                        WaitIO(&(Cache[i].IOR));
                        Cache[i].active = FALSE;
                }
                Cache[i].block = 0;
        }
        LastCachedBlock = 0;
}

/***********************************************************************
***
***  AbortCacheAllBut
***
*** Abort all requests but the one mentioned.
***
***********************************************************************/
AbortCacheAllBut(block)
        ULONG block;
{
        REG UWORD i;

        D(Debug0("Abort all but %ld\n",block));
        for (i = 0; i < CacheSize; i++)
        {
                if (Cache[i].active)
                {
                        if (!CheckIO(&(Cache[i].IOR)) &&
                            block != Cache[i].block)
                        {
                                /* MUST mark as invalid before waiting! */
                                /* this is to keep quickread safe.      */
                                Cache[i].block = 0;
                                AbortIO(&(Cache[i].IOR));

                        /* no need to wait, it'll be done when it's needed */
                                /*WaitIO(&(Cache[i].IOR));*/
                                /*Cache[i].active = FALSE;*/
                        }
                }
        }
        LastCachedBlock = block;
}

/***********************************************************************
***
***  AbortCacheLessThan
***
*** Abort all requests below the on mentioned
***
***********************************************************************/
AbortCacheLessThan(block)
        ULONG block;
{
        REG UWORD i;

        DD(Debug0("Abort less than %ld\n",block));
        for (i = 0; i < CacheSize; i++)
        {
                if (Cache[i].block < block &&
                    Cache[i].active &&
                    !CheckIO(&(Cache[i].IOR)))
                {
                        /* MUST mark as invalid before waiting! */
                        /* this is to keep quickread safe.      */
                        Cache[i].block = 0;
                        AbortIO(&(Cache[i].IOR));

                        /* no need to wait, it'll be done when it's needed */
                        /*WaitIO(&(Cache[i].IOR));*/
                        /*Cache[i].active = FALSE;*/
                }
        }
}
#endif

/***********************************************************************
***
***  FlushCache
***
***********************************************************************/
FlushCache()
{
        /* no need to clear index values, since LRU list is empty */
        DirCacheFirst = INVALID_DIR_INDEX;
        DirCacheHighWater = -1;
#ifdef COMPLEX_CACHE
        FlushDataCache();
#else
        CacheBlock = 0;
#endif
}

/***********************************************************************
***  InCache
***
***     Returns if a given block is in the cache.  (REJ)
***     done of TRUE means it only looks for completed blocks.
***     done of FALSE means look for uncompleted blocks as well.
***
***********************************************************************/
LONG InCache(lbn,done)
        ULONG lbn;
        int done;
{
#ifdef COMPLEX_CACHE
        REG UWORD i;
        for (i = 0; i < CacheSize; i++)
        {
                if (lbn == Cache[i].block)
                {
                        /* done means it must be complete */
                        /* if complete, check io_Error as well! */
                        return (!done || !Cache[i].active ||
                                (CheckIO(&(Cache[i].IOR)) &&
                                 !Cache[i].IOR.io_Error));
                }
        }
        return FALSE;
#else
        if (CacheBlock && lbn >= CacheBlock && lbn < CacheBlock+CacheSize)
                return TRUE;
        return FALSE;
#endif
}

/***********************************************************************
***  DirCacheFull
***
***     Returns whether the dir cache is full or not. (REJ)
***
***********************************************************************/
LONG DirCacheFull()
{
        return DirCacheHighWater >= DirCacheSize-1;
}

/***********************************************************************
***
***  ReadBlock
***
***     V34.2 - changes to stat gathering
***
***********************************************************************/
#ifdef COMPLEX_CACHE
        /* Alternate scheme to try to keep blocks streaming into memory */

        /* We have multiple buffers.  Each has it's own IOR.  As each   */
        /* block is read, IF the current file has not been Seek()ed     */
        /* backwards, the previous block (if cached) will be dumped, and*/
        /* the ior will be sent to prefetch the next block of data from */
        /* the drive.  However, we will NOT prefetch past the end of a  */
        /* file, nor will we do this on files with backward Seeks.      */
        /* Actually, we take all blocks that are lower and roll them    */
        /* over after a hit, to handle Seek forwards.                   */

        /* If we get a cache miss, we will AbortIO any IOR's, and then  */
        /* restart all for successive blocks starting at the new        */
        /* location.  This does NOT help switching between two files!   */
        /* Perhaps we should only reuse blocks as needed (plus any we   */
        /* AbortIO'd).  This _might_ help with the two-file problem, but*/
        /* I suspect it's not worth it. - REJ                           */

        /* Careful: with quick read stuff, we must NOT wait on a hit.   */
        /* Also, is it safe to have LastCachedBlock change during a     */
        /* QuickRead? Since it will never try a quickread if the FS is  */
        /* running, it can only be a problem during a Wait.             */

        /* parameters: back_seeked - boolean flag                       */
        /*             eof - end of file lba, or -1 for non-file        */
        /* global:     LastCachedBlock - the last block REQUESTED       */

UBYTE *ReadBlock(lbn,eof,back_seeked,quick)
        ULONG   lbn;
        ULONG   eof;
        int     back_seeked,quick;
{
        REG UWORD i,j;

        DD(Debug0("\tb %ld, eof %ld, %ld\n",lbn,eof,back_seeked));

        /* FIX!  why do we keep this? Should only do when debugging is on? */
        /* ditto for other stats variables.  Should be a define.  - REJ */
        BlockCount++;

        for (i = 0; i < CacheSize; i++)
        {
                if (lbn == Cache[i].block)
                {
                    /* We have a cache hit! */
                    DD(Debug0("Cache hit %ld %s\n",lbn,
                             Cache[i].active ?
                                (!CheckIO(&(Cache[i].IOR)) ? "(not done)" : "")
                                : ""));
                    D(Debug0(Cache[i].active ?
                                (!CheckIO(&(Cache[i].IOR)) ? "(" : ".")
                                : "."));
                    /* now if the previous block is in the cache, drop it */
                    /* and start an IOR for the next block.               */
                    /* Better yet, do all blocks older than the current.  */
                    /* MUST NEVER read ahead if back_seeked is true!      */
                    /* QuickRead WILL cause new IORs to be sent!!!!       */
                    if (eof != 0xffffffff /* not a data block */ &&
                        !back_seeked && eof > LastCachedBlock)
                    {
                        /* abort active requests earlier than hit */
                        /* since we've never seeked back, should be great */
//                      AbortCacheLessThan(lbn);

                        for (j = 0; j < CacheSize; j++)
                        {
                            if (Cache[j].block < lbn)
                            {
                            /* don't start if we're quick and it's not done */
                                if (!quick || 
                                    !Cache[j].active ||
                                    CheckIO(&(Cache[j].IOR)))
                                {
                                        DD(Debug0("Reuse %ld for b %ld\n",j,
                                                 LastCachedBlock+1));
                                        StartCacheRead(j,++LastCachedBlock);
                                }
                            }
                        }
                    }
                    /* QuickRead uses InCache, which checks io_Error, */
                    /* so retrys are safe.                            */
                    /* now we must wait if the block isn't ready yet  */
                    while (Cache[i].active)
                    {
                        WaitIO(&(Cache[i].IOR));
                        if (!Cache[i].IOR.io_Error ||
                             Cache[i].IOR.io_Error != CDERR_DMAFAILED ||
                             Cache[i].retry-- == 0)
                        {
                            /* Carl never checked for errors here! */
                            /* Fix!? should we do something about errs???? */
                            DD(Debug0("Got data\n"));
                            Cache[i].active = FALSE;
                        } else {
                            /* retry */
                            D(Debug0("Retry %ld lbn %ld\n",Cache[i].retry,
                                     Cache[i].block));
                            StartDevIO(&(Cache[i].IOR),CD_READ,
                                       Cache[i].block << BlockShift,
                                       BlockSize,Cache[i].data);
                        }
                        D(Debug0(")"));
                    }

                    /* fix? should we remove this stats stuff? */
                    HitCount++;
                    return Cache[i].data;
                }
        }

        /* we had no cache hit.  Grab some blocks and start stuff going */
        D(Debug0("Miss: %ld",lbn));

        /* This abort code is safe with QuickRead since it NEVER Waits! */

        /* first Abort any active IOR's */
        AbortCacheLessThan(0xffffffff);         /* MAX_ULONG */

        /* now find a block to satisfy this request */
        /* for now, we'll start IORs for all blocks */
        LastCachedBlock = lbn-1;
        for (i = 0; i < CacheSize; i++)
                StartCacheRead(i,++LastCachedBlock);

        /* FIX! remove this statistics stuff!!! */
        ReadCount += BlockSize;

        /* now return when the first block is ready */
        /* we KNOW cache[0].IOR is active! */
        /* QuickRead uses InCache, which checks io_Error, so retrys are safe */
        while (Cache[0].active)
        {
/* FIX!! should be a subroutine! */
                WaitIO(&(Cache[0].IOR));
                if (!Cache[0].IOR.io_Error ||
                     Cache[0].IOR.io_Error != CDERR_DMAFAILED ||
                     Cache[0].retry-- == 0)
                {
                    /* Carl never checked for errors here! */
                    /* Fix!? should we do something about errs???? */
                    if (Cache[0].IOR.io_Error)
                            Cache[0].block  = 0;
                    Cache[0].active = FALSE;
                } else {
                    /* retry */
                    D(Debug0("Retry %ld lbn %ld\n",Cache[0].retry,
                             Cache[0].block));
                    StartDevIO(&(Cache[0].IOR),CD_READ,
                               Cache[0].block << BlockShift,
                               BlockSize,Cache[0].data);
                }
        }
        DD(Debug0("Got first block...\n"));
        D(Debug0(".\n"));

        return Cache[0].data;
}
#else
UBYTE *ReadBlock(lbn)
        ULONG   lbn;
{
        Debug3("\tb%lx\n",lbn); 

        /* FIX!  why do we keep this? Should only do when debugging is on? */
        /* ditto for other stats variables.  Should be a define.  - REJ */
        BlockCount++;

        if (CacheBlock && lbn >= CacheBlock && lbn < CacheBlock+CacheSize)
        {
                HitCount++;
                D(Debug0("rcache-H: lbn=%ld\n",lbn));
                return &Cache[(lbn-CacheBlock) << BlockShift];
        }
        ReadCount += CacheSize;
        D(Debug0("rcache-M: lbn=%ld\n",lbn));

        ReadBytes(Cache,lbn<<BlockShift,CacheSize<<BlockShift);
        CacheBlock = lbn;
        return Cache;
}
#endif


/***********************************************************************
***
***  GetDirBlock
***
*** Modified to add simple LRU scheme.  Max dircachesize is 0xfffe
*** blocks (yeah, right).  The only reason I used a uword is that iso.c
*** sets it as a uword from some params it reads.
************************************************************************/
DIREC *GetDirBlock(lbn)
        ULONG lbn;
{
        REG DIREC *dr;
        REG UWORD i,prev;
	int err;

        /* first scan the list for a match.  Scan in LRU order */
        prev = INVALID_DIR_INDEX;
        i = DirCacheFirst;

        if (i != INVALID_DIR_INDEX)
        {
            while (1) {
                DD(Debug0("LRU index %ld, lbn=%ld\n",i,DirCacheIndex[i]));
                if (lbn == DirCacheIndex[i])
                {
                        DD(Debug0("LRU hit, prev = %ld!\n",prev));
                        /* move entry to front of list - don't move if first */
                        if (prev != INVALID_DIR_INDEX)
                        {
                                DirCacheList[prev] = DirCacheList[i];
                                DirCacheList[i] = DirCacheFirst;
                                DirCacheFirst = i;
                        }
                        return (DIREC *)(&DirCache[i<<BlockShift]);
                }
                /* we must keep prev and i valid if we didn't find it! */
                if (DirCacheList[i] == INVALID_DIR_INDEX)
                        break;

                /* get next entry in list */
                prev = i;
                i = DirCacheList[i];
            }
        }

        /* failure, not cached. i is either INVALID_DIR_INDEX (nothing
           cached), or it's the index of the least-recently-used block.  Only
           replace the LRU block if the cache is full.
        */

        /* this handles the i == INVALID_DIR_INDEX case! */
        if (DirCacheHighWater < DirCacheSize-1)
        {
                /* we have unused blocks to use */
                prev = i;       /* this will still be the end of the list */

                /* this is the block to load into */
                i = ++DirCacheHighWater;
        }

        /* i CANNOT be INVALID_DIR_INDEX here */
        DD(Debug0("LRU miss, prev = %ld, i = %ld, high = %ld\n",prev,i,
                 DirCacheHighWater));

        /* remove from end of list */
        if (prev != INVALID_DIR_INDEX)
                DirCacheList[prev] = INVALID_DIR_INDEX;

        /* add to head of list */
        DirCacheList[i] = DirCacheFirst;
        DirCacheFirst   = i;  /* this node will be at head of the list */

        /*
        ** Now that the device driver does prefetching, we
        ** read read directly to the directory cache, rather than
        ** flushing the file cache.
        **
        ** Previous code looped until a good block came in, but
        ** the test was buggy.  This code retries until read
        ** is ok.
        */
        dr = (DIREC *)&DirCache[i<<BlockShift];
        D(Debug0("DirBlk-M: lbn=%ld\n",lbn));

	/* continue trying until the disk is removed - probably not wise... */
	/* it should probably give up after a while... */
	do {
        	err = ReadBytes(dr,lbn<<BlockShift,BlockSize);
		if (err == TDERR_DiskChanged)
		{
			D(Debug0("Disk removed!!!\n"));
			/* we must make sure this doesn't look valid. */
			/* use an invalid block number, since the LRU */
			/* scheme doesn't make it easy to remove it.  */
			DirCacheIndex[i] = 0xffffffff;
			PktRes2 = ERROR_NO_DISK;
			return NULL;
		}
 	} while (err);       /* V24.4 - Must read it! */

        DirCacheIndex[i] = lbn;

        /*
         * if there are empty entries in the dir cache, and the next block
         * is also a directory cache block, it's nice to read them in,
         * since you tend to have N directory blocks, then the data for
         * those files.  This helps avoid a seek back to the directory after
         * reading each file or two (hopefully).  Mostly it helps at
         * startup.
         *
         * This is handled in dir.c, since it knows if the next block is
         * a directory block (using DirCacheFull to know if empty slots exist).
         */

        return dr;
}


/***********************************************************************
***
***  ReadBytes
***
***********************************************************************/
ReadBytes(buf,offset,size)      /* Bytes, bytes, bytes */
        char *buf;
        ULONG offset;
        ULONG size;
{
        REG int err;
        REG int i;

        DD(Debug0("cd_read      : buf=%lx,lbn=%ld,size=%lx\n",buf,offset>>BlockShift,size));

        for (i = 0; i < Retry; i++)
        {
                err = DoDevIO(CD_READ,offset,size,buf);
                if (err == 0) return 0;
                ReadErrs++;
                DD(Debug0("\tE%lx.%lx.%lx.%lx\n",err,offset,size,buf));
                if (err != CDERR_DMAFAILED) break;
        }
        return err;
}


/***********************************************************************
***
***  StartReadBytes
***
***********************************************************************/
StartReadBytes(buf,offset,size) /* Bytes, bytes, bytes */
        char *buf;
        ULONG offset;
        ULONG size;
{
        REG int err;
        REG int i;

        DD(Debug0("cd_startread : buf=%lx,lbn=%ld,size=%lx\n",buf,offset>>BlockShift,size));

        /* Must NOT be used again unitl WaitReadBytes is called!!!!! */
        StartDevIO(&DevIOReq,CD_READ,offset,size,buf);
}


/***********************************************************************
***
***  WaitReadBytes
***
***********************************************************************/
WaitReadBytes(buf,offset,size)  /* Bytes, bytes, bytes */
        char *buf;
        ULONG offset;
        ULONG size;
{
        REG int err;
        REG int i;

        for (i = 0; i < Retry; i++)
        {
                WaitIO(&DevIOReq);
                err = DevIOReq.io_Error;
                if (err == 0) return 0;
                ReadErrs++;
                DD(Debug0("\tE%lx.%lx.%lx.%lx\n",err,offset,size,buf));
                if (err != CDERR_DMAFAILED) break;
                StartDevIO(&DevIOReq,CD_READ,offset,size,buf);
        }
        return err;
}


/***********************************************************************
***
***  ReadCD
***
***********************************************************************/
ReadCD(buf,blk,len)
        char *buf;
        ULONG blk;
        ULONG len;
{
        return ReadBytes(buf, blk << BlockShift, len << BlockShift);
}


/***********************************************************************
***
***  ReMount
***
***     Called when a disk change interrupt has been detected in 
***     the main dispatch loop.
***
***     Re-initializes data structures each time, but this routine
***     happens so rarely that this is not a concern.
***
* FIX??? should we abort any active IOR's for complex cache???
***********************************************************************/
ReMount()
{
        /* V34.2 moved inhibit check to main.c */
        InsertFlag = FALSE;

        if (InputPends) return FALSE;   /* it's busy, ignore it */
        InputPends = TRUE;

        DoDevIO(CD_CHANGESTATE,0,0,0);
        if (DevIOReq.io_Actual)         /* true when no disk */
        {
                InpEvent.ie_Class = IECLASS_DISKREMOVED;
                UnMount();
        }
        else
        {
                InpEvent.ie_Class = IECLASS_DISKINSERTED;
                Mount(FALSE);
        }
        InpEvent.ie_NextEvent = NULL;
        /* No time stamp. */

        InpPacket.dp_Link = (struct Message *) &InpReq;
        InpPacket.dp_Port = FSPort; 
        InpPacket.dp_Type = ACTION_DISK_CHANGE;

        InpReq.io_Message.mn_Node.ln_Name = (char *) &InpPacket;
        InpReq.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
        InpReq.io_Message.mn_ReplyPort = FSPort;
        InpReq.io_Command = IND_WRITEEVENT;     
        InpReq.io_Data = (APTR) &InpEvent;
        InpReq.io_Length = sizeof(InpEvent);

        SendIO(&InpReq);

        return TRUE;
}

