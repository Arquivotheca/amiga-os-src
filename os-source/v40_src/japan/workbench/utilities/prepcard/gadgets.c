#include "prepcard.h"

/* Advanced settings */

static struct GadGadget far GA[] = {
	{CYCLE_KIND,    160, 24,   180, 20, CM_GADGET_TYPE,  MSG_PREP_TYPE_GAD,  0},
	{CYCLE_KIND,    160, 45,   180, 20, CM_GADGET_SPEED, MSG_PREP_SPEED_GAD, 0},
	{CYCLE_KIND,    160, 66,   180, 20, CM_GADGET_SIZE,  MSG_PREP_UNITS_GAD, 0},
	{SLIDER_KIND,	184, 89,   156, 12, CM_GADGET_UNITS, MSG_PREP_UNNUM_GAD, 0},
	{TEXT_KIND,	160, 105,  180, 20, CM_DEFAULT,      MSG_PREP_SIZE_GAD,  0},
	{CYCLE_KIND,    160, 147,  180, 20, CM_GADGET_EDC,   MSG_PREP_EDC_GAD,   0},
	{CYCLE_KIND,    160, 168,  180, 20, CM_GADGET_BSIZE, MSG_PREP_BKSZ_GAD,  0},
	{TEXT_KIND,	160, 190,  180, 20, CM_DEFAULT,      MSG_PREP_MAXB_GAD,  0},
	{INTEGER_KIND,	500, 24,   96,  20, CM_GADGET_TSEC,  MSG_PREP_MAXE_GAD,  0},
	{INTEGER_KIND,	500, 45,   96,  20, CM_GADGET_SECT,  MSG_PREP_SECE_GAD,  0},
	{INTEGER_KIND,	500, 66,   96,  20, CM_GADGET_TRKC,  MSG_PREP_TRKE_GAD,  0},
	{INTEGER_KIND,	500, 87,   96,  20, CM_GADGET_CYLS,  MSG_PREP_CYLE_GAD,  0},
	{CHECKBOX_KIND,	500, 105,  32,  20, CM_GADGET_GEO,   MSG_PREP_GEOE_GAD,  0},
	{TEXT_KIND,	160, 4,    180, 20, CM_DEFAULT,      MSG_PREP_DINFO_GAD, PLACETEXT_IN|NG_HIGHLABEL},
	{TEXT_KIND,	160, 126,  180, 20, CM_DEFAULT,      MSG_PREP_FINFO_GAD, PLACETEXT_IN|NG_HIGHLABEL},
	{TEXT_KIND,	500, 4,    96,  20, CM_DEFAULT,      MSG_PREP_GINFO_GAD, PLACETEXT_IN|NG_HIGHLABEL},
	{BUTTON_KIND,	432, 217,  180, 20, CM_GADGET_RESET, MSG_PREP_CANCEL_GAD, 0 },
	{BUTTON_KIND,	8,   217,  180, 20, CM_GADGET_CONT,  MSG_PREP_CONTINUE_GAD, 0 },


};

/* Basic gadgets */

static struct GadGadget far GG[] = {
	{BUTTON_KIND,   8,   235, 180, 20, CM_MAKEDISK, MSG_PREP_FORMAT_GAD, 0},
	{BUTTON_KIND,   218, 235, 180, 20, CM_MAKERAM,  MSG_PREP_RAM_GAD,    0},
	{BUTTON_KIND,   432, 235, 180, 20, CM_QUIT,     MSG_PREP_QUIT_MENU,  0},
};

static struct TextAttr far topaz8 = {
	"coral.font",16,FS_NORMAL,FPF_DISKFONT
};

static ULONG usizes[] = {
	512,2*1024,8*1024,32*1024,128*1024,512*1024,2*1024*1024,0,
};
	
struct Gadget *CreateAGadget( struct cmdVars *cmv, struct Gadget **GADS, struct GadGadget *gg, ULONG tags, ... )
{
register struct cmdVars *cv;
struct NewGadget ng;

	cv = cmv;

