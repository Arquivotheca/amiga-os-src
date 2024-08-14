#include "prepcard.h"
#include "tuples.h"

#define DTYPE_NOCARD	0xfe
#define DTYPE_UNKNOWN	0xff

/**
 ** 12/4/91 - darren - No public includes for cardmark yet!!
 ** No time to wait - include in check.c file for now
 **/

/****** Cardmark header ******
*
*       The Cardmark header defines a particular card as a
*       Cardmark card, as opposed to other card types.
*       It is identical with the bookmark header.
*/

struct cmk_Header {
        UWORD mb_MemoryId;
        UBYTE mb_HeaderLen;
        UBYTE mb_HeaderSum;
        ULONG mb_MemSize;
        UWORD mb_MaxSize;
        UWORD mb_CheckSum;
        APTR  mb_FirstMark;
        APTR  mb_FirstFree;
        APTR  mb_FreeSpace;
};


/****** ID words for mb_MemoryId field */


#define MT_DIAG         (0x1111)
#define MT_BOOKMARK     (('B'<<8)+'K')  /* bookmark memory */
#define MT_READWRITE    (('R'<<8)+'W')  /* read/write ram card */
#define MT_READONLY     (('R'<<8)+'O')  /* read only rom card */
#define MT_RAMDISK      (('R'<<8)+'D')  /* a ram disk */
#define MT_OTHER        (('O'<<8)+'K')  /* used as something else */

#define TPLFMTTYPE_CARDMARK 0x91	/* private format type */

/**
 ** Defines for MS-DOS pseudo-floppy disk
 **/

#define MSDOS1	0xe9
#define MSDOS2	0xeb
#define MSDOS3  0x90

