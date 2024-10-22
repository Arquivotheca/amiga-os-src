#include "prepcard.h"
#include "tuples.h"

/* attribute memory types - figured out via some memory massage code */

#define ATTR_OVERLAPPED		0
#define ATTR_NONE		1
#define ATTR_ISMEM		2
#define ATTR_ISROM		3
#define ATTR_SMALLMEM		4

/* minimum attribute memory size (bytes) we need for CISTPL_DEVICE info
 * and link in real/unique attribute memory (e.g., HP Palm-Top cards)
 */

#define CHECKATTRCNT		16

/* preparation error returns */

#define PREP_ERROR_NOERROR	0
#define PREP_ERROR_NOCARD	1
#define PREP_ERROR_WP		2
#define PREP_ERROR_TOOSMALL	3
#define PREP_ERROR_NOTWRITABLE	4
#define PREP_ERROR_REMOVED	5
#define PREP_ERROR_MINBLOCKS	6

LONG WriteCISTPL_DEVICE( struct cmdVars *cmv, ULONG *size, UWORD attrmemtype, BOOL isADEVICE, struct DeviceTData *dt, ULONG *targetat );
LONG WriteCISTPL_FORMAT( struct cmdVars *cmv, ULONG size, UBYTE *mem);
BOOL WriteTuple(struct cmdVars *cv,UBYTE *volatile mem, UBYTE skip, UBYTE len, UBYTE *data );
UBYTE FujiDelay(UBYTE *volatile mem, UBYTE val);
VOID WriteBlock(struct cmdVars *cv, UBYTE *data, UBYTE *EDCLOC, ULONG block, UWORD blocksize, UBYTE edclen);
UWORD compute_crc(UBYTE *bufptr, UWORD len);
UBYTE compute_cksum(UBYTE *bufptr, UWORD len);

static ULONG usizes[8] = {
	512,2*1024,8*1024,32*1024,128*1024,512*1024,2*1024*1024
};

static UBYTE CISEND_tup[2] = {
	CISTPL_END,CISTPL_END,
};

static UBYTE CISTAR_tup[5] = {
	CISTPL_LINKTARGET,3,'C','I','S',
};

#define VER2_SIZE 29

static UBYTE CISVER2_tup[VER2_SIZE] = {
	CISTPL_VERS_2,
	25,			/* link */
	0,			/* struct version */
	0,			/* full comply */
	00,02,			/* data start at 512 bytes */
	0,0,			/* reserved */
	0,0,			/* VS bytes */
	1,			/* 1 CIS */
	'C','o','m','m','o','d','o','r','e',0x0,
	'A','m','i','g','a',0x0,
	CISTPL_END,CISTPL_END
};

#define T_SHORT		2
#define ST_ROOT		1
#define NRESBLK		2
#define WORDSPERBLOCK	  128
#define HASHSIZE	  (WORDSPERBLOCK - 56)
#define NUMBMPAGES	  26

struct RootBlock
{
    ULONG            rb_Type;
    ULONG            rb_HeaderKey;
    ULONG            rb_HighSeqNum;
    ULONG            rb_HashSize;
    ULONG            rb_Reserved;
    ULONG            rb_CheckSum;
    ULONG            rb_HashTable[HASHSIZE];
    ULONG            rb_BMFlag;
    ULONG            rb_BitMapPages[NUMBMPAGES];
    struct DateStamp rb_LastDate;
    UBYTE            rb_DiskName[13*4];
    struct DateStamp rb_CreateDate;
    ULONG            rb_HashChain;
    ULONG            rb_Parent;
    ULONG            rb_Extension;
    ULONG            rb_SecondaryType;
};


