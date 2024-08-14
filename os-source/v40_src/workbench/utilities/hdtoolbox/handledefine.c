/* handleDefine.c - code to handle the drive definition screen */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/dos.h>
#include <libraries/filehandler.h>

#include <dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include "refresh.h"
#include "protos.h"
#include <clib/alib_protos.h>

#include "global.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "windows.h"
#include "scsidisk.h"
#include "scsi.h"

/*
struct IntuiText NDSizeIText = {
	1,0,JAM1,0,0,&TOPAZ80,"",NULL,
};
*/

/* define globals */
struct DriveDef *NewDef;
int EditFlag;		/* TRUE mean editing */

char *OverwriteType[] = {
	"Are you sure you want to overwrite",
	"the old copy of the drive type?",
	" ",
	"Select \"Continue\" to overwrite it.",
	"Select \"Cancel\" to go back to editing.",
	NULL,
};

char *ReadCapText[] = {
	"This will read in as much data as possible from the drive.",
	"Some information (such as the number of heads) may not be",
	"available.",
	" ",
	"(Note that Heads times Blocks per Track usually equals",
	"Blocks per Cylinder, but not always.)",
	NULL,
};

#ifdef USE_SYNCHRONOUS
char *UseSynch[] = {
	"WARNING:",
	"Some SCSI drives will not boot in synchronous mode,",
	"and may even cause the system to hang if synchronous mode",
	"is enabled.",
	" ",
	"You can disable use of synchronous mode with a jumper or",
	"dip switch on an A590 or A2091.  Consult your manual if",
	"you wish to do this.",
	" ",
	"If you are in doubt, DO NOT leave synchronous set to 'Yes'.",
	NULL,
};
#endif

//
USHORT NDReadParmsFlags = 0;
USHORT NDReducedFlags   = 0;
USHORT NDPreCompFlags   = 0;
USHORT NDReselectFlags  = 0;

LONG NDBlocksSInfoLongInt    = 0;
LONG NDCylBlocksSInfoLongInt = 0;
LONG NDHeadsSInfoLongInt    = 0;
LONG NDCylindersSInfoLongInt = 0;
LONG NDParkWhereSInfoLongInt = 0;
LONG NDPreCompSInfoLongInt   = 0;
LONG NDReducedSInfoLongInt   = 0;

extern UWORD XtraTop;
extern UWORD TypeListPosition;

char NDSizeText[30];
//


int
HandleDefine (w,msg)
	struct Window *w;
	struct IntuiMessage *msg;
{
	register struct Gadget *gad;
	register int done = FALSE;
	LONG cyl;
	UWORD id;

