#include "flash.h"
#include <dos/filehandler.h>
#include <dos/dosextens.h>

extern ULONG totalsizes[];
extern ULONG zonesizes[];

#define	DEBUG 1
#ifdef  DEBUG
extern void kprintf(char *,...);
#define D(a)    kprintf a
#else
#define D(a)
#endif

#define INPUT_TRACKDISK	0
#define INPUT_CARDDISK	1
#define INPUT_RAMDRIVE	2
#define INPUT_FILE	3

#define BB_LEN	   86		/*   "    "    "    "  "  2.x     "       "   */

UBYTE bb_img[] = {
	0,
	0,
	3,
	0x70,
	0x43,
	0xFA,
	0,
	0x3E,
	0x70,
	0x25,
	0x4E,
	0xAE,
	0xFD,
	0xD8,
	0x4A,
	0x80,
	0x67,
	0xC,
	0x22,
	0x40,
	0x8,
	0xE9,
	00,
	06,
	00,
	0x22,
	0x4E,
	0xAE,
	0xFE,
	0x62,
	0x43,
	0xFA,
	00,
	0x18,
	0x4E,
	0xAE,
	0xFF,
	0xA0,
	0x4A,
	0x80,
	0x67,
	0xA,
	0x20,
	0x40,
	0x20,
	0x68,
	00,
	0x16,
	0x70,
	00,
	0x4E,
	0x75,
	0x70,
	0xFF,
	0x4E,
	0x75,
	'd','o','s','.','l','i','b','r','a','r','y',
	0,
	'e','x','p','a','n','s','i','o','n','.','l','i','b','r','a','r','y',
	0,
};


struct IntelCIS {
	/* DEVICE */

	UBYTE	ics_dtuple;
	UBYTE	ics_pad0;
	UBYTE	ics_dlink;
	UBYTE	ics_pad1;
	UBYTE	ics_dtype;
	UBYTE	ics_pad2;
	UBYTE	ics_dsize;
	UBYTE	ics_pad3;

	/* LONGLINK_C */

	UBYTE	ics_ltuple;
	UBYTE	ics_pad4;
	UBYTE	ics_llink;
	UBYTE	ics_pad5;
	UBYTE	ics_lllo;
	UBYTE	ics_pad6;
	UBYTE	ics_llhi;
	UBYTE	ics_pad7;
	UBYTE	ics_lhl0;
	UBYTE	ics_pad8;
	UBYTE	ics_lhhi;
	UBYTE	ics_pad9;

	/* END */

	UBYTE	ics_etuple;
	UBYTE	ics_pad10;
	UBYTE	ics_elink;
	UBYTE	ics_pad11;

	/* LINK_TARGET */

	UBYTE	ics_TARtuple;
	UBYTE	ics_TARlink;
	UBYTE	ics_TARstring[3];

	/* FORMAT */

	UBYTE	ics_ftuple;
	UBYTE	ics_flink;
	UBYTE	ics_ftype;
	UBYTE	ics_fedc;
	UBYTE	ics_foffset[4];
	UBYTE	ics_fsize[4];
	UBYTE	ics_fbsize[2];
	UBYTE	ics_fblocks[4];
	UBYTE	ics_fedclock[4];

	/* GEOMETRY */

	UBYTE	ics_gtuple;
	UBYTE	ics_glink;
	UBYTE	ics_gsectors;
	UBYTE	ics_gtracks;
	UBYTE	ics_gcyls[2];

	/* VER2 */

	UBYTE	ics_vtuple;
	UBYTE	ics_vlink;
	UBYTE	ics_vversion;
	UBYTE	ics_vcomply;
	UBYTE	ics_vstart[2];
	UBYTE	ics_vreserved[2];
	UBYTE	ics_vVS[2];
	UBYTE	ics_vCIS;
	UBYTE	ics_vCommodore[10];
	UBYTE	ics_vAmiga[6];

	/* END */

	UBYTE	ics_eetuple;
	UBYTE	ics_eelink;

};

static UBYTE devicetypes[] = {
	(0x5<<4)|4,		/* FlashROM, 100ns */
	(0x5<<4)|3,		/* FlashROM, 150ns */
	(0x5<<4)|2,		/* FlashROM, 200ns */
	(0x5<<4)|1,		/* FlashROM, 250ns */
};

static UBYTE devicesizes[] = {
	(0x1<<3)|3,		/* 2-1 units of 32K */
	(0x0<<3)|4,		/* 1-1 units of 128K */
	(0x1<<3)|4,		/* 2-1 units of 128K */
	(0x0<<3)|5,		/* 1-1 units of 512K */
	(0x1<<3)|5,		/* 2-1 units of 512K */
	(0x0<<3)|6,		/* 1-1 units of 2M */
	(0x1<<3)|6,		/* 2-1 units of 2M */
};

