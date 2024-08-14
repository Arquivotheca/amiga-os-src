/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	KeyConvert.h - header file for key conversion subroutines	 *
 *  Written 22 Mar 88 by Talin									 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* error codes returned */

#define	KEY_ERR_NOT_RAWKEY	-2
#define KEY_ERR_TOO_LONG	-1

/* special key codes */

enum special_keys {
	UP_ARROW=0x01, DOWN_ARROW, RIGHT_ARROW, LEFT_ARROW, HELP_KEY,
	FUNKEY1=0x80,   FUNKEY2,   FUNKEY3,   FUNKEY4,   FUNKEY5,
		 FUNKEY6,   FUNKEY7,   FUNKEY8,   FUNKEY9,   FUNKEY10,
		 FUNKEY11,  FUNKEY12,
	S_FUNKEY1=0x90, S_FUNKEY2, S_FUNKEY3, S_FUNKEY4, S_FUNKEY5,
		 S_FUNKEY6, S_FUNKEY7, S_FUNKEY8, S_FUNKEY9, S_FUNKEY10,
		 S_FUNKEY11, S_FUNKEY12,
	INSERT_KEY=0xa0, PAGEUP_KEY, PAGEDN_KEY, PAUSE_KEY, HOME_KEY,
	END_KEY
};

#define SHIFT				0x0003
#define CONTROL				0x0008
#define ALT					0x0030
#define L_AMIGA				0x0040
#define R_AMIGA				0x0080
#define NUMPAD				0x0100
#define KEY_REPEAT			0x0200

/* end of KeyConvert.h */