	switch (msg->Class) {
	case GADGETUP:
	case GADGETDOWN:
		gad = (struct Gadget *) (msg->IAddress);

		id  = gad->GadgetID & ~REFRESH_MASK;

		switch (id) {
		case NDBLOCKS:
			NewDef->Initial_RDB->rdb_Sectors =
					((struct StringInfo *)
					gad->SpecialInfo)->LongInt;	//

			/* lets recalc cylblocks too */
			NewDef->Initial_RDB->rdb_CylBlocks =
				NewDef->Initial_RDB->rdb_Sectors *
				NewDef->Initial_RDB->rdb_Heads;
			RecalcSize(w);
			break;

		case NDCYLBLOCKS:
			NewDef->Initial_RDB->rdb_CylBlocks =
					((struct StringInfo *)
					gad->SpecialInfo)->LongInt;	//

			/* lets recalc cylblocks too */
			RecalcSize(w);
			break;

		case NDHEADS:
			NewDef->Initial_RDB->rdb_Heads =
					((struct StringInfo *)
					gad->SpecialInfo)->LongInt;	//

			/* lets recalc cylblocks too */
			NewDef->Initial_RDB->rdb_CylBlocks =
				NewDef->Initial_RDB->rdb_Sectors *
				NewDef->Initial_RDB->rdb_Heads;
			RecalcSize(w);
			break;

		case NDCYLINDERS:
			cyl = NewDef->Initial_RDB->rdb_Cylinders;

			if (((struct StringInfo *)
			    NDReduced->SpecialInfo)->LongInt == cyl)
				NewDef->Initial_RDB->rdb_ReducedWrite =
					((struct StringInfo *)
					gad->SpecialInfo)->LongInt;	//

			if (((struct StringInfo *)
			    NDPreComp->SpecialInfo)->LongInt == cyl)
				NewDef->Initial_RDB->rdb_WritePreComp =
					((struct StringInfo *)
					gad->SpecialInfo)->LongInt;	//

			if (((struct StringInfo *)
			    NDParkWhere->SpecialInfo)->LongInt == cyl)
				NewDef->Initial_RDB->rdb_Park =
					((struct StringInfo *)
					gad->SpecialInfo)->LongInt;	//

			NewDef->Initial_RDB->rdb_Cylinders =
					((struct StringInfo *)
					gad->SpecialInfo)->LongInt;	//

			ResetStrings(w);

			RecalcSize(w);
			break;

		case NDREADPARMS:
			DoReadParms(w);
			break;

#ifdef USEPARK
		case NDPARKHEAD:
			if (NDParkHead.Flags & SELECTED)
			{
				if (NDParkSecondsSInfo.LongInt == 0)
					NDParkSecondsSInfo.LongInt = 15;

				NewDef->Initial_RDB->rdb_AutoParkSeconds =
						     NDParkSecondsSInfo.LongInt;
				MyOnGadget(w,NDParkWhere);
				MyOnGadget(w,&NDParkSeconds);
			} else {
				NewDef->Initial_RDB->rdb_AutoParkSeconds = 0;
				MyOffGadget(w,NDParkWhere);
				MyOffGadget(w,&NDParkSeconds);
			}
			break;
#endif
		case NDPARKWHERE:
			NewDef->Initial_RDB->rdb_Park =
					((struct StringInfo *)
					gad->SpecialInfo)->LongInt;	//
			break;

#ifdef USEPARK
		case NDPARKSECONDS:
			NewDef->Initial_RDB->rdb_AutoParkSeconds =
						     NDParkSecondsSInfo.LongInt;
			break;
#endif
#ifdef USEADAPTECS
		case NDNORMAL:
//			GadMXSet(w,&NDNormal);
//			GadMXClr(w,&NDNormalST506);
//			GadMXClr(w,&NDAdaptec);
			SetSCSI(w);
			NewDef->ControllerType = SCSI;
			break;

		case NDADAPTEC:
//			GadMXSet(w,&NDAdaptec);
//			GadMXClr(w,&NDNormalST506);
//			GadMXClr(w,&NDNormal);
			SetSt506(w);
			NewDef->ControllerType = SCSI|ADAPTEC;
			break;

		case NDNORMALST506:
//			GadMXSet(w,&NDNormalST506);
//			GadMXClr(w,&NDAdaptec);
//			GadMXClr(w,&NDNormal);
			SetSt506(w);
			NewDef->ControllerType = ST506;
			break;
#endif

		case NDNAME:
		case NDMANUNAME:
		case NDDRIVENAME:
		case NDREVISIONNAME:
			break;	/* nothing to do, just use it when done */

		case NDREDUCED:
			if (((struct StringInfo *)gad->SpecialInfo)->LongInt
				> ((struct StringInfo *)
				NDCylinders->SpecialInfo)->LongInt)	//
			{
				NewDef->Initial_RDB->rdb_ReducedWrite =
					((struct StringInfo *)
					NDCylinders->SpecialInfo)->LongInt;	//
				ResetStrings(w);
			} else
				NewDef->Initial_RDB->rdb_ReducedWrite =
					((struct StringInfo *)
					gad->SpecialInfo)->LongInt;	//
			break;

		case NDPRECOMP:
			if (((struct StringInfo *)gad->SpecialInfo)->LongInt
				> ((struct StringInfo *)
				NDCylinders->SpecialInfo)->LongInt)	//
			{
				NewDef->Initial_RDB->rdb_WritePreComp =
					((struct StringInfo *)
					NDCylinders->SpecialInfo)->LongInt;	//
				ResetStrings(w);
			} else
				NewDef->Initial_RDB->rdb_WritePreComp =
					((struct StringInfo *)
					gad->SpecialInfo)->LongInt;	//
			break;

		case NDRESELECT:
//			if (NDReselect.Flags & SELECTED)
			if (gad->Flags & SELECTED)		//
			    NewDef->Initial_RDB->rdb_Flags &= ~RDBFF_NORESELECT;
			else
			    NewDef->Initial_RDB->rdb_Flags |=  RDBFF_NORESELECT;
			break;

#ifdef USE_SYNCHRONOUS
		case NDSYNCHRONOUS:
			if (NDSynchronous.Flags & SELECTED)
			{
			    /* warn the user about synchronous mode! */
			    LongNotify(w,UseSynch);
			    NewDef->Initial_RDB->rdb_Flags |=  RDBFF_SYNCH;

			} else
			    NewDef->Initial_RDB->rdb_Flags &= ~RDBFF_SYNCH;

			break;
#endif

		case NDCANCEL:
			FreeDef(NewDef);
			done = HANDLE_CANCEL;
			break;

		case NDOK:
			/* GODDAMN blank-padded strings! */

			if (strlen(NDManuNameSIBuff)     == 0 &&
			    strlen(NDDriveNameSIBuff)    == 0 &&
			    strlen(NDRevisionNameSIBuff) == 0)
			{
				Notify(w,"You must give a name for the drive!",
				       0);
				break;
			}

			{
			short i;

			/* these may have changed without a message */
			NewDef->Initial_RDB->rdb_Sectors =
				((struct StringInfo *)
				NDBlocks->SpecialInfo)->LongInt;	//

			NewDef->Initial_RDB->rdb_CylBlocks =
				((struct StringInfo *)
				NDCylBlocks->SpecialInfo)->LongInt;	//

			NewDef->Initial_RDB->rdb_Heads =
				((struct StringInfo *)
				NDHeads->SpecialInfo)->LongInt;		//

			NewDef->Initial_RDB->rdb_Cylinders =
				((struct StringInfo *)
				NDCylinders->SpecialInfo)->LongInt;	//

			NewDef->Initial_RDB->rdb_WritePreComp =
				((struct StringInfo *)
				NDPreComp->SpecialInfo)->LongInt;	//

			NewDef->Initial_RDB->rdb_ReducedWrite =
				((struct StringInfo *)
				NDReduced->SpecialInfo)->LongInt;	//

			NewDef->Initial_RDB->rdb_Park =
				((struct StringInfo *)
				NDParkWhere->SpecialInfo)->LongInt;	//

			RecalcSize(w);

			/* update only if selected */
#ifdef USEPARK
			if (NDParkHead.Flags & SELECTED)
				NewDef->Initial_RDB->rdb_AutoParkSeconds =
						     NDParkSecondsSInfo.LongInt;
			else
#endif
				NewDef->Initial_RDB->rdb_AutoParkSeconds = 0;

			/* blank pad all the strings */
			strncpy(NewDef->Initial_RDB->rdb_DiskVendor,
				NDManuNameSIBuff,8);
			for (i = strlen(NDManuNameSIBuff); i < 8; i++)
				NewDef->Initial_RDB->rdb_DiskVendor[i] = ' ';

			strncpy(NewDef->Initial_RDB->rdb_DiskProduct,
				NDDriveNameSIBuff,16);
			for (i = strlen(NDDriveNameSIBuff); i < 16; i++)
				NewDef->Initial_RDB->rdb_DiskProduct[i] = ' ';

			strncpy(NewDef->Initial_RDB->rdb_DiskRevision,
				NDRevisionNameSIBuff,4);
			for (i = strlen(NDRevisionNameSIBuff); i < 4; i++)
				NewDef->Initial_RDB->rdb_DiskRevision[i] = ' ';
			}

			if (NewDef->ControllerType & ADAPTEC)
			{
			   strcpy(&NewDef->Initial_RDB->rdb_ControllerVendor[0],
			          "Adaptec");
			   NewDef->Initial_RDB->rdb_ControllerVendor[7] = ' ';
			}

			/* did he change the name?  if so, don't overwrite! */
			if (EditFlag && !SameDrive(NewDef->Initial_RDB,
						   CurrType->Initial_RDB))
			{
				/* not the same name - change editflag */
				EditFlag = FALSE;
			}

			/* same name - does he want to overwrite? */
			if (EditFlag && !AskSure(w,&OverwriteType[0]))
				break;

			if (WriteDef(w,NewDef,EditFlag))
			{
				/* link into appropriate list		*/
				/* and recalculate the numbers in lists */
				NewDef->Prev = NewDef->Succ = NULL;
				if (NewDef->ControllerType & SCSI)
				{
					CheckDefDates(&SCSI_Defs,NewDef);
					SCSI_DefNum = CountDefs(SCSI_Defs);
				} else {
					CheckDefDates(&St506_Defs,NewDef);
					St506_DefNum = CountDefs(St506_Defs);
				}

				// Put correct data to CurrType...etc.
				if (!EditFlag)	// Bug Fix!
				{
//					DisplayType    = NULL;
					TypeListPosition = 0;	//
					DisplayTypeNum = 0;
//					FirstType =	// Bug Fix !
					FirstType = CurrType = DisplayType =	//
						(TypeType & SCSI ? SCSI_Defs : St506_Defs);
				}
				else		// Bug Fix!
				{
					struct DriveDef *def;
					WORD i;
					FirstType = DisplayType
						= (TypeType & SCSI ? SCSI_Defs : St506_Defs);
					for (i = 0,def = FirstType;
					i <= TypeListPosition && def;
					i++,def = def->Succ)
						CurrType = def;
				}

				done = HANDLE_DONE;
			} else
				DisplayBeep(w->WScreen);
			break;

		}

		break;
//
//	case MENUPICK:			// I don't want to use menu now.
//		HandleMenus(w,msg);
//		break;

	}

