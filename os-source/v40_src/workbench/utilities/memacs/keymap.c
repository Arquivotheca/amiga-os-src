/* routines to remap the keyboard */

#include "ed.h"
#if	AMIGA

#define LO_TYPE_SIZE 64
#define LO_SIZE 256
#define HI_TYPE_SIZE 64
#define HI_SIZE 256

#define UP_KEY 12
#define DOWN_KEY 13
#define RIGHT_KEY 14
#define LEFT_KEY 15

#define F1_KEY 16
#define F2_KEY 17
#define F3_KEY 18
#define F4_KEY 19
#define F5_KEY 20
#define F6_KEY 21
#define F7_KEY 22
#define F8_KEY 23
#define F9_KEY 24
#define F10_KEY 25

#define N0_KEY 15
#define N1_KEY 29
#define N2_KEY 30
#define N3_KEY 31
#define N4_KEY 45
#define N5_KEY 46
#define N6_KEY 47
#define N7_KEY 61
#define N8_KEY 62
#define N9_KEY 63

#define ENTER_KEY 3
#define RETURN_KEY 4

#define MINUSN_KEY 10
#define HELP_KEY 31
#define PERIODN_KEY 60

extern struct IOStdReq consoleIO;

ULONG pad1; /* assure long-word aligning */
struct KeyMap MyKeyMap; /* my RAM keymap ptrs */

ULONG LoKeyMap[LO_SIZE];           /* RAM LoKeyMap (must be word aligned) */
ULONG HiKeyMap[HI_SIZE];           /* RAM HiKeyMap (must be word aligned) */
UBYTE LoKeyMapTypes[LO_TYPE_SIZE]; /* RAM LoKeyMapTypes */
UBYTE HiKeyMapTypes[HI_TYPE_SIZE]; /* RAM HiKeyMayTypes */
ULONG *OrgLoKeyMap, *OrgHiKeyMap;  /* the original ROM ptrs */
UBYTE *OrgLoKeyMapTypes, *OrgHiKeyMapTypes; /* the original ROM ptrs */

int InitKeyMap() /* Initialize the keymap structure */
{
int i;
if ((i=ReadKeyMap())) { 
/* set the key map pointers to point to the tables to be changed */
    OrgLoKeyMapTypes = MyKeyMap.km_LoKeyMapTypes; /* save ROM pointers */
    OrgLoKeyMap = MyKeyMap.km_LoKeyMap;
    OrgHiKeyMapTypes = MyKeyMap.km_HiKeyMapTypes;
    OrgHiKeyMap = MyKeyMap.km_HiKeyMap;

    MyKeyMap.km_LoKeyMapTypes = LoKeyMapTypes;
    MyKeyMap.km_LoKeyMap = LoKeyMap;
    MyKeyMap.km_HiKeyMapTypes = HiKeyMapTypes;
    MyKeyMap.km_HiKeyMap = HiKeyMap;
}
return(i);
}

void InitMyTable() /* copy the original key tables to my tables */
{
UBYTE *bb;
ULONG *ll;
int i;

for (i=0, bb=OrgLoKeyMapTypes; i<LO_TYPE_SIZE; i++) LoKeyMapTypes[i] = *bb++;
for (i=0,ll=OrgLoKeyMap;i<(LO_SIZE/sizeof(ULONG)); i++) LoKeyMap[i] = *ll++;
for (i=0, bb=OrgHiKeyMapTypes; i<HI_TYPE_SIZE; i++) HiKeyMapTypes[i] = *bb++;
for (i=0,ll=OrgHiKeyMap;i<(HI_SIZE / sizeof(ULONG)); i++) HiKeyMap[i] = *ll++;
}

int ReadKeyMap() /* read the original key map pointers */
{
LONG i;
	consoleIO.io_Command = CD_ASKKEYMAP;
	consoleIO.io_Length = sizeof(MyKeyMap);
	consoleIO.io_Data = (APTR)&MyKeyMap;
	DoIO(&consoleIO);
	if (i=consoleIO.io_Error) return(FALSE);
	return(TRUE);
}

int WriteKeyMap(keymap) /* write the new my key map ptrs */
struct KeyMap *keymap;
{
LONG i;
	consoleIO.io_Command = CD_SETKEYMAP;
	consoleIO.io_Length = sizeof(*keymap);
	consoleIO.io_Data = (APTR)keymap;
	DoIO(&consoleIO);
	if (i=consoleIO.io_Error) return(FALSE);
	return(TRUE);
}

void SetKeyPad()
{
int i,j;

InitMyTable(); /* get table to a known state */

/* set the types to strings */
LoKeyMapTypes[N0_KEY] = KCF_STRING; /* 0 */
for (i=N1_KEY; i<=N3_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /*1 2 3 */
for (j=N4_KEY; j<=N6_KEY; j++) LoKeyMapTypes[j] = KCF_STRING; /* 4 5 6 */
for (i=PERIODN_KEY; i<=N9_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /*. 7 8 9*/
HiKeyMapTypes[ENTER_KEY] = KCF_STRING; /* enter */
HiKeyMapTypes[MINUSN_KEY] = KCF_STRING; /* - */

/* now provide the values */
LoKeyMap[N0_KEY] = (ULONG)"\003\002\233#~"; /* 0 */
LoKeyMap[N1_KEY] = (ULONG)"\003\002\233$~"; /* 1 */
LoKeyMap[N2_KEY] = (ULONG)"\003\002\233%~"; /* 2 */
LoKeyMap[N3_KEY] = (ULONG)"\003\002\233&~"; /* 3 */
LoKeyMap[N4_KEY] = (ULONG)"\003\002\233'~"; /* 4 */
LoKeyMap[N5_KEY] = (ULONG)"\003\002\233(~"; /* 5 */
LoKeyMap[N6_KEY] = (ULONG)"\003\002\233)~"; /* 6 */
LoKeyMap[PERIODN_KEY] = (ULONG)"\003\002\233-~"; /* . */
LoKeyMap[N7_KEY] = (ULONG)"\003\002\233*~"; /* 7 */
LoKeyMap[N8_KEY] = (ULONG)"\003\002\233+~"; /* 8 */
LoKeyMap[N9_KEY] = (ULONG)"\003\002\233,~"; /* 9 */
HiKeyMap[ENTER_KEY] = (ULONG)"\003\002\233.~"; /* enter */
HiKeyMap[MINUSN_KEY] = (ULONG)"\003\002\233/~"; /* - */

/* Tell the console device about it */
WriteKeyMap(&MyKeyMap);
}

void ResetKeyPad() /* set amiga keyboard */
{
InitMyTable();
WriteKeyMap(&MyKeyMap);
}

#endif