void DrawDiskPrompt( struct cmdVars *cv, BOOL blink, STRPTR str )
{
	ClearBox(cv,STAT_BOX_ID);

	if(!blink)
	{
		StatusBox(cv,0,0,TRUE,STAT_BOX_ID,str);
		StatusBox(cv,0,2,TRUE,STAT_BOX_ID,"Select CONFIRM to continue");

	}
}

/*
 * Prompt for disk
 */

BOOL PromptForDisk( struct cmdVars *cv, STRPTR str, BOOL waitcard )
{

struct IntuiMessage *imsg;
ULONG	class;
UWORD	code;
struct	Gadget *gad;
ULONG	signals;
BOOL	waiting;
BOOL	confirmed;
int	iticks;
UBYTE	blink;

	confirmed = FALSE;
	waiting = TRUE;
	iticks = 0;
	blink = 0x1;

	DrawDiskPrompt(cv,FALSE,str);

	AbortOnOff(cv,FALSE);
	ConfirmOnOff(cv,FALSE);

	if(cv->cv_IsInserted && waitcard)
	{
		Signal(cv->cv_Task,1L<<cv->cv_Signal);
	}

	while(waiting)
	{
		signals = Wait((1L<<cv->cv_win->UserPort->mp_SigBit)|(1L<<cv->cv_Signal));

	/* Release card everytime if waiting for carddisk */

		if(signals & (1L<<cv->cv_Signal))
		{
			if(cv->cv_IsRemoved || waitcard)
			{
				cv->cv_IsRemoved = FALSE;
				cv->cv_IsInserted = FALSE;
				ReleaseCard(&cv->cv_CardHandle,0L);
			}
		}

		while(imsg = GT_GetIMsg(cv->cv_win->UserPort))
		{

			class = imsg->Class;
			code = imsg->Code;
			gad = imsg->IAddress;

			GT_ReplyIMsg( imsg );

			if(class == IDCMP_GADGETUP)
			{

				if((ULONG)gad->UserData == CM_GADGET_ABORT)
				{
					waiting = FALSE;
				}

				if((ULONG)gad->UserData == CM_GADGET_CONFIRM)
				{
					confirmed = TRUE;
					waiting = FALSE;
				}
			}
			if(class == IDCMP_INTUITICKS)
			{

				iticks++;
				if(iticks > 5)
				{
					DrawDiskPrompt(cv,(BOOL)blink&0x01,str);
					iticks = 0;
					blink++;
				}
			}
		}
	}

	ConfirmOnOff(cv,TRUE);

	return(confirmed);
}

BOOL DoWriteCIS( struct cmdVars *cv, struct DriveGeometry *dg )
{
struct IntelCIS *ics;
ULONG temp;
APTR hold;
BOOL success;

	success = FALSE;

	if(ics = (struct IntelCIS *)AllocMem(512L,MEMF_PUBLIC|MEMF_CLEAR))
	{

		ics->ics_dtuple = 0x01;
		ics->ics_dlink = 0x02;

		ics->ics_ltuple = 0x12;
		ics->ics_llink = 0x4;
		ics->ics_lllo = 0x18;

		ics->ics_etuple = 0xff;
		ics->ics_elink = 0xff;


		ics->ics_TARtuple = 0x13;
		ics->ics_TARlink = 0x03;
		ics->ics_TARstring[0] = 'C';
		ics->ics_TARstring[1] = 'I';
		ics->ics_TARstring[2] = 'S';

		ics->ics_ftuple = 0x41;
		ics->ics_flink = 0x14;
		ics->ics_foffset[1] = 0x02;	/* 512 */

		ics->ics_gtuple = 0x42;
		ics->ics_glink = 0x4;

		ics->ics_eetuple = 0xff;
		ics->ics_eelink = 0xff;

		ics->ics_vtuple = 0x40;		/* VER2 */
		ics->ics_vlink = 25;
		ics->ics_vstart[1] = 0x02;	/* data at 512 bytes */
		ics->ics_vCIS = 1;		/* 1 CIS */

		strcpy(&ics->ics_vCommodore[0],"Commodore");
		strcpy(&ics->ics_vAmiga[0],"Amiga");


	/* now figure out device tuple */

		ics->ics_dtype = devicetypes[cv->cv_SpeedIndex];
		ics->ics_dsize = devicesizes[cv->cv_TotalIndex];

	/* figure formatting */

		temp = totalsizes[cv->cv_TotalIndex];
		temp -= 512L;

		ics->ics_fsize[0] = temp & 0xFF;
		ics->ics_fsize[1] = temp>>8 & 0xFF;
		ics->ics_fsize[2] = temp>>16 & 0xFF;
		ics->ics_fsize[3] = temp>>24 & 0xFF;

		temp = dg->dg_TotalSectors;

		ics->ics_fblocks[0] = temp & 0xFF;
		ics->ics_fblocks[1] = temp>>8 & 0xFF;
		ics->ics_fblocks[2] = temp>>16 & 0xFF;
		ics->ics_fblocks[3] = temp>>24 & 0xFF;
		
		temp = dg->dg_SectorSize;

		ics->ics_fbsize[0] = temp & 0xFF;
		ics->ics_fbsize[1] = temp>>8 & 0xFF;

	/* geometry tuple */

		ics->ics_gsectors = (UBYTE)dg->dg_TrackSectors;
		ics->ics_gtracks = (UBYTE)dg->dg_Heads;
		ics->ics_gcyls[0] = (UBYTE)((dg->dg_Cylinders) & 0xFF);
		ics->ics_gcyls[1] = (UBYTE)((dg->dg_Cylinders>>8) & 0xFF);
		

	/* write to card */

		cv->cv_ZoneHandle.zh_to = cv->cv_CardMemMap->cmm_CommonMemory;
		hold = cv->cv_ZoneHandle.zh_from;
		cv->cv_ZoneHandle.zh_from = (APTR)ics;
		cv->cv_ZoneHandle.zh_size = 512L;

		if(WriteZone(&cv->cv_ZoneHandle))
		{
			success = TRUE;
		}
		cv->cv_ZoneHandle.zh_from = (APTR)hold;

		FreeMem(ics,512L);
	}
	return(success);
}