	return done;
}

/* modified to handle cylblocks != cyls * blocks */

#if 0
void
RecalcSize (w)
	struct Window *w;
{
	register struct RigidDiskBlock *rdb;
	register LONG rem,meg,size;
	static char temp[30] = "";

	/* erase old one */
	NDSizeIText.FrontPen = 2;
	PrintIText(rp,&NDSizeIText,300,92);

	rdb = NewDef->Initial_RDB;

	rdb->rdb_LoCylinder = 2;	/* FIX! */
	rdb->rdb_HiCylinder = rdb->rdb_Cylinders - 1;
	rdb->rdb_RDBBlocksLo = 0;
	rdb->rdb_RDBBlocksHi = 2 * rdb->rdb_CylBlocks - 1;	/* FIX! */

	size = rdb->rdb_CylBlocks * rdb->rdb_HiCylinder *
	       rdb->rdb_BlockBytes;

	rem  = size / 1024;			/* K */
	meg  = rem  / 1024;

	sprintf(temp,"%ldK (%ld Meg)",rem,meg);

	NDSizeIText.IText = temp;
	NDSizeIText.FrontPen = 1;
	PrintIText(rp,&NDSizeIText,300,92);
}
#endif

void
RecalcSize (w)
	struct Window *w;
{
	RecalcSizeSetup();

	GT_SetGadgetAttrs(NDSize,w,NULL,GTTX_Text,NDSizeText,TAG_DONE);
}

void
RecalcSizeSetup (void)
{
	register struct RigidDiskBlock *rdb;
	register LONG rem,meg,size;

	rdb = NewDef->Initial_RDB;

	rdb->rdb_LoCylinder = 2;	/* FIX! */
	rdb->rdb_HiCylinder = rdb->rdb_Cylinders - 1;
	rdb->rdb_RDBBlocksLo = 0;
	rdb->rdb_RDBBlocksHi = 2 * rdb->rdb_CylBlocks - 1;	/* FIX! */

	size = rdb->rdb_CylBlocks * rdb->rdb_HiCylinder *
	       rdb->rdb_BlockBytes;

	rem  = size / 1024;			/* K */
	meg  = rem  / 1024;

	sprintf(NDSizeText,"%ldK (%ld Meg)",rem,meg);
}

#if 0
void
DefineDraw (rp)
	struct RastPort *rp;
{
	RecalcSize(rp);
}
#endif

int
DefineSetup(w)
	struct Window *w;
{
	EditFlag = FALSE;

	if (!(NewDef = AllocNew(DriveDef)))
		return FALSE;

	if (!(NewDef->Initial_RDB = AllocNew(RigidDiskBlock)))
	{
		FreeMem((char *) NewDef,sizeof(*NewDef));
		return FALSE;
	}

	NewDef->Filename[0] = '\0';	/* paranoia */

#ifdef USEADAPTECS
	NDAdaptec.Flags	    &= ~SELECTED;
	NDNormal.Flags	    &= ~SELECTED;
	NDNormalST506.Flags &= ~SELECTED;
#endif
	NDReselectFlags    &= ~SELECTED;

#ifdef USE_SYNCHRONOUS
	NDSynchronous.Flags &= ~SELECTED;
#endif

#ifdef USEADAPTECS
	NDAdaptec.Flags	    &= ~GADGDISABLED;
	NDNormal.Flags	    &= ~GADGDISABLED;
	NDNormalST506.Flags &= ~GADGDISABLED;
#endif

	if (TypeType == ST506)
	{
		NewDef->ControllerType = ST506;
		NDReadParmsFlags   |= GADGDISABLED;
		NDReselectFlags    |= GADGDISABLED;
#ifdef USE_SYNCHRONOUS
		NDSynchronous.Flags |= GADGDISABLED;
#endif
		NDPreCompFlags     |= GADGDISABLED;
		NDReducedFlags     |= GADGDISABLED;
#ifdef USEADAPTECS
		NDNormalST506.Flags |= SELECTED;
#endif
		*(NewDef->Initial_RDB) = St506RDB;
	} else {
		NewDef->ControllerType = SCSI;
		NDReadParmsFlags   &= ~GADGDISABLED;
		NDReselectFlags    &= ~GADGDISABLED;
#ifdef USE_SYNCHRONOUS
		NDSynchronous.Flags &= ~GADGDISABLED;
#endif
		NDPreCompFlags     |= GADGDISABLED;
		NDReducedFlags     |= GADGDISABLED;
#ifdef USEADAPTECS
		NDNormal.Flags      |= SELECTED;
#endif
		*(NewDef->Initial_RDB) = ScsiRDB;
	}

