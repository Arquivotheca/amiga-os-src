/* routines to remap the keyboard */

#include "st.h"
#include "term.h"
#include "meta.h"
#include "menu.h"
#include "devices/console.h"
#include "devices/keymap.h"

/*#define DEBUG /* */

#define LO_TYPE_SIZE 64
#define LO_SIZE 256
#define HI_TYPE_SIZE 64
#define HI_SIZE 256

/* LOW KEYMAP DEFINITIONS */
#define N0_KEY 15	/* rawkey $0F (15) */
#define N1_KEY 29	/* rawkey $1D (29) */
#define N2_KEY 30	/* rawkey $1E (30) */
#define N3_KEY 31	/* rawkey $1F (31) */
#define N4_KEY 45	/* rawkey $2D (45) */
#define N5_KEY 46	/* rawkey $2E (46) */
#define N6_KEY 47	/* rawkey $2F (47) */
#define N7_KEY 61	/* rawkey $3D (61) */
#define N8_KEY 62	/* rawkey $3E (62) */
#define N9_KEY 63	/* rawkey $3F (63) */
#define PERIODN_KEY 60	/* rawkey $3C (60) */

/* HIGH KEYMAP DEFINITIONS */
#define BACKSPACE_KEY 1	/* rawkey $41 (64 + 1) */
#define ENTER_KEY 3	/* rawkey $43 (64 + 3) */
#define RETURN_KEY 4	/* rawkey $44 (64 + 4) */
#define DELETE_KEY 6	/* rawkey $46 (64 + 6) */
#define MINUSN_KEY 10	/* rawkey $4A (64 + 10) */
#define UP_KEY 12	/* rawkey $4C (64 + 12) */
#define DOWN_KEY 13	/* rawkey $4D (64 + 13) */
#define RIGHT_KEY 14	/* rawkey $4E (64 + 14) */
#define LEFT_KEY 15	/* rawkey $4F (64 + 15) */
#define F1_KEY 16	/* rawkey $50 (64 + 16) */
#define F2_KEY 17	/* rawkey $51 (64 + 17) */
#define F3_KEY 18	/* rawkey $52 (64 + 18) */
#define F4_KEY 19	/* rawkey $53 (64 + 19) */
#define F5_KEY 20	/* rawkey $54 (64 + 20) */
#define F6_KEY 21	/* rawkey $55 (64 + 21) */
#define F7_KEY 22	/* rawkey $56 (64 + 22) */
#define F8_KEY 23	/* rawkey $57 (64 + 23) */
#define F9_KEY 24	/* rawkey $58 (64 + 24) */
#define F10_KEY 25	/* rawkey $59 (64 + 25) */
#define PF1N_KEY 26	/* rawkey $5A (64 + 26) */
#define PF2N_KEY 27	/* rawkey $5B (64 + 27) */
#define PF3N_KEY 28	/* rawkey $5C (64 + 28) */
#define	PF4N_KEY 29	/* rawkey $5D (64 + 29) */
#define PLUSN_KEY 30	/* rawkey $5E (64 + 30) */
#define HELP_KEY 31	/* rawkey $5F (64 + 31) */

extern struct IOStdReq consoleIO;

ULONG pad1; /* assure long-word aligning */
struct KeyMap MyKeyMap; /* my RAM keymap ptrs */

ULONG LoKeyMap[LO_SIZE];           /* my RAM LoKeyMap (must be word aligned) */
ULONG HiKeyMap[HI_SIZE];           /* my RAM HiKeyMap (must be word aligned) */
UBYTE LoKeyMapTypes[LO_TYPE_SIZE]; /* my RAM LoKeyMapTypes */
UBYTE HiKeyMapTypes[HI_TYPE_SIZE]; /* my RAM HiKeyMayTypes */
ULONG *OrgLoKeyMap, *OrgHiKeyMap;  /* the original ROM ptrs */
UBYTE *OrgLoKeyMapTypes, *OrgHiKeyMapTypes; /* the original ROM ptrs */