BOOL DoDiskCopy( struct cmdVars *cmv, struct DriveGeometry *dg,
  struct IOExtTD *ior, STRPTR prompt, BOOL cc0, BOOL verify, BOOL doprompt )
{
register struct cmdVars *cv;
ULONG total,zone,temp,size,nsize,offset;
struct DriveGeometry dvg;
BOOL	waitfordisk,firstpass,nodisk,tryread,trywrite,completed,askfordisk;
ULONG	Vpp;
UBYTE   *message;
BOOL	firstblock;
ULONG	*bblock;
ULONG	checksum;
int	i;

	cv = cmv;

	cv->cv_ZoneHandle.zh_to = cv->cv_CardMemMap->cmm_CommonMemory;

	total = totalsizes[cv->cv_TotalIndex];
	zone = zonesizes[cv->cv_ZoneIndex];

	temp = total;
	
	if(zone < (256*1024)) zone = 256*1024;

	if(cc0)
	{
		zone = cv->cv_ZoneBufSize;
	}

	cv->cv_ZoneHandle.zh_size = zone;

	/* reserve space for CIS? */

	if(cv->cv_SourceIndex <= 2)
	{
		temp -= 512L;
		cv->cv_ZoneHandle.zh_to = (UBYTE *) ( (ULONG)cv->cv_CardMemMap->cmm_CommonMemory + 512L);
	}

	waitfordisk = TRUE;
	firstpass = TRUE;
	completed = FALSE;
	firstblock = TRUE;

	offset = 0L;

	Vpp = CARD_VOLTAGE_12V;

	if(verify) Vpp = CARD_VOLTAGE_0V;