VOID CardCheck( struct cmdVars *cmv )
{
register struct cmdVars *cv;
register struct CardHandle *ch;

/* what did I find? */

BOOL isDEVICE;
BOOL isFORMAT;
BOOL isDISK;
BOOL isGEOMETRY;
BOOL isVERS1;
BOOL isVERS2;
BOOL isAMIGAXIP;
BOOL isRETRY;
BOOL isCARDMARK;
BOOL isPSEUDO;

/* CISTPL_DEVICE info */

ULONG devtype, devspeed, devsize;

/* CISTPL_FORMAT info */

struct TP_Format *tpf;
UWORD BlockSize;
ULONG Offset, NBytes, NBlock, EDCLOC;
UBYTE edctype,edclen;

/* CISTPL_GEOMETRY */

struct TP_Geometry *tpg;
UWORD SecTrk,TrkCyl,TotCyl;

/* CISTPL_AMIGAXIP */

struct TP_AmigaXIP *tpa;
struct Resident *res;
UWORD *match;
ULONG *backptr;

/* storage */

UBYTE tbuf[257+8];

UBYTE v1buf[258];
UBYTE v2buf[258];

struct DeviceTData dt;

UBYTE cardstatus;
struct cmk_Header *ckh;
UBYTE *common;

	cv = cmv;
	ch = &cv->cv_CardHandle;

	cv->cv_IsCORRUPT = FALSE;

	cv->cv_FirstInfoDraw = TRUE;

	common = cv->cv_CardMemMap->cmm_CommonMemory;

/* Clear window */

	SetAPen(cv->cv_win->RPort,0L);
	RectFill(cv->cv_win->RPort,
		(ULONG)cv->cv_win->BorderLeft,
		(ULONG)cv->cv_win->BorderTop,
		(ULONG)(cv->cv_win->Width) - (cv->cv_win->BorderRight + 1),
		(ULONG)(cv->cv_win->Height) - (cv->cv_win->BorderBottom + 46)
		);


/* special case if busy preping */

	if(cv->cv_PrepBusy)
	{
		DrawDeviceInfo(cv, DTYPE_NOCARD, 0L, 0L );
		return;

	}

/* do not allow release while examining */
	
	ObtainSemaphore(&cv->cv_CardSemaphore);

	cv->cv_NoCard = TRUE;

	if(cv->cv_IsInserted && !cv->cv_IsRemoved)
	{

/* obtain status of battery (if applicable), and WP */

		cardstatus = ReadCardStatus();

/* obtain CISTPL_DEVICE tuple */

		cv->cv_NoCard = FALSE;

		ReleaseSemaphore(&cv->cv_CardSemaphore);

		isDEVICE = FALSE;
		devtype = DTYPE_UNKNOWN;

		if(ChildCopyTuple(cv,ch,tbuf,(ULONG)CISTPL_DEVICE,4L))
		{

			if(DeviceTuple(tbuf,&dt))
			{
				isDEVICE = TRUE;

				devtype = (ULONG)dt.dtd_DTtype;
				devsize = dt.dtd_DTsize;
				devspeed = dt.dtd_DTspeed;

			}
			else
			{
				cv->cv_IsCORRUPT = TRUE;
			}
		}
/* obtain CISTPL_FORMAT tuple */


		isFORMAT = FALSE;
		isDISK = FALSE;
		isGEOMETRY = FALSE;
		isCARDMARK = FALSE;

		if(ChildCopyTuple(cv,ch,tbuf,(ULONG)CISTPL_FORMAT,20L))
		{
			isFORMAT = TRUE;

			tpf = (struct TP_Format *)&tbuf;


/* check if CD-TV cardmark card */

			if(tpf->TPL_LINK >= 12)
			{
				if(tpf->TPLFMT_TYPE[0] == TPLFMTTYPE_CARDMARK)
				{
					Offset = (tpf->TPLFMT_OFFSET[0]     ) +
					(tpf->TPLFMT_OFFSET[1] << 8) +
					(tpf->TPLFMT_OFFSET[2] << 16)+
					(tpf->TPLFMT_OFFSET[3] << 24);

/* CD-TV requires hard-coded offset of 512! */

					if(Offset == 512)
					{
						ckh = (struct cmk_Header *)(common + Offset);


						ObtainSemaphore(&cv->cv_CardSemaphore);

						if(!cv->cv_NoCard)
						{
							if(ckh->mb_MemoryId == MT_BOOKMARK)
							{

								if(ckh->mb_HeaderLen ==
									(UBYTE)sizeof(struct cmk_Header))
								{

									isCARDMARK = TRUE;
								}
							}
						}

						ReleaseSemaphore(&cv->cv_CardSemaphore);
					}

				}
			}

/* check if long enough for DISK like (but do not require EDCLOC since
 * the HP PalmTop doesn't write it though it should!)
 */

			if(tpf->TPL_LINK >= 16)
			{
				if(tpf->TPLFMT_TYPE[0] == TPLFMTTYPE_DISK)
				{
					edctype = (((tpf->TPLFMT_EDC[0]) & TPLFMT_EDC_MASK)>>TPLFMT_EDC_SHIFT) ;
					edclen = tpf->TPLFMT_EDC[0] & TPLFMT_LENGTH_MASK;

					if(edctype <= TPLFMTEDC_PCC)
					{
	
						isDISK = TRUE;

						Offset = (tpf->TPLFMT_OFFSET[0]     ) +
						(tpf->TPLFMT_OFFSET[1] << 8) +
						(tpf->TPLFMT_OFFSET[2] << 16)+
						(tpf->TPLFMT_OFFSET[3] << 24);

						NBytes = (tpf->TPLFMT_NBYTE[0]     ) +
						(tpf->TPLFMT_NBYTE[1] << 8) +
						(tpf->TPLFMT_NBYTE[2] << 16)+
						(tpf->TPLFMT_NBYTE[3] << 24);

						BlockSize = (tpf->TPLFMT_BKSZ[0]  ) +
						(tpf->TPLFMT_BKSZ[1] << 8);

						NBlock = (tpf->TPLFMT_NBLOCKS[0]     ) +
						(tpf->TPLFMT_NBLOCKS[1] << 8) +
						(tpf->TPLFMT_NBLOCKS[2] << 16)+
						(tpf->TPLFMT_NBLOCKS[3] << 24);

						EDCLOC = 0L;

						if(tpf->TPL_LINK >= 20)
						{
							EDCLOC =
							(tpf->TPLFMT_EDCLOC[0]     ) +
							(tpf->TPLFMT_EDCLOC[1] << 8) +
							(tpf->TPLFMT_EDCLOC[2] << 16)+
							(tpf->TPLFMT_EDCLOC[3] << 24);
						}

/* checks for corruption */

						if(isDEVICE)
						{
							if((NBytes + Offset) > devsize)
							{
								cv->cv_IsCORRUPT = TRUE;
							}

						}

/* given error detection, and maybe EDCLOC, make sure everything fits within NBytes */

						if(edctype <= TPLFMTEDC_CRC)
						{
							if(edctype == TPLFMTEDC_CHSUM)
							{
								if(edclen != 1)
								{
									cv->cv_IsCORRUPT = TRUE;
								}
							}

							if(edctype == TPLFMTEDC_CRC)
							{
								if(edclen != 2)
								{
									cv->cv_IsCORRUPT = TRUE;
								}
							}

							if(NBytes < (NBlock * (BlockSize + edclen)))
							{

								cv->cv_IsCORRUPT = TRUE;
							}

							if(edctype)
							{
								if(EDCLOC)
								{
									if(EDCLOC < Offset)
									{
										cv->cv_IsCORRUPT = TRUE;
									}

									if(EDCLOC > (Offset + NBytes)-(NBlock * edclen))
									{
										cv->cv_IsCORRUPT = TRUE;
									}
								}
							}
						}

/* make sure block size is 128, 256, 512, 1024, or 2048 */


						if(BlockSize < 128 || BlockSize > 2048)
						{
							cv->cv_IsCORRUPT = TRUE;
						}

						if(BlockSize & 127)
						{
							cv->cv_IsCORRUPT = TRUE;
						}


						if(ChildCopyTuple(cv,ch,tbuf,(ULONG)CISTPL_GEOMETRY,4L))
						{
							tpg = (struct TP_Geometry *)&tbuf;

							if(tpg->TPL_LINK >= 4)
							{
								isGEOMETRY = TRUE;
								SecTrk = (UWORD)tpg->TPLGEO_SPT[0];
								TrkCyl = (UWORD)tpg->TPLGEO_TPC[0];
								TotCyl = (UWORD)(tpg->TPLGEO_NCLI[0]) + 
								(tpg->TPLGEO_NCLI[1]<<8);
							}
							else
							{
								cv->cv_IsCORRUPT = TRUE;
							}

/* check product of sec/trk * trk/cyl * cyl must be <= # of blocks */

							if(NBlock < (ULONG)(SecTrk * TrkCyl * TotCyl))
							{
								cv->cv_IsCORRUPT = TRUE;
							}

/* check for illegal geometry */
							if(SecTrk == 0 || TrkCyl == 0 || TotCyl == 0)
							{
								cv->cv_IsCORRUPT = TRUE;
							}

						}
					}
				}
			}
		}

/* check for CISTPL_AMIGAXIP tuple */


		isAMIGAXIP = FALSE;

		if(ChildCopyTuple(cv,ch,tbuf,(ULONG)CISTPL_AMIGAXIP,6L))
		{
			ObtainSemaphore(&cv->cv_CardSemaphore);

			if(!cv->cv_NoCard)
			{
				tpa = (struct TP_AmigaXIP *)&tbuf[0];

				if(tpa->TPL_LINK == 6)
				{
					Offset = ((tpa->TP_XIPLOC[0] ) +
						 (tpa->TP_XIPLOC[1] << 8 ) +
						 (tpa->TP_XIPLOC[2] << 16) +
						 (tpa->TP_XIPLOC[3] << 24));


					res = (struct Resident *) (Offset + (ULONG)common);

					if(res > (struct Resident *)
						((UBYTE *)0xA00000 - sizeof(struct Resident)))
					{
						cv->cv_IsCORRUPT = TRUE;
					}
					if((ULONG)res & 0x01)
					{
						cv->cv_IsCORRUPT = TRUE;
					}
					if(!cv->cv_IsCORRUPT)
					{
						match = (UWORD *)res;
						if(*match == RTC_MATCHWORD)
						{
							match++;

							backptr = (ULONG *)match;

							if( (ULONG)res == *backptr )
							{
								isAMIGAXIP = TRUE;
							}
						}
					}
				}
			}
			ReleaseSemaphore(&cv->cv_CardSemaphore);
		}

/* check for CISTPL_VERS_1 tuple */

		isVERS1 = FALSE;
		if(ChildCopyTuple(cv,ch,v1buf,(ULONG)CISTPL_VERS_1,-1L))
		{
			isVERS1 = TRUE;
		}

/* check for CISTPL_VERS_2 tuple */

		isVERS2 = FALSE;
		if(ChildCopyTuple(cv,ch,v2buf,(ULONG)CISTPL_VERS_2,-1L))
		{
			isVERS2 = TRUE;
		}


/* check for pseudo floppy - no Layer-2 information is present */

		isPSEUDO = FALSE;

		if(!isVERS2 && !isFORMAT && !isAMIGAXIP)
		{
			ObtainSemaphore(&cv->cv_CardSemaphore);

			if(!cv->cv_NoCard)
			{
				if(common[0] == MSDOS1 ||
				  (common[0] == MSDOS2 && common[2] == MSDOS3))
				{
					isPSEUDO = TRUE;
				}
			}
			ReleaseSemaphore(&cv->cv_CardSemaphore);

		}

/* release semaphore - card can be released now */

	}
        else
	{
		ReleaseSemaphore(&cv->cv_CardSemaphore);
	}

/*
 * Horrible kludge for Intel's Modem 2400+!!  Since this card does not
 * properly initialize for some 1/2 second plus after reset, this kludge
 * retries cards which have no CISTPL_DEVICE, CISTPL_FORMAT, or CISTPL_AMIGAXIP
 * tuple.  The Modem card won't have a readable CISTPL_DEVICE tuple until its
 * finished initializiing.
 */
	isRETRY = FALSE;

	if(!isDEVICE && !isFORMAT && !isAMIGAXIP)
	{
		Disable();

		if(cv->cv_RetryCard)
		{
			cv->cv_RetryCard--;
			Enable();

			isRETRY = TRUE;

			PutChildMsg(cv, CHILD_WAKEME, NULL);
		}
		else
		{
			Enable();
		}
	}
		
/**
 ** Now print info in some kind of readable order
 **
 **/

	if(!isRETRY)
	{
		
		if(cv->cv_NoCard)
		{
			devtype = DTYPE_NOCARD;

			isFORMAT = FALSE;
			isAMIGAXIP = FALSE;
			isDISK = FALSE;
			isGEOMETRY = FALSE;
			isCARDMARK = FALSE;
			isVERS1 = FALSE;
			isVERS2 = FALSE;
			isPSEUDO = FALSE;
		}

		DrawDeviceInfo(cv, devtype, devspeed, devsize );

		DrawFormatInfo(cv, devtype, isFORMAT, isAMIGAXIP,isDISK, isPSEUDO, isCARDMARK);

		if(!isDISK)
		{
			isGEOMETRY = -1;
		}

		DrawGeoInfo(cv,(ULONG)NBlock,(UWORD)BlockSize,isGEOMETRY,(UWORD)SecTrk,(UWORD)TrkCyl,(UWORD)TotCyl);

		DrawBatteryInfo(cv, devtype, cardstatus);

		if(isVERS1)
		{
			DrawVers1Info( cv, (struct TP_Vers_1 *)v1buf );
		}
		else
		{
			DrawVers2Info( cv, (struct TP_Vers_2 *)v2buf, isVERS2 );
		}

		if(cv->cv_IsCORRUPT)
		{
			DisplayError(cv,MSG_PREP_CORRUPT_ERROR);

		}
		if(isPSEUDO)
		{
			DisplayError(cv,MSG_PREP_PSEUDO_WARN);
		}
	}	
	
}