InitKeyMapping() /* master initialization */
{
int i;
	if ((i=ReadKeyMap())) { /* if read the original ROM keymap ptrs ok */
		/* set my key map ptrs to point to the tables that Im gonna change */
		OrgLoKeyMapTypes = MyKeyMap.km_LoKeyMapTypes; /* save ROM ptr */
		OrgLoKeyMap = MyKeyMap.km_LoKeyMap; /* save ROM ptr */
		OrgHiKeyMapTypes = MyKeyMap.km_HiKeyMapTypes; /* save ROM ptr */
		OrgHiKeyMap = MyKeyMap.km_HiKeyMap; /* save ROM ptr */
		MyKeyMap.km_LoKeyMapTypes = LoKeyMapTypes;
		MyKeyMap.km_LoKeyMap = LoKeyMap;
		MyKeyMap.km_HiKeyMapTypes = HiKeyMapTypes;
		MyKeyMap.km_HiKeyMap = HiKeyMap;
	}
	return(i);
}

InitMyTable() /* copy the org key tables to my tables */
{
UBYTE *bb;
ULONG *ll;
int i;

	for (i=0, bb=OrgLoKeyMapTypes; i<LO_TYPE_SIZE; i++) LoKeyMapTypes[i] = *bb++;
	for (i=0, ll=OrgLoKeyMap; i<(LO_SIZE / sizeof(ULONG)); i++) LoKeyMap[i] = *ll++;
	for (i=0, bb=OrgHiKeyMapTypes; i<HI_TYPE_SIZE; i++) HiKeyMapTypes[i] = *bb++;
	for (i=0, ll=OrgHiKeyMap; i<(HI_SIZE / sizeof(ULONG)); i++) HiKeyMap[i] = *ll++;
	/* I always want the HELP key to send a linefeed character */
	HiKeyMapTypes[HELP_KEY] = KCF_STRING; /* HELP sends a string */
	HiKeyMap[HELP_KEY] = (ULONG)"\001\002\012"; /* HELP is linefeed */
}

ReadKeyMap() /* read the original key map ptrs */
{
LONG i;
	consoleIO.io_Command = CD_ASKKEYMAP;
	consoleIO.io_Length = sizeof(MyKeyMap);
	consoleIO.io_Data = &MyKeyMap;
	DoIO(&consoleIO);
	if (i=consoleIO.io_Error) {
		return(FALSE);
	}
	else return(TRUE);
}

WriteKeyMap() /* write the new my key map ptrs */
{
LONG i;
	consoleIO.io_Command = CD_SETKEYMAP;
	consoleIO.io_Length = sizeof(MyKeyMap);
	consoleIO.io_Data = &MyKeyMap;
	DoIO(&consoleIO);
	if (i=consoleIO.io_Error) {
		return(FALSE);
	}
	else return(TRUE);
}

vt52Normal() /* set vt52 keyboard for normal keys (default) */
{
int i;
	InitMyTable(); /* get table to a known state */
/* set cursor keys */
	for (i=UP_KEY; i<=LEFT_KEY; i++) HiKeyMapTypes[i] = KCF_STRING;
	HiKeyMap[UP_KEY] = (ULONG)"\002\002\033A"; /* up */
	HiKeyMap[DOWN_KEY] = (ULONG)"\002\002\033B"; /* down */
	HiKeyMap[RIGHT_KEY] = (ULONG)"\002\002\033C"; /* right */
	HiKeyMap[LEFT_KEY] = (ULONG)"\002\002\033D"; /* left */
/* misc */
	set_linefeed_mode(); /* return sends CR ($0d) */
/* set numeric pad (vt52 mode) */
	AnsiKeypadNumeric(1); /* does a restore fkeys and a write */
}

vt52PFKeys()
{
	HiKeyMap[PF1N_KEY] = (ULONG)"\002\002\033P"; /* PF1 */
	HiKeyMap[PF2N_KEY] = (ULONG)"\002\002\033Q"; /* PF2 */
	HiKeyMap[PF3N_KEY] = (ULONG)"\002\002\033R"; /* PF3 */
	HiKeyMap[PF4N_KEY] = (ULONG)"\002\002\033S"; /* PF4 */
}