	while(waitfordisk)
	{
		waitfordisk = FALSE;

		if(doprompt)
		{
			askfordisk = PromptForDisk(cv,prompt,cc0);
		}
		else
		{
			askfordisk = TRUE;
		}
		if(askfordisk)
		{
			doprompt = TRUE;		/* always ask from now on */
			nodisk = TRUE;			/* Assume no disk for now */

			AbortOnOff(cv,TRUE);

			ClearBox(cv,STAT_BOX_ID);

			ior->iotd_Req.io_Command = TD_CHANGESTATE;

			if(!(DoIO((struct IOStdReq *)ior)))
			{
				if(ior->iotd_Req.io_Actual == 0L)
				{
					nodisk = FALSE;	/* Yup, there is a disk in this drive */

					if(firstpass)
					{						
						firstpass = FALSE;

						if(dg == NULL)
						{
							dg = &dvg;

							ior->iotd_Req.io_Data = dg;
							ior->iotd_Req.io_Length = sizeof(struct DriveGeometry);
							ior->iotd_Req.io_Command = TD_GETGEOMETRY;

							if(DoIO((struct IOStdReq *)ior))
							{
								DisplayError(cv,"ERROR: Error obtaining drive Geometry");
								return(FALSE);
							}
						}
						size = dg->dg_TotalSectors * dg->dg_SectorSize;

						if(size > temp)
						{
							AbortOnOff(cv,TRUE);
							DisplayError(cv,"ERROR: Disk is larger than FlashROM");
							return(FALSE);
						}
					}

					tryread = TRUE;

					while(tryread)
					{
						/* read from source into zone buffer */

						tryread = FALSE;

						AbortOnOff(cv,TRUE);

						ior->iotd_Req.io_Data = cv->cv_ZoneHandle.zh_from;
						bblock = (ULONG *)cv->cv_ZoneHandle.zh_from;

						ior->iotd_Req.io_Offset = offset;

						ior->iotd_Req.io_Command = CMD_READ;

						if(size >= zone)
						{
							nsize = zone;
						}
						else
						{
							nsize = size;
						}
						ior->iotd_Req.io_Length = nsize;

						StatusBox(cv,0,1,TRUE,STAT_BOX_ID,"Reading .....");

						if(!(DoIO((struct IOStdReq *)ior)))
						{
							ior->iotd_Req.io_Command = TD_MOTOR;
							ior->iotd_Req.io_Length = 0;
							DoIO((struct IOStdReq *)ior);

							if(firstblock && cv->cv_BootOn)
							{
								D(("Install boot block\n"));

								for(i=2;i<512;i++)
								{
									bblock[i] = 0L;
								}

								CopyMem((char *)&bb_img[0],(char *)&bblock[2], BB_LEN);

								checksum = bblock[1] = bblock[0];

			                                        for (i=2; i<(((BB_LEN+3)>>2)+2); i++)
                        			                {
			                                            bblock[1] += bblock[i];
                        			                    if (bblock[1] < checksum)
			                                            {
                        			                        ++bblock[1];
			                                            }
			                                            checksum = bblock[1];
                        			                }
			                                        bblock[1] = ~bblock[1];

							}
retrycc0:
							trywrite = TRUE;

							if(cc0)
							{
								trywrite = FALSE;

								message = "PROGRAM PART";
								if(verify) message = "VERIFY PART";

								if(PromptForInsert(cv,message))
								{
									trywrite = TRUE;

								}
							}

							while(trywrite)
							{
								trywrite = FALSE;

								if(CardProgramVoltage(&cv->cv_CardHandle,Vpp))
								{
									Stabilize1MS(&cv->cv_ZoneHandle);

									if(verify)
									{
										AbortOnOff(cv,TRUE);
										message = "Verifing ....";
									}
									else
									{

										AbortOnOff(cv,FALSE);
										message = "Writing .....";

									}

									StatusBox(cv,0,1,TRUE,STAT_BOX_ID,message);

									cv->cv_ZoneHandle.zh_size = nsize;

									if(!verify)
									{
										if(WriteZone(&cv->cv_ZoneHandle))
										{
											AbortOnOff(cv,TRUE);

											offset += nsize;
											size -= nsize;
											cv->cv_ZoneHandle.zh_to = (APTR)((ULONG)cv->cv_ZoneHandle.zh_to + nsize);

											/* do not prompt for disk again if not CC0 */

											if(!cc0)
											{
												tryread=TRUE;
											}
											else
											{
												waitfordisk = TRUE;
											}

											if(size == 0)	/* done? */
											{
												if(cv->cv_SourceIndex <= 2)
												{
													DoWriteCIS(cv,dg);
												}

												tryread = FALSE;
												waitfordisk = FALSE;
												completed = TRUE;
											}

										}
										else
										{
											AbortOnOff(cv,TRUE);

											if(cv->cv_IsInserted)
											{
												if(cv->cv_IsRemoved == FALSE)
												{
													if(DisplayQuery(cv,"ERROR: Error Writing drive data.  Retry?"))
													{
														trywrite = TRUE;
													}								
												}
												else
												{
													goto retrycc0;
												}
											}
											else
											{
												goto retrycc0;
											}
										}
									}
									else
									{
										if(VerifyWrite(&cv->cv_ZoneHandle))
										{

											offset += nsize;
											size -= nsize;
											cv->cv_ZoneHandle.zh_to = (APTR)((ULONG)cv->cv_ZoneHandle.zh_to + nsize);

											/* do not prompt for disk again if not CC0 */

										
											if(!cc0)
											{
												tryread=TRUE;
											}
											else
											{
												waitfordisk = TRUE;
											}

											if(size == 0)	/* done? */
											{
												tryread = FALSE;
												waitfordisk = FALSE;
												completed = TRUE;
											}

										}
										else
										{
											if(cv->cv_IsInserted)
											{
												if(cv->cv_IsRemoved == FALSE)
												{
													DisplayError(cv,"ERROR: Data does not match");
												}
												else
												{
													goto retrycc0;
												}
											}
											else
											{
												goto retrycc0;
											}

										}
									}

									CardProgramVoltage(&cv->cv_CardHandle,CARD_VOLTAGE_0V);
									Stabilize1MS(&cv->cv_ZoneHandle);
								}									
								else
								{
									if(DisplayQuery(cv,"ERROR: Unable to access FlashROM.  Retry?"))
									{
										goto retrycc0;
									}
								}
							}

						}
						else
						{
							ior->iotd_Req.io_Command = TD_MOTOR;
							ior->iotd_Req.io_Length = 0;
							DoIO((struct IOStdReq *)ior);


							if(DisplayQuery(cv,"ERROR: Error Reading drive data.  Retry?"))
							{
								tryread = TRUE;
							}								
						}
						firstblock = FALSE;
					}
				}
			}

			AbortOnOff(cv,TRUE);

			if(nodisk)
			{
				if(DisplayQuery(cv,"ERROR: No disk in drive.  Retry?"))
				{
					waitfordisk = TRUE;
				}
			}
		}
	}
	return(completed);
}