/* 
 * Copy tuple on context of child task.  This makes it possible to break
 * a circular tuple chain by forcing a card change after N amount of time.
 * It is also reasonable to assume that a tuple will be found in a reasonable
 * amount of time.  The only reason it might not is because of extremely
 * heavy multi-tasking, or interrupts (arg), but more likely because the
 * tuple chain is corrupt such that CopyTuple() is walking through a
 * random chain of memory -- eventually CopyTuple() will stop because there
 * are checks to make sure it doesn't look past the end of common/attr
 * memory.  Or someone corrupts the chain such that there is a circular link!
 * This is infact possible, but almost requires an intentional effort.  No
 * harm would come of it, except for an endless walking of the chain which
 * will stop when the card is removed.
 */

BOOL ChildCopyTuple( struct cmdVars *cmv, struct CardHandle *ch, UBYTE *tuplebuf, ULONG tuplecode, ULONG tuplesize )
{
register struct cmdVars *cv;
struct PrepMsg pm;
struct TupleMsg tm;
struct MsgPort *tport;
struct timerequest *tr;
BOOL sendtime, sendchild;
ULONG mask;

	cv = cmv;

	pm.pm_msg.mn_Node.ln_Type = 0;
	pm.pm_msg.mn_Length = sizeof(struct PrepMsg);
	pm.pm_msg.mn_ReplyPort = cv->cv_ReplyPort;
	pm.pm_Command = CHILD_COPYTUPLE;

	tm.tm_ch = ch;
	tm.tm_tuplebuf = tuplebuf;
	tm.tm_tuplecode = tuplecode;
	tm.tm_tuplesize = tuplesize;

	tm.tm_result = FALSE;

	pm.pm_Data = &tm;

	if(tport = CreateMsgPort())
	{
		if(tr = CreateIORequest( tport, sizeof(struct timerequest)))
		{
			if(!(OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)tr,0L)))
			{

/* send timer request */
				tr->tr_node.io_Command = TR_ADDREQUEST;
				tr->tr_time.tv_secs = 5L;
				tr->tr_time.tv_micro = 0L;

				sendtime = TRUE;
				sendchild = TRUE;
				mask = (1L<<cv->cv_ReplyPort->mp_SigBit)|
					(1L<<tport->mp_SigBit);

				if(!cv->cv_NoCard)
				{
					SendIO( (struct IORequest *) tr );
/* and send request to child */

					PutMsg( cv->cv_ChildPort, (struct Message *)&pm );

/* wait for replies */

					while(sendtime || sendchild)
					{
						Wait( mask );

/* check if child has finished copying */

						if(GetMsg(cv->cv_ReplyPort))
						{

							sendchild = FALSE;

							if(sendtime)
							{
								AbortIO( tr );
							}
						}

/* if timer request has expired, force a card change */

						if(GetMsg(tport))
						{
							sendtime = FALSE;

							if(sendchild)
							{
								cv->cv_IsCORRUPT = TRUE;
/* force a break by forcing a card change */
								CardForceChange();
							}
						}
					}
				}

				CloseDevice( tr );
			}	
			DeleteIORequest( tr );
		}
		DeleteMsgPort( tport );
	}


	return(tm.tm_result);
}