ULONG PrepCard( struct cmdVars *cmv, BOOL disk )
{

register struct cmdVars *cv;
register UWORD *volatile temp;
register UWORD pattern;
UWORD dupont;
ULONG size;
UBYTE *volatile attr;
UBYTE *volatile comm;
UBYTE *volatile half;
UBYTE status;
UWORD error;
ULONG failat;
UWORD attrmemtype;
UBYTE tuple[8];
struct DeviceTData dt;
int x;
UBYTE link;
BOOL isADEVICE;
BOOL isFUJI;
BOOL is4Meg;
BOOL isSizing;
UBYTE memsetcnt;
ULONG targetat;

	cv = cmv;

	error = PREP_ERROR_NOCARD;
	targetat = 0L;
	cv->cv_NoCard = TRUE;

	if(cv->cv_FromCLI)
	{
		PutStr(GetString(cv,MSG_PREP_BUSY_PROMPT));
	}
	else
	{
		if(!(DisplayQuery(cv,MSG_PREP_UI_PROMPT)))
		{
			return(RETURN_OK);
		}

		cv->cv_PrepBusy = TRUE;
		CardCheck(cv);
	}

/* do not allow card release while writing info */

	ObtainSemaphore(&cv->cv_CardSemaphore);

	if(cv->cv_IsInserted && !cv->cv_IsRemoved )
	{

		BeginCardAccess(&cv->cv_CardHandle);

		cv->cv_NoCard = FALSE;

		error = PREP_ERROR_WP;

/* start by trying to write CISTPL_DEVICE tuple */

		status = ReadCardStatus();

		if(status & CARD_STATUSF_WR)
		{

			attr = cv->cv_CardMemMap->cmm_AttributeMemory;
			comm = cv->cv_CardMemMap->cmm_CommonMemory;
			half = comm + 0x200000L;

/* size common memory, and leave as 0's so CRC/CheckSum disks are already right */

			size = 0L;

			temp = (UWORD *volatile)comm;

			*temp = 0;

			CacheClearU();

			if(*temp == 0)
			{
				pattern = 0xAA55;

/* invert pattern (if needed) as we do a reasonably quick mem size, looking
 * for memory wrapping
 */

				isSizing = TRUE;
				temp+=256;

				while(isSizing)
				{

					CacheClearU();

					if(*temp == pattern)
					{
						pattern ^= 0xFFFF;
					}

/* this is a kludge for the dupont card which reads back junk for
 * some time after writing past the end of its address space; the
 * word is mirrored for many cycles
 */

					*(temp+1) = 0x0000;

					*temp = (UWORD)pattern;

					CacheClearU();

					dupont = *(temp+1);

					if(*temp == pattern && *comm == 0 && dupont == 0)
					{
						size++;
						*temp = 0;
					}
					else
					{
						isSizing = FALSE;
					}

					temp+=256;
					if(temp >= (UWORD *volatile)attr)
					{
						isSizing = FALSE;
					}
				}

				error = PREP_ERROR_NOTWRITABLE;

				if(size)
				{
					size++;
					size <<= 9;



/* Figure out what kind of attribute memory we have -
 *
 * None, or too small an amount to use
 *
 * Unique, writable memory.  (e.g., would be the HP PalmTop cards ).
 *
 * Overlapped (e.g., Poquet cards).
 *
 * ROM (we have none, but we have to deal with this case of Device
 * tuple, or worse [ARGG!!!] tupleS in attribute memory ROM!  This
 * case will be dealt with by trying to salvage the CISTPL_DEVICE
 * tuple.  To salvage it, we have to check it, and determine if it
 * describes an SRAM, or DRAM card at least as large as our memory
 * sizing operation.  If not, well...
 *
 * Worse, we need to then look after the CISTPL_DEVICE tuple, and
 * scan for the following tuples in ROM attribute memory -
 *
 *  CISTPL_FORMAT, CISTPL_GEOMETRY, and maybe CISTPL_VERS_2.
 *
 * If any of these are in non-writeable ROM, ACK!!!!!  We can't
 * change them, and there is no point trying to write our tuples since
 * they will be caught after any of the ROM tuples.
 *
 */

/* make a copy of the CISTPL_DEVICE tuple which already exists, if one exists */

					is4Meg = FALSE;
					isADEVICE = FALSE;
					isFUJI = FALSE;

/* assume there is no ATTR mem to start */

					attrmemtype = ATTR_NONE;

					if(*attr == (UBYTE)CISTPL_DEVICE)
					{
						link = *(attr + 2);

						if(link > 4) link = 4;

						for(x = 0 ; x < (link + 3) ; x++)
						{
							tuple[x] = attr[x*2];
						}

						if(DeviceTuple(tuple,&dt))
						{
							isADEVICE = TRUE;
						}

/* assume if its not 0xFF, then its at least ROM memory */
						attrmemtype = ATTR_ISROM;

					}

/* See if we can set at least 16 bytes of memory to 0xFF, clearing
 * common memory first, so we can check for overlap later
 */


					for(x = 0, memsetcnt = 0; x < (CHECKATTRCNT*2); x+=2)
					{

/* This is a kludge to work around a BUG in the A600/A1200 PCMCIA hardware.
 * It incorrectly provides a 0x200000 (or greater) address when REG is
 * TRUE, causing > 2MEG cards which ignore the REG line to write to
 * 0x800000 instead of 0x600000.  This is due to the way the address
 * lines are passed through to the card causing the high address bit to
 * be '1' whenever the CPU generates an address of >9Meg (true for
 * 0x800000, 0x900000, and TRUE for 0xAxxxxx!
 */

						*(half + x) = 0x55;

						CacheClearU();

						*(comm + x) = 0x00;

						CacheClearU();

						*(attr + x) = 0xAA;

						CacheClearU();

						if(*(attr + x) == 0xAA)
						{
							memsetcnt++;
						}
					}

					if(memsetcnt == CHECKATTRCNT)
					{
/* assume that the write succeeded, even though it may have infact just
 * written a pattern which was already there - check again later.
 */
						attrmemtype = ATTR_ISMEM;
					}
					else
					{

/* retry using a delay for the Fujitsu SRAM card which appears not to
 * like being read back too soon after writes.
 */
						for(x = 0, memsetcnt = 0; x < (CHECKATTRCNT*2); x+=2)
						{

							*(attr + x) = 0xAA;

							FujiDelay((attr + x),0xAA);

							if(*(attr + x) != 0xAA)
							{
								break;
							}
							memsetcnt++;
						}

						if(memsetcnt == CHECKATTRCNT)
						{
							attrmemtype = ATTR_ISMEM;
							isFUJI = TRUE;
						}
					}

/* check to see if this overlapped into common memory, which was set to 0's above */
	
					for(x = 0, memsetcnt = 0; x < (CHECKATTRCNT*2); x+=2)
					{
/* Check if attribute memory write overlapped into common memory */

						if(comm[x] == 0xAA || half[x] == 0xAA)
						{

/* If less 0x600000 and 0x800000 are the same, assume partial decode of address space
 * and card is < 2Megs
 */

							if(comm[x] == 0xAA)
							{
								if(half[x] == 0xAA)
								{
									memsetcnt++;
								}
							}

/* If attribute memory mirrored at start of common memory, but not at 0x800000, then
 * assume partial decode of < 2Meg space
 */
							if(half[x] != 0xAA)
							{
								memsetcnt++;
							}
							else
							{

/* If pattern at 0x800000 matches attribute memory, but 0x600000 does not, set
 * flag indicating this card has no attribute memory
 */
								if(comm[x] == 0x00)
								{
									is4Meg = TRUE;
								}
							}
						}
					}

/* Check for >2Meg case */
					if(is4Meg)
					{
						attrmemtype = ATTR_NONE;
						memsetcnt = 0;
					}

					if(memsetcnt == CHECKATTRCNT)
					{

/* well, now we know we actually have some RAM here!!! its overlapped, but its RAM! */

						attrmemtype = ATTR_OVERLAPPED;
					}

/* Check again - Make sure this is really memory, and enough memory */

					if(attrmemtype == ATTR_ISMEM)
					{

						for(x = 0, memsetcnt = 0; x < (CHECKATTRCNT*2); x+=2 )
						{
							*(attr + x) = CISTPL_NO_LINK;

							if(isFUJI) FujiDelay((attr + x),CISTPL_NO_LINK);

							if(*(attr + x) == CISTPL_NO_LINK)
							{
								memsetcnt++;
							}

							*(attr + x) = CISTPL_END;

							if(isFUJI) FujiDelay((attr + x),CISTPL_END);
						} 


						if(!memsetcnt)
						{
/* nothing really wrote, so conclude its not RAM at all, but NO MEM */

							attrmemtype = ATTR_NONE;
						}
						else
						{
							if(memsetcnt < CHECKATTRCNT)
							{
/* something wrote, but it wasn't enough to be usable for the basic tuples which need to
 * go into ATTR mem (if at all possible)
 */
								attrmemtype = ATTR_SMALLMEM;
							}
						}
					}


/* Figure out what we would like to write for our CISTPL_DEVICE tuple */

					error = PREP_ERROR_NOCARD;

					if(!cv->cv_IsRemoved)
					{					

						error = WriteCISTPL_DEVICE(cv,&size,attrmemtype,isADEVICE,&dt,&targetat);

/* write CISTPL_VER_2 tuple for compliance */

						if(!error)
						{
							comm += (targetat + 5);

							error = PREP_ERROR_NOTWRITABLE;

							if(WriteTuple(cv,comm,1,VER2_SIZE,&CISVER2_tup[0]))
							{
								error = PREP_ERROR_NOERROR;

/* write CISTPL_FORMAT, and CISTPL_GEOMETRY if disklike */

								comm += (VER2_SIZE - 2);

								if(disk)
								{
									error = WriteCISTPL_FORMAT(cv,size,comm);
								}
							}
						} /* write device tuple ok? */
					} /* still have a card?? */
				} /* if card sized */
 			} /* if I could write a 0 to start of common mem */
 		} /* if write enabled */

		EndCardAccess(&cv->cv_CardHandle);

	} /* if inserted, and not removed */
 
	ReleaseSemaphore(&cv->cv_CardSemaphore);
	
	cv->cv_PrepBusy = FALSE;
	if(cv->cv_FromCLI)
	{
		PutStr(GetString(cv,MSG_PREP_DONE_PROMPT));
	}

 	failat = RETURN_OK;
	if(error)
	{
		if(error == PREP_ERROR_NOTWRITABLE)
		{
/* did we fail mid-stream because of a card removal? */

			if(cv->cv_IsRemoved || cv->cv_NoCard)
			{
				error = PREP_ERROR_REMOVED;
			}
		}

		DisplayError(cv,MSG_PREP_NOCARD_ERROR + (error - 1L));
		failat = RETURN_ERROR;

	}

/* force card to be reexamined, and info displayed */

	cv->cv_FirstInfoDraw = FALSE;

	return(failat);
}