struct IOExtTD *OpenDrive(struct cmdVars *cv, STRPTR str)
{
struct MsgPort *port;
struct IOExtTD *ior;

	if(port = CreateMsgPort())
	{
		if(ior = (struct IOExtTD *)CreateIORequest(port,(ULONG)sizeof(struct IOExtTD)))
		{
			if(!(OpenDevice(str,0L,(struct IOStdReq *)ior,0L)))
			{
				return(ior);
			}
			DeleteIORequest((struct IOStdReq *)ior);
		}
		DeleteMsgPort(port);
	}

	AbortOnOff(cv,TRUE);
	DisplayError(cv,"ERROR: Unable to open SOURCE device");
	return(NULL);

}

void CloseDrive(struct cmdVars *cv,struct IOExtTD *ior)
{

struct MsgPort *port;
	port = ior->iotd_Req.io_Message.mn_ReplyPort;

	CloseDevice((struct IOStdReq *)ior);
	DeleteIORequest((struct IOStdReq *)ior);
	DeleteMsgPort(port);
}


void DoTrackDisk( struct cmdVars *cmv )
{
register struct cmdVars *cv;
struct IOExtTD *ior;

	cv = cmv;

	if(ior = OpenDrive(cv,"trackdisk.device"))
	{

		if(DoDiskCopy(cv,NULL,ior,"INSERT FLOPPY IN DRIVE 0",FALSE,FALSE,TRUE))
		{
			if(cv->cv_VerifyOn)
			{
				DoDiskCopy(cv,NULL,ior,"INSERT FLOPPY IN DRIVE 0",FALSE,TRUE,FALSE);
			}
		}
		CloseDrive(cv,ior);
	}
}

void DoCardDisk( struct cmdVars *cmv )
{
register struct cmdVars *cv;
struct IOExtTD *ior;

	cv = cmv;

	if(ior = OpenDrive(cv,"carddisk.device"))
	{

		if(DoDiskCopy(cv,NULL,ior,"INSERT DISK-LIKE CARD IN SLOT",TRUE,FALSE,TRUE))
		{
			if(cv->cv_VerifyOn)
			{
				DoDiskCopy(cv,NULL,ior,"INSERT DISK-LIKE CARD IN SLOT",TRUE,TRUE,TRUE);
			}
		}
		CloseDrive(cv,ior);
	}
}

