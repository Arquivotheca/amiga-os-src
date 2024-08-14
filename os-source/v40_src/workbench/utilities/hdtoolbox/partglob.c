/* HandlePart's globals - PartGlob.c */

#include "prep_strux.h"
#include <intuition/intuition.h>

struct PartitionBlock *CurrentPart,*PrevPart,*NextPart,*save_part;

struct PartitionBlock savecurrpart;	/* for handlefs */

LONG PrevPMax,NextPMin,Cylinders;

WORD Init = 0;

ULONG DefaultEnvironment[17] = {
	11,
	128,
	0,
	0, /* changed */
	1,
	0, /* changed */
	2,
	0,
	0,
	2, /* changed */
	2, /* changed */
	30,
	0,
	/* more????? */
};

LONG PartUpdate,FSUpdate;

extern UWORD PartPatternImage[];

UWORD *PartPattern = &PartPatternImage[0];

extern struct TextAttr TOPAZ80;

struct IntuiText PartIText[] = {
	{1,0,JAM1,239+13*8,24,&TOPAZ80,"0",&PartIText[1]},	/* address */
	{1,0,JAM1,239+20*8,24,&TOPAZ80,"0",&PartIText[2]},	/* Unit number */
	{1,0,JAM1,600,35,&TOPAZ80,"9999",&PartIText[3]},	/* max cylinder */
	{1,0,JAM1,424,79,&TOPAZ80,"Size:",&PartIText[4]},	/* for slider */
	{1,0,JAM1,412,88,&TOPAZ80,"9999 Meg",&PartIText[5]},	/* for slider */
	{1,0,JAM1,231,24,&TOPAZ80," SCSI ",NULL},		/* scsi/st506 */
};

//BYTE use_advanced = FALSE;	// There is AdvancedFlags instead of this.
