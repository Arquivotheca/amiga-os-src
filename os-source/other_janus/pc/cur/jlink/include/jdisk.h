/*
 * Communication Structures used
 * by IOCTL
 *
 * This block is returned after an IOCTL Read command:
 */

struct Ident {			/* IOCTL Identify */
	UWORD	I_Marker;	/* Marker, must be "JD" */
	UWORD	I_FTbl;		/* Current Handle for each Unit */
	UWORD	I_CS;		/* our Code Segment */
	UBYTE	I_Length;	/* Length of this reply, = SIZE Ident */
	UBYTE	I_Version;	/* driver version */
	UBYTE	I_BaseDrv;	/* base drive # */
	UBYTE	I_Units;	/* units we now */
	UBYTE	I_Init;		/* Driver has been used if 1 */
	UBYTE	I_pad;
};

/*
 * Identy, returned in I_Marker
 */
#define	JD	(256*'D'+'K')	/* Identy marker */

/*
 * This Structure is used to link or unlink a file:
 */
struct ILink {
	ULONG	IL_Handle;	/* handle */
	UWORD	IL_Sectors;	/* # of sectors in file */
	UBYTE	IL_Command;	/* Command on call */
	UBYTE	IL_Status;	/* Return Status */
	UBYTE	IL_Flag;	/* flags */
	UBYTE	IL_pad;
	UBYTE	IL_Name[128];	/* filename */
};

/* 
 * Commands given in IL_Command
 */
#define	IL_Link		0	/* Link Command */
#define	IL_UnLink	1	/* Unlink */

/* 
 * IL_Flags:
 */
#define	ILF_RO		(1 << 0)	/*  Read Only */

/*
 * IL Error, returned in IL_Status
 */
#define	ILE_OK		0	/* nothing wrong */
#define	ILE_COMERR	1	/* unknown command */
#define	ILE_LINKED	2	/* drive is linked */
#define	ILE_CLOSE	3	/* error while closing vdrive */
#define	ILE_NOT_LINKED	4	/* Nothing linked */
#define	ILE_NO_SERVICE	5	/* couldn't get dosserv */