/**
 ** Draw CISTPL_DEVICE information
 **/

VOID DrawDeviceInfo( struct cmdVars *cmv, ULONG type, ULONG speed, ULONG size )
{
register struct cmdVars *cv;

STRPTR info[4];

UBYTE typebuf[24];
UBYTE speedbuf[24];
UBYTE sizebuf[24];

UBYTE *temp;

BOOL isSPEED;
BOOL isSIZE;
int x;

	cv = cmv;

	isSPEED = FALSE;
	isSIZE = FALSE;

	for(x = 0; x < 4; x++) info[x] = NULL;

	info[0] = typebuf;

	if(type == DTYPE_NOCARD)
	{
/* is no card in slot? */

		if(cv->cv_CardInUse)
		{
			temp = GetString(cv, MSG_PREP_ERROR_INUSE);
		}
		else
		{
			temp = GetString(cv, MSG_PREP_CARD_NOCARD);
		}

		if(cv->cv_PrepBusy)
		{
			temp = GetString(cv, MSG_PREP_PREP_BUSY);
			ChangeBegAttrs(cv,cv->cv_QuitGad,GA_Disabled, (BOOL)TRUE,TAG_DONE);
		}
		DisabledPrep( cv, TRUE );

	}
	else
	{

/* some kind of storage device? */

		ChangeBegAttrs(cv,cv->cv_QuitGad,GA_Disabled,(BOOL)FALSE,TAG_DONE);
		DisabledPrep( cv, FALSE );

		if(type <= DTYPE_DRAM)
		{
			temp = GetString(cv,type + MSG_PREP_CARD_NULL);
			isSPEED = TRUE;
			isSIZE = TRUE;
		}
		else
		{

/* an I/O card? */
			if(type == DTYPE_IO)
			{
				temp = GetString(cv,MSG_PREP_CARD_IO);
				isSPEED = TRUE;
			}
/* Some reserved type? */

			else
			{
				temp = GetString(cv,MSG_PREP_CARD_UNKNOWN);
			}
		}
	}

	PrintLine(typebuf,20,TRUE,temp,NULL);

	if(isSPEED)
	{
		info[1] = speedbuf;
		PrintLine(speedbuf,20,TRUE,GetString(cv,MSG_PREP_SPEED_FMT),speed);
	}

	if(isSIZE)
	{
		info[2] = sizebuf;
		PrintLine(sizebuf,20,TRUE,GetString(cv,MSG_PREP_SIZE_FMT),size);
	}

	InfoBox( cv, GetString(cv,MSG_PREP_DEVICE_LABEL), 6, 15, 20, &info[0] );

}