vt52Alt() /* set vt52 keyboard for alternate keys (non-default) */
{
int i;
/* set numeric pad */
	LoKeyMapTypes[N0_KEY] = KCF_STRING; /* 0 */
	for (i=N1_KEY; i<=N3_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /* 1 2 3 */
	for (i=N4_KEY; i<=N6_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /* 4 5 6 */
	for (i=PERIODN_KEY; i<=N9_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /* . 7 8 9 */
	HiKeyMapTypes[ENTER_KEY] = KCF_STRING; /* enter */
	HiKeyMapTypes[MINUSN_KEY] = KCF_STRING; /* - */
	HiKeyMapTypes[PLUSN_KEY] = KCF_STRING; /* +(amiga) or ,(vt100) */
	HiKeyMapTypes[PF1N_KEY] = KCF_STRING; /* PF1 */
	HiKeyMapTypes[PF2N_KEY] = KCF_STRING; /* PF2 */
	HiKeyMapTypes[PF3N_KEY] = KCF_STRING; /* PF3 */
	HiKeyMapTypes[PF4N_KEY] = KCF_STRING; /* PF4 */
	LoKeyMap[N0_KEY] = (ULONG)"\003\002\033?p"; /* 0 */
	LoKeyMap[N1_KEY] = (ULONG)"\003\002\033?q"; /* 1 */
	LoKeyMap[N2_KEY] = (ULONG)"\003\002\033?r"; /* 2 */
	LoKeyMap[N3_KEY] = (ULONG)"\003\002\033?s"; /* 3 */
	LoKeyMap[N4_KEY] = (ULONG)"\003\002\033?t"; /* 4 */
	LoKeyMap[N5_KEY] = (ULONG)"\003\002\033?u"; /* 5 */
	LoKeyMap[N6_KEY] = (ULONG)"\003\002\033?v"; /* 6 */
	LoKeyMap[PERIODN_KEY] = (ULONG)"\003\002\033?n"; /* . */
	LoKeyMap[N7_KEY] = (ULONG)"\003\002\033?w"; /* 7 */
	LoKeyMap[N8_KEY] = (ULONG)"\003\002\033?x"; /* 8 */
	LoKeyMap[N9_KEY] = (ULONG)"\003\002\033?y"; /* 9 */
	HiKeyMap[ENTER_KEY] = (ULONG)"\003\002\033?M"; /* enter */
	HiKeyMap[MINUSN_KEY] = (ULONG)"\003\002\033?m"; /* - */
	HiKeyMap[PLUSN_KEY] = (ULONG)"\003\002\033?l"; /* +(amiga) or ,(vt100) */
	vt52PFKeys();
	Restore_User_FKeys(NUMFUNC); /* restore all user fkeys */
	WriteKeyMap(); /* invoke the new keys */
}

vt100Appl() /* set vt100 keyboard for application keys (default) */
{
int i;
   InitMyTable(); /* get table to a known state */
/* set cursor keys */
	for (i=UP_KEY; i<=LEFT_KEY; i++) HiKeyMapTypes[i] = KCF_STRING;
	AnsiCursorNormal();
/* misc */
	set_linefeed_mode(); /* return sends CR ($0d) */
/* set numeric pad */
	AnsiKeypadAppl(); /* does a restore fkeys and a write */
}

AnsiCursorNormal() /* set ansi keyboard for cursor normal keys */
{
/* set cursor keys */
	HiKeyMap[UP_KEY] = (ULONG)"\003\002\033[A"; /* up */
	HiKeyMap[DOWN_KEY] = (ULONG)"\003\002\033[B"; /* down */
	HiKeyMap[RIGHT_KEY] = (ULONG)"\003\002\033[C"; /* right */
	HiKeyMap[LEFT_KEY] = (ULONG)"\003\002\033[D"; /* left */
}

AnsiCursorAppl() /* set ansi keyboard for cursor appl keys */
{
/* set cursor keys */
	HiKeyMap[UP_KEY] = (ULONG)"\003\002\033OA"; /* up */
	HiKeyMap[DOWN_KEY] = (ULONG)"\003\002\033OB"; /* down */
	HiKeyMap[RIGHT_KEY] = (ULONG)"\003\002\033OC"; /* right */
	HiKeyMap[LEFT_KEY] = (ULONG)"\003\002\033OD"; /* left */
}

vt100PFKeys()
{
	HiKeyMap[PF1N_KEY] = (ULONG)"\003\002\033OP"; /* PF1 */
	HiKeyMap[PF2N_KEY] = (ULONG)"\003\002\033OQ"; /* PF2 */
	HiKeyMap[PF3N_KEY] = (ULONG)"\003\002\033OR"; /* PF3 */
	HiKeyMap[PF4N_KEY] = (ULONG)"\003\002\033OS"; /* PF4 */
}

AnsiKeypadNumeric(vt52) /* set ansi keyboard for numeric keypad */
int vt52; /* flag : 0-vt100, 1-vt52 */
{
int i;
	LoKeyMapTypes[N0_KEY] = KC_VANILLA; /* 0 */
	for (i=N1_KEY; i<=N3_KEY; i++) LoKeyMapTypes[i] = KC_VANILLA; /* 1 2 3 */
	for (i=N4_KEY; i<=N6_KEY; i++) LoKeyMapTypes[i] = KC_VANILLA; /* 4 5 6 */
	for (i=PERIODN_KEY; i<=N9_KEY; i++) LoKeyMapTypes[i] = KC_VANILLA; /* . 7 8 9 */
	HiKeyMapTypes[ENTER_KEY] = KC_NOQUAL; /* enter */
	HiKeyMapTypes[MINUSN_KEY] = KCF_ALT; /* - */
	HiKeyMapTypes[PLUSN_KEY] = KCF_ALT; /* +(amiga) or ,(vt100) */
	HiKeyMapTypes[PF1N_KEY] = KCF_STRING; /* PF1 */
	HiKeyMapTypes[PF2N_KEY] = KCF_STRING; /* PF2 */
	HiKeyMapTypes[PF3N_KEY] = KCF_STRING; /* PF3 */
	HiKeyMapTypes[PF4N_KEY] = KCF_STRING; /* PF4 */
	LoKeyMap[N0_KEY] = (ULONG)0xb0b03030; /* 0 */
	LoKeyMap[N1_KEY] = (ULONG)0xb1b13131; /* 1 */
	LoKeyMap[N2_KEY] = (ULONG)0xb2b23232; /* 2 */
	LoKeyMap[N3_KEY] = (ULONG)0xb3b33333; /* 3 */
	LoKeyMap[N4_KEY] = (ULONG)0xb4b43434; /* 4 */
	LoKeyMap[N5_KEY] = (ULONG)0xb5b53535; /* 5 */
	LoKeyMap[N6_KEY] = (ULONG)0xb6b63636; /* 6 */
	LoKeyMap[N7_KEY] = (ULONG)0xb7b73737; /* 7 */
	LoKeyMap[N8_KEY] = (ULONG)0xb8b83838; /* 8 */
	LoKeyMap[N9_KEY] = (ULONG)0xb9b93939; /* 9 */
	LoKeyMap[PERIODN_KEY] = (ULONG)0xaeae2e2e; /* . */
	HiKeyMap[ENTER_KEY] = (ULONG)0x0000000d; /* enter */
	HiKeyMap[MINUSN_KEY] = (ULONG)0x0000ff2d; /* - */
	HiKeyMap[PLUSN_KEY] = (ULONG)0x0000ff2c; /* +(amiga) or ,(vt100) */
	if (vt52) {
		vt52PFKeys();
	}
	else { /* vt100 */
		vt100PFKeys();
	}
	Restore_User_FKeys(NUMFUNC); /* restore all user fkeys */
	WriteKeyMap(); /* invoke the new keys */
}

AnsiKeypadAppl() /* set ansi keyboard for appl keypad */
{
int i;
	LoKeyMapTypes[N0_KEY] = KCF_STRING; /* 0 */
	for (i=N1_KEY; i<=N3_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /* 1 2 3 */
	for (i=N4_KEY; i<=N6_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /* 4 5 6 */
	for (i=PERIODN_KEY; i<=N9_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /* . 7 8 9 */
	HiKeyMapTypes[ENTER_KEY] = KCF_STRING; /* enter */
	HiKeyMapTypes[MINUSN_KEY] = KCF_STRING; /* - */
	HiKeyMapTypes[PLUSN_KEY] = KCF_STRING; /* +(amiga) or ,(vt100) */
	HiKeyMapTypes[PF1N_KEY] = KCF_STRING; /* PF1 */
	HiKeyMapTypes[PF2N_KEY] = KCF_STRING; /* PF2 */
	HiKeyMapTypes[PF3N_KEY] = KCF_STRING; /* PF3 */
	HiKeyMapTypes[PF4N_KEY] = KCF_STRING; /* PF4 */
	LoKeyMap[N0_KEY] = (ULONG)"\003\002\033Op"; /* 0 */
	LoKeyMap[N1_KEY] = (ULONG)"\003\002\033Oq"; /* 1 */
	LoKeyMap[N2_KEY] = (ULONG)"\003\002\033Or"; /* 2 */
	LoKeyMap[N3_KEY] = (ULONG)"\003\002\033Os"; /* 3 */
	LoKeyMap[N4_KEY] = (ULONG)"\003\002\033Ot"; /* 4 */
	LoKeyMap[N5_KEY] = (ULONG)"\003\002\033Ou"; /* 5 */
	LoKeyMap[N6_KEY] = (ULONG)"\003\002\033Ov"; /* 6 */
	LoKeyMap[PERIODN_KEY] = (ULONG)"\003\002\033On"; /* . */
	LoKeyMap[N7_KEY] = (ULONG)"\003\002\033Ow"; /* 7 */
	LoKeyMap[N8_KEY] = (ULONG)"\003\002\033Ox"; /* 8 */
	LoKeyMap[N9_KEY] = (ULONG)"\003\002\033Oy"; /* 9 */
	HiKeyMap[ENTER_KEY] = (ULONG)"\003\002\033OM"; /* enter */
	HiKeyMap[MINUSN_KEY] = (ULONG)"\003\002\033Om"; /* - */
	HiKeyMap[PLUSN_KEY] = (ULONG)"\003\002\033Ol"; /* +(amiga) or ,(vt100) */
	vt100PFKeys();
	Restore_User_FKeys(NUMFUNC); /* restore all user fkeys */
	WriteKeyMap(); /* invoke the new keys */
}

AnsiKeypadHex() /* set ansi keyboard for hex keypad */
{
int i;
	LoKeyMapTypes[N0_KEY] = KCF_STRING; /* 0 */
	for (i=N1_KEY; i<=N3_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /* 1 2 3 */
	for (i=N4_KEY; i<=N6_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /* 4 5 6 */
	for (i=PERIODN_KEY; i<=N9_KEY; i++) LoKeyMapTypes[i] = KCF_STRING; /* . 7 8 9 */
	HiKeyMapTypes[ENTER_KEY] = KCF_STRING; /* enter */
	HiKeyMapTypes[MINUSN_KEY] = KCF_STRING; /* - */
	HiKeyMapTypes[PLUSN_KEY] = KCF_STRING; /* +(amiga) or ,(vt100) */
	HiKeyMapTypes[PF1N_KEY] = KCF_STRING; /* PF1 */
	HiKeyMapTypes[PF2N_KEY] = KCF_STRING; /* PF2 */
	HiKeyMapTypes[PF3N_KEY] = KCF_STRING; /* PF3 */
	HiKeyMapTypes[PF4N_KEY] = KCF_STRING; /* PF4 */
	LoKeyMap[N0_KEY] = (ULONG)"\001\0020"; /* 0 */
	LoKeyMap[N1_KEY] = (ULONG)"\001\0021"; /* 1 */
	LoKeyMap[N2_KEY] = (ULONG)"\001\0022"; /* 2 */
	LoKeyMap[N3_KEY] = (ULONG)"\001\0023"; /* 3 */
	LoKeyMap[N4_KEY] = (ULONG)"\001\0024"; /* 4 */
	LoKeyMap[N5_KEY] = (ULONG)"\001\0025"; /* 5 */
	LoKeyMap[N6_KEY] = (ULONG)"\001\0026"; /* 6 */
	LoKeyMap[PERIODN_KEY] = (ULONG)"\001\002."; /* . */
	LoKeyMap[N7_KEY] = (ULONG)"\001\0027"; /* 7 */
	LoKeyMap[N8_KEY] = (ULONG)"\001\0028"; /* 8 */
	LoKeyMap[N9_KEY] = (ULONG)"\001\0029"; /* 9 */
	HiKeyMap[ENTER_KEY] = (ULONG)"\001\002\015"; /* enter */
	HiKeyMap[MINUSN_KEY] = (ULONG)"\001\002E"; /* - */
	HiKeyMap[PLUSN_KEY] = (ULONG)"\001\002F"; /* +(amiga) or ,(vt100) */
	HiKeyMap[PF1N_KEY] = (ULONG)"\001\002A"; /* PF1 */
	HiKeyMap[PF2N_KEY] = (ULONG)"\001\002B"; /* PF2 */
	HiKeyMap[PF3N_KEY] = (ULONG)"\001\002C"; /* PF3 */
	HiKeyMap[PF4N_KEY] = (ULONG)"\001\002D"; /* PF4 */
	Restore_User_FKeys(NUMFUNC); /* restore all user fkeys */
	WriteKeyMap(); /* invoke the new keys */
}

ttyNormal() /* set tty keyboard */
{
int i;
	InitMyTable(); /* get table to a known state */
/* set cursor keys */
	for (i=UP_KEY; i<=LEFT_KEY; i++) HiKeyMapTypes[i] = KCF_STRING;
	HiKeyMapTypes[UP_KEY] = KCF_NOP; /* kill cursor up */
	HiKeyMap[DOWN_KEY] = (ULONG)"\001\002\012"; /* down (linefeed) */
	HiKeyMap[RIGHT_KEY] = (ULONG)"\001\002\040"; /* right (space) */
	HiKeyMap[LEFT_KEY] = (ULONG)"\001\002\010"; /* left (backspace) */
/* set function keys */
	Restore_User_FKeys(NUMFUNC); /* restore all user fkeys */
	WriteKeyMap(); /* invoke the new keys */
}

AmigaNormal() /* set amiga keyboard */
{
	InitMyTable(); /* get table to a known state */
/* set function keys */
	Restore_User_FKeys(NUMFUNC); /* restore all user fkeys */
	WriteKeyMap(); /* invoke the new keys */
}


set_linefeed_mode() /* return sends CR ($0d) */
{
	HiKeyMapTypes[RETURN_KEY] = KCF_STRING;
	HiKeyMap[RETURN_KEY] = (ULONG)"\001\002\015"; /* the string is CR */
}

set_newline_mode() /* return sends CR/LF ($0d$0a) */
{
	HiKeyMapTypes[RETURN_KEY] = KCF_STRING;
	HiKeyMap[RETURN_KEY] = (ULONG)"\002\002\015\012"; /* the string is CR/LF */
}

char fkeys[FMAX][FLEN+2];

Set_Function_Key(s,n) /* set function key 'n' to string 's' */
char *s;
UBYTE n;
{
UBYTE len;
	if ((n < FMIN) || (n > FMAX) || ((len = strlen(s)) > FLEN)) {
#ifdef DEBUG
printf("setting of F%ld failed, str='%s'\n",n,s);
#endif
		return(FALSE);
	}
	n--; /* set to base 0 not base 1 */
	HiKeyMapTypes[n + F1_KEY] = KCF_STRING; /* its a string */
	fkeys[n][0] = len; /* set length */
	fkeys[n][1] = 2; /* the string is 2 posns away from the length defn */
	strcpy(&fkeys[n][2],s); /* copy the string */
	HiKeyMap[n+F1_KEY] = (ULONG)&fkeys[n][0]; /* assign to function key */
	return(TRUE);
}

Restore_User_FKeys(number)
UBYTE number;
{
UBYTE i;
	for (i=0; i<number; i++)
		Set_Function_Key(&fkeys[i][2],i+1); /* restore user defns */
}

SetDelAndBksp(mode)
int mode;
{
	if (mode == SUBITEMDELNORMAL) {
		HiKeyMap[BACKSPACE_KEY] = 0x08080808;
		HiKeyMap[DELETE_KEY] = 0x7f7f7f7f;
	}
	else {
		HiKeyMap[BACKSPACE_KEY] = 0x7f7f7f7f;
		HiKeyMap[DELETE_KEY] = 0x08080808;
	}
}

RogueKeypad()
{
	int i;

	for (i=N1_KEY; i<=N3_KEY; i++) {
		LoKeyMapTypes[i] = KC_VANILLA; /* 1 2 3 */
	}
	for (i=N4_KEY; i<=N6_KEY; i++) {
		LoKeyMapTypes[i] = KC_VANILLA; /* 4 5 6 */
	}
	for (i=N7_KEY; i<=N9_KEY; i++) {
		LoKeyMapTypes[i] = KC_VANILLA; /* 7 8 9 */
	}
	LoKeyMap[N1_KEY] = (ULONG)0x62624262; /* 1 is b */
	LoKeyMap[N2_KEY] = (ULONG)0x6a6a4a6a; /* 2 is j */
	LoKeyMap[N3_KEY] = (ULONG)0x6e6e4e6e; /* 3 is n */
	LoKeyMap[N4_KEY] = (ULONG)0x68684868; /* 4 is h */
	LoKeyMap[N5_KEY] = (ULONG)0x2e2e2e2e; /* 5 is . */
	LoKeyMap[N6_KEY] = (ULONG)0x6c6c4c6c; /* 6 is l */
	LoKeyMap[N7_KEY] = (ULONG)0x79795979; /* 7 is y */
	LoKeyMap[N8_KEY] = (ULONG)0x6b6b4b6b; /* 8 is k */
	LoKeyMap[N9_KEY] = (ULONG)0x75755575; /* 9 is u */
	WriteKeyMap(); /* invoke the new keys */
}