	NDReselectFlags &= ~SELECTED;
	if ((NDReselectFlags & GADGDISABLED) == 0)
	{
	    if (NewDef->Initial_RDB->rdb_Flags & RDBFF_NORESELECT)
		NDReselectFlags &= ~SELECTED;
	    else
		NDReselectFlags |= SELECTED;
	}

#ifdef USE_SYNCHRONOUS
	NDSynchronous.Flags &= ~SELECTED;
	if ((NDSynchronous.Flags & GADGDISABLED) == 0)
	{
	    if (NewDef->Initial_RDB->rdb_Flags & RDBFF_SYNCH)
		NDSynchronous.Flags |= SELECTED;
	    else
		NDSynchronous.Flags &= ~SELECTED;
	}
#endif
#ifdef USEPARK
	if (NewDef->Initial_RDB->rdb_AutoParkSeconds)
	{
		NDParkHead.Flags    |= SELECTED;
		NDParkWhere.Flags   &= ~GADGDISABLED;
		NDParkSeconds.Flags &= ~GADGDISABLED;
	} else {
		NDParkHead.Flags    &= ~SELECTED;
		NDParkWhere.Flags   |= GADGDISABLED;
		NDParkSeconds.Flags |= GADGDISABLED;
	}
#endif
	/* set cyl,sec,head,park gadgets */
	ResetStrings(NULL);

	NDManuNameSIBuff[0]	= '\0';
	NDDriveNameSIBuff[0]	= '\0';
	NDRevisionNameSIBuff[0]	= '\0';

#ifdef USEADAPTECS
	/* change name of standard st506 to xt if needed */
	strcpy(&(NDNormalST506.GadgetText->IText[9]),xt_name);
#endif

	RecalcSizeSetup();	//

	return TRUE;
}

/* the initial routine for editing a drive type */

int
EditSetup(w)
	struct Window *w;
{
	EditFlag = TRUE;

	if (!(NewDef = AllocNew(DriveDef)))
		return FALSE;

	*NewDef = *CurrType;
	NewDef->Filename[0] = '\0';
	NewDef->Initial_RDB = NULL;
	NewDef->Prev = NewDef->Succ = NULL;

	if (!(NewDef->Initial_RDB = AllocNew(RigidDiskBlock)))
	{
		FreeMem((char *) NewDef,sizeof(*NewDef));
		return FALSE;
	}
	*(NewDef->Initial_RDB) = *(CurrType->Initial_RDB);

#ifdef USEADAPTECS
	NDAdaptec.Flags	    &= ~SELECTED;
	NDNormal.Flags	    &= ~SELECTED;
	NDNormalST506.Flags &= ~SELECTED;
#endif
	NDReselectFlags    &= ~SELECTED;
#ifdef USE_SYNCHRONOUS
	NDSynchronous.Flags &= ~SELECTED;
#endif

#ifdef USEADAPTECS
	NDAdaptec.Flags	    |= GADGDISABLED;
	NDNormal.Flags	    |= GADGDISABLED;
	NDNormalST506.Flags |= GADGDISABLED;
#endif
	NDReadParmsFlags   |= GADGDISABLED;
	NDPreCompFlags     |= GADGDISABLED;
	NDReducedFlags     |= GADGDISABLED;
	NDReselectFlags    |= GADGDISABLED;
#ifdef USE_SYNCHRONOUS
	NDSynchronous.Flags |= GADGDISABLED;
#endif
	NDPreCompFlags     |= GADGDISABLED;
	NDReducedFlags     |= GADGDISABLED;

	switch (NewDef->ControllerType) {
	case ST506:
#ifdef USEADAPTECS
		NDNormalST506.Flags |= SELECTED;
		NDNormalST506.Flags &= ~GADGDISABLED;
#endif
		break;

	case SCSI:
		NDReadParmsFlags   &= ~GADGDISABLED;
		NDReselectFlags    &= ~GADGDISABLED;
#ifdef USE_SYNCHRONOUS
		NDSynchronous.Flags &= ~GADGDISABLED;
#endif
#ifdef USEADAPTECS
		NDNormal.Flags      |= SELECTED;
		NDNormal.Flags      &= ~GADGDISABLED;
#endif
		break;

	case SCSI|ADAPTEC:
#ifdef USEADAPTECS
		NDAdaptec.Flags     |= SELECTED;
		NDAdaptec.Flags     &= ~GADGDISABLED;
#endif
		NDReselectFlags    &= ~GADGDISABLED;
#ifdef USE_SYNCHRONOUS
		NDSynchronous.Flags &= ~GADGDISABLED;
#endif
		NDReadParmsFlags   &= ~GADGDISABLED;
		NDPreCompFlags     &= ~GADGDISABLED;
		NDReducedFlags     &= ~GADGDISABLED;
		break;
	}

	NDReselectFlags &= ~SELECTED;

	if ((NDReselectFlags & GADGDISABLED) == 0)
	{
	    if (NewDef->Initial_RDB->rdb_Flags & RDBFF_NORESELECT)
		NDReselectFlags &= ~SELECTED;
	    else
		NDReselectFlags |= SELECTED;
	}

#ifdef USE_SYNCHRONOUS
	NDSynchronous.Flags &= ~SELECTED;

	if ((NDSynchronous.Flags & GADGDISABLED) == 0)
	{
	    if (NewDef->Initial_RDB->rdb_Flags & RDBFF_SYNCH)
		NDSynchronous.Flags |= SELECTED;
	    else
		NDSynchronous.Flags &= ~SELECTED;
	}
#endif
#ifdef USEPARK
	if (NewDef->Initial_RDB->rdb_AutoParkSeconds)
	{
		NDParkHead.Flags    |= SELECTED;
		NDParkWhere.Flags   &= ~GADGDISABLED;
		NDParkSeconds.Flags &= ~GADGDISABLED;
	} else {
		NDParkHead.Flags    &= ~SELECTED;
		NDParkWhere.Flags   |= GADGDISABLED;
		NDParkSeconds.Flags |= GADGDISABLED;
	}
#endif
	/* set cyl,sec,head,park gadgets */
	ResetStrings(NULL);