	ng.ng_LeftEdge   = gg->gg_LeftEdge + cv->cv_sp->WBorLeft;
	ng.ng_TopEdge    = gg->gg_TopEdge + cv->cv_sp->WBorTop + cv->cv_sp->Font->ta_YSize + 1;
	ng.ng_Width	 = gg->gg_Width;
	ng.ng_Height	 = gg->gg_Height;
	ng.ng_GadgetText = GetString(cv,gg->gg_Label);
	ng.ng_TextAttr   = &topaz8;
	ng.ng_GadgetID   = 0;
	ng.ng_Flags      = gg->gg_GadgetFlags;
	ng.ng_VisualInfo = cv->cv_VI;
	ng.ng_UserData   = (APTR)gg->gg_Command;

	return(*GADS = CreateGadgetA(gg->gg_GadgetKind,*GADS,
				&ng,(struct TagItem *)&tags));

}

VOID ChangeBegAttrs(struct cmdVars *cv, struct Gadget *gad, ULONG tags, ... )
{
	GT_SetGadgetAttrsA(gad, cv->cv_win , NULL, (struct TagList *)&tags );

}
/* enable/disable basic gadgets */

VOID DisabledPrep( struct cmdVars *cv, BOOL disable )
{
	ChangeBegAttrs(cv,cv->cv_FormatGad,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

	ChangeBegAttrs(cv,cv->cv_RamGad,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

}

/* change attrs of an advanced settings gadget */

VOID ChangeAdvAttrs(struct cmdVars *cv, struct Gadget *gad, ULONG tags, ... )
{
	GT_SetGadgetAttrsA(gad, cv->cv_awin , NULL, (struct TagList *)&tags );

}
/* Make basic gadgets */

BOOL MakeGadgets( struct cmdVars *cmv )
{
register struct cmdVars *cv;
register struct Gadget **GADS;


	cv=cmv;


	GADS = &cv->cv_GADS;


	cv->cv_gadgets = CreateContext(GADS);


	cv->cv_FormatGad = CreateAGadget(cv,GADS,&GG[0],
		TAG_DONE);

	cv->cv_RamGad = CreateAGadget(cv,GADS,&GG[1],
		TAG_DONE);

	cv->cv_QuitGad = CreateAGadget(cv,GADS,&GG[2],
		TAG_DONE);

	if(cv->cv_QuitGad)
	{
		return(TRUE);
	}

	FreeGadgets(cv->cv_gadgets);
	return(FALSE);

}

/* Make advanced gadgets */

BOOL MakeAdvGadgets( struct cmdVars *cmv )
{
register struct cmdVars *cv;
register struct Gadget **GADS;

	cv=cmv;

	GADS = &cv->cv_advGADS;

	cv->cv_advgadgets = 	CreateContext(GADS);

	cv->cv_TypeGad = 	CreateAGadget(cv,GADS,&GA[0],
		GTCY_Labels,	(STRPTR *)&cv->cv_Types[0],
		GTCY_Active,	(UWORD)cv->cv_DefaultType,
		TAG_DONE);

	cv->cv_SpeedGad =	CreateAGadget(cv,GADS,&GA[1],
		GTCY_Labels,	(STRPTR *)&cv->cv_Speeds[0],
		GTCY_Active,	(UWORD)cv->cv_DefaultSpeed,
		TAG_DONE);

	cv->cv_UnitsGad =	CreateAGadget(cv,GADS,&GA[2],
		GTCY_Labels,	(STRPTR *)&cv->cv_Units[0],
		GTCY_Active,	(UWORD)cv->cv_DefaultUnits,
		TAG_DONE);

	cv->cv_UnitNumGad =	CreateAGadget(cv,GADS,&GA[3],
		GTSL_Min,	(UWORD)1,
		GTSL_Max,	(UWORD)32,
		GTSL_Level,	(UWORD)1,
		GTSL_MaxLevelLen, (UWORD)2,
		GTSL_LevelFormat, (STRPTR)"%2ld",
		GTSL_LevelPlace, PLACETEXT_LEFT,
		GA_Disabled,	(BOOL)TRUE,
		TAG_DONE);

	cv->cv_TotalSizeGad =	CreateAGadget(cv,GADS,&GA[4],
		GTTX_Border,	(BOOL)TRUE,
		TAG_DONE);

	cv->cv_EDCGad =		CreateAGadget(cv,GADS,&GA[5],
		GTCY_Labels,	(STRPTR *)&cv->cv_EDC[0],
		GTCY_Active,	(UWORD)cv->cv_DefaultEDC,
		TAG_DONE);

	cv->cv_BKSZGad =	CreateAGadget(cv,GADS,&GA[6],
		GTCY_Labels,	(STRPTR *)&cv->cv_BKSZ[0],
		GTCY_Active,	(UWORD)cv->cv_DefaultBKSZ,
		TAG_DONE);

	cv->cv_MaxBlocksGad =	CreateAGadget(cv,GADS,&GA[7],
		GTTX_Border,	(BOOL)TRUE,
		TAG_DONE);

	cv->cv_EnterBlkGad =	CreateAGadget(cv,GADS,&GA[8],
		GA_TabCycle,	(BOOL)TRUE,
		GA_Disabled,	(BOOL)TRUE,
		GTIN_MaxChars,	(UWORD)5,
		TAG_DONE);

	cv->cv_EnterSecGad =	CreateAGadget(cv,GADS,&GA[9],
		GA_TabCycle,	(BOOL)TRUE,
		GA_Disabled,	(BOOL)TRUE,
		GTIN_MaxChars,	(UWORD)3,
		TAG_DONE);

	cv->cv_EnterTrkGad =	CreateAGadget(cv,GADS,&GA[10],
		GA_TabCycle,	(BOOL)TRUE,
		GA_Disabled,	(BOOL)TRUE,
		GTIN_MaxChars,	(UWORD)3,
		TAG_DONE);

	cv->cv_EnterCylGad =	CreateAGadget(cv,GADS,&GA[11],
		GA_TabCycle,	(BOOL)TRUE,
		GA_Disabled,	(BOOL)TRUE,
		GTIN_MaxChars,	(UWORD)5,
		TAG_DONE);

	cv->cv_EnableGeoGad =	CreateAGadget(cv,GADS,&GA[12],
		GTCB_Checked,	(BOOL)cv->cv_DefaultGEO,
		TAG_DONE);

	cv->cv_DevLabGad =	CreateAGadget(cv,GADS,&GA[13],
		TAG_DONE);

	cv->cv_DiskLabGad =	CreateAGadget(cv,GADS,&GA[14],
		TAG_DONE);

	cv->cv_GeoLabGad =	CreateAGadget(cv,GADS,&GA[15],
		TAG_DONE);

	cv->cv_AdvCanGad =	CreateAGadget(cv,GADS,&GA[16],
		TAG_DONE);

	cv->cv_AdvConGad =	CreateAGadget(cv,GADS,&GA[17],
		TAG_DONE);

	if(cv->cv_AdvConGad)
	{
		return(TRUE);
	}
	FreeGadgets(cv->cv_advgadgets);
	return(FALSE);

}

/* Make advanced display */

VOID MakeAdvDisplay( struct cmdVars *cmv )
{
register struct cmdVars *cv;

	cv = cmv;

	if(!cv->cv_IsAdvanced)
	{
		if(MakeAdvGadgets( cv ))
		{
			if(MakeAdvWindow( cv ))
			{

				cv->cv_IsAdvanced = TRUE;
				DisplayUnits( cv );
			}

		}

		if(!cv->cv_IsAdvanced)
		{
			FreeGadgets(cv->cv_advgadgets);
			DisplayError(cv,MSG_PREP_NO_MEM_ERROR);
		}
	}
	else
	{
		WindowToFront(cv->cv_awin);
		ActivateWindow(cv->cv_awin);
	}
}

/* change in an advanced gadget */

VOID NewGadget( struct cmdVars *cmv, struct Gadget *gad, UWORD code )
{
register struct cmdVars *cv;

	cv = cmv;

	switch((ULONG)gad->UserData)
	{
		case CM_GADGET_TYPE:
			cv->cv_DefaultType = code;
			break;

		case CM_GADGET_SPEED:
			cv->cv_DefaultSpeed = code;
			break;

		case CM_GADGET_SIZE:
			cv->cv_DefaultUnits = code;
			DisplayUnits( cv );
			break;

		case CM_GADGET_UNITS:
			cv->cv_DefaultUnitNum = code;
			DisplayUnits( cv );
			break;

		case CM_GADGET_RESET:
			CloseAdvWindow(cv);
			Defaults( cv );
			break;

		case CM_GADGET_CONT:
			WindowToFront(cv->cv_win);
			ActivateWindow(cv->cv_win);
			break;

		case CM_GADGET_EDC:
			cv->cv_DefaultEDC = code;
			DisplayMaxBlocks( cv );
			break;

		case CM_GADGET_BSIZE:
			cv->cv_DefaultBKSZ = code;
			DisplayMaxBlocks( cv );
			break;

		case CM_GADGET_GEO:
			cv->cv_DefaultGEO = cv->cv_EnableGeoGad->Flags & GFLG_SELECTED;
			DisplayGeoStuff( cv, 0 );
			break;

		case CM_GADGET_TSEC:
			cv->cv_GeoBlocks = ((struct StringInfo *)cv->cv_EnterBlkGad->SpecialInfo)->LongInt;
			DisplayGeoStuff( cv, 1 );
			break;

		case CM_GADGET_SECT:
			cv->cv_GeoSecs = ((struct StringInfo *)cv->cv_EnterSecGad->SpecialInfo)->LongInt;
			DisplayGeoStuff( cv, 2 );
			break;

		case CM_GADGET_TRKC:
			cv->cv_GeoTrks = ((struct StringInfo *)cv->cv_EnterTrkGad->SpecialInfo)->LongInt;
			DisplayGeoStuff( cv, 3 );
			break;

		case CM_GADGET_CYLS:
			cv->cv_GeoCyls = ((struct StringInfo *)cv->cv_EnterCylGad->SpecialInfo)->LongInt;
			DisplayGeoStuff( cv, 4 );
			break;

		default: break;

	}

}

#define ROLLOVER 9999

VOID DisplayUnits( struct cmdVars *cmv )
{
register struct cmdVars *cv;
ULONG len;
UBYTE *total;

	cv = cmv;

/* remove text string */

	ChangeAdvAttrs(cv,cv->cv_TotalSizeGad,GTTX_Text,NULL,TAG_DONE);

	cv->cv_TotalSize = 0L;

	if(cv->cv_DefaultUnits == MSG_PREP_UNITS_UNKNOWN - MSG_PREP_UNITS_512)
	{
		ChangeAdvAttrs(cv, cv->cv_UnitNumGad, GA_Disabled, TRUE, TAG_DONE );
		cv->cv_GadUnitNumDis = TRUE;
	}
	else
	{

		len = (ULONG)(usizes[cv->cv_DefaultUnits] * cv->cv_DefaultUnitNum);

		cv->cv_TotalSize = len;

		total = "";
		if(len >= 16*1024)
		{
			total = "K";
			if( (len >>= 10) > ROLLOVER)
			{
				len >>= 10;
        	               	total = "M";
			}
		}

		DoSPrintF(&cv->cv_TotalSizeText[0],
			GetString(cv,MSG_PREP_TOTAL_SIZE),len,total);
				
		ChangeAdvAttrs(cv,cv->cv_TotalSizeGad,GTTX_Text,&cv->cv_TotalSizeText[0],TAG_DONE);

		if(cv->cv_GadUnitNumDis)
		{
			ChangeAdvAttrs(cv, cv->cv_UnitNumGad, GA_Disabled, FALSE, TAG_DONE );
			cv->cv_GadUnitNumDis = FALSE;
		}
	}

	DisplayMaxBlocks( cv );

}

VOID DisplayMaxBlocks( struct cmdVars *cmv )
{
register struct cmdVars *cv;
UWORD blocksize;
UBYTE *str;
BOOL distotal;
ULONG maxsize;

	cv = cmv;

	ChangeAdvAttrs(cv,cv->cv_MaxBlocksGad,GTTX_Text,NULL,TAG_DONE);

	cv->cv_TotalBlocks = 0L;

	distotal = TRUE;

	if(cv->cv_TotalSize)
	{
		maxsize = cv->cv_TotalSize;
/*
 * Limits disk size to 4 megs, which is a worst case of 32K blocks if
 * block size is 128, and no error detection is used.  This would meet
 * the maximum address space allowed for Amiga machines, and avoid the
 * problem of trying to set-up a linear geometry for a card which can
 * hold more than 64K blocks (would require a card larger than 8 megs!)
 */
		if(maxsize > MAXDISKSIZE) maxsize = MAXDISKSIZE;

/* block size is a power of 2 [128...2048], so use index value to calc */

		blocksize = 1L << (cv->cv_DefaultBKSZ + 7);

/* plus # of bytes needed for EDC; doesn't matter if they are imbedded
 * or in a table -- same amount of space required either way.
 */

		blocksize += cv->cv_DefaultEDC;

/* calc totalblocks == totalsize minus 512 byte for CIS in common memory,
 * which leaves total amount of free space, and then divide whats left
 * by block size
 */

		cv->cv_TotalBlocks = (maxsize-512L)/blocksize;

/* minimum for TotalSize is 512, so worst case is 0/N, which equals
 * 0 as expected.
 */
		
/* set random number for minimum blocks - minimum is a random 31 blocks
 * based on a case of a 16K card with 512 byte blocks - 16K is the minimum
 * allowable address decoding allowed for the HOST system, and 31 blocks
 * provides for a reasonable amount of space for reserved blocks, block
 * mapping, and other basic file system needs.
 */
		if(cv->cv_TotalBlocks < MINDISKBLOCKS )
		{
			str = GetString(cv,MSG_PREP_TOTBK_SMALL);
		}
		else
		{
			distotal = FALSE;
			str = GetString(cv,MSG_PREP_TOTBK_FMT);
		}
	}
	else
	{

/* indicate auto calc */

		str = GetString(cv, MSG_PREP_TOTBK_AUTO);
	}

	DoSPrintF(&cv->cv_MaxBlocksText[0],str,cv->cv_TotalBlocks);

/* disable/enable geometry settings */

	ChangeAdvAttrs(cv, cv->cv_EnableGeoGad, GA_Disabled, distotal, TAG_DONE );
	
	cv->cv_GadGeoDis = distotal;

	ChangeAdvAttrs(cv,cv->cv_MaxBlocksGad,
		GTTX_Text, &cv->cv_MaxBlocksText[0],
		TAG_DONE);

/* Display GEO stuff */

	DisplayGeoStuff( cv, 0 );
}

/**
 ** Display GEOmetry information
 **/

VOID DisplayGeoStuff( struct cmdVars *cmv, UWORD code)
{
register struct cmdVars *cv;
BOOL disablegads;
LONG tblks,tcyls,tsecs, ttrks;
struct Gadget *active;
BOOL beep;

	cv = cmv;

	disablegads = TRUE;

	tblks = cv->cv_GeoBlocks;
	tsecs = cv->cv_GeoSecs;
	ttrks = cv->cv_GeoTrks;
	tcyls = cv->cv_GeoCyls;

	if(tblks > (LONG)cv->cv_TotalBlocks) tblks = cv->cv_TotalBlocks;
	if(tblks < (LONG)MINDISKBLOCKS) tblks = MINDISKBLOCKS;

	switch( code )
	{
		case 0:
/* change in GEOMETRY enable gadget, or device size */

			if(cv->cv_GadGeoDis)
			{
/* reset all gadgets if size too small, or undefined */

				tblks = 0L;
				tsecs = 0;
				ttrks = 0;
				tcyls = 0;
			}
			else
			{

/* start with default (linear) geometry */

				tblks = cv->cv_TotalBlocks;

				if(cv->cv_DefaultGEO)
				{
/* if Geometry check enabled, then also enable geometry gadgets */
					
					disablegads = FALSE;
					CalcBestGeometry(tblks,&tsecs,&ttrks,&tcyls,FALSE);
				}
				else
				{
/* Geometry check not enabled, so default to lineary geometry */

					CalcBestGeometry(tblks,&tsecs,&ttrks,&tcyls,TRUE);
			
				}
			}

			break;

/* change in Total Sectors */


		case 1:

/* change in Sectors/Track; adjust trks, and cyls */

		case 2:
			if(tsecs <= 0L) tsecs = 1L;
			if(tsecs > tblks) tsecs = tblks;
			if(tsecs > 255L) tsecs = 255L;

			if(tsecs * ttrks * tcyls > tblks)
			{
				ttrks = 1L;
				tcyls = tblks/tsecs;				
			}

/* change in Tracks/Cylinder; adjust cylinders, and maybe tracks */

		case 3:

			if(ttrks <= 0L) ttrks = 1L;
			if(ttrks > tblks) ttrks = tblks;
			if(ttrks > 255L) ttrks = 255L;

			while(tsecs * ttrks * tcyls > tblks)
			{
				tcyls--;
				if(tcyls <= 0L)
				{
					tcyls = 1L;
					ttrks--;
                                        if(ttrks <= 0L) ttrks = 1L;
					ttrks = tblks/(tsecs * tcyls);
				}
			}

/* change in Total Cylinders; adjust cylinders */

		case 4:

			if(tcyls <= 0L) tcyls = 1L;
			if(tcyls > tblks) tcyls = tblks;
			if(tcyls > 65535L) tcyls = 65535L;

			if(tsecs * ttrks * tcyls > tblks)
			{
				tcyls = tblks/(tsecs * ttrks);
			}


/* gadgets must be enabled because we just got a gadget message */

			disablegads = FALSE;


		default:

			break;

	}

/* determine if DisplayBeep is appropriate */

	beep = FALSE;

	switch(code)
	{
		case 1:
			if(tblks != cv->cv_GeoBlocks) beep = TRUE;
			break;
		case 2:
			if(tsecs != cv->cv_GeoSecs) beep = TRUE;
			break;
		case 3:
			if(ttrks != cv->cv_GeoTrks) beep = TRUE;
			break;
		case 4:
			if(tcyls != cv->cv_GeoCyls) beep = TRUE;

		default:
			break;
	}

	cv->cv_GeoBlocks = tblks;
	cv->cv_GeoSecs = tsecs;
	cv->cv_GeoTrks = ttrks;
	cv->cv_GeoCyls = tcyls;

	active = NULL;

	if(cv->cv_EnterBlkGad->Flags & GFLG_SELECTED) active = cv->cv_EnterBlkGad;
	if(cv->cv_EnterSecGad->Flags & GFLG_SELECTED) active = cv->cv_EnterSecGad;
	if(cv->cv_EnterTrkGad->Flags & GFLG_SELECTED) active = cv->cv_EnterTrkGad;
	if(cv->cv_EnterCylGad->Flags & GFLG_SELECTED) active = cv->cv_EnterCylGad;


	ChangeAdvAttrs(cv,cv->cv_EnterBlkGad,
		GA_Disabled,	disablegads,
		GTIN_Number,	cv->cv_GeoBlocks,
		TAG_DONE);

	ChangeAdvAttrs(cv,cv->cv_EnterSecGad,
		GA_Disabled,	disablegads,
		GTIN_Number,	cv->cv_GeoSecs,
		TAG_DONE);

	ChangeAdvAttrs(cv,cv->cv_EnterTrkGad,
		GA_Disabled,	disablegads,
		GTIN_Number,	cv->cv_GeoTrks,
		TAG_DONE);

	ChangeAdvAttrs(cv,cv->cv_EnterCylGad,
		GA_Disabled,	disablegads,
		GTIN_Number,	cv->cv_GeoCyls,
		TAG_DONE);


	if(beep) DisplayBeep(cv->cv_sp);

	if(active)
	{
		ActivateGadget(active,cv->cv_awin,NULL);
	}
}

VOID DoSPrintF(UBYTE *buf, STRPTR fmt, long arg1, ... )
{
	SPrintF(buf,fmt,&arg1);
}

/* Default gadget/settings for advanced display/formatting */

VOID Defaults( struct cmdVars *cmv )
{
register struct cmdVars *cv;
register ULONG x;

	cv = cmv;

/* initialize string pointers for cycle gadgets */

	for(x = MSG_PREP_CARD_SRAM; x < (MSG_PREP_CARD_DRAM + 1); x++)
	{
		cv->cv_Types[x-MSG_PREP_CARD_SRAM] = GetString(cv,x);
	}
	cv->cv_Types[x-MSG_PREP_CARD_SRAM] = NULL;

	for(x = MSG_PREP_SPEED_250NS; x < (MSG_PREP_SPEED_100NS + 1); x++)
	{
		cv->cv_Speeds[x-MSG_PREP_SPEED_250NS] = GetString(cv,x);
	}
	cv->cv_Speeds[x-MSG_PREP_SPEED_250NS] = NULL;

	for(x = MSG_PREP_UNITS_512; x < (MSG_PREP_UNITS_UNKNOWN + 1); x++)
	{
		cv->cv_Units[x-MSG_PREP_UNITS_512] = GetString(cv,x);
	}
	cv->cv_Units[x-MSG_PREP_UNITS_512] = NULL;

	for(x = MSG_PREP_EDCC_NONE; x < (MSG_PREP_EDCC_CRC + 1); x++)
	{
		cv->cv_EDC[x-MSG_PREP_EDCC_NONE] = GetString(cv,x);
	}
	cv->cv_EDC[x-MSG_PREP_EDCC_NONE] = NULL;

	for(x = MSG_PREP_BKSZ_128; x < (MSG_PREP_BKSZ_2048 + 1); x++)
	{
		cv->cv_BKSZ[x-MSG_PREP_BKSZ_128] = GetString(cv,x);
	}
	cv->cv_BKSZ[x-MSG_PREP_BKSZ_128] = NULL;

/* and defaults for these choices */

	cv->cv_DefaultType = (MSG_PREP_CARD_SRAM - MSG_PREP_CARD_SRAM);
	cv->cv_DefaultSpeed = (MSG_PREP_SPEED_250NS - MSG_PREP_SPEED_250NS);
	cv->cv_DefaultUnits = (MSG_PREP_UNITS_UNKNOWN - MSG_PREP_UNITS_512);
	cv->cv_DefaultUnitNum = 1;
	cv->cv_DefaultEDC = (MSG_PREP_EDCC_NONE - MSG_PREP_EDCC_NONE);
	cv->cv_DefaultBKSZ = (MSG_PREP_BKSZ_512 - MSG_PREP_BKSZ_128);
	cv->cv_DefaultGEO = TRUE;
	cv->cv_GadUnitNumDis = TRUE;

}

/**
 ** Calculate best possible geometry with flag indicating we'd prefer
 ** a linear geometry.
 **
 ** This routine calcs sectors, and cylinders; set tracks to 1 per cylinder.
 **/

VOID CalcBestGeometry(LONG blocks, LONG *sectrk, LONG *trkcyl, LONG *cyl, BOOL linear)
{
register LONG div;
LONG result,bestcyls, bestsecs;

	bestsecs = 1L;
	bestcyls = blocks;

	if(!linear)
	{

/* try 1-25 sectors per cyl */

		for(div = bestsecs; div < 26; div++)
		{
        		result = blocks/div;
			if((result * div) >= (bestcyls * bestsecs))
			{
				bestcyls = result;
				bestsecs = div;
			}
		}
	}

	*sectrk = bestsecs;
	*trkcyl = 1L;
	*cyl = bestcyls;
}