/**
 ** Write a CISTPL_FORMAT tuple
 **/

LONG WriteCISTPL_FORMAT( struct cmdVars *cmv, ULONG size, UBYTE *mem)
{
register struct cmdVars *cv;
register struct TP_Format *tpf;
UBYTE tuples[26];
UBYTE geotuple[10];

LONG error;
UBYTE defedc;
UBYTE edclen;
ULONG defsize;
ULONG defblks;
LONG EDCLOC;
UWORD blocksize;
BOOL useADV;
BOOL useGEO;
LONG sectrk, trkcyl, cyls;
struct RootBlock *root;
LONG i,*p, sum, rootBlk, wordsPerBlk;

	cv = cmv;

	error = PREP_ERROR_MINBLOCKS;

	tpf = (struct TP_Format *)&tuples[0];

	tpf->TPL_CODE = CISTPL_FORMAT;
	tpf->TPL_LINK = 20;

	tpf->TPLFMT_TYPE[0] = TPLFMTTYPE_DISK;

	useADV = FALSE;

	if(cv->cv_IsAdvanced)
	{
		if(cv->cv_TotalSize)
		{
			if(cv->cv_TotalBlocks >= MINDISKBLOCKS)
			{
				useADV = TRUE;


			}
		}
	}

	defedc = 0;		/* no error detection */
	edclen = 0;		/* 0 length error detection */

	if(cv->cv_IsAdvanced)
	{
		if(cv->cv_DefaultEDC == TPLFMTEDC_CHSUM)
		{
			edclen = 1;
			defedc = (1<<3)|TPLFMTEDC_CHSUM;
		}

		if(cv->cv_DefaultEDC == TPLFMTEDC_CRC)
		{
			edclen = 2;
			defedc = (2<<3)|TPLFMTEDC_CRC;
		}
	}


	tpf->TPLFMT_EDC[0] = defedc;

	tpf->TPLFMT_OFFSET[0] = 0x00;
	tpf->TPLFMT_OFFSET[1] = 0x02;
	tpf->TPLFMT_OFFSET[2] = 0x0;
	tpf->TPLFMT_OFFSET[3] = 0x0;

/* limit to 4 meg max size */

	if(size > MAXDISKSIZE) size = MAXDISKSIZE;

	defsize = size - 512;
	blocksize = 512;
	defblks = defsize/blocksize;

	if(cv->cv_IsAdvanced)
	{

		blocksize = 1L << (cv->cv_DefaultBKSZ + 7);

		defblks = defsize/(blocksize + edclen);

		if(useADV)
		{
			defblks = cv->cv_TotalBlocks;

			if(cv->cv_DefaultGEO)
			{
				defblks = cv->cv_GeoBlocks;
			}

		}

		defsize = defblks * (blocksize + edclen);
	}

	tpf->TPLFMT_NBYTE[0] = (defsize) & 0xFF;
	tpf->TPLFMT_NBYTE[1] = (defsize >> 8) & 0xFF;
	tpf->TPLFMT_NBYTE[2] = (defsize >> 16) & 0xFF;
	tpf->TPLFMT_NBYTE[3] = (defsize >> 24) & 0xFF;

	tpf->TPLFMT_BKSZ[0] = (blocksize) & 0xFF;
	tpf->TPLFMT_BKSZ[1] = (blocksize >> 8) & 0xFF;


	tpf->TPLFMT_NBLOCKS[0] = (defblks) & 0xFF;
	tpf->TPLFMT_NBLOCKS[1] = (defblks >> 8) & 0xFF;
	tpf->TPLFMT_NBLOCKS[2] = (defblks >> 16) & 0xFF;
	tpf->TPLFMT_NBLOCKS[3] = (defblks >> 24) & 0xFF;

/* place checksums in table, CRC's in blocks */

	EDCLOC = 0L;

	if(edclen == 1)
	{

		EDCLOC = (ULONG)((defblks * blocksize)+512);
	}


	tpf->TPLFMT_EDCLOC[0] = (EDCLOC) & 0xFF;
	tpf->TPLFMT_EDCLOC[1] = (EDCLOC >> 8) & 0xFF;
	tpf->TPLFMT_EDCLOC[2] = (EDCLOC >> 16) & 0xFF;
	tpf->TPLFMT_EDCLOC[3] = (EDCLOC >> 24) & 0xFF;

	tuples[22] = CISTPL_END;
	tuples[23] = CISTPL_END;

	if(defblks >= MINDISKBLOCKS)
	{
		error = PREP_ERROR_NOTWRITABLE;

		if(WriteTuple(cv,mem,1,24,(UBYTE *)&tpf[0]))
		{
			mem += 22;

			geotuple[0] = CISTPL_GEOMETRY;
			geotuple[1] = 4;
			useGEO = TRUE;
			
			CalcBestGeometry(defblks,&sectrk,&trkcyl,&cyls,FALSE);

			if(useADV)
			{
				if(cv->cv_DefaultGEO)
				{
					sectrk = cv->cv_GeoSecs;
					trkcyl = cv->cv_GeoTrks;
					cyls = cv->cv_GeoCyls;
        			}
				else
				{
					useGEO = FALSE;
				}
			}

			geotuple[2] = (UBYTE)sectrk;
			geotuple[3] = (UBYTE)trkcyl;
			geotuple[4] = (UBYTE)cyls & 0xFF;
			geotuple[5] = (UBYTE)( (cyls>>8) & 0xFF);
			geotuple[6] = CISTPL_END;
			geotuple[7] = CISTPL_END;

			if(useGEO)
			{
				if(WriteTuple(cv,mem,1,8,(UBYTE *)&geotuple[0]))
				{
					error = PREP_ERROR_NOERROR;
				}
			}
			else
			{
				error = PREP_ERROR_NOERROR;
			}
		}
	}

	if(error == PREP_ERROR_NOERROR)
	{
/* fill partition with 0's, clears CRC's, and CHECKSUMS */

		memset(cv->cv_CardMemMap->cmm_CommonMemory + 512, 0L, defsize);

/* try to do a quick format on this card so it will be useable from WB */

		wordsPerBlk = (blocksize >> 2);

		if(wordsPerBlk == WORDSPERBLOCK)
		{
			if(root = (struct RootBlock *)AllocVec(blocksize,MEMF_PUBLIC|MEMF_CLEAR))
			{

				root->rb_Type		= T_SHORT;
				root->rb_HashSize	= HASHSIZE;
				root->rb_SecondaryType	= ST_ROOT;

			/* let validator make bitmap - this fields are already FALSE, and 0!!

				root->rb_BMFlag		= FALSE;
				root->rb_BitMapPages[0] = 0;
			*/

				DateStamp(&root->rb_CreateDate);
				DateStamp(&root->rb_LastDate);

				root->rb_DiskName[0] = strlen(GetString(cv,MSG_PREP_EMPTY_NAME));
				strcpy(&root->rb_DiskName[1],GetString(cv,MSG_PREP_EMPTY_NAME));

				root->rb_CheckSum = sum = 0;
				for (i = 0, p = (LONG *)root; i < wordsPerBlk; sum = sum + *p++, i++);
				root->rb_CheckSum = -sum;

				if(useGEO)
				{
					defblks = (ULONG)(sectrk * trkcyl * cyls);

				}

				rootBlk = (defblks - 1 + NRESBLK) >> 1;

				WriteBlock(cv, (UBYTE *)root,
						(UBYTE *)EDCLOC,
						rootBlk,
						blocksize,
						edclen);

				p = (LONG *)root;

				/* make a dos disk */

				for(i = 0; i < wordsPerBlk; i++) p[i] = ID_FFS_DISK;

				WriteBlock(cv, (UBYTE *)p,
						(UBYTE *)EDCLOC,
						0L,
						blocksize,
						edclen);

				FreeVec((UBYTE *)root);

			}
		
		}

		CacheClearU();	/* flush data to card */

		if(cv->cv_IsRemoved)
		{
			error = PREP_ERROR_NOTWRITABLE;
		}
		
	}
	return(error);
}