void DoRamDisk( struct cmdVars *cmv )
{
register struct cmdVars *cv;
struct IOExtTD *ior;
struct DriveGeometry dg;
UBYTE buf[524];
struct DosList *dl;
struct FileSysStartupMsg *fsm;
UBYTE *fname;
BOOL scanning;
struct DosEnvec *de;
ULONG bsize,sectrk,trkcyl,cyls;

	cv = cmv;

	if(ior = OpenDrive(cv,"ramdrive.device"))
	{
		/* figure out drive geometry the slow way */

		ior->iotd_Req.io_Data = (APTR)buf;
		ior->iotd_Req.io_Offset = 0L;
		ior->iotd_Req.io_Command = CMD_READ;
		ior->iotd_Req.io_Length = 512L;

		if(!(DoIO((struct IOStdReq *)ior)))
		{
			dl = LockDosList(LDF_DEVICES|LDF_READ);

			fsm = NULL;
			scanning = TRUE;

			while(scanning)
			{
				if(dl = NextDosEntry(dl,LDF_DEVICES|LDF_READ))
				{
					if(dl->dol_misc.dol_handler.dol_Startup > 64L)
					{
						fsm = (struct FileSysStartupMsg *)(((ULONG)dl->dol_misc.dol_handler.dol_Startup) << 2);
 
						if(fsm->fssm_Unit == 0)
						{
							fname = (UBYTE *)(((ULONG)fsm->fssm_Device) << 2);
							fname++;
 
							if(!(strcmp(fname,"ramdrive.device")))
							{
								scanning = FALSE;
							}
							else
							{
								fsm = NULL;
							}
						}
					}
				}
				else
				{
					scanning = FALSE;
				}
			}

			UnLockDosList(LDF_DEVICES|LDF_READ);

			if(fsm)
			{
				de = (struct DosEnvec *)(((ULONG)fsm->fssm_Environ) << 2);
				if(de->de_TableSize >= DE_UPPERCYL)
				{

					bsize = de->de_SizeBlock << 2;
					sectrk = de->de_BlocksPerTrack;
					trkcyl = de->de_Surfaces;
					cyls = (de->de_HighCyl - de->de_LowCyl) + 1;

					dg.dg_SectorSize = bsize;
					dg.dg_TotalSectors = cyls * trkcyl * sectrk;
					dg.dg_Cylinders = cyls;
					dg.dg_CylSectors = trkcyl * sectrk;
					dg.dg_Heads = trkcyl;
					dg.dg_TrackSectors = sectrk;

					D(("bsize = %ld cyls = %ld trkcyl = %ld sectrk = %ld\n",bsize,cyls,trkcyl,sectrk));

					if(DoDiskCopy(cv,&dg,ior,"COPY FROM RAMDRIVE?",FALSE,FALSE,FALSE))
					{
						if(cv->cv_VerifyOn)
						{
							DoDiskCopy(cv,&dg,ior,"COPY FROM RAMDRIVE?",FALSE,TRUE,FALSE);
						}
					}


				}
				else
				{
					DisplayError(cv,"ERROR: Invalid mount list");
				}
			}
			else
			{
				DisplayError(cv,"ERROR: No mount list entry");
			}
		}
		else
		{
			DisplayError(cv,"ERROR: Error reading from ramdrive");
		}

		CloseDrive(cv,ior);
	}

}