/**
 ** Draw CISTPL_FORMAT tuple information
 **/

VOID DrawFormatInfo( struct cmdVars *cmv, ULONG devtype, BOOL isFORMAT, BOOL isXIP, BOOL isDISK, BOOL isPSEUDO, BOOL isCARDMARK )
{
register struct cmdVars *cv;

STRPTR info[2];

UBYTE typebuf[54];
ULONG message;

	cv = cmv;

	info[1] = NULL;

	message = -1L;

	if(isFORMAT)
	{
/* Format tuple indicates the card is being used for data storage */

		if(isDISK)
		{
/* Type checked out as disk-like */

			message = MSG_PREP_FORMAT_DISK;
			
		}
		else
		{
/* hmm, not disk-like, but indicate there is data on the card */

			message = MSG_PREP_FORMAT_UNKNOWN;

			if(isCARDMARK)
			{
				message = MSG_PREP_FORMAT_CRDMARK;
			}
		}
	}
	else
	{
		if(devtype == DTYPE_SRAM || devtype == DTYPE_DRAM)
		{

/* no data storage, but is a RAM card - is System RAM usable */

			message = MSG_PREP_FORMAT_RAM;
		}
		else
		{
/* might be an I/O card eh? */

			if(devtype == DTYPE_UNKNOWN)
			{
/* no idea whats on it */
				message = MSG_PREP_FORMAT_NOPREP;
			}
		}
	}

	if(isXIP)
	{
/* checked out as an Amiga XIP card */

		message = MSG_PREP_FORMAT_XIP;
	}

/* check if pseudo disk format */

	if(isPSEUDO)
	{
		message = MSG_PREP_FORMAT_PSEUDO;
	}

	if(message == -1L)
	{
		PrintLine(typebuf,51,TRUE,GetString(cv,MSG_PREP_NA),NULL);
	}
	else
	{
		PrintLine(typebuf,51,FALSE,GetString(cv,message),NULL);
	}

	info[0] = typebuf;

	InfoBox(cv, GetString(cv,MSG_PREP_FORMAT_LABEL), 188, 15, 52, &info[0] );

}