	/* careful, they aren't null terminated! */
	strncpy(&NDManuNameSIBuff[0],NewDef->Initial_RDB->rdb_DiskVendor,8);
	strncpy(&NDDriveNameSIBuff[0],NewDef->Initial_RDB->rdb_DiskProduct,16);
	strncpy(&NDRevisionNameSIBuff[0],NewDef->Initial_RDB->rdb_DiskRevision,
		4);
	NDManuNameSIBuff[8]     = '\0';
	NDDriveNameSIBuff[16]   = '\0';
	NDRevisionNameSIBuff[4] = '\0';

	strip_trail(NDManuNameSIBuff);
	strip_trail(NDDriveNameSIBuff);
	strip_trail(NDRevisionNameSIBuff);

	RecalcSizeSetup();	//

	return TRUE;
}

/* if w != NULL, remove, fiddle, add, refresh */

#if 0
void
ResetStrings (w)
	struct Window *w;
{
	int block_pos,cylblock_pos,head_pos,cyl_pos,where_pos,precomp_pos;
	int reduced_pos;

	if (w)
	{
		block_pos    = RemoveGList(w,NDBlocks,1L);
		cylblock_pos = RemoveGList(w,NDCylBlocks,1L);
		head_pos     = RemoveGList(w,NDHeads,1L);
		cyl_pos      = RemoveGList(w,NDCylinders,1L);
		where_pos    = RemoveGList(w,NDParkWhere,1L);
#ifdef USEPARK
		secs_pos     = RemoveGList(w,&NDParkSeconds,1L);
#endif
		precomp_pos  = RemoveGList(w,NDPreComp,1L);
		reduced_pos  = RemoveGList(w,NDReduced,1L);
	}

	stcul_d(NDBlocksSIBuff,NewDef->Initial_RDB->rdb_Sectors);
	NDBlocksSInfo.LongInt = NewDef->Initial_RDB->rdb_Sectors;
	stcul_d(NDCylBlocksSIBuff,NewDef->Initial_RDB->rdb_CylBlocks);
	NDCylBlocksSInfo.LongInt = NewDef->Initial_RDB->rdb_CylBlocks;
	stcul_d(NDHeadsSIBuff,NewDef->Initial_RDB->rdb_Heads);
	NDHeadsSInfo.LongInt = NewDef->Initial_RDB->rdb_Heads;
	stcul_d(NDCylindersSIBuff,NewDef->Initial_RDB->rdb_Cylinders);
	NDCylindersSInfo.LongInt = NewDef->Initial_RDB->rdb_Cylinders;
	stcul_d(NDParkWhereSIBuff,NewDef->Initial_RDB->rdb_Park);
	NDParkWhereSInfo.LongInt = NewDef->Initial_RDB->rdb_Park;
#ifdef USEPARK
	stcul_d(NDParkSecondsSIBuff,NewDef->Initial_RDB->rdb_AutoParkSeconds);
	NDParkSecondsSInfo.LongInt = NewDef->Initial_RDB->rdb_AutoParkSeconds;
#endif
	stcul_d(NDPreCompSIBuff,NewDef->Initial_RDB->rdb_WritePreComp);
	NDPreCompSInfo.LongInt = NewDef->Initial_RDB->rdb_WritePreComp;
	stcul_d(NDReducedSIBuff,NewDef->Initial_RDB->rdb_ReducedWrite);
	NDReducedSInfo.LongInt = NewDef->Initial_RDB->rdb_ReducedWrite;

	if (w)
	{
		// add them back in reverse order!!!!
		AddGList(w,NDReduced,reduced_pos,1L,NULL);
		AddGList(w,NDPreComp,precomp_pos,1L,NULL);
#ifdef USEPARK
		AddGList(w,&NDParkSeconds,secs_pos,1L,NULL);
#endif
		AddGList(w,NDParkWhere,where_pos,1L,NULL);
		AddGList(w,NDCylinders,cyl_pos,1L,NULL);
		AddGList(w,NDHeads,head_pos,1L,NULL);
		AddGList(w,NDCylBlocks,cylblock_pos,1L,NULL);
		AddGList(w,NDBlocks,block_pos,1L,NULL);

		RefreshGList(NDBlocks,w,NULL,1L);
		RefreshGList(NDCylBlocks,w,NULL,1L);
		RefreshGList(NDHeads,w,NULL,1L);
		RefreshGList(NDCylinders,w,NULL,1L);
		RefreshGList(NDParkWhere,w,NULL,1L);
#ifdef USEPARK
		RefreshGList(&NDParkSeconds,w,NULL,1L);
#endif
		RefreshGList(NDPreComp,w,NULL,1L);
		RefreshGList(NDReduced,w,NULL,1L);
	}

}
#endif

void
ResetStrings (w)
	struct Window *w;
{
	NDBlocksSInfoLongInt = NewDef->Initial_RDB->rdb_Sectors;
	NDCylBlocksSInfoLongInt = NewDef->Initial_RDB->rdb_CylBlocks;
	NDHeadsSInfoLongInt = NewDef->Initial_RDB->rdb_Heads;
	NDCylindersSInfoLongInt = NewDef->Initial_RDB->rdb_Cylinders;
	NDParkWhereSInfoLongInt = NewDef->Initial_RDB->rdb_Park;
#ifdef USEPARK
	NDParkSecondsSInfoLongInt = NewDef->Initial_RDB->rdb_AutoParkSeconds;
#endif
	NDPreCompSInfoLongInt = NewDef->Initial_RDB->rdb_WritePreComp;
	NDReducedSInfoLongInt = NewDef->Initial_RDB->rdb_ReducedWrite;