BOOL DoFileCopy( struct cmdVars *cmv, LONG file,
	STRPTR prompt, BOOL verify, BOOL doprompt )
{
register struct cmdVars *cv;
ULONG total,zone,temp,size,nsize,offset;
BOOL	tryread,trywrite,completed,waitfordisk;
BOOL	firstblock;
ULONG	Vpp;
UBYTE   *message, *ubp;
ULONG	*bblock;
LONG 	rlen;

	if(!file)	return(FALSE);

	cv = cmv;

	cv->cv_ZoneHandle.zh_to = cv->cv_CardMemMap->cmm_CommonMemory;

	total = totalsizes[cv->cv_TotalIndex];
	zone = zonesizes[cv->cv_ZoneIndex];

	temp = total;
	
	if(zone < (256*1024)) zone = 256*1024;

#ifdef CCC
	if(cc0)
		{
		zone = cv->cv_ZoneBufSize;
		}
#endif

	cv->cv_ZoneHandle.zh_size = zone;

	completed = FALSE;
	firstblock = TRUE;

	offset = 0L;

	Vpp = CARD_VOLTAGE_12V;

	if(verify) Vpp = CARD_VOLTAGE_0V;

	doprompt = TRUE;		/* always ask from now on */

	AbortOnOff(cv,TRUE);

	ClearBox(cv,STAT_BOX_ID);

	Seek(file,0,OFFSET_END);
	size = Seek(file,0,OFFSET_BEGINNING);

	if(size > temp)
		{
		AbortOnOff(cv,TRUE);
		DisplayError(cv,"ERROR: File is larger than FlashROM");
		return(FALSE);
		}


	tryread = TRUE;

	while(tryread)
		{
		/* read from source into zone buffer */
		tryread = FALSE;

		AbortOnOff(cv,TRUE);


		bblock = (ULONG *)cv->cv_ZoneHandle.zh_from;

		if(size >= zone)
			{
			nsize = zone;
			}
		else
			{
			nsize = size;
			}

		Seek(file,offset,OFFSET_BEGINNING);
		StatusBox(cv,0,1,TRUE,STAT_BOX_ID,"Reading .....");

		rlen = Read(file, bblock, nsize);

		if(firstblock)
			{
			ubp = (UBYTE *)bblock;
		     	/* if "loadc" device tuple in file, change it */
		     	if((ubp[0] == 0x13)&&
		          (ubp[1] == 0x03)&&
		          (ubp[2] == 'C')&&
		          (ubp[3] == 'I')&&
		          (ubp[4] == 'S')&&
		          (ubp[5] == 0x01)&&
		          (ubp[6] == 0x02)&&
		          (ubp[7] == 0x61))
				{
				ClearBox(cv,STAT_BOX_ID);
				StatusBox(cv,0,1,TRUE,STAT_BOX_ID,
					"Changing device tuple to Flashrom");
				Delay(80);
				ClearBox(cv,STAT_BOX_ID);
				ubp[7] = devicetypes[cv->cv_SpeedIndex];
				ubp[8] = devicesizes[cv->cv_TotalIndex];
				}
			else if((ubp[0] == 0x01)&&
				(ubp[2] == 0x02)&&
				(ubp[4] == 0x61))
				{
				ubp[4] = devicetypes[cv->cv_SpeedIndex];
				ubp[6] = devicesizes[cv->cv_TotalIndex];
				ClearBox(cv,STAT_BOX_ID);
				StatusBox(cv,0,1,TRUE,STAT_BOX_ID,
					"Changing device tuple to Flashrom");
				Delay(80);
				ClearBox(cv,STAT_BOX_ID);
				}				
			}

retrycc0:

		trywrite = TRUE;

		while(trywrite)
			{
			trywrite = FALSE;
			if(CardProgramVoltage(&cv->cv_CardHandle,Vpp))
				{
				Stabilize1MS(&cv->cv_ZoneHandle);

				if(verify)
					{
					AbortOnOff(cv,TRUE);
					message = "Verifing ....";
					}
				else
					{
					AbortOnOff(cv,FALSE);
					message = "Writing .....";
					}

				StatusBox(cv,0,1,TRUE,STAT_BOX_ID,message);

				cv->cv_ZoneHandle.zh_size = nsize;

				if(!verify)
					{
					if(WriteZone(&cv->cv_ZoneHandle))
						{
						AbortOnOff(cv,TRUE);
						offset += nsize;
						size -= nsize;
						cv->cv_ZoneHandle.zh_to = (APTR)((ULONG)cv->cv_ZoneHandle.zh_to + nsize);
						/* do not prompt for disk again if not CC0 */

						tryread = TRUE;

						if(size == 0)	/* done? */
							{
							tryread = FALSE;
							waitfordisk = FALSE;
							completed = TRUE;
							}
						}
					else
						{
						AbortOnOff(cv,TRUE);
						if(cv->cv_IsInserted)
							{
							if(cv->cv_IsRemoved == FALSE)
								{
								if(DisplayQuery(cv,"ERROR: Error Writing drive data.  Retry?"))
									{
									trywrite = TRUE;
									}								
								}
							else
								{
								goto retrycc0;
								}
							}
						else
							{
							goto retrycc0;
							}
						}
					}
				else
					{
					if(VerifyWrite(&cv->cv_ZoneHandle))
						{
						offset += nsize;
						size -= nsize;
						cv->cv_ZoneHandle.zh_to = (APTR)((ULONG)cv->cv_ZoneHandle.zh_to + nsize);
						/* do not prompt for disk again if not CC0 */

						tryread = TRUE;

						if(size == 0)	/* done? */
							{
							tryread = FALSE;
							waitfordisk = FALSE;
							completed = TRUE;
							}
						}
					else
						{
						if(cv->cv_IsInserted)
							{
							if(cv->cv_IsRemoved == FALSE)
								{
								DisplayError(cv,"ERROR: Data does not match");
								}
							else
								{
								goto retrycc0;
								}
							}
						else
							{
							goto retrycc0;
							}
						}
					}
				CardProgramVoltage(&cv->cv_CardHandle,CARD_VOLTAGE_0V);
				Stabilize1MS(&cv->cv_ZoneHandle);
				}				
			else
				{
				if(DisplayQuery(cv,"ERROR: Unable to access FlashROM.  Retry?"))
					{
					goto retrycc0;
					}
				}
			}

		firstblock = FALSE;
		}
	AbortOnOff(cv,TRUE);
	return(completed);
}



BOOL DoFromFile( struct cmdVars *cmv )
{
register struct cmdVars *cv;
UBYTE *filename;
LONG file = 0, error = 0;

	cv = cmv;

	if(filename = getfilename(cv, "Source binary image file",cv->cv_Filename,
		FILENAME_SIZEOF, NULL, FREQ_LOAD))
		{
	    	if(file = Open(filename,MODE_OLDFILE))
			{
			if(DoFileCopy(cv, file,
				    "INSERT FLOPPY IN DRIVE 0", FALSE, FALSE))
				{
				if(cv->cv_VerifyOn)
					{
					DoFileCopy(cv, file,
				    	 "INSERT FLOPPY IN DRIVE 0", TRUE, FALSE);
					}
				}
			else error = 1;
	    		Close(file);
			}
		else (error=1);
	    	}
	else (error=1);

	if(error) DisplayError(cv,"ERROR: Binary-File load\nNo source file");
	return(error ? FALSE : TRUE);
}


