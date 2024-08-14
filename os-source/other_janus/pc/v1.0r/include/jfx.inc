#ifndef	AmigaDskReq
/*
 * Disk request structure for higher level amiga file request
 * from 8086, uses to live in PARAMETER RAM.
 */

struct AmigaDskReq {
short adr_Fnctn,	/* function code (see below) */
	adr_File,	/* file number */
	adr_Offset_h,	/* byte offset into file high */
	adr_Offset_l,	/* byte offset into file low */
	adr_Count_h,	/* number of bytes to transfer high */
	adr_Count_l,	/* number of bytes to transfer low */
	adr_BufferAddr,	/* offset into MEMF_BUFFER memory for buffer */
	adr_Err;	/* return code, 0 if all OK */
	};

/*
 *  function codes for AmigaDskReq adr_Fnctn word
 */

#define	ADR_FNCTN_INIT		0	/* currently not used */
#define	ADR_FNCTN_READU		1	/* given file, offset, count, buffer */
#define	ADR_FNCTN_WRITE		2	/* given file, offset, count, buffer */
#define	ADR_FNCTN_SEEK 		3	/* given file, offset */
#define	ADR_FNCTN_INFO 		4	/* currently not used */
#define	ADR_FNCTN_OPEN_OLD 	5	/* given ASCIIZ pathname in buffer */
#define	ADR_FNCTN_OPEN_NEW 	6	/* given ASCIIZ pathname in buffer */
#define	ADR_FNCTN_CLOSE		7	/* given file */
#define	ADR_FNCTN_DELETE	8	/* given ASCIIZ pathname in buffer */
/*
 * error codes for adr_Err, returned in low byte
 */
#define	ADR_ERR_OK		0	/* no error */
#define	ADR_ERR_OFFSET		1	/* not used */
#define	ADR_ERR_COUNT		2	/* not used */
#define	ADR_ERR_FILE		3	/* file does not exist */
#define	ADR_ERR_FNCT		4	/* illegal function code */
#define	ADR_ERR_EOF		5	/* offset past end of file */
#define	ADR_ERR_MULPL		6	/* not used */
#define	ADR_ERR_FILE_COUNT	7	/* too many open files  */
#define	ADR_ERR_SEEK		8	/* seek error */
#define	ADR_ERR_READ		9	/* read went wrong */
#define	ADR_ERR_WRITE	       10	/* write error */
#define	ADR_ERR_LOCKED	       11	/* file is locked */

#endif	/* AmigaDskReq */