	if (w)
	{
		GT_SetGadgetAttrs(NDBlocks,w,NULL,
			GTIN_Number,NewDef->Initial_RDB->rdb_Sectors,TAG_DONE);

		GT_SetGadgetAttrs(NDCylBlocks,w,NULL,
			GTIN_Number,NewDef->Initial_RDB->rdb_CylBlocks,TAG_DONE);

		GT_SetGadgetAttrs(NDHeads,w,NULL,
			GTIN_Number,NewDef->Initial_RDB->rdb_Heads,TAG_DONE);

		GT_SetGadgetAttrs(NDCylinders,w,NULL,
			GTIN_Number,NewDef->Initial_RDB->rdb_Cylinders,TAG_DONE);

		GT_SetGadgetAttrs(NDParkWhere,w,NULL,
			GTIN_Number,NewDef->Initial_RDB->rdb_Park,TAG_DONE);
#ifdef USEPARK	// In the other part, NDParkSeconds gadget is supposed to be not
		// for pointer type.
		GT_SetGadgetAttrs(NDParkSeconds,w,NULL,
			GTIN_Number,NewDef->Initial_RDB->rdb_AutoParkSeconds,TAG_DONE);
#endif
		GT_SetGadgetAttrs(NDPreComp,w,NULL,
			GTIN_Number,NewDef->Initial_RDB->rdb_WritePreComp,TAG_DONE);

		GT_SetGadgetAttrs(NDReduced,w,NULL,
			GTIN_Number,NewDef->Initial_RDB->rdb_ReducedWrite,TAG_DONE);

	}
}

/* appends: note - can make file longer due to multiple copies of a def */

int
WriteDef (w,def,flag)
	struct Window *w;
	register struct DriveDef *def;
	int flag;	/* TRUE mean editing */
{
	struct DiskDrive dd;
	register BPTR fh;
	LONG temp;

	if (!(fh = Open(NDNameSIBuff,MODE_OLDFILE)))
		fh = Open(NDNameSIBuff,MODE_NEWFILE);
	else {
          if (Read(fh,(char *) &temp,4L)) {
            if (temp != IDNAME_DRIVE) {
               Notify(w,"%s is not a file of drive definitions!", (LONG) NDNameSIBuff);
               return FALSE;
               }
            if (!flag) Seek(fh,0L,OFFSET_END);	/* append to file */
            else Seek(fh,def->Offset,OFFSET_BEGINNING);
            }
          }

	if (fh)
	{
		/* set up dd */
		DateStamp((struct DateStamp *)&(def->ds));

		dd.Ident	  = IDNAME_DRIVE;
		dd.ControllerType = def->ControllerType;
		dd.Reserved1      = def->Reserved1;
		dd.Reserved2      = def->Reserved2;
		dd.rdb	          = *(def->Initial_RDB);
		dd.ds		  = def->ds;

		def->Offset = Seek(fh,0L,OFFSET_CURRENT);

		if (Write(fh,(char *) &dd,sizeof(dd)) == sizeof(dd))
		{
			strcpy(def->Filename,NDNameSIBuff);
			Close(fh);
			return TRUE;
		} else {
			Notify(w,"Error writing to %s!",(LONG) NDNameSIBuff);
			Close(fh);
			return FALSE;
		}
	}

	Notify(w,"Can't Open %s!",(LONG) NDNameSIBuff);
	return FALSE;
}

void
FreeDef (def)
	register struct DriveDef *def;
{
	if (def) {
		FreeRDB(def->Initial_RDB);
		FreeMem((char *) def,sizeof(*def));
	}
}

void
SetSCSI (w)
	register struct Window *w;
{
	if (NewDef->ControllerType & (ST506|ADAPTEC|OMTI))
	{
		MyOffGadget(w,NDPreComp);
		MyOffGadget(w,NDReduced);
		MyOnGadget(w,NDReadParms);
		MyOnGadget(w,NDReselect);
#ifdef USE_SYNCHRONOUS
		MyOnGadget(w,&NDSynchronous);
#endif
		*(NewDef->Initial_RDB) = ScsiRDB;
	}
}

void
SetSt506 (w)
	register struct Window *w;
{
	if (!(NewDef->ControllerType & (ST506|ADAPTEC|OMTI)))
	{
		MyOnGadget(w,NDPreComp);
		MyOnGadget(w,NDReduced);
		MyOffGadget(w,NDReadParms);
// Right now this function is never called. But I changed
// MyOffGadget(), so I should add below 2 sentenses.
		GT_SetGadgetAttrs(NDReselect,w,NULL,GTCB_Checked,FALSE,TAG_DONE);
		NDReselectFlags &= ~SELECTED;
		MyOffGadget(w,NDReselect);
#ifdef USE_SYNCHRONOUS
		MyOffGadget(w,&NDSynchronous);
#endif
		*(NewDef->Initial_RDB) = St506RDB;
	}
}

/* read info from SCSI drive */

extern struct SCSICmd cmdblk;

void
DoReadParms (w)
	struct Window *w;
{
	unsigned char inquiry[10];
	unsigned char data[255];
	long blocks;
	register long cyls,tracksize,i,heads = 1;
	int  opened = FALSE;
	register struct IOStdReq *ior  = NULL;
	struct MsgPort  *port = NULL;
	register struct Drive    *d;
	LONG zones = 1,alt_sectors = 0,alt_tracks = 0,alt_volume = 0,cylblocks;
	LONG blocks_lost = -1,write_precomp = 0,reduced_write = 0;
	LONG landing_zone = 0,tracksperzone = 1,oldheads,oldtracksize;
	LONG sectorsize;	/* 39.16 */

	d = SelectedDrive;

	if ((TypeType == SCSI && !(d->Flags & SCSI)) ||
	    (TypeType == ST506 && !(d->Flags & ST506)))
	{
		Notify(w,"Selected drive on main screen is wrong type!",0);
		return;
	}

	/* say what will happen */
	if (!AskSure(w,&ReadCapText[0]))
		return;

	/* tell what address being used */
/*	Notify(w,"Using drive selected on main screen (Address %ld)",
	       d->Addr);
*/
	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	if (i = OpenDevice(d->DeviceName,d->Addr + d->Lun * 10,
			   (struct IORequest *) ior,0L))
	{
		Notify(w,"Error %d on device open!",i);
		goto cleanup;
	}
	opened = TRUE;

	/* 39.16 --- first find out the sectorsize and set it */
	sectorsize = ReadSectorSize(w,d);
	if (sectorsize > 0)
		NewDef->Initial_RDB->rdb_BlockBytes = sectorsize;

	/* first we try mode_sense */

	inquiry[0] = S_MODE_SENSE;
	inquiry[1] = 0;
	inquiry[2] = 3;	/* page 3 - we want sectors per track, current values */
	inquiry[3] = 0;
	inquiry[4] = 255;
	inquiry[5] = 0;