/**
 ** Write a block to card with CHECKSUM or CRC if needed
 **
 ** Cannot use .device here since PrepCard owns the card, not the .device
 **/

VOID WriteBlock(struct cmdVars *cv, UBYTE *data, UBYTE *EDCLOC, ULONG block, UWORD blocksize, UBYTE edclen)
{
register UBYTE *partition;
UWORD crc;

	partition = cv->cv_CardMemMap->cmm_CommonMemory + 512;

	partition += (blocksize * block);

/* adjust if CRC */

	if(edclen == 2)
	{
		partition += (block * 2);
	}

	CopyMem((APTR)data,(APTR)partition,(ULONG)blocksize);

/* store CRC in block */

	if(edclen == 2)
	{
		partition += blocksize;
		crc = compute_crc(data,blocksize);

		*partition = (crc & 0xFF);
		*(partition + 1) = ((crc >> 8) & 0xFF);
		
	}

/* store checksum in table */

	if(edclen == 1)
	{
		EDCLOC += (ULONG)cv->cv_CardMemMap->cmm_CommonMemory;

		EDCLOC += block;

		*EDCLOC = compute_cksum(data,blocksize);
	}
}

UWORD compute_crc(UBYTE *bufptr, UWORD len)
{
register int     i;
register UWORD   crc;

	crc = 0;
        while (len--) {
                crc ^= (UWORD)(*bufptr++) << 8;
                for (i = 0; i < 8; ++i) {
                        if (crc & 0x8000)
                                crc = (crc << 1) ^ 0x1021;
                        else
                                crc <<= 1;
                }
        }
        return(crc);
}

