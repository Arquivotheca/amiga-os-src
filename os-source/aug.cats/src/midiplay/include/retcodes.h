/*************************************************************
*
*    Return Codes used through out the system.
*
*************************************************************/
#define RC_FAILED     -2    /* An operation failed */
#define RC_DONE       -1    /* Done with an operation */
#ifndef RC_OK	/* already defined in rexx/errors.h */
#define RC_OK          0	/* No error */
#endif
#define RC_CANT_FIND   1	/* File not found */
#define RC_BAD_ILBM    2	/* Bad ILBM file */
#define RC_READ_ERROR  3	/* Read error */
#define RC_CANT_FIND_INS 4  /* Can't find instrument */
#define RC_BAD_COMP    5	/* Bad compression type */
#define RC_NO_MEM      6	/* No memory */
#define RC_BAD_ANIM    7	/* Bad ANIM file */
#define RC_PAUSED_ANIM 8    /*  */
#define RC_OFF_SCREEN  9    /* Off the screen (no user-message for this one!) */
#define RC_EXPR_SYNTAX 10   /* Syntax error */
#define RC_NO_TRANS    11   /* No translator library */
#define RC_NAR_DEV     12   /* Unable to open the narratore device */
#define RC_SMUS_ACTIVE 13   /* SMUS is currently active (can not speak) */
#define RC_BAD_8SVX    14   /* Bad 8SVX file */
#define RC_TOO_MANY    15   /* Too many sounds or whatever... */
#define RC_NO_AUDIO    16   /* No audio device */
#define RC_BAD_SMUS    17   /* Bad SMUS */
#define RC_TOO_MANY_INSTR 18
#define RC_NO_CM        19    /* No Cache Manager */
#define RC_NO_SCREEN	20    /* No screen, not a good situation */
#define RC_DOB_INLIST	21    /* DOB list already in the active list */
#define RC_VOICE_ACTIVE	22  /* Voice active */
#define RC_SERIAL_BUSY	23   /* Serial Device is BUSY */
#define RC_AUDIO_BUSY	24    /* Audio Device BUSY */
#define RC_CIA_BUSY		25    /* CIA Device BUSY */
#define	RC_NO_INSTR		26
#define	RC_TOO_DEEP		27
#define	RC_MISSING_FILE	28
#define	RC_NESTED_FORMS	29		/* they can't nest! */
#define	RC_NO_CDAUDIO	30
#define	RC_NO_CDTV		RC_NO_CDAUDIO
#define	RC_CDAUDIO_BADTRACK 31
#define	RC_CDAUDIO_BADRANGE	32
#define	RC_SCREEN_LOCKED	33
#define	RC_NOT_ANIMBRUSH	34
#define	RC_NO_AVMIDI		35
#define	RC_NO_REALTIME		36

#define	RC_MAX_ERRORS	RC_NO_REALTIME

#define RC_ABORTED    100	/* Abort */