/**
 ** Display CISTPL_GEOMETRY info if disk-like
 **/

VOID DrawGeoInfo( struct cmdVars *cmv, ULONG nblocks, UWORD blocksize, BOOL isGEO, UWORD SecTrk, UWORD TrkCyl, UWORD Cyls )
{
register struct cmdVars *cv;
STRPTR info[6];
UBYTE bytesperblock[54];
UBYTE blocksperdisk[54];
UBYTE sectorspertrk[54];
UBYTE trackspercyls[54];
UBYTE cylsperdisk[54];

	cv = cmv;


/* disk-like, so we at least know block size, and total blocks */

	PrintLine(bytesperblock,51,FALSE,GetString(cv,MSG_PREP_GEO_BYTES),(ULONG)blocksize);
	info[0] = bytesperblock;
	PrintLine(blocksperdisk,51,FALSE,GetString(cv,MSG_PREP_GEO_BLKS),nblocks);
	info[1] = blocksperdisk;
	info[2] = NULL;

	if(isGEO)
	{
		if((UWORD)isGEO != (UWORD)-1)
		{
			PrintLine(sectorspertrk,51,FALSE,GetString(cv,MSG_PREP_GEO_SECT),(ULONG)SecTrk);
			PrintLine(trackspercyls,51,FALSE,GetString(cv,MSG_PREP_GEO_TCYL),(ULONG)TrkCyl);
			PrintLine(cylsperdisk,51,FALSE,GetString(cv,MSG_PREP_GEO_CYLS),(ULONG)Cyls);

			info[2] = sectorspertrk;
			info[3] = trackspercyls;
			info[4] = cylsperdisk;
			info[5] = NULL;
		}
		else
		{
			PrintLine(bytesperblock,51,TRUE,GetString(cv,MSG_PREP_NA),NULL);
			info[0] = bytesperblock;
			info[1] = NULL;
		}		
	}

	InfoBox(cv,GetString(cv,MSG_PREP_GEO_LABEL),188,56,52,&info[0]);

}

/**
 ** Draw Battery level
 **/