UBYTE compute_cksum(UBYTE *bufptr, UWORD len)
{
register UBYTE cksum;

	cksum = 0;

	while(len--)
	{
		cksum += (*bufptr & 0xFF);
		bufptr++;
	}
	return(cksum);
}
/**
 ** Write a CISTPL_DEVICE tuple
 **/

LONG WriteCISTPL_DEVICE( struct cmdVars *cmv, ULONG *msize, UWORD attrmemtype, BOOL isADEVICE, struct DeviceTData *dt, ULONG *targetat )
{
register struct cmdVars *cv;
UBYTE deftype,defspeed;
UBYTE units,usize;
UBYTE *volatile attr;
UBYTE *volatile comm;

int x, y;
int lasty,lastx;
int besty,bestx;
BOOL match;
ULONG size,bestsize;
LONG error;
UBYTE CISDEV_tup[8];
UBYTE CISLINK_tup[8];
BOOL scanrom;

	error = PREP_ERROR_TOOSMALL;

	cv = cmv;

	deftype = 6<<4;		/* SRAM   */
	defspeed = 1;		/* 250 ns */

/* calc default units, and unit size - best fit requires least # of units, but if we can't
 * find an exact match, take the best fit we can get
 */

	size = *msize;

	size &= ~(511L);
	match = FALSE;
	lasty = 0;
	lastx = 1;

	for(x = 1; x < 33; x++)
	{
		for(y = 0; y < 8; y++)
		{

			if( (usizes[y] * x) <= size )
			{
				besty = y;
				bestx = x;
				bestsize = (usizes[y] * x);

				if( (usizes[y] * x) == size )
				{
					if(y >= lasty)
					{
						lasty = y;
						lastx = x;
						match = TRUE;
					}
				}
			}
		}
	}	

	if(!match)
	{
		lastx = bestx;
		lasty = besty;

/* It turns out we might have come up with an actual size that cannot be
 * expressed using unit size, and # of units.  An example would be 17K
 * actual memory size.  The best we could get would be 16K using 512 byte
 * unit sizes, or 18K using 2K size units.  While this is unlikely to be
 * the case, its caught by the double loop above, and size is adjusted
 * if needed to be the best possible size which can be expressed.
 *
 * In the case above we'd come up with 16K actual size expressed as 2
 * 8K units.
 *
 */

		size = bestsize;
	}


	units = lastx;
	usize = lasty;

	if(cv->cv_IsAdvanced)
	{
		deftype = (UBYTE)((cv->cv_DefaultType + 6)<<4);
		defspeed = (UBYTE)cv->cv_DefaultSpeed + 1;

		if(cv->cv_TotalSize)
		{
			usize = (UBYTE)cv->cv_DefaultUnits;
			units = (UBYTE)cv->cv_DefaultUnitNum;
		}
	}

/* make sure advanced requested units, and unit size is no larger than
 * the actual size we calculated.
 */

	if((ULONG)(units * usizes[usize]) <= size)
	{

		size = (ULONG)(units * usizes[usize]);

		units--;
		units <<= 3;


		CISDEV_tup[0] = CISTPL_DEVICE;		/* code */
		CISDEV_tup[1] = 4;			/* link */
		CISDEV_tup[2] = (deftype|defspeed);	/* type, WPS == 0, speed */
		CISDEV_tup[3] = (usize|units);		/* units * usize */
		CISDEV_tup[4] = 0;			/* reserve for future use */
		CISDEV_tup[5] = 0;			/* reserve for future use */

		CISLINK_tup[0] = CISTPL_LONGLINK_C;	/* code */
		CISLINK_tup[1] = 4;			/* link */
		CISLINK_tup[2] = 0;			/* default is start of common mmem */
		CISLINK_tup[3] = 0;
		CISLINK_tup[4] = 0;
		CISLINK_tup[5] = 0;

		comm = cv->cv_CardMemMap->cmm_CommonMemory;
		attr = cv->cv_CardMemMap->cmm_AttributeMemory;

/*
 * Overlapped requires adjusting the long link to skip over the CISTPL_DEVICE tuple
 */
		error = PREP_ERROR_NOTWRITABLE;

		if(attrmemtype == ATTR_OVERLAPPED || attrmemtype == ATTR_ISMEM)
		{
			if(WriteTuple(cv,attr,2,6,&CISDEV_tup[0]))
			{
				if(attrmemtype == ATTR_OVERLAPPED)
				{
					CISLINK_tup[2] = 0x1C;				
					*targetat = 0x1CL;
				}

				attr += (6*2);

				if(WriteTuple(cv,attr,2,6,&CISLINK_tup[0]))
				{
					attr += (6*2);

					if(WriteTuple(cv,attr,2,2,&CISEND_tup[0]))
					{
						if(attrmemtype == ATTR_OVERLAPPED)
						{
							comm = attr + (2*2);
						}
						if(WriteTuple(cv,comm,1,5,&CISTAR_tup[0]))
						{
							comm+=5;
							if(WriteTuple(cv,comm,1,2,&CISEND_tup[0]))
							{
								error = PREP_ERROR_NOERROR;
							}
						}
					}
				}
			}
		}


/*
 * If mem is too small, make sure we have CISTPL_END in attr mem, and
 * continue as if its no mem.
 */

		if(attrmemtype == ATTR_SMALLMEM)
		{
			*attr = CISTPL_END;
			*(attr + 2) = CISTPL_END;
			attrmemtype = ATTR_NONE;
		}

		if(attrmemtype == ATTR_NONE)
		{
			if(WriteTuple(cv,comm,1,5,&CISTAR_tup[0]))
			{
				comm += 5;
				if(WriteTuple(cv,comm,1,6,&CISDEV_tup[0]))
				{
					comm+=6;
					if(WriteTuple(cv,comm,1,2,&CISEND_tup[0]))
					{
						*targetat += 6;
						error = PREP_ERROR_NOERROR;
					}
				}
			}
		}
/* Worst case is ROM!!!
 *
 * If there is already a CISTPL_DEVICE tuple here, see if we can use
 * it.  Note that if there is not a 01 at the beg of attribute memory,
 * we have no problem since whatever there is invalid, but...
 * 
 * IF it is a 0x1, then we have to assume its a CISTPL_DEVICE tuple,
 * and in that case we have to decide if -
 *
 * IF its a valid CISTPL_DEVICE tuple, we might be able to use
 * it as is (e.g., it describes a card of the same type, speed, and a
 * size which as at least as large as we need.  Actually speed, and
 * type (if it says SRAM or DRAM) can likely be ignored even.
 *
 * IF its not a valid CISTPL_DEVICE tuple, we cannot change this
 * card, or use it.
 *
 * Finally if its usable, we need to check it again later for other
 * tuples in non-writeable attribute memory (e.g., CISTPL_FORMAT)
 * which we might want to remove, or change.
 */
		if(attrmemtype == ATTR_ISROM)
		{

			if(isADEVICE)
			{
				if( dt->dtd_DTsize >= size )
				{
					if(dt->dtd_DTtype == DTYPE_SRAM || dt->dtd_DTsize == DTYPE_DRAM)
					{
						if(dt->dtd_DTspeed <= 250L)
						{
/* well, we can use this (good enough), but can we still write other important info
 * to common memory?   Scan for info past the CISTPL_DEVICE tuple in ROM
 */

							size = dt->dtd_DTsize;

							scanrom = TRUE;

							while(scanrom)
							{
/* check for an CISTPL_END, or a LONGLINK_C, in which case we are in good shape */
/* an alternative is a LINK of CISTPL_END, which means the same thing 		*/

								if(attr[2] == CISTPL_END)
								{
									scanrom = FALSE;
									error = PREP_ERROR_NOERROR;
								}
/* skip over CISTPL_NULL */
								if(*attr == CISTPL_NULL)
								{
									attr += 2;
								}
								else
								{
/* next tuple at body*2 + 2 bytes for code + 2 bytes for link - initially we
 * skip past the first tuple which we already know is a CISTPL_DEVICE tuple
 */
									attr += ((attr[2] * 2)+4);

									if(scanrom)
									{
										if(*attr == CISTPL_END|| *attr == CISTPL_LONGLINK_C)
										{
											if(*attr == CISTPL_LONGLINK_C)
											{
/* pick up where the LINK_TARGET needs to be */
/* I really should look for a LONGLINK_A tuple here too, but forget it.
 * Its just so darn unlikely, and a worthless thing to do, not this trip!
 */

												*targetat =	attr[4] +
														attr[6] << 8 +
														attr[8] << 16 +
														attr[10] << 24;

											}
											scanrom = FALSE;
											error = PREP_ERROR_NOERROR;
										}

/* check for other kinds of tuples which we might want to rewrite */

										if(*attr == CISTPL_FORMAT || *attr == CISTPL_GEOMETRY)
										{
/*
 * If so, stop, and we will return that we cannot rewrite this info.
 * We would like to be able to rewrite the VERS2 tuple too, but we can live
 * with not being able to do so
 */


											scanrom = FALSE;
										}
									}

								}
/* do not scan past end of common memory */

								if(attr >= (UBYTE *volatile)0xA20000)
								{

									scanrom = FALSE;
									error = PREP_ERROR_NOTWRITABLE;
								}
							}

/* If we are able to use this info in ROM, then add the LINK_TARGET, and we
 * are done.
 */

							if(!error)
							{
								error = PREP_ERROR_NOTWRITABLE;

/* make sure ROM LONKLINK is within 4 megs, plus some reasonable buffer */

								if(*targetat <= (4*1024*1024 - 10))
								{
									comm += *targetat;
									if(WriteTuple(cv,comm,1,5,&CISTAR_tup[0]))
									{
										comm+=5;
										if(WriteTuple(cv,comm,1,2,&CISEND_tup[0]))
										{
											error = PREP_ERROR_NOERROR;
										}
									}
								}
							}
						}
					}
				}
			}
		}

	}

/* Replace actual size, since it might have been modified by our code above
 * which took actual size, and came up with the best possible units,
 * unit size, and actual size.
 */

	*msize = size;

	return(error);
}