	if (!DoSCSI(ior,(UWORD *) inquiry,6,(UWORD *) data,255,
		    SCSIF_READ|SCSIF_AUTOSENSE))
	{
		i = 4 + data[3];	/* block descriptor length */
		if (cmdblk.scsi_Actual < i + 12)
{
/*Notify(w,"Sense Actual = %ld",cmdblk.scsi_Actual);*/
			goto try_inquiry;
}
		/* ignore blocks - can get from read_capacity */

		/* page descriptors */

/*
Notify(w,"Page = %ld",data[i] & 0x3f);
Notify(w,"Page len = %ld",data[i+1]);
*/
		if ((data[i] & 0x3f) != 3 || data[i+1] < 10)
			goto try_inquiry;

		tracksize = (data[i+10] << 8) | data[i+11];

/*
Notify(w,"tracksize = %ld",tracksize);
*/
		if (tracksize == 0)
			goto try_inquiry;

		tracksperzone = (data[i+2] << 8) | data[i+3];
		alt_sectors   = (data[i+4] << 8) | data[i+5];
		alt_tracks    = (data[i+6] << 8) | data[i+7];
		alt_volume    = (data[i+8] << 8) | data[i+9];
/*
Notify(w,"tpz = %ld",tracksperzone);
Notify(w,"alt_sectors = %ld",alt_sectors);
Notify(w,"alt_tracks = %ld",alt_tracks);
Notify(w,"alt_volume = %ld",alt_volume);
*/
		/* we have tracksize, now try for heads */
		inquiry[2] = 4;	/* we want cylinders & heads,current values */

		if (!DoSCSI(ior,(UWORD *) inquiry,6,(UWORD *) data,255,
			    SCSIF_READ|SCSIF_AUTOSENSE))
		{
			i = 4 + data[3];	/* block descriptor length */
			if (cmdblk.scsi_Actual < i + 6)
{
/*Notify(w,"Second Sense Actual = %ld",cmdblk.scsi_Actual);*/
				goto try_inquiry;
}
			/* page descriptors */
/*
Notify(w,"Page = %ld",data[i] & 0x3f);
Notify(w,"Page len = %ld",data[i+1]);
*/
			if ((data[i] & 0x3f) != 4 || data[i+1] < 4)
				goto try_inquiry;

			/* cyls are raw, and may include totally invisible   */
			/* cylinders used by the drive for it's own purposes */
			cyls  = (data[i+2] << 16) |
				(data[i+3] << 8)  |
				(data[i+4]);
			heads =  data[i+5];
			if (heads == 0)		/* avoid x/0 problems */
				heads = 1;
			cylblocks = heads * tracksize;


			/* save for later GODDAMN SCSI VENDORS!!! */
			oldheads = heads;
			oldtracksize = tracksize;

/*
Notify(w,"heads = %ld",heads);
Notify(w,"cyls = %ld",cyls);
Notify(w,"cylblocks = %ld",cylblocks);
*/
			if (data[i+1] >= 15) {
				write_precomp = (data[i+6] << 16) |
						(data[i+7] << 8)  |
						 data[i+8];
				reduced_write = (data[i+9]  << 16) |
						(data[i+10] << 8)  |
						 data[i+11];
				landing_zone  = (data[i+14] << 16) |
						(data[i+15] << 8)  |
						 data[i+16];
			}
/*
Notify(w,"write precomp = %ld",write_precomp);
Notify(w,"reduced write = %ld",reduced_write);
Notify(w,"landing zone = %ld",landing_zone);
*/
			if (tracksperzone == 0)
				zones = 1;
			else
				/* make sure it rounds up */
				zones = (heads*cyls + tracksperzone-1) /
					tracksperzone;
/*
Notify(w,"Zones = %ld",zones);
*/
			/* now adjust raw numbers... round up (+ heads-1) */
			if (alt_volume || alt_tracks)
			    cyls -= (alt_volume + alt_tracks*zones + heads-1) /
				    heads;

			/* we're going to kodiak's definitions of CylBlocks */
			/* Sectors * Heads does not have to equal CylBlocks */
			if (alt_sectors)
			{
				if (tracksperzone == heads)
					cylblocks -= alt_sectors;
				else if (tracksperzone%heads == 0 &&
					 alt_sectors%(tracksperzone/heads) == 0)
				{
				  /* integer number of sectors/cyl mapped out */
					cylblocks -=
					      alt_sectors%(tracksperzone/heads);
				} else if (alt_sectors % tracksperzone == 0) {
					/* N sectors per track */
					tracksize -= alt_sectors/tracksperzone;
					cylblocks = heads*tracksize;
				} else {
					/* else there's no good multiple! */
					goto try_inquiry;
				}
			}

/*
Notify(w,"heads = %ld",heads);
Notify(w,"cyls = %ld",cyls);
Notify(w,"cylblocks = %ld",cylblocks);
*/
			/* now sanity check */
			blocks = ReadCapacity(w,d,0,0);

/*
Notify(w,"blocks = %ld",blocks);
Notify(w,"cyls*cylblocks = %ld",cyls * cylblocks);
*/
			/* do the numbers add up? */
			if (blocks < cyls * cylblocks)
			{
				/* check if it's an even number of cyls   */
				/* SCSI drives grab a cylinder or two for */
				/* themselves to store private info.      */

				if ((cyls*cylblocks-blocks) % cylblocks == 0)
				{
					/* yes - reduce cylinder count */
					cyls = blocks/cylblocks;
				} else {
					blocks_lost = -1;
					goto try_inquiry;
				}
			} else {
				/* we're right on, or are missing some blocks */
				/* try to use them if we can */
				cyls += (blocks-cyls*cylblocks)/cylblocks;
			}

			blocks_lost = blocks - cyls*cylblocks;

/*
Notify(w,"blocks lost = %ld",blocks_lost);
*/
			/* set geometry no matter how far off */
			SetGeometry(w,NewDef->Initial_RDB,cyls,heads,
				    tracksize,cylblocks,write_precomp,
				    reduced_write,landing_zone);

		} /* else mode_sense for page 4 failed */

	} /* else mode_sense for page 3 failed */

	/* now we try inquiry */
try_inquiry:

	inquiry[0] = S_INQUIRY;
	inquiry[1] =
	inquiry[2] =
	inquiry[3] = 0;
	inquiry[4] = 36;
	inquiry[5] = 0;