VOID DrawBatteryInfo( struct cmdVars *cmv, ULONG devtype, UBYTE status )
{
register struct cmdVars *cv;
STRPTR info[2];
UBYTE batlevel[24];
ULONG message;
BOOL report;

	cv = cmv;

	info[1] = NULL;
	info[0] = batlevel;

	report = FALSE;

	if(devtype == DTYPE_SRAM || devtype == DTYPE_UNKNOWN)
	{
		message = MSG_PREP_BATTERY_OK;
		if(status &= (CARD_STATUSF_BVD1|CARD_STATUSF_BVD2))
		{
			if(status == CARD_STATUSF_BVD2)
			{
				message = MSG_PREP_BATTERY_LOW;
			}
			else
			{
				message = MSG_PREP_BATTERY_FAIL;
			}
		}

/* print battery condition for SRAM, or unknown IF its not OK such as
 * an SRAM card whose type is unknown because the battery failed
 */

		if(devtype == DTYPE_SRAM || message != MSG_PREP_BATTERY_OK)
		{
			PrintLine(batlevel,20,TRUE,GetString(cv,message),NULL);
			report = TRUE;
		}

	}

	if(!report)
	{
		PrintLine(batlevel,20,TRUE,GetString(cv,MSG_PREP_NA),NULL);
	}

	InfoBox(cv,GetString(cv,MSG_PREP_BATTERY_STATUS),6,92,20,&info[0]);
}

/**
 ** Draw CISTPL_VERS1 product info
 **/
VOID DrawVers1Info( struct cmdVars *cmv, struct TP_Vers_1 *tpv )
{
register struct cmdVars *cv;
STRPTR info[4];
UBYTE manufact[76];
UBYTE product[76];
UBYTE prodinfo[76];
UBYTE link;
UBYTE major,minor;
UBYTE *manu, *name, *pinfo;
UBYTE *max;

	cv = cmv;

	info[3] = NULL;

	link = tpv->TPL_LINK;
	max = (UBYTE *)tpv + (link + 2);

/* make sure tuple contains at least maj.min rev info, and 1 byte of info */

	if(link >= 3)
	{
		major = tpv->TPLLV1_MAJOR[0];
		minor = tpv->TPLLV1_MINOR[0];

		if(manu = IsString((UBYTE *)&tpv->TPLLV1_INFO[0], max))
		{
			if(name = IsString(manu + strlen(manu) + 1, max))
			{
				PrintLine(manufact,72,FALSE,manu,NULL);

                                PrintLine(product,72,FALSE,name,NULL);
				info[0] = manufact;
				info[1] = product;

				pinfo = IsString(name + strlen(name) + 1, max);

				if(pinfo == NULL) pinfo = "";

				PrintLine(prodinfo,72,FALSE,
					GetString(cv,MSG_PREP_PINFO_STATUS),
					(ULONG)major,
					(ULONG)minor,
					pinfo);

				info[2] = prodinfo;

			
InfoBox(cv,GetString(cv,MSG_PREP_PROD_LABEL),6,128,75,&info[0]);
			}

		}

		
	}


}

/**
 ** Draw CISTPL_VERS2 product info
 **/
VOID DrawVers2Info( struct cmdVars *cmv, struct TP_Vers_2 *tpv, BOOL isVERS2 )
{
register struct cmdVars *cv;
STRPTR info[4];
UBYTE standard[76];
UBYTE oemmsg[76];
UBYTE infomsg[76];

UBYTE link;
UBYTE *max;
UBYTE *oem,*xinfo;
ULONG message;
BOOL report;

	cv = cmv;

	info[0] = standard;
	info[1] = NULL;
	report = FALSE;

	if(isVERS2)
	{
		link = tpv->TPL_LINK;
		max = (UBYTE *)tpv + (link + 2);

		if(link >= 10)
		{
			message = MSG_PREP_NONSTANDARD;
 
			if(tpv->TPLLV2_VERS[0] == 0)
			{
				if(tpv->TPLLV2_COMPLY[0] == 0)
				{
					if(tpv->TPLLV2_NHDR[0] == 1)
					{

						message = MSG_PREP_STANDARD;
					}
				}
			}
			
			PrintLine(standard,72,FALSE,GetString(cv,message),NULL);
 
			report = TRUE;

			if(oem = IsString((UBYTE *)&tpv->TPLLV2_OEM[0], max))
			{
				xinfo = IsString( oem + strlen(oem) + 1, max);
 
				if(xinfo == NULL) xinfo = "";
 
				PrintLine(oemmsg,72,FALSE,oem,NULL);
				PrintLine(infomsg,72,FALSE,xinfo,NULL);
 
				info[1] = oemmsg;
				info[2] = infomsg;
		 		info[3] = NULL;

			}
		}
	}

	if(!report)
	{
		PrintLine(standard,72,TRUE,GetString(cv,MSG_PREP_NA),NULL);
	}

	InfoBox(cv,GetString(cv,MSG_PREP_INFO_LABEL),6,160,75,&info[0]);
}