/**
 ** Code to copy a tuple to attribute, or common memory.
 **/

BOOL WriteTuple(struct cmdVars *cv,UBYTE *volatile mem, UBYTE skip, UBYTE len, UBYTE *data )
{
int x;
BOOL wrote;

	wrote = TRUE;

	for(x = 0; x < len; x++)
	{

		*mem = *data;

		CacheClearU();

		if(skip == 2) FujiDelay(mem,*data);

		if(*mem != *data)
		{
			wrote = FALSE;
			break;

		}
		mem += skip;
		data++;
	}
	return(wrote);
}

/* delay during attr mem writes for FUJI type SRAM cards
 * Fujitsu SRAM cards appear to require roughly 5 milliseconds after
 * a write to attribute mem for the value to read back properly.
 * 
 * I'll do a poll loop, which should fall through quickly for most
 * cards, and has enough slop to be long enough for the Fujitsu cards.
 */

#define FUJIDELAY		500
#define FUJIRETRY		50

UBYTE FujiDelay(UBYTE *volatile mem, UBYTE val)
{
UBYTE *volatile ciapra;
UBYTE cia;
int retry,delay;

	ciapra = (UBYTE *volatile)0xbfe001;

	for(retry = 0; retry < FUJIRETRY; retry++)
	{
		for(delay = 0; delay < FUJIDELAY; delay++)
		{
			cia = *ciapra;
		}	

		if(*mem == val)
		{

			break;
		}

	}

	return(cia);	
}