	if (!DoSCSI(ior,(UWORD *) inquiry,6,(UWORD *) data,36,
		    SCSIF_READ|SCSIF_AUTOSENSE))
	{
		if (cmdblk.scsi_Actual < 36 || data[4] < 31)
		{
			Notify(w,"Insufficient data returned by inquiry",0);
			goto cleanup;
		}
		if (data[0] != 0)
		{
			Notify(w,"Unit is not a disk (type %ld)!",data[0]);
			goto cleanup;
		}
		if ((data[3] & 0x0f) > 2) /* accept 0 (unspecified), 1, or 2 */
		{
			Notify(w,"Don't understand Inquiry data (type 0x%lx)!",
			       data[3]);
			goto cleanup;
		}

		memcpy(NDManuNameSIBuff,&data[8],8);
		NDManuNameSIBuff[8] = '\0';
		strip_trail(NDManuNameSIBuff);

		memcpy(NDDriveNameSIBuff,&data[16],16);
		NDDriveNameSIBuff[16] = '\0';
		strip_trail(NDDriveNameSIBuff);

		memcpy(NDRevisionNameSIBuff,&data[32],4);
		NDRevisionNameSIBuff[4] = '\0';
		strip_trail(NDRevisionNameSIBuff);

		RefreshGList(NDManuName,w,NULL,1L);
		RefreshGList(NDDriveName,w,NULL,1L);
		RefreshGList(NDRevisionName,w,NULL,1L);

		blocks = ReadCapacity(w,d,0,0);
		if (blocks != -1)
		{
			heads = 1;

			/* PMI = 1 means next block before next "delay" */
			tracksize = ReadCapacity(w,d,1,0);
			if (tracksize == -1 || tracksize == blocks)
			{
				Notify(w,"Cannot find track size, assuming %ld",
				       17 * 4);
				tracksize = 17 * 4;	/* punt */
			}

			if (tracksize == oldheads*oldtracksize)
			{
				heads     = oldheads;
				tracksize = oldtracksize;
			}

			/* make sure cyls < 4096 (?) */
			cyls = blocks/(heads*tracksize);
			while (cyls > 4095)
			{
				tracksize <<= 1;
				cyls = blocks/(heads*tracksize);
			}

/*
Notify(w,"cyls = %ld",cyls);
Notify(w,"cylblocks = %ld",heads*tracksize);
Notify(w,"block_lost = %ld",blocks-cyls*heads*tracksize);
*/
			/* is this closer than I got via normal means? */
			if (blocks_lost == -1 ||
			    blocks - cyls*heads*tracksize < blocks_lost)
			{
				SetGeometry(w,NewDef->Initial_RDB,cyls,heads,
					    tracksize,heads*tracksize,
					    write_precomp,
					    reduced_write,landing_zone);
			}
		} else
			Notify(w,"Cannot read device size!",NULL);

	} else
		Notify(w,"Drive does not support the SCSI Inquiry command!",
		       NULL);

cleanup:
	if (opened)
		CloseDevice((struct IORequest *) ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);
}

void
SetGeometry (w,rdb,cyls,heads,sectors,cylblocks,precomp,reduced,park)
	struct Window *w;
	register struct RigidDiskBlock *rdb;
	register LONG cyls,heads,sectors,cylblocks,precomp,reduced,park;
{
	if (park == 0)
		park = cyls;
	if (precomp == 0)
		precomp = cyls;
	if (reduced == 0)
		reduced = cyls;

	rdb->rdb_Sectors      = sectors;
	rdb->rdb_Heads        = heads;
	rdb->rdb_Cylinders    = cyls;
	rdb->rdb_CylBlocks    = cylblocks;
	rdb->rdb_Park	      = park;
	rdb->rdb_WritePreComp = precomp;
	rdb->rdb_ReducedWrite = reduced;

	ResetStrings(w);

	RecalcSize(w);
}

#if 0
void
MyOnGadget (w,gad)
	register struct Window *w;
	register struct Gadget *gad;
{
	register UWORD pos;

	if (gad->Flags & GADGDISABLED)
	{
		pos = RemoveGList(w,gad,1L);
		gad->Flags &= ~GADGDISABLED;
		SetAPen(w->RPort,0L);
		SetDrMd(w->RPort,JAM1);
		RectFill(w->RPort,gad->LeftEdge,gad->TopEdge,
			 gad->LeftEdge + gad->Width  - 1,
			 gad->TopEdge  + gad->Height - 1);
		AddGList(w,gad,pos,1L,NULL);
		RefreshGList(gad,w,NULL,1L);
	}
}

void
MyOffGadget (w,gad)
	register struct Window *w;
	register struct Gadget *gad;
{
	register UWORD pos;

	if (!(gad->Flags & GADGDISABLED))
	{
		pos = RemoveGList(w,gad,1L);
		gad->Flags |= GADGDISABLED;
		gad->Flags &= ~SELECTED;
		SetAPen(w->RPort,0L);
		SetDrMd(w->RPort,JAM1);
		RectFill(w->RPort,gad->LeftEdge,gad->TopEdge,
			 gad->LeftEdge + gad->Width  - 1,
			 gad->TopEdge  + gad->Height - 1);
		AddGList(w,gad,pos,1L,NULL);
		RefreshGList(gad,w,NULL,1L);
	}
}
#endif

void
MyOnGadget (w,gad)	//
	register struct Window *w;
	register struct Gadget *gad;
{
	if (gad->Flags & GADGDISABLED)
	{
		GT_SetGadgetAttrs(gad,w,NULL,GA_Disabled,FALSE,TAG_DONE);
	}
}

void
MyOffGadget (w,gad)	//
	register struct Window *w;
	register struct Gadget *gad;
{
	if (!(gad->Flags & GADGDISABLED))
	{
// We don't use this function for CHECKBOX_KIND.
//		gad->Flags &= ~SELECTED;
		GT_SetGadgetAttrs(gad,w,NULL,GA_Disabled,TRUE,TAG_DONE);
	}
}

/* strip trailing blanks */

void
strip_trail (str)
	register char *str;
{
	register char *orig = str;

	str += strlen(str) - 1;

	while (*str == ' ' && str >= orig)
		*str-- = '\0';
}