/**
 ** Make sure we point to a NULL terminated string.
 **/

UBYTE *IsString( UBYTE *start, UBYTE *max )
{
register UBYTE *scan;
register BOOL found;

	scan = start;
	found = FALSE;

	while(scan <= max )
	{
		if(*scan == 0x0)
		{
			found = TRUE;
			break;
		}
		scan++;
	}

	if(found)
	{
		return(start);		
	}
	else
	{
		return(NULL);
	}
}
 
/**
 ** Print line into line buffer/truncate/center
 **/

#define MAXLEN 80

VOID PrintLine(UBYTE *buf, UWORD maxlen, BOOL center, STRPTR fmt, long arg1, ...)
{
ULONG len,centerat;
UBYTE tbuf[MAXLEN];

	SPrintC(&len,fmt,&arg1);

	if(len < MAXLEN)
	{
		SPrintF(tbuf,fmt,&arg1);

/* truncate? */
		len = strlen(tbuf);

		if(len > maxlen)
		{
			tbuf[maxlen] = 0x0;
			len = maxlen;
		}

/* center copy? */

		centerat = 0L;

		if(center)
		{
			if(len < maxlen)
			{
				if(center == -1)
				{
					centerat = (ULONG)maxlen - (UWORD)len;
				}
				else
				{
					centerat = (ULONG)(maxlen-(UWORD)len)/2;
				}
					memset(buf,32,centerat);

			}
		}
		strcpy(&buf[centerat],tbuf);

	}
}

/**
 ** Print label, and 1-X # of lines inside of a box at X/Y
 **/

VOID InfoBox( struct cmdVars *cmv, STRPTR label, UWORD x, UWORD y, UWORD maxlen, STRPTR lines[] )
{
register struct cmdVars *cv;
register struct RastPort *rp;
register STRPTR str;
struct DrawInfo *di;
UWORD linecnt;
UWORD boxh,boxw;
ULONG centerat;
ULONG lablen;

	cv = cmv;

	di = GetScreenDrawInfo(cv->cv_sp);

	x += cv->cv_sp->WBorLeft;
	y += cv->cv_sp->WBorTop + cv->cv_sp->Font->ta_YSize + 1;

	rp = cv->cv_win->RPort;

/* print label */

	lablen = strlen(label);
	if(lablen > maxlen)
	{
		lablen = maxlen;
		label[maxlen] = 0x0;
	}	
	centerat = ((maxlen - lablen)/2);

	SetAPen(rp, (ULONG)di->dri_Pens[HIGHLIGHTTEXTPEN]);
	Move(rp, x + 8*centerat ,y);
	Text(rp,label,strlen(label));

	y += 3;

/* calc size of box */

	linecnt = 0;
	str = lines[linecnt];

	while(str)
	{
		linecnt++;
		str = lines[linecnt];
	}

	boxh = (linecnt * 16) + 4;
	boxw = (maxlen * 8) + 8;

/* Draw bevel box for text */

	BevelBox(cv,rp,x,y,boxw,boxh,
		GTBB_Recessed, (BOOL)TRUE,
		GT_VisualInfo, cv->cv_VI,
		TAG_DONE);

	y +=16;
	x += 4;

	SetAPen(rp,(ULONG)di->dri_Pens[TEXTPEN]);

/* Draw lines of text in box */

	linecnt = 0;
	str = lines[linecnt];

	while(str)
	{
		Move(rp,x,y);
		Text(rp,str,strlen(str));
		linecnt++;
		y+=16;
		str = lines[linecnt];
	}

	FreeScreenDrawInfo(cv->cv_sp,di);
}

/**
 ** Draw Bevel Box
 **/

VOID BevelBox( struct cmdVars *cv, struct RastPort *rp, WORD l, WORD t, WORD w, WORD h, ULONG tag1, ...)
{
	DrawBevelBoxA(rp,(ULONG)l,(ULONG)t,(LONG)w,(ULONG)h,(struct TagItem *)&tag1);
}