void DoProgram( struct cmdVars *cmv )
{
register struct cmdVars *cv;
register ULONG total,zone,temp;
ULONG avail;
BOOL failed;

#ifdef IFCACHECONTROL
ULONG oldcachebits;
#endif
UBYTE info[80];


	cv = cmv;

	total = totalsizes[cv->cv_TotalIndex];
	zone = zonesizes[cv->cv_ZoneIndex];
	temp = total / zone;

	avail = AvailMem(MEMF_PUBLIC|MEMF_LARGEST);
	
	cv->cv_ZoneBufSize = zone;

	if(zone < (256*1024)) cv->cv_ZoneBufSize = 256*1024;

	if(zone < avail)
	{
		if(avail >= (1*1024*1024))
		{
			cv->cv_ZoneBufSize = (1*1024*1024);
		}
		else
		{
			if(avail >= (512*1024))
			{
				cv->cv_ZoneBufSize = (512*1024);
			}
		}

	}

	failed = FALSE;

	D(("Buffer size = %ld\n",cv->cv_ZoneBufSize));

	if(PromptForInsert(cv,"PROGRAM PART"))
	{
		ClearBox(cv,STAT_BOX_ID);

		if(cv->cv_ZoneHandle.zh_from = AllocMem(cv->cv_ZoneBufSize,MEMF_PUBLIC|MEMF_CLEAR))
		{
			cv->cv_ZoneHandle.zh_to = cv->cv_CardMemMap->cmm_CommonMemory;
			cv->cv_ZoneHandle.zh_size = zone;


#ifdef	IFCACHECONTROL
	/* sorry about this, but I need the data cache turned off, and do not
	  intend to protect in Forbid() -- too long
	*/

			oldcachebits = (CacheControl(0L,CACRF_EnableD) & CACRF_EnableD);
#endif

			if(CardProgramVoltage(&cv->cv_CardHandle,CARD_VOLTAGE_12V))
			{
				Stabilize1MS(&cv->cv_ZoneHandle);

				total = 0;
				while(total < temp)
				{
					DoSprintF(info,"Check Blank    Zone: %3ld of %3ld",(total+1L),temp);

					StatusBox(cv,0,1,TRUE,STAT_BOX_ID,info);

					if(!(CheckBlank(&cv->cv_ZoneHandle)))
					{

						if(cv->cv_EraseOn)
						{
							DoSprintF(info,"Erase Pass: 1  Zone: %3ld of %3ld",(total+1L),temp);

							StatusBox(cv,0,1,TRUE,STAT_BOX_ID,info);

							if(WriteZone(&cv->cv_ZoneHandle))
							{
								DoSprintF(info,"Erase Pass: 2  Zone: %3ld of %3ld",(total+1L),temp);

								StatusBox(cv,0,1,TRUE,STAT_BOX_ID,info);

								if(!(EraseZone(&cv->cv_ZoneHandle)))
								{
									DisplayError(cv,"ERROR: Operation failed\nErase failed");
									failed = TRUE;
									total = temp;
								}
							}
							else
							{
								DisplayError(cv,"ERROR: Operation failed\nUnable to erase");
								failed = TRUE;
								total = temp;
							}
						}
						else
						{
							if(!(DisplayQuery(cv,"WARNING: FlashROM is not blank")))
							{
								failed = TRUE;
							}
							total = temp;
						}
					}
					cv->cv_ZoneHandle.zh_to = (APTR)((ULONG)cv->cv_ZoneHandle.zh_to + zone);
					total++;
				}


				CardProgramVoltage(&cv->cv_CardHandle,CARD_VOLTAGE_0V);
				Stabilize1MS(&cv->cv_ZoneHandle);

				if(!(failed))
				{
					switch(cv->cv_SourceIndex)
					{
						case INPUT_TRACKDISK:
							DoTrackDisk(cv);
							break;

						case INPUT_CARDDISK:
							DoCardDisk(cv);
							break;

						case INPUT_RAMDRIVE:
							DoRamDisk(cv);
							break;

						case INPUT_FILE:
							DoFromFile(cv);
							break;

						default:
							break;
					}					

				}

			}
			else
			{
				DisplayError(cv,"ERROR: Unable to access card");
			}

#ifdef	IFCACHECONTROL
			CacheControl(1L,oldcachebits);
#endif

			FreeMem(cv->cv_ZoneHandle.zh_from,cv->cv_ZoneBufSize);
		}
		else
		{
			DisplayError(cv,"ERROR: Insufficient Memory");
		}


	}
}
