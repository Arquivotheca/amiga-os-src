/*	ASCII.H - ascii control codes
*
*	Mitchell/Ware		Version 1.03		7/28/87
*/

#define	NUL	0x00
#define	SOH	0x01
#define	STX	0x02
#define	ETX	0x03
#define	EOT	0x04
#define	ENQ	0x05
#define	ACK	0x06
#define	BEL	0x07
#define	BS	0x08
#define	HT	0x09
#define	LF	0x0A
#define	VT	0x0B
#define	FF	0x0C
#define	CR	0x0D
#define	SO	0x0E
#define	SI	0x0F
#define	DLE	0x10
#define	DC1	0x11
#define	DC2	0x12
#define	DC3	0x13
#define	DC4	0x14
#define	NAK	0x15
#define	SYN	0x16
#define	ETB	0x17
#define	CAN	0x18
#define	EM	0x19
#define	SUB	0x1A
#define	ESC	0x1B
#define	FS	0x1C
#define	GS	0x1D
#define	RS	0x1E
#define	US	0x1F
#define	SP	0x20
#define	DEL	0x7F

/*      Edit codes used by EdTools & FIELD routines (keymap mws1)
*/
#define NL      '\n'    /* newline      */

#define UPARROW 0x0B    /* end-character        */
#define DNARROW 0x0A    /* end-character        */

#define LTARROW 0x0F      
#define RTARROW 0x0C

#define HOME    RS      /* end-character        */
#define TAB     HT      /* end-character        */
#
/* Help & Function keys
*/
#define	HELP	0x80

#define	F1	0x81
#define	F2	0x82
#define	F3	0x83
#define	F4	0x84
#define	F5	0x85
#define	F6	0x86
#define	F7	0x87
#define	F8	0x88
#define	F9	0x89
#define	F10	0x8A

#define	SHELP	0x90

#define	SF1	0x91
#define	SF2	0x92
#define	SF3	0x93
#define	SF4	0x94
#define	SF5	0x95
#define	SF6	0x96
#define	SF7	0x97
#define	SF8	0x98
#define	SF9	0x99
#define	SF10	0x9A

/*      Control codes
*/
#define CTRLA   0x01
#define CTRLB   0x02
#define CTRLC   0x03
#define CTRLD   0x04
#define CTRLE   0x05
#define CTRLF   0x06
#define CTRLG   0x07
#define CTRLH   0x08
#define CTRLI   0x09
#define CTRLJ   0x0A
#define CTRLK   0x0B
#define CTRLL   0x0C
#define CTRLM   0x0D
#define CTRLN   0x0E
#define CTRLO   0x0F
#define CTRLP   0x10
#define CTRLQ   0x11
#define CTRLR   0x12
#define CTRLS   0x13
#define CTRLT   0x14
#define CTRLU   0x15
#define CTRLV   0x16
#define CTRLW   0x17
#define CTRLX   0x18
#define CTRLY   0x19
#define CTRLZ   0x